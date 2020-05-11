
#include "CK_MAX1426.h"
#include "CK_CONFIGURATION.h"

#include "USBD_CDC/CK_USBD_INTERFACE.h"

#include "CK_TIME_HAL.h"
#include "CK_GPIO.h"

#include "stm32f4xx.h"

/*
 *
 * MAX1426 ADC PINS:
 * MAX1426 samples at falling edge of the ADC_CLK and the data is available
 * at the rising edge of the ADC_CLK.
 *
 * PC10 -> ADC_OE, Active Low Output enable pin, when enabled conversion starts
 *
 * PA9  -> ADC_CLK, TIM1_CH2
 * PA10 -> ADC_CLK_EXTIN, TIM1_CH3 and EXTI
 *
 * PB10 -> ADC_D0
 * PB11 -> ADC_D1
 * PB12 -> ADC_D2
 * PB13 -> ADC_D3
 * PB14 -> ADC_D4
 * PB15 -> ADC_D5
 * PC6  -> ADC_D6
 * PC7  -> ADC_D7
 * PC8  -> ADC_D8
 * PC9  -> ADC_D9
 *
 *
 */
#define MAX1426_EXTI_GPIO		GPIOA
#define MAX1426_EXTI_PIN		10

#define MAX1426_CLK_GPIO		GPIOA
#define MAX1426_CLK_PIN			9

#define MAX1426_D0_5_GPIO		GPIOB
#define MAX1426_D6_9_GPIO		GPIOC

#define MAX1426_OE_GPIO			GPIOC
#define MAX1426_OE_PIN			10

#define MAX1426_D0				10
#define MAX1426_D1				11
#define MAX1426_D2				12
#define MAX1426_D3				13
#define MAX1426_D4				14
#define MAX1426_D5				15
#define MAX1426_D6				6
#define MAX1426_D7				7
#define MAX1426_D8				8
#define MAX1426_D9				9


#define DEBUG_TIMING			0

#define TEST_MODE				0

uint32_t MAX1426_BUFFER_SIZE = 0;

uint16_t adc_data_10bit = 0;
uint16_t adc_data_counter = 0;

uint8_t gpiob_read = 0;
uint8_t gpioc_read = 0;

TIM_HandleTypeDef htim1;

uint16_t test_counter = 0;

uint8_t recordDone = 0;

uint16_t max1426_buffer[4096];
uint32_t max1426_buffer_counter = 0;


void CK_MAX1426_Init(void){

	// MAX1426 Implementation.

	// External interrupt in for triggering rising edge of the adc clock
	// MAX1426 samples the data at the falling edge of the clock
	// new data is available at the rising edge of the clock

	// Init external interrupt pin for adc clock rising edge triggering

	// GPIO Clock is enabled in the main

	MAX1426_BUFFER_SIZE = CK_CONFIGURATION_GetSampleNumber();

	CK_GPIO_Init(MAX1426_EXTI_GPIO, MAX1426_EXTI_PIN, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_PULLDOWN);

    CK_GPIO_Init(MAX1426_CLK_GPIO, MAX1426_CLK_PIN, CK_GPIO_AF, CK_GPIO_AF1, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);

	CK_GPIO_Init(MAX1426_OE_GPIO, MAX1426_OE_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_HIGH, CK_GPIO_NOPUPD);
	CK_MAX1426_ADC_OutputDisable();

	// MAX1426 Data Input
	CK_GPIO_Init(MAX1426_D0_5_GPIO, MAX1426_D0, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D0_5_GPIO, MAX1426_D1, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D0_5_GPIO, MAX1426_D2, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D0_5_GPIO, MAX1426_D3, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D0_5_GPIO, MAX1426_D4, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D0_5_GPIO, MAX1426_D5, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D6_9_GPIO, MAX1426_D6, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D6_9_GPIO, MAX1426_D7, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D6_9_GPIO, MAX1426_D8, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(MAX1426_D6_9_GPIO, MAX1426_D9, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);

	CK_MAX1426_EXTI_Init();

	CK_MAX1426_PWM_Init();

}

void CK_MAX1426_EXTI_Init(void){

	RCC->APB2ENR |= 1u << 14; // SYSCFG Clock Enable

	// 0u for GPIOA, 1u for GPIOB, 2u for GPIOC
	SYSCFG->EXTICR[2] |= (0u << 8); // External interrupt 10 is set to GPIOA Pin10

	EXTI->IMR  |= (1u << 10);	// Line 10 Interrupt Mask Request

	EXTI->EMR  |= (1u << 10);	// Line 10 Event Mask Request

	EXTI->RTSR |= (1u << 10);	// Rising edge detection for each line

	//EXTI->FTSR |= (1u << 10);   // Falling edge detection for each line

	NVIC_SetPriority(EXTI15_10_IRQn, 0);

	NVIC_EnableIRQ(EXTI15_10_IRQn);

}

void CK_MAX1426_PWM_Init(void){

	__HAL_RCC_TIM1_CLK_ENABLE();

	// TIMER1 CH2 Configuration
	htim1.Instance 					= TIM1;
	htim1.Init.Prescaler 			= 0; // Divide 168MHz to 0+1 = 168MHz timer clock
	htim1.Init.CounterMode 			= TIM_COUNTERMODE_UP;
	uint32_t pwm_freq 				= CK_CONFIGURATION_GetSamplingFrequency() * 1000;
	if(pwm_freq >= 500000){
		pwm_freq = 500000;
	}
	htim1.Init.Period 				= 168000000 / pwm_freq;
	htim1.Init.ClockDivision 		= TIM_CLOCKDIVISION_DIV2;
	htim1.Init.RepetitionCounter 	= 0;
	htim1.Init.AutoReloadPreload 	= TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_PWM_Init(&htim1) != HAL_OK);

	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode	 	= TIM_OCMODE_PWM1;
	sConfigOC.OCPolarity 	= TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity 	= TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode 	= TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState 	= TIM_OCIDLESTATE_SET;
	sConfigOC.OCNIdleState 	= TIM_OCNIDLESTATE_RESET;

	sConfigOC.Pulse = htim1.Init.Period / 2; // 50% Duty_cycle
	if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK);

}

void CK_MAX1426_PWM_Start(void){

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

}

void CK_MAX1426_PWM_Stop(void){
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
}

void CK_MAX1426_ADC_OutputDisable(void){
	CK_GPIO_SetPin(MAX1426_OE_GPIO, MAX1426_OE_PIN);
}

void CK_MAX1426_ADC_OutputEnable(void){
	CK_GPIO_ClearPin(MAX1426_OE_GPIO, MAX1426_OE_PIN);
}

uint16_t CK_MAX1426_Get_ADCResult(void){
	return adc_data_10bit;
}

uint16_t CK_MAX1426_Get_Counter(void){
	return max1426_buffer_counter;
}

void CK_MAX1426_Reset_Counter(void){
	max1426_buffer_counter = 0;
}

uint8_t CK_MAX1426_IsRecordDone(void){
	return recordDone;
}

void CK_MAX1426_ResetRecordDone(void){
	recordDone = 0;
}

void CK_MAX1426_TransferSamples(void){

	#if DEBUG_TIMING_
		uint32_t adc_tx_t1 = CK_TIME_GetMicroSec();
	#endif

	uint16_t current_data = 0;
	uint8_t adc_lsb = 0;
	uint8_t adc_msb = 0;

	for(int i = 0; i < MAX1426_BUFFER_SIZE; i++){

		current_data = max1426_buffer[i];

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

	#if DEBUG_TIMING_
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

void EXTI15_10_IRQHandler(void){

	// It will come here at the rising edge of the ADC_CLK
	// Read data from D0-D9 pins at once as quick as possible.

	// This part is taking 1-2 microseconds so 250KHz sample would be fine.

	#if DEBUG_TIMING
		uint32_t time1 = CK_TIME_GetMicroSec();
	#endif

	gpiob_read = (uint8_t)((GPIOB->IDR >> 10) & 0x3F);
	gpioc_read = (uint8_t)((GPIOC->IDR >> 6) & 0x0F);

	adc_data_10bit = (((gpioc_read << 6) | gpiob_read) & 0x03FF);

	max1426_buffer[max1426_buffer_counter++] = adc_data_10bit;

	if(max1426_buffer_counter == MAX1426_BUFFER_SIZE){

		recordDone = 1;
	}

	#if DEBUG_TIMING
		uint32_t t_buffer[MAX1426_BUFFER_SIZE];
		UNUSED(t_buffer);
		static uint32_t t_buffer_counter = 0;

		uint32_t time2 = CK_TIME_GetMicroSec() - time1;

		t_buffer[t_buffer_counter++] = time2;
		if(t_buffer_counter == MAX1426_BUFFER_SIZE){
			t_buffer_counter = 0;
		}


	#endif

	EXTI->PR |= 1u << 10; // Clear interrupt

}







