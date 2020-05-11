
#include "CK_SYSTEM.h"

#define CK_PWR_CR_VOS_SCLAE1        			3u << 14		// SCALE1 SELECTED (FOR 180 MHz )
#define CK_PWR_CR_ODEN						    1u << 16		// Overdrive for 180MHz necessary
#define CK_PWR_CR_ODSWEN                    	1u << 17		// Overdrive switching enable

#define CK_RCC_APB1ENR_PWREN        			1u << 28		// PWREN BIT
#define CK_RCC_CR_HSEON             			1u << 16		// External Osc On
#define CK_RCC_CR_HSERDY						1u << 17		// HSE Ready Bit
#define CK_RCC_CR_HSEBYP                    	1u << 18		// Bypass Mode for disabling PLL
#define CK_RCC_CR_PLLONOFF                  	1u << 24		// PLL ON OFF Bit
#define CK_RCC_CR_PLLRDY                    	1u << 25		// PLL ON OFF Bit

#define CK_RCC_PLLCFGR_PLLSource_HSE			1u << 22		// HSE Selected as PLL Source
#define CK_RCC_PLLCFGR_PLLM_8               	8u << 0		    // PPLM is at default 16 we write 8 instead
#define CK_RCC_PLLCFGR_PLLM_4               	4u << 0		    // PPLM is at default 16 we write 8 instead
#define CK_RCC_PLLCFGR_PLLN_168             	168u << 6	    // PPLN is not zero at default we clear first
#define CK_RCC_PLLCFGR_PLLN_180             	180u << 6	    // PPLN is not zero at default we clear first
#define CK_RCC_PLLCFGR_PLLN_360             	360u << 6	    // PPLN is not zero at default we clear first
#define CK_RCC_PLLCFGR_PLLN_336             	336u << 6	    // PPLN is not zero at default we clear first
#define CK_RCC_PLLCFGR_PLLQ_7               	7u << 24		// Defines USB Clock 360/7 = 48MHz

#define CK_FLASH_ACR_LATENCY_5              	5u << 0 		// Latency is 5
#define CK_RCC_CFGR_SYSCLKSOURCE_PLLCLK     	2u << 0 		// PLL Selected as System Clock
#define CK_RCC_CFGR_PRE1_DIV4               	5u << 10 	    // PRE1 DIV4 PLLN/4 => 180/4 = 45MHz OR 168/4 = 42
#define CK_RCC_CFGR_PRE2_DIV2               	4u << 13 	    // PRE2 DIV2 PLLN/2 => 180/2 = 90MHz OR 168/4 = 84

void CK_SYSTEM_SetSystemClock(systemClock_e clk){

	if(clk == SYSTEM_CLK_180MHz){

		F_CPU = 180000000L;

		RCC->APB1ENR |= CK_RCC_APB1ENR_PWREN;

		RCC->CR |= CK_RCC_CR_HSEON;
		while((RCC->CR & CK_RCC_CR_HSERDY) == 0);//Wait Until HSE Ready

		RCC->CR |= CK_RCC_CR_HSEBYP;//First Disable PLL by going Bypass Mode to set parameters
		while((RCC->CR & CK_RCC_CR_PLLRDY) != 0);/* Wait till PLL is ready */

		RCC->PLLCFGR = 0;
		/*PLLP->DIV2 is selected (00) when we erase register*/
		RCC->PLLCFGR |= CK_RCC_PLLCFGR_PLLM_4 | CK_RCC_PLLCFGR_PLLN_180 | CK_RCC_PLLCFGR_PLLSource_HSE | CK_RCC_PLLCFGR_PLLQ_7;

		RCC->CR |= CK_RCC_CR_PLLONOFF;//Enable PLL
		while((RCC->CR & CK_RCC_CR_PLLRDY) == 0);/* Wait till PLL is ready */

		PWR->CR |= CK_PWR_CR_ODEN | CK_PWR_CR_ODSWEN;

		FLASH->ACR |= CK_FLASH_ACR_LATENCY_5;

		RCC->CFGR |= CK_RCC_CFGR_SYSCLKSOURCE_PLLCLK | CK_RCC_CFGR_PRE1_DIV4 | CK_RCC_CFGR_PRE2_DIV2;
		while(((RCC->CFGR>>2) & 2u) != 2);//Wait SWS Status to show PLL enabled

		// Calculate system clock frequency and update HAL variable
		// Peripherals might use this variable in their configuration.
		SystemCoreClock = CK_SYSTEM_GetSystemClock();

	}
	else if(clk == SYSTEM_CLK_168MHz){

		F_CPU = 168000000L;

		RCC->APB1ENR |= CK_RCC_APB1ENR_PWREN;

		RCC->CR |= CK_RCC_CR_HSEON;
		while((RCC->CR & CK_RCC_CR_HSERDY) == 0);//Wait Until HSE Ready

		RCC->CR |= CK_RCC_CR_HSEBYP;//First Disable PLL by going Bypass Mode to set parameters
		while((RCC->CR & CK_RCC_CR_PLLRDY) != 0);/* Wait till PLL is ready */

		RCC->PLLCFGR = 0;
		/*PLLP->DIV2 is selected (00) when we erase register*/
		RCC->PLLCFGR |= CK_RCC_PLLCFGR_PLLM_4 | CK_RCC_PLLCFGR_PLLN_168 | CK_RCC_PLLCFGR_PLLSource_HSE | CK_RCC_PLLCFGR_PLLQ_7;

		RCC->CR |= CK_RCC_CR_PLLONOFF;//Enable PLL
		while((RCC->CR & CK_RCC_CR_PLLRDY) == 0);/* Wait till PLL is ready */

		PWR->CR |= CK_PWR_CR_ODEN | CK_PWR_CR_ODSWEN;

		FLASH->ACR |= CK_FLASH_ACR_LATENCY_5;

		RCC->CFGR |= CK_RCC_CFGR_SYSCLKSOURCE_PLLCLK | CK_RCC_CFGR_PRE1_DIV4 | CK_RCC_CFGR_PRE2_DIV2;
		while(((RCC->CFGR>>2) & 2u) != 2);//Wait SWS Status to show PLL enabled

		// Calculate system clock frequency and update HAL variable
		// Peripherals might use this variable in their configuration.
		SystemCoreClock = CK_SYSTEM_GetSystemClock();

	}

	// To test system clocks 180 180 45 90 Mhz should be seen for STM32F429
	// To test system clocks 168 168 42 84 Mhz should be seen for STM32F405
	uint32_t frequencies = HAL_RCC_GetHCLKFreq();
	frequencies = HAL_RCC_GetHCLKFreq();
	frequencies = (HAL_RCC_GetHCLKFreq() >> APBPrescTable[(RCC->CFGR & RCC_CFGR_PPRE1)>> RCC_CFGR_PPRE1_Pos]);
	frequencies = HAL_RCC_GetPCLK2Freq();
	UNUSED(frequencies);

}

uint32_t CK_SYSTEM_GetSystemClock(void){

	// External cyrstal connected to MCU
	uint64_t EXTERNAL_CRSYTAL = 8000000UL;

	uint32_t pllm = 0U, pllvco = 0U, pllp = 0U;

	uint32_t sysclockfreq = 0U;

    pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
    pllvco = (uint32_t) ((((uint64_t) EXTERNAL_CRSYTAL * ((uint64_t) ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / (uint64_t)pllm);
    pllp = ((((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos) + 1U) *2U);

    sysclockfreq = pllvco/pllp;

    return sysclockfreq;

}

