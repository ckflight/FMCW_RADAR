
#ifndef CK_GPIO_H_
#define CK_GPIO_H_

#include "stm32f4xx.h"

typedef enum
{
  CK_GPIO_INPUT                 = 0u,
  CK_GPIO_OUTPUT                = 1u,
  CK_GPIO_AF               	    = 2u,
  CK_GPIO_ANALOG                = 3u

}CK_GPIOx_Mode;

typedef enum
{
  CK_GPIO_PUSHPULL              = 0u,
  CK_GPIO_OPENDRAIN             = 1u

}CK_GPIOx_Type;

typedef enum
{
  CK_GPIO_LOW              		= 0u,
  CK_GPIO_MEDIUM              	= 1u,
  CK_GPIO_HIGH					= 2u,
  CK_GPIO_VERYHIGH              = 3u

}CK_GPIOx_Speed;

typedef enum
{
	CK_GPIO_NOPUPD              = 0u,
	CK_GPIO_PULLUP              = 1u,
	CK_GPIO_PULLDOWN            = 2u

}CK_GPIOx_PUPD;


typedef enum
{
	CK_GPIO_AF0               = 0u,
	CK_GPIO_AF1               = 1u,
	CK_GPIO_AF2               = 2u,
	CK_GPIO_AF3               = 3u,
	CK_GPIO_AF4               = 4u,
	CK_GPIO_AF5               = 5u,
	CK_GPIO_AF6               = 6u,
	CK_GPIO_AF7               = 7u,
	CK_GPIO_AF8               = 8u,
	CK_GPIO_AF9               = 9u,
	CK_GPIO_AF10              = 0xAu,
	CK_GPIO_AF11              = 0xBu,
	CK_GPIO_AF12              = 0xCu,
	CK_GPIO_AF13              = 0xDu,
	CK_GPIO_AF14              = 0xEu,
	CK_GPIO_AF15              = 0xFu,
	CK_GPIO_NOAF              = 0x10u

}CK_GPIOx_AFx;

void CK_GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin, CK_GPIOx_Mode GPIOx_Mode, CK_GPIOx_AFx GPIOx_AF,
				  CK_GPIOx_Type GPIOx_Type, CK_GPIOx_Speed GPIOx_Speed, CK_GPIOx_PUPD GPIOx_PUPD);

void CK_GPIO_ClockEnable(GPIO_TypeDef* GPIOx);

void CK_GPIO_SetPin(GPIO_TypeDef* GPIOx,uint16_t GPIOx_Pin);

void CK_GPIO_ClearPin(GPIO_TypeDef* GPIOx,uint16_t GPIOx_Pin);

uint8_t CK_GPIO_ReadPin(GPIO_TypeDef* GPIOx ,uint16_t GPIOx_Pin);

#endif /* CK_GPIO_H_ */
