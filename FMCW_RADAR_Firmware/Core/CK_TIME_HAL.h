
#ifndef CK_TIME_HAL_H_
#define CK_TIME_HAL_H_


void CK_TIME_SetTimeOut(uint32_t time);

uint32_t CK_TIME_GetTimeOut(void);

void HAL_IncTick(void);

uint32_t HAL_GetTick(void);

uint32_t CK_TIME_GetMicroSec_SYSTICK(void);

uint32_t CK_TIME_GetMilliSec_SYSTICK(void);

uint32_t CK_TIME_GetMicroSec_DWT(void);

uint32_t CK_TIME_GetMilliSec_DWT(void);

uint32_t CK_TIME_GetMicroSec(void);

uint32_t CK_TIME_GetMilliSec(void);

void CK_TIME_DelayMilliSec(uint32_t msec);

void CK_TIME_DelayMicroSec(uint32_t usec);

#endif /* CK_TIME_HAL_H_ */
