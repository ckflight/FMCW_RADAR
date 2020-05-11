
#include "CK_DAC.h"
#include "CK_GPIO.h"
#include "CK_TIME_HAL.h"


// DAC's internal triangle waveform generator is used
#define DAC_WAVEFORM_GENERATE

#define DAC_OUT_GPIO		GPIOA
#define DAC_OUT_PIN			4


#define DAC_START			0		//2072
#define DAC_END				4095	//3438


uint16_t dac_counter = DAC_START;

int dac_count_way = 1;

RAMP_TYPE selected_ramp;

/*
 * NOTES:
 * Triangular is better compared to sawtooth in fast vehicle detection
 * and for rest they are almost same in Radar's perspective.
 * Also in triangular mode, i do not need to know if waveform period is done since
 * it will count down so if 6GHz is send 5.9GHz will be mixed not start freq. of 5.6GHz
 * Also STM32F405 has internal TRIANGLE generator reaching 400microsecond period
 * without disturbing the code with MHz level interrupts.
 *
 */

void CK_DAC_Init(RAMP_TYPE ramp){

	selected_ramp = ramp;

	// PA4 DAC_OUT1
	// Clock is enabled by GPIO Init function

	// APB1ENR TIM6 4th bit, DAC 29th bit
	RCC->APB1ENR |= (1u << 29);

	GPIOA->MODER |= (3u << 8); // pin 4 is in analog mode.
	//GPIOA->MODER |= (3u << 10); // pin 5 is in analog mode.

	#if defined(DAC_WAVEFORM_GENERATE)
	// DAC Trigger enable, software trigger selected, output buffer is enabled default
	// Triangle generator setup number bigger than 11 to max aplitude of 3.3v (bit 24-27)
	// Triangle generation 2u to bits 22-23
	// Trigger enable for waveform generation bit 18
	// Trigger select is software bits 19-21 (Conversion starts with SWTRIG bit)
	// Internal counter will be incremented after each trigger event
	// Bit17 is output buffer enabled.
	// Timer7 trigger TRGO event is selected for TSEL bits
	// I select Update event in timer7 cr2 register
	// Now triangle is auto generated internally by timer7 pulses
	// And it can be as fast as APB1 clock speed

	//DAC1
	// 5.7 to 5.9 GHZ range will be used. HMC431 generates 5 to 6.2 GHz range
	// 5.7 GHz -> 3V(HMC431 Datasheet) -> 3V / 3 = 1V    -> (4096 * 1V) / 3.3V = 1241 is start value of DAC
	// 5.9 GHz -> 5V(HMC431 Datasheet) -> 5V / 3 = 1.65V -> (4096 * 1.65V) / 3.3V = 2047 is end value of DAC
	// Set MAMP1 is max value internal counter reaches.
	// Internal counter is added to DHR value and output voltage is generated.
	// 7u << 8 is 255 max for counter with 1650 for start value. I will generate 100 MHz
	// from 5.8 to 5.9GHz. In this way it might have a better resolution for short distance.
	DAC->CR |= (7u << 8) | (2u << 6) | (2u << 3) | (1u << 2) | (1u << 1);

	DAC->DHR12R1 = 1650;

	//DAC2 for testing not implemented on hardware!!!
	//DAC->CR |= (12u << 24) | (2u << 22) | (2u << 19) | (1u << 18) | (1u << 17);

	#else
	// DAC Trigger enable, software trigger selected, output buffer is enabled default
	// DAC1
	DAC->CR |= (1u << 1) | (1u << 2) | (7u << 3);

	// DAC2
	//DAC->CR |= (1u << 17) | (1u << 18) | (7u << 19);
	#endif

	// Enable DAC1
	DAC->CR |= (1u<<0);

	// Enable DAC2
	//DAC->CR |= (1u<<16);

	CK_DAC_TIMER_Init();

}

void CK_DAC_TIMER_Init(void){

	// APB1ENR TIM6 4th bit, TIM7 5th bit, TIM5 3th bit
	// APB1 42MHz x 2 = Timer Clock 84MHz
	RCC->APB1ENR |= (1u << 5);

	/* Reference manual p.153
	 * If the APB prescaler is configured to a division factor of 1,
	 * the timer clock frequencies (TIMxCLK) are set to PCLKx.
	 * Otherwise, the timer clock frequencies are twice the frequency of the
	 * APB domain to which the timers are connected: TIMxCLK = 2xPCLKx.
	 */

	#if defined(DAC_WAVEFORM_GENERATE)
		// In triangle waveform usage ARR and PSC can be set for max speed
		// where triangle period is down to 400 microsec at max with PSC = 0 and ARR = 1

		// CLK = TimerClkb(84MHz) / (PSC + 1)
		// Some measured settings:
		// PSC 1, ARR 84 -> 4.4ms period for 3V to 5V range (looks good on scope fft)
		// PSC 2, ARR 84 -> 6.4ms period for 3V to 5V range
		// PSC 4, ARR 168 -> 20ms period for 3V to 5V range
		// PSC 4, ARR 336 -> 40ms period for 3V to 5V range (MIT Radar is using 40ms)
		// Low period work better.
		TIM7->PSC = 1;
		TIM7->ARR = 336;

		// Master mode selection for TRGO event for DAC
		// Update event is selected
		TIM7->CR2 |= (2u << 4);

	#else
		// CLK = TimerClkb(84MHz) / (PSC + 1)
		TIM7->PSC = 1;

		// 84 is minimum can be set without disturbing the code with interrupt
		// And it cannot reach higher speed even if for example 42 is set.
		TIM7->ARR = 84;	// 84 for 2microsec interrupt

		HAL_NVIC_EnableIRQ(TIM7_IRQn);

	#endif

	// Update generate
	TIM7->EGR |= (1u << 0);

	// Auto reload enable, update source
	TIM7->CR1 |= (1u << 7) | (1u << 2);

	// update interrupt enabled
	TIM7->DIER |= 1u << 0;

	// counter enable
	TIM7->CR1 |= (1u <<0);

}

void CK_DAC_LoadValue(uint16_t value){

	// DAC1 right alligned 12bit data register
	DAC->DHR12R1 = value & 0x0FFF;
	DAC->DHR12R2 = value & 0x0FFF;

	// Software trigger to start conversion
	// DHR value is loaded to DORx after 3 APB1 clocks
	// this bit is cleared auto after conversion
	DAC->SWTRIGR |= 1u<<0; // soft. trig dac1
	DAC->SWTRIGR |= 1u<<1; // soft. trig dac2

}

void CK_DAC_Sawtooth(void){

	if(dac_counter == DAC_END){
		dac_counter = DAC_START;
	}
	else{
		dac_counter++;
	}

	CK_DAC_LoadValue(dac_counter);

}

void CK_DAC_Triangular(void){

	if(dac_count_way == 1){
		if(dac_counter == DAC_END){
			dac_count_way = 0;
		}
		else{
			dac_counter++;
		}
	}
	else{
		if(dac_counter == DAC_START){
			dac_count_way = 1;
		}
		else{
			dac_counter--;
		}
	}

	CK_DAC_LoadValue(dac_counter);
}

#if defined(DAC_WAVEFORM_GENERATE)
void TIM7_IRQHandler(void){

	// For triangle generation of software trigger this must be done.
	// Will not use soft. trigger for waveform generation.
	//DAC->SWTRIGR |= 1u<<1; // soft. trig dac2
	TIM7->SR = 0;

}
#else

// If DAC will be driven manually with software trigger.
void TIM7_IRQHandler(void){

	uint32_t status = TIM7->SR;

	if(status == 1){

		// This part for detecting DAC timer frequency.
		/*
		static int count = 0;
		if(count == 0){
			dac_timer_t1 = CK_TIME_GetMicroSec();
			count = 1;
		}
		else{
			dac_timer_t2 = CK_TIME_GetMicroSec() - dac_timer_t1;
			count = 0;
		}
		*/

		if(selected_ramp == TRIANGULAR_WAVEFORM_DAC){
			CK_DAC_Triangular();
		}
		else if(selected_ramp == SAWTOOTH_WAVEFORM_DAC){
			CK_DAC_Sawtooth();
		}

		TIM7->SR = 0;

	}

}

#endif
