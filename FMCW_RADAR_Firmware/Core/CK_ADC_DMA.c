
#include "stm32f4xx.h"

#include "USBD_CDC/CK_USBD_INTERFACE.h"
#include "CK_TIME_HAL.h"
#include "CK_GPIO.h"
#include "CK_ADC_DMA.h"

// Debug timing has CK_ADC_DMA_Start(), use it without main loop.
#define DEBUG_TIMING			0

#define TEST_MODE				0

#define ADC_BUFFER_SIZE			512

uint16_t adc_dma_buffer1[ADC_BUFFER_SIZE];

uint32_t adc_t1 = 0, adc_t2 = 0, adc_t3 = 0;

int samplingIsCompleted = 0;

void CK_ADC_DMA_Init(void){

	RCC->AHB1ENR |= (1u << 22); // DMA2 clock enable
	RCC->APB2ENR |= (1u << 8);  // 8 is ADC1, 9 ADC2 and 10 ADC3

	// PC0 ADC123_IN10 (Channel 10 at either ADC1,2 or 3)
	GPIOC->MODER |= (3u << 0); // pin 0 is in analog mode.

	// DMA2 Stream 0, Channel 0 is ADC1
	// Clear flags before enabling DMA
	DMA2->LIFCR = 0x3D << 0;

	// Set peripheral data register address
	DMA2_Stream0->PAR = (uint32_t)(&ADC1->DR);

	DMA2_Stream0->M0AR = (uint32_t)(&adc_dma_buffer1);

	// Every ADC_BUFFER_SIZE transfer will create interrupt i suppose.
	// Enabling DMA again will load the same value
	DMA2_Stream0->NDTR = ADC_BUFFER_SIZE;

	// Half and Full Transfer Complete
	DMA2_Stream0->CR |= (1u << 3) | (1u << 4);

	// DMA is flow controller
	DMA2_Stream0->CR |= (0u << 5);

	// Data transfer direction is Peripheral to Memory, Memory address increment after each data tx
	DMA2_Stream0->CR |= (0u << 6) | (1 << 10);

	// Circular Mode, Double Buffer Mode
	// DMA2_Stream0->CR |= (1u << 8) | (1u << 18);

	// In normal mode NDTR = 0 when Full TX is done.
	// Use CK_ADC_DMA_Start() for starting conversion and dma transfer again after Full TX.
	DMA2_Stream0->CR |= (0u << 8) | (0u << 18);

	// Peripheral size 16 bit, Memory size 16 bit, Priority very high, channel 0 is selected
	DMA2_Stream0->CR |= (1u << 11) | (1u << 13) | (3u << 16) | (0u << 25);

	// Enable interrupt
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);

	// APB2 clock is 84MHz, ADCCLK = APB2 / prescalar
	// 0->2, 1->4, 2->6, 3->8
	// ADCCLK = 82/6 = 14 MHz.
	ADC->CCR |= (2u << 16);

	// 0->3cycle, 1->15, 2->28, 3->56, 4->84, 5->112, 6->144, 7->480
	// Tsampling = (12 (fix adc 12 bit) + 15 cycle) * (1/14MHz) = 1.92 microsecond
	// Fsampling = 520KSample / sec for channel 10
	ADC1->SMPR1 |= (1u << 0);

	// I assing the channel 10 to the first conversion
	ADC1->SQR3 |= (10u << 0);

	//ADC1->CR1 |= (1u << 5); // End of conversion interrupt enable.

	ADC1->CR2 |= (1u << 1); // continuous conversion,

	ADC1->CR2 |= (1u << 0) | (1u << 8) | (1u << 9); // Enable ADC, DMA, DDS

	#if DEBUG_TIMING

		CK_ADC_DMA_Start();

		adc_t1 = CK_TIME_GetMicroSec();

	#endif
}

void CK_ADC_DMA_Start(void){

	samplingIsCompleted = 0;

	CK_ADC_DMA_DMAStop();
	CK_ADC_DMA_ADCStop();

	// DMA2 Stream0 Clear Interrupts
	DMA2->LIFCR = 0x3D << 0;

	// ADC1 Clear Interrupts
	ADC1->SR = 0;

	DMA2_Stream0->NDTR = ADC_BUFFER_SIZE;

	CK_ADC_DMA_DMAStart();
	CK_ADC_DMA_ADCStart();

}

void CK_ADC_DMA_DMAStop(void){

	DMA2_Stream0->CR &= ~(1u<<0); // DMA Stream stop

}

void CK_ADC_DMA_DMAStart(void){

	DMA2_Stream0->CR |= (1u<<0); // DMA Stream start

}

void CK_ADC_DMA_ADCStop(void){

	ADC1->CR2 &= ~(1u << 30); // Conversion stop, can only be set when adc is enabled
}

void CK_ADC_DMA_ADCStart(void){

	ADC1->CR2 |= (1u << 30); // Conversion start, can only be set when adc is enabled
}

int CK_ADC_DMA_IsTxComplete(void){

	return samplingIsCompleted;

}

void CK_ADC_DMA_TransferSamples(void){

	#if DEBUG_TIMING
		uint32_t adc_tx_t1 = CK_TIME_GetMicroSec();
	#endif

	uint16_t current_data = 0;
	uint8_t adc_lsb = 0;
	uint8_t adc_msb = 0;

	for(int i = 0; i < ADC_BUFFER_SIZE; i++){

		current_data = adc_dma_buffer1[i];

		#if TEST_MODE
			static uint32_t counter = 0;
			current_data = (uint16_t)counter++;
			if(counter == 65536) counter = 0;
		#endif

		adc_msb = (uint8_t)((current_data >> 8) & 0xFF);
		adc_lsb = (uint8_t)(current_data & 0xFF);

		CK_USBD_WriteTxCircularBuffer(adc_msb);
		CK_USBD_WriteTxCircularBuffer(adc_lsb);

	}

	#if DEBUG_TIMING
		uint32_t  adc_tx_t2 = CK_TIME_GetMicroSec() - adc_tx_t1;
		adc_tx_t2++;

		uint32_t adc_tx_t3 = CK_TIME_GetMicroSec();

		CK_USBD_Transmit();

		uint32_t adc_tx_t4 = CK_TIME_GetMicroSec() - adc_tx_t3;
		UNUSED(adc_tx_t4);
	#else
		CK_USBD_Transmit();
	#endif

}

void DMA2_Stream0_IRQHandler(void){

	static int n1 = 0;UNUSED(n1);
	static int n2 = 0;UNUSED(n2);

	// For Stream0 Bit4 is Half Transfer Complete and Bit5 is Transfer Complete
	static uint32_t status = 0;
	status = DMA2->LISR;

	// DMA2 Stream0 Clear Interrupts
	DMA2->LIFCR = 0x3D << 0;

	// Half Transfer Complete
	if( ( ( status & (1u << 4) ) >> 4 ) == 1){

		// I used these for debugging and it is working.
		// n1 is half size of DMA2_Stream0->NDTR
		n1 = DMA2_Stream0->NDTR;

		#if DEBUG_TIMING
			// For ADC_BUFFER_SIZE = 512 Half TX takes 492 microsecond
			adc_t2 = CK_TIME_GetMicroSec() - adc_t1;
		#endif
	}

	// Transfer Complete
	if( ( ( status & (1u << 5) ) >> 5 ) == 1){

		// I used these for debugging and it is working.
		// n2 is 0 (NDTR is decremented after each dma trasnfer)
		n2 = DMA2_Stream0->NDTR;

		samplingIsCompleted = 1;

		#if DEBUG_TIMING
			// For ADC_BUFFER_SIZE = 512 Full TX takes 986 microsecond
			// 986 / 512 = 1.926 microsecond = Tsampling in ADC1->SMPR1, perfect.
			adc_t3 = CK_TIME_GetMicroSec() - adc_t1;

			CK_ADC_DMA_Start();
			adc_t1 = CK_TIME_GetMicroSec();
		#endif

	}

}
