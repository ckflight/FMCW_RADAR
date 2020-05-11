
#ifndef INC_CK_MAX1426_H_
#define INC_CK_MAX1426_H_

#include "stm32f4xx.h"

void CK_MAX1426_Init(void);


void CK_MAX1426_EXTI_Init(void);

void TIMER1_Init(void);

void CK_MAX1426_PWM_Init(void);

void CK_MAX1426_PWM_Start(void);

void CK_MAX1426_PWM_Stop(void);

void CK_MAX1426_ADC_OutputDisable(void);

void CK_MAX1426_ADC_OutputEnable(void);

uint16_t CK_MAX1426_Get_ADCResult(void);

uint16_t CK_MAX1426_Get_Counter(void);

void CK_MAX1426_Reset_Counter(void);

uint8_t CK_MAX1426_IsRecordDone(void);

void CK_MAX1426_ResetRecordDone(void);

void CK_MAX1426_TransferSamples(void);

#endif /* INC_CK_MAX1426_H_ */
