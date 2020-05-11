
#ifndef INC_CK_TQP5523_H_
#define INC_CK_TQP5523_H_

#include "stm32f4xx.h"

void CK_TQP5523_Init(void);

void CK_TQP5523_Update(void);

void CK_TQP5523_CheckOutputPower(void);

void CK_TQP5523_StartConversion(void);

void CK_TQP5523_Disable(void);

void CK_TQP5523_Enable(void);

float CK_TQP5523_ReadDetectorOutputVoltage(void);

int CK_TQP5523_ReadDetectorOutputDBM(void);

#endif /* INC_CK_TQP5523_H_ */
