
#include "CK_TQP5523.h"
#include "CK_GPIO.h"
#include "CK_TIME_HAL.h"
#include "USBD_CDC/CK_USBD_INTERFACE.h"

/*
 * PB0 -> DET_OUT (ADC12_IN8), 0.3 to 1V output voltage to indicate output dBm of PA
 * PB1 -> PA_EN Active High. Pulled Down by default. (When radar is functioning activate it.)
 *
 */

#define DET_OUT_GPIO			GPIOB
#define DET_OUT_PIN				0

#define PA_EN_GPIO				GPIOB
#define PA_EN_PIN				1

uint32_t adc_data = 0;
float detector_output_voltage = 0.0f;
int detector_output_dBm = 0;

uint32_t previousTime = 0;
uint32_t currentTime = 0;

int state = 0;
int isMeasured = 0;

// TQP5523 Datasheet Page 6
uint8_t TQP5523_DBM_PROFILE[6]   = {8,   16,  20,  22,  24,  26};
float TQP5523_VOLTAGE_PROFILE[6] = {0.4, 0.5, 0.6, 0.7, 0.8, 0.9};

void CK_TQP5523_Init(void){

	// PA_EN Pin
	CK_GPIO_Init(PA_EN_GPIO, PA_EN_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_MEDIUM, CK_GPIO_NOPUPD);

	CK_TQP5523_Disable();

	// PB0 ADC12_IN8 (Channel 8 at either ADC1 or 2)

	// Default sampling time is 3 cycle set in ADC_SMPRx register
	// so one conversion takes 12(bitresolution)+3 cycles = 15 cycles
	// For 42MHz ADCCLK it makes 2.8Msps
	// Default bit resolution is 12bit at ADC_CR1 register.

	// Channel selection
	// ADCx_SQR1 L[3:0] bits default 0 which is only 1 channel conversion

	RCC->APB2ENR |= (1u << 9); // 8 is ADC1, 9 ADC2 and 10 ADC3
	GPIOB->MODER |= (3u << 0); // pin 0 analog mode.

	// APB2 clock is 82MHz, ADCCLK = APB2 / prescalar
	// 0->2, 1->4, 2->6, 3->8 (Default is 2, 0u << 16)
	// Main ADC Clock for all ADC and fix.
	// ADCCLK = 82/6 = 14 MHz sampling is configured according to CK_ADC_DMA
	ADC->CCR |= (2u << 16);

	//0->3cycle, 1->15, 2->28, 3->56, 4->84, 5->112, 6->144, 7->480
	// ADC2 Channel 8 Sampling time = 12 + 480 = 492
	// Tsampling = 492 * (1/14MHz) = 35 microsecond for channel 8
	ADC2->SMPR2 |= (7u << 24); // sampling time cycle selection for channel 8

	// I assing the channel 8 to the first conversion of ADC1
	ADC2->SQR3 |= (8u << 0);

	ADC2->CR1 |= (1u << 5); // End of conversion interrupt enable.

	// Call start conversion whenever want to read adc.
	// It will go to interrupt once it is finished.
	// Or, continuous conversion goes to interrupt every 50microsec.
	//ADC2->CR2 |= (1u << 1); // continuous conversion,

	ADC2->CR2 |= (1u << 0); // Enable ADC

	HAL_NVIC_EnableIRQ(ADC_IRQn);

}

void CK_TQP5523_Update(void){

	currentTime = CK_TIME_GetMicroSec();
	float dTime = currentTime - previousTime;

	// 50Hz
	if(dTime > 50000){

		previousTime = currentTime;

		if(state == 0){

			state = 1;
			CK_TQP5523_StartConversion();

		}
		else{

			state = 0;
			CK_USBD_StringPrint("TQP55 Detector Output Voltage: ");
			CK_USBD_FloatPrintln(detector_output_voltage);

			CK_USBD_Transmit();

		}

	}
}

void CK_TQP5523_CheckOutputPower(void){

	isMeasured = 0;

	CK_TQP5523_StartConversion();

	while(!isMeasured);

	for(int i = 0; i < 5; i++){

		float volt1 = TQP5523_VOLTAGE_PROFILE[i];
		float volt2 = TQP5523_VOLTAGE_PROFILE[i+1];

		if(detector_output_voltage >= volt1 && detector_output_voltage <= volt2){

			if((detector_output_voltage - volt1) < (detector_output_voltage - volt2)){
				detector_output_dBm = TQP5523_DBM_PROFILE[i];
			}
			else{
				detector_output_dBm = TQP5523_DBM_PROFILE[i+1];
			}

		}
		else if(detector_output_voltage < volt1){
			//detector_output_dBm = 0;
		}
		else if(detector_output_voltage > volt2){
			//detector_output_dBm = 27;
		}

	}

}

void CK_TQP5523_StartConversion(void){

	// Conversion start, can be set only when adc is enabled
	ADC2->CR2 |= (1u << 30);
}

void CK_TQP5523_Disable(void){

	CK_GPIO_ClearPin(PA_EN_GPIO, PA_EN_PIN);
}

void CK_TQP5523_Enable(void){

	CK_GPIO_SetPin(PA_EN_GPIO, PA_EN_PIN);
}

float CK_TQP5523_ReadDetectorOutputVoltage(void){

	return detector_output_voltage;
}

int CK_TQP5523_ReadDetectorOutputDBM(void){

	return detector_output_dBm;
}

void ADC_IRQHandler(void){

	/*
	static uint32_t t1 = 0;
	static int temp = 0;
	if(temp==0){
		temp=1;
		t1 = CK_TIME_GetMicroSec();
	}
	else{
		uint32_t t2 = CK_TIME_GetMicroSec() - t1;
		uint32_t t3 = t2;
		temp=0;
	}
	*/

	uint32_t stat1 = ADC2->SR;

	if((stat1 & (1u << 1)) == 2){

		adc_data = ADC2->DR & 0xFFFF;
		detector_output_voltage = (float)adc_data / 4096.0;
		detector_output_voltage *= 3.0;

		isMeasured = 1;

		ADC2->SR = 0; // Clear status register.

	}
}








