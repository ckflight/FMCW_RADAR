
#ifndef INC_CK_CONFIGURATION_H_
#define INC_CK_CONFIGURATION_H_

#include "stm32f4xx.h"

void CK_CONFIGURATION_Init(void);

void CK_CONFIGURATION_InitHardware(uint8_t mode);

uint32_t CK_CONFIGURATION_GetRecordTimeCounter(void);

uint8_t CK_CONFIGURATION_GetRecordTime(void);

uint32_t CK_CONFIGURATION_GetSweepTime(void);

uint32_t CK_CONFIGURATION_GetSweepGap(void);

uint32_t CK_CONFIGURATION_GetSamplingFrequency(void);

uint32_t CK_CONFIGURATION_GetSampleNumber(void);

uint32_t CK_CONFIGURATION_GetSweepStartFrequency(void);

uint32_t CK_CONFIGURATION_GetSweepBandwith(void);

uint8_t CK_CONFIGURATION_GetTXMode(void);

uint8_t CK_CONFIGURATION_GetGainValue(void);

uint8_t CK_CONFIGURATION_SweepType(void);

uint8_t CK_CONFIGURATION_DecodeData(uint8_t rx_data);



#endif /* INC_CK_CONFIGURATION_H_ */
