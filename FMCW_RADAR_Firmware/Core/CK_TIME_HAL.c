
#include "CK_SYSTEM.h"
#include "CK_TIME_HAL.h"

#define USE_HAL		1

#define USE_DWT		0

/*
 *
 * VERY VERY IMPORTANT:
 *
 * USE DWT only for debugging when it is activated
 * because after debugger disconnected code does not start.
 * When debugging i mean to see how much microsecond a method is taking precisely enable USE DWT
 * When code will be used on system comment it and use systick.
 */

#define CK_SYSTICK_CTRL_ENABLE				1u << 0
#define CK_SYSTICK_CTRL_INT					1u << 1
#define CK_SYSTICK_CTRL_CLKSOURCE			1u << 2
#define CK_SYSTICK_LOAD_MASK			    0xFFFFFFu

int isFirst = 1;

uint32_t sysTickCounter = 0;

uint32_t timeout_num;

void CK_TIME_SetTimeOut(uint32_t time){

	timeout_num = time;

}

uint32_t CK_TIME_GetTimeOut(void){

	return timeout_num;

}

// Overwrite of weak HAL decleration
void HAL_IncTick(void){

	sysTickCounter++;

	if(timeout_num > 0){

		timeout_num--;

	}

}

// Overwrite of weak HAL decleration
uint32_t HAL_GetTick(void){

  return sysTickCounter ;

}

uint32_t CK_TIME_GetMicroSec_SYSTICK(void){

#if !USE_HAL

	//HAL Initialises This Part

	if(isFirst == 1){

		isFirst = 0;

		SysTick->LOAD = ((uint32_t)((F_CPU/1000)-1)); // 1mS

		SysTick->VAL = 0;

		SysTick->CTRL |= CK_SYSTICK_CTRL_ENABLE | CK_SYSTICK_CTRL_INT | CK_SYSTICK_CTRL_CLKSOURCE;

	}

#endif

	uint32_t ticks ;
	uint32_t count ;

	SysTick->CTRL;

	do{
		ticks = SysTick->VAL;

		count = sysTickCounter;
	}
	while (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk);

	// Sometimes when this method called consequtively
	// it returns smaller compared to the previous call
	// and negative number is results too big result like 4988339 so ignore those values.

	return count * 1000 + (SysTick->LOAD + 1 - ticks) / (F_CPU / 1000000);


}

uint32_t CK_TIME_GetMilliSec_SYSTICK(void){

#if !USE_HAL

	//HAL Initialises This Part

	if(isFirst == 1){

		isFirst = 0;

		SysTick->LOAD = ((uint32_t)((F_CPU/1000)-1)); // 1mS

		SysTick->VAL = 0;

		SysTick->CTRL |= CK_SYSTICK_CTRL_ENABLE | CK_SYSTICK_CTRL_INT | CK_SYSTICK_CTRL_CLKSOURCE;

	}

#endif

	return sysTickCounter;
}

// DWT measures more precisely
uint32_t CK_TIME_GetMicroSec_DWT(void){

	// DWT counts the number of cpu clock passed so it is precise.
	// Systick sometimes returns smaller number between 2 consequtive calls
	// I added some logic to prevent returning smaller number than the previous
	// read but DWT can now be used.

	static int firstconfig = 0;
	static uint8_t clock_ = 0;

	if(firstconfig == 0){
		firstconfig = 1;
		clock_ = (F_CPU / 1000000);

		DWT->CTRL 	|= 1 ; 	// enable the counter
		DWT->CYCCNT  = 0; 	// reset the counter
	}

	// DWT counter rounds up after around 15 seconds.
	// so only one value at each 15th second could be wrong.
	uint32_t counter = DWT->CYCCNT / clock_;
	return counter;
}

uint32_t CK_TIME_GetMilliSec_DWT(void){

	static int firstconfig = 0;
	static uint8_t clock_ = 0;

	if(firstconfig == 0){
		firstconfig = 1;
		clock_ = (F_CPU / 1000000);

		DWT->CTRL 	|= 1 ; 	// enable the counter
		DWT->CYCCNT  = 0; 	// reset the counter
	}

	// DWT counter rounds up after around 15 seconds.
	// so only one value at each 15th second could be wrong.
	uint32_t counter = DWT->CYCCNT / clock_;
	return counter / 1000;
}

uint32_t CK_TIME_GetMicroSec(void){

#if USE_DWT && !USE_HAL
	return CK_TIME_GetMicroSec_DWT();
#else
	return CK_TIME_GetMicroSec_SYSTICK();
#endif

}

uint32_t CK_TIME_GetMilliSec(void){

#if USE_DWT && !USE_HAL
	return CK_TIME_GetMilliSec_DWT();
#else
	return CK_TIME_GetMilliSec_SYSTICK();
#endif

}

void CK_TIME_DelayMilliSec(uint32_t msec){

	while(msec--)CK_TIME_DelayMicroSec(1000);

}

void CK_TIME_DelayMicroSec(uint32_t usec){

	uint32_t now = CK_TIME_GetMicroSec();

	while (CK_TIME_GetMicroSec() - now < usec);

}



