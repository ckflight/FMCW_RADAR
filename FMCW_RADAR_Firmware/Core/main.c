
#include "stm32f4xx_hal.h"

#include "USBD_CDC/CK_USBD_INTERFACE.h"
#include "CK_TIME_HAL.h"
#include "CK_SYSTEM.h"
#include "CK_GPIO.h"

#include "CK_TQP5523.h"
#include "CK_MCP4022.h"

#include "CK_MAX1426.h"
#include "CK_ADC_DMA.h"

#include "CK_DAC.h"
#include "CK_ADF4158.h"

#include "CK_CONFIGURATION.h"
#include "CK_MICROCARD.h"

/*
 * I will use a sawtooth ramp with 1ms period and 200MHz bandwith
 * 5.8-6.0 GHz range is 5v to 8.3v -> 1.67 to 2.77
 * so DAC range is from 2072 to 3438.
 * Approximately 1 microsecond timer update is necessary.
 *
 * Hardware parameters are configured by python script at the beginning of radar measurement.
 *
 * Radar range:
 * Distance 		= (c * Ts * Fif) / (2 * B)
 * Apprx. 500meters	= (3*10^8 * 1*10^-3 * 500*10^3) / (2 * 200 * 10^6)
 *
 */

#define MIXER_EN_GPIO			GPIOA
#define MIXER_EN_PIN			0

#define SYNC_SIGNAL_GPIO		GPIOB
#define SYNC_SIGNAL_PIN			9

#define FPGA_MODE				0

uint32_t RECORD_TIME_COUNTER 	= 10000; 	// default
uint8_t	 TX_MODE	 			= 0;		// 0 for continuous tx, 1 for on off with tx
uint8_t	 DATA_LOG				= 0;		// 0 for USB Transfer, 1 for Microcard Log
uint8_t	 ADC_SELECT				= 0;		// 0 for ADC DMA, 1 for External ADC MAX1426
uint8_t  PLL_SELECT				= 1;		// 0 for DAC, 1 for PLL

uint32_t record_counter = 0;

int isRestarted = 0;


typedef enum{

	IDLE,
	START_SAMPLING,
	CHECK_SAMPLING,
	TRANSFER_SAMPLES,
	FINISH_SAMPLING

}SAMPLING_STATES_E;

SAMPLING_STATES_E sampling_state = IDLE;

int main(void){

	CK_SYSTEM_SetSystemClock(SYSTEM_CLK_168MHz);

	HAL_Init();

	CK_GPIO_ClockEnable(GPIOA);
	CK_GPIO_ClockEnable(GPIOB);
	CK_GPIO_ClockEnable(GPIOC);
	CK_GPIO_ClockEnable(GPIOD);

	/*
	 * PA0 -> MIXER_EN, Enables or disables bias circuit and the internal detector.
	 * 		  See datasheet pg28 bias circuit. Use pull high
	 */
	CK_GPIO_Init(MIXER_EN_GPIO, MIXER_EN_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_SetPin(MIXER_EN_GPIO, MIXER_EN_PIN);

	/*
	 * Sync pulse is used to check if sampling and usb transfer is occurring before
	 * muxout so ramp timings. Use scope to check state machine.
	 */
	CK_GPIO_Init(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_ClearPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

	CK_USBD_Init();

	// adc uses it pin
	//MCP4022_Init();

	CK_TQP5523_Init(); // Start before configuration

	CK_CONFIGURATION_InitHardware(1); // 1 receive python, 2 default values for debugging, 3 for fpga

	RECORD_TIME_COUNTER 	= CK_CONFIGURATION_GetRecordTimeCounter();

	TX_MODE 				= CK_CONFIGURATION_GetTXMode();

	DATA_LOG				= CK_CONFIGURATION_GetDataLog();

	ADC_SELECT  			= CK_CONFIGURATION_GetAdcSelect();

	PLL_SELECT				= CK_CONFIGURATION_GetPLLSelect();

	uint8_t gain_value 		= CK_CONFIGURATION_GetGainValue();


	if(DATA_LOG == 1){
		CK_MICROCARD_Init(SPI_DMA_INTERRUPT_MULTIBLOCK);
	}

	//MCP4022_SetValue(gain_value);

	if(PLL_SELECT == 1){
		CK_ADF4158_Init(SAWTOOTH_WAVEFORM);
	}
	else{
		CK_DAC_Init(SAWTOOTH_WAVEFORM_DAC);
	}

	if(ADC_SELECT == 1){
		CK_MAX1426_Init();
	}
	else{
		CK_ADC_DMA_Init();
	}

	while(1){

		#if FPGA_MODE

			static int is_system_started = 0;

			if(!is_system_started){

				CK_TQP5523_Enable();

				uint8_t record_time = CK_CONFIGURATION_GetRecordTime();

				for(int i = 0; i < record_time * 1000; i++){

					CK_TIME_DelayMicroSec(1000);

				}

				CK_TQP5523_Disable();

				is_system_started = 1;

			}


		#else

			switch(sampling_state){

			case IDLE:

				if(TX_MODE == 0){
					CK_TQP5523_Enable();
				}

				sampling_state = START_SAMPLING;

				break;

			case START_SAMPLING:

				if(CK_ADF4158_RampCompleted() == 0 && CK_ADF4158_RampStarted() == 1){

					if(TX_MODE == 1){
						CK_TQP5523_Enable();
					}

					if(ADC_SELECT == 1){

						CK_MAX1426_Reset_Counter();
						CK_MAX1426_ResetRecordDone();
						CK_MAX1426_ADC_OutputEnable();
						CK_MAX1426_PWM_Start();

					}
					else{

						// Start sampling
						CK_ADC_DMA_Start();

					}

					sampling_state = CHECK_SAMPLING;

					// Use these pulse to check sync with muxout.
					CK_GPIO_ClearPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

				}

				break;

			case CHECK_SAMPLING:

				if(ADC_SELECT == 1){

					// Check is buffer full
					if(CK_MAX1426_IsRecordDone() == 1){

						if(TX_MODE == 1){
							CK_TQP5523_Disable();
						}

						CK_MAX1426_ADC_OutputDisable();
						CK_MAX1426_PWM_Stop();

						sampling_state = TRANSFER_SAMPLES;

					}

				}
				else{

					// Check full dma transfer is completed
					if(CK_ADC_DMA_IsTxComplete() == 1){

						if(TX_MODE == 1){
							CK_TQP5523_Disable();
						}

						// DMA is not in circular mode so it stops automatically after NDTR transfers

						// Use these pulse to check sync with muxout.
						CK_GPIO_SetPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

						sampling_state = TRANSFER_SAMPLES;
					}
				}

				break;

			case TRANSFER_SAMPLES:

				if(CK_ADF4158_RampCompleted() == 1 && CK_ADF4158_RampStarted() == 0){

					if(DATA_LOG == 0){

						if(record_counter < RECORD_TIME_COUNTER){

							record_counter++;

							CK_GPIO_ClearPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

							if(ADC_SELECT == 1){
								CK_MAX1426_TransferSamplesUSB();
							}
							else{
								CK_ADC_DMA_TransferSamplesUSB();
							}


							CK_GPIO_SetPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

							sampling_state = START_SAMPLING;

						}
						else{

							if(TX_MODE == 0){
								CK_TQP5523_Disable();
							}

							sampling_state = FINISH_SAMPLING;
						}

					}
					else{

						if(!CK_MICROCARD_IsCompleted()){

							CK_GPIO_ClearPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

							if(ADC_SELECT == 1){
								CK_MAX1426_TransferSamplesMicroCard();
							}
							else{
								// Implement microcard log for adc dma
							}

							CK_GPIO_SetPin(SYNC_SIGNAL_GPIO, SYNC_SIGNAL_PIN);

							sampling_state = START_SAMPLING;

						}

						else{

							if(TX_MODE == 0){
								CK_TQP5523_Disable();
							}

							sampling_state = FINISH_SAMPLING;
						}

					}

				}

				break;

			case FINISH_SAMPLING:

				NVIC_SystemReset();

				break;

			}


		#endif


	}

}
