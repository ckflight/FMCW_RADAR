
#ifndef CK_SYSTEM_H_
#define CK_SYSTEM_H_

#include "stm32f4xx.h"

uint32_t F_CPU; 											    // CK_TIME_HAL uses

typedef enum{

	SYSTEM_CLK_180MHz,

	SYSTEM_CLK_168MHz

}systemClock_e;

void CK_SYSTEM_SetSystemClock(systemClock_e clk);

uint32_t CK_SYSTEM_GetSystemClock(void);

#endif
