
#ifndef CK_DAC_H_
#define CK_DAC_H_

#include "stm32f4xx.h"

uint32_t dac_timer_t1, dac_timer_t2;

typedef enum{

	SAWTOOTH_WAVEFORM_DAC,
	TRIANGULAR_WAVEFORM_DAC

}RAMP_TYPE;

void CK_DAC_Init(RAMP_TYPE ramp);

void CK_DAC_TIMER_Init(void);

void CK_DAC_LoadValue(uint16_t value);

void CK_DAC_Sawtooth(void);

void CK_DAC_Triangular(void);


#endif
