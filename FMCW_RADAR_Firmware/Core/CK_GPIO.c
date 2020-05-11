
#include "CK_GPIO.h"

void CK_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin, CK_GPIOx_Mode GPIOx_Mode, CK_GPIOx_AFx GPIOx_AF,
				  CK_GPIOx_Type GPIOx_Type, CK_GPIOx_Speed GPIOx_Speed, CK_GPIOx_PUPD GPIOx_PUPD)
{
	if(GPIOx_Mode == CK_GPIO_OUTPUT || GPIOx_Mode == CK_GPIO_AF){

		if(GPIOx_AF != CK_GPIO_NOAF){
			if(GPIOx_Pin < 8){
				GPIOx->AFR[0] |= GPIOx_AF<<(GPIOx_Pin*4);
			}
			else{
				GPIOx->AFR[1] |= GPIOx_AF<<((GPIOx_Pin-8)*4);
			}
		}
	}

	GPIOx->MODER |= GPIOx_Mode << (GPIOx_Pin*2);
	GPIOx->OSPEEDR |= GPIOx_Speed<<(GPIOx_Pin*2);

	if(GPIOx_Type == CK_GPIO_PUSHPULL){
		GPIOx->OTYPER &= ~(1u<<GPIOx_Pin);
	}
	else if(GPIOx_Type == CK_GPIO_OPENDRAIN){
		GPIOx->OTYPER |= 1u<<GPIOx_Pin;
	}

	if(GPIOx_PUPD == CK_GPIO_NOPUPD){
		GPIOx->PUPDR &= ~(3u<<(GPIOx_Pin*2));
	}
	else{
		GPIOx->PUPDR |= GPIOx_PUPD<<(GPIOx_Pin*2);
	}

}

void CK_GPIO_ClockEnable(GPIO_TypeDef* GPIOx){
	uint16_t port_clk = ((uint32_t)GPIOx - (GPIOA_BASE)) / ((GPIOB_BASE) - (GPIOA_BASE));
	RCC->AHB1ENR |= 1u<<port_clk;
}

void CK_GPIO_SetPin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin){

	GPIOx->BSRR |= 1u<<GPIOx_Pin;

}

void CK_GPIO_ClearPin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin){

	GPIOx->BSRR |= 1u<<(GPIOx_Pin+16);

}

uint8_t CK_GPIO_ReadPin(GPIO_TypeDef* GPIOx ,uint16_t GPIOx_Pin){

	uint16_t temp = (GPIOx->IDR & (uint16_t)(1u<<GPIOx_Pin)) >> GPIOx_Pin;

	if(temp == 0x01){
		return 0x01;
	}
	return 0x00;

}

