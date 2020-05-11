
#ifndef INC_CK_ADF4158_H_
#define INC_CK_ADF4158_H_

#include "stm32f4xx.h"

typedef enum{
	SAWTOOTH_WAVEFORM,
	TRIANGULAR_WAVEFORM

}WAVEFORM_TYPE;


void CK_ADF4158_Init(WAVEFORM_TYPE wf);

void CK_ADF4158_Configure_Sweep(WAVEFORM_TYPE wf, double startFreq, double bw, double rampTime, int rampDel);

void CK_ADF4158_WriteRegister(uint32_t data);

void CK_ADF4158_DeviceEnable(void);

uint32_t CK_ADF4158_GetPulseReceived(void);

int CK_ADF4158_RampStarted(void);

int CK_ADF4158_RampCompleted(void);

#endif /* INC_CK_ADF4158_H_ */
