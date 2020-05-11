
#include "CK_ADF4158.h"

#include "CK_SPI.h"
#include "CK_GPIO.h"
#include "CK_TIME_HAL.h"

#include "math.h"

#include "CK_CONFIGURATION.h"

/*
 * ADF4158 Freq. Synth. will be used later so not initialize them now.
 * PA1 -> ADF_MUXOUT
 * PC5 -> ADF_TXDATA (this will be cleared when rf disabled)
 * PC4 -> ADF_CE (low disables, high enables adf4158 chip)
 * PA2 -> ADF_LE (make it low before write register and high after write, make it high before read)
 * PA5 -> ADF_CLK  (SPI1_CLK)
 * PA7 -> ADF_DATA (SPI1_MOSI)
 *
 */

#define ADF_MUXOUT_GPIO			GPIOA
#define ADF_MUXOUT_PIN			1

#define ADF_MUXOUT_OUTPUT_GPIO	GPIOB
#define ADF_MUXOUT_OUTPUT_PIN	8

#define ADF_TXDATA_GPIO			GPIOC
#define ADF_TXDATA_PIN			5

#define ADF_LE_GPIO				GPIOA
#define ADF_LE_PIN				2

#define ADF_CE_GPIO				GPIOC
#define ADF_CE_PIN				4

#define ADF_SPI					SPI1

#define FREQ_PFD				30000000

uint32_t muxout_start = 0, muxout_pulse = 0;
int rampStarted = 0, rampCompleted = 0;

float sweep_period = 0.0f;

// Default parameters
float waveform_ramp = 1.03e-3;
float waveform_gap  = 570e-6;

void CK_ADF4158_Init(WAVEFORM_TYPE wf){

	// Read rampDel length high pulse on this pin to know ramp start and end
	CK_GPIO_Init(ADF_MUXOUT_GPIO, ADF_MUXOUT_PIN, CK_GPIO_INPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);

	// Muxout input signal is outputted to gpio available side of the board to connect to scope easily.
	CK_GPIO_Init(ADF_MUXOUT_OUTPUT_GPIO, ADF_MUXOUT_OUTPUT_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);

	RCC->APB2ENR |= 1u << 14; // SYSCFG Clock Enable

	// 0u for GPIOA, 1u for GPIOB, 2u for GPIOC
	SYSCFG->EXTICR[0] |= (0u << 4); // External interrupt is set to GPIOA Pin1

	EXTI->IMR  |= (1u << 1);	// Line 1 Interrupt Mask Request

	EXTI->EMR  |= (1u << 1);	// Line 1 Event Mask Request

	EXTI->RTSR |= (1u << 1);	// Rising edge detection for each line

	EXTI->FTSR |= (1u << 1);   // Falling edge detection for each line

	//NVIC_SetPriority(EXTI1_IRQn, 0);

	NVIC_EnableIRQ(EXTI1_IRQn);

	CK_GPIO_Init(ADF_TXDATA_GPIO, ADF_TXDATA_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_ClearPin(ADF_TXDATA_GPIO, ADF_TXDATA_PIN);

	CK_GPIO_Init(ADF_LE_GPIO, ADF_LE_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_SetPin(ADF_LE_GPIO, ADF_LE_PIN);

	CK_GPIO_Init(ADF_CE_GPIO, ADF_CE_PIN, CK_GPIO_OUTPUT, CK_GPIO_NOAF, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_ClearPin(ADF_CE_GPIO, ADF_CE_PIN);

	CK_SPI_Init(ADF_SPI);

	CK_ADF4158_DeviceEnable();

	float sweep_t = (float)CK_CONFIGURATION_GetSweepTime() / 1000000.0f;
	float sweep_g  = (float)CK_CONFIGURATION_GetSweepGap() / 1000000.0f;
	sweep_period = sweep_t + sweep_g;

	int sweep_gap = (int)(sweep_g * 1000000);

	// 5ms gap burned system.
	if(sweep_gap > 1500){
		sweep_gap = 1500;
	}

	uint32_t sw_start = CK_CONFIGURATION_GetSweepStartFrequency();
	double sweep_start_freq_ = sw_start * 1e7;

	uint32_t sw_bw = CK_CONFIGURATION_GetSweepBandwith();
	double sweep_bw_ = sw_bw * 1e6;

	int sweep_type = CK_CONFIGURATION_SweepType();
	if(sweep_type == 0){
		wf = SAWTOOTH_WAVEFORM;
	}
	else{
		wf = TRIANGULAR_WAVEFORM;
	}

	CK_ADF4158_Configure_Sweep(wf, sweep_start_freq_, sweep_bw_, sweep_period, sweep_gap);
	//CK_ADF4158_Configure_Sweep(SAWTOOTH_WAVEFORM, 5.6e9, 100e6, 1.03e-3, 570); // working
	//CK_ADF4158_Configure_Sweep(TRIANGULAR_WAVEFORM, 5.6e9, 100e6, 2.03e-3, 0); // working

}

void CK_ADF4158_Configure_Sweep(WAVEFORM_TYPE wf, double startFreq, double bw, double rampTime, int rampDel){

	// Resolution frequency = FREQ_PFD / 2^25;
	double fres = ((double)FREQ_PFD)/(1 << 25);
	unsigned int devmax = 1 << 15;
	unsigned int clk2 = 1;

	// RFOut = [N + (FRAC / 2^25)] * FREQ_PFD
	unsigned int n = startFreq / FREQ_PFD;
	unsigned int frac_msb = ((startFreq / FREQ_PFD) - n) * (1 << 12);
	unsigned int frac_lsb = ((((startFreq / FREQ_PFD) - n) * (1 << 12)) - frac_msb) * (1 << 13);

	// Datasheet selects freq. resolution and then makes calculations.
	// If fdev is selected too small dev_offset becomes a negative number so 0 is asssigned
	// and rampDel will not work. "bw / (rampTime * 1000000)" works perfectly with generating gap as selected.
	unsigned int fdev = bw / (rampTime * 1000000);
	unsigned int steps = bw / fdev;
	double timer = rampTime / steps;
	unsigned int clk1 = (FREQ_PFD * timer);

	int dev_offset = (int)ceil(log2(fdev/(fres*devmax)));
	if(dev_offset < 0){
		dev_offset = 0;
	}

	unsigned int dev = fdev/(fres * (1 << dev_offset));

	uint32_t data = 0;

	// R7 Register
	// [18] RAMP DEL FL : '1' ENABLE
	// [17] RAMP DEL 	: '1' ENABLE
	// [16] DEL CLK SEL	: '1' PFD x CLK1
	// [15] DEL ENABLE 	: '1' ENABLE
	// [14:3] DEL 12Bit Word
	data = 0;
	if(rampDel > 0 && wf == SAWTOOTH_WAVEFORM){
		data |= (1u << 18) | (1u << 17) | (1u << 16) | (1u << 15) | (rampDel << 3) | (7u << 0);
		CK_ADF4158_WriteRegister(data);
	}

	// R6 Register
	// [22:3] STEP WORD 				: steps
	// [2:0] R6 Register control bits 	: '110'
	data = 0;
	data |= (steps << 3) | (6u << 0);
	CK_ADF4158_WriteRegister(data);

	// R5 Register
	// [22:19] DEVIATION OFFSET : dev_offset
	// [18:3] DEVIATION WORD: dev
	// [2:0] R5 Register control bits : '101'
	data = 0;
	data |= (dev_offset << 19) | (dev << 3) | (5u << 0);
	CK_ADF4158_WriteRegister(data);

	// R4 Register
	// [30:26] MODULATOR MODE : DISABLE WHEN FRAC = 0 	: '01110' now frac. is used so 0
	// [24:23] NEG BLEED CURRENT 						: ON '11' , OFF '00' with MUX READ BACK
	// [22:21] READ BACK TO MUX 						: ENABLE '11' (Page 30 says 3 not 2)
	// [20:19] CLK DIV MODE 							: RAMP DIVIDER '11'
	// [18:7] 12 BIT CLK2 DIVER VALUE 					: 1
	// [2:0] R4 Register control bits 					: '100'
	data = 0;
	data |= (0u << 26) | (0u << 23) | (3u << 21) | (3u << 19) | (clk2 << 7) | (4u << 0);
	CK_ADF4158_WriteRegister(data);

	// R3 Register
	// [11:10] RAMP MODE 					: Continuous Triangle '01', Continuout Sawtooth '00'
	// [2:0] R3 Register control bits 		: '011'
	// [6] Phase detector polarity of VCO 	: Positive '1'
	// [2:0] R3 Register control bits 		: '011'
	data = 0;
	if(wf == TRIANGULAR_WAVEFORM){
		data |= (1u << 10);
	}
	else if(wf == SAWTOOTH_WAVEFORM){
		data |= (0u << 10);
	}
	data |= (1u << 6) | (3u << 0);
	CK_ADF4158_WriteRegister(data);

	// R2 Register
	// [28] CSR Enable to improve lock times and removes noise makes waveform perfect.
	// [27:24] CP CURRENT SETTING 		: Icpmax = 25.5 / Rset(5.49K) = 4.65 mA : 14 for Icp 4.69mA
	// [22] PRESCALAR 					: 8/9 '1' for above 3GHz operations
	// [21] DIVIDE BY 2 BIT 			: '0' default
	// [20] REFERENCE DOUBLER 			: '0' default
	// [19:15] R COUNTER 				: 1
	// [14:3] CLK1 DIVIDER 				: clk1
	// [2:0] R3 Register control bits 	: '010'
	data = 0;
	data |= (1u << 28) | (14u << 24) | (1u << 22) | (1u << 15) | (clk1 << 3) | (2u << 0);
	CK_ADF4158_WriteRegister(data);

	// R1 Register
	// [27:15] 13 Bit LSB FRAC Value
	// [2:0] R1 Register control bits : '001'
	data = 0;
	data |= (frac_lsb << 15) | (1u << 0);
	CK_ADF4158_WriteRegister(data);

	// R0 Register
	// [31] RAMP ON 					: '1' RAMP ENABLE
	// [30:27] READ BACK TO MUX 		: 15 (Generates pulse at MUXOUT pin with R4 READ BACK TO MUX)
	// [26:15] 12 BIT INT VALUE 		: n
	// [14:3] 13 Bit MSB FRAC Value
	// [2:0] R0 Register control bits 	: '000'
	data = 0;
	data |= (1u << 31) | (15u << 27) | (n << 15) | (frac_msb << 3) | (0u << 0);
	CK_ADF4158_WriteRegister(data);


}

void CK_ADF4158_WriteRegister(uint32_t data){

	CK_GPIO_ClearPin(ADF_LE_GPIO, ADF_LE_PIN);

	CK_SPI_Transfer(ADF_SPI, (uint8_t)(data >> 24));
	CK_SPI_Transfer(ADF_SPI, (uint8_t)(data >> 16));
	CK_SPI_Transfer(ADF_SPI, (uint8_t)(data >> 8));
	CK_SPI_Transfer(ADF_SPI, (uint8_t)data);

	CK_GPIO_SetPin(ADF_LE_GPIO, ADF_LE_PIN);

}

void CK_ADF4158_DeviceEnable(void){

	CK_GPIO_SetPin(ADF_CE_GPIO, ADF_CE_PIN);
}

uint32_t CK_ADF4158_GetPulseReceived(void){
	return muxout_pulse;
}

// MUXOUT gives high during gap between ramps and low during ramp.
int CK_ADF4158_RampStarted(void){
	return rampStarted;
}

int CK_ADF4158_RampCompleted(void){
	return rampCompleted;
}

void EXTI1_IRQHandler(void){

	if(((ADF_MUXOUT_GPIO->IDR & (1u << ADF_MUXOUT_PIN)) >> ADF_MUXOUT_PIN) == 1){

		CK_GPIO_SetPin(ADF_MUXOUT_OUTPUT_GPIO, ADF_MUXOUT_OUTPUT_PIN);

		muxout_start = CK_TIME_GetMicroSec();

		rampCompleted = 1;
		rampStarted = 0;

	}
	else if(((ADF_MUXOUT_GPIO->IDR & (1u << ADF_MUXOUT_PIN)) >> ADF_MUXOUT_PIN) == 0){

		CK_GPIO_ClearPin(ADF_MUXOUT_OUTPUT_GPIO, ADF_MUXOUT_OUTPUT_PIN);

		if(muxout_start != 0){

			muxout_pulse = CK_TIME_GetMicroSec() - muxout_start;
			muxout_start = 0;

			rampCompleted = 0;
			rampStarted = 1;

		}
	}

	EXTI->PR |= 1u << 1; //Clear Interrupt

}













