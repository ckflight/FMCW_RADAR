
#ifndef CK_SPI_H_
#define CK_SPI_H_

#include "stm32f4xx.h"

typedef enum
{
	CK_SPIx_CR1_Fclk_Div2                 = 0u<<3,
	CK_SPIx_CR1_Fclk_Div4                 = 1u<<3,
	CK_SPIx_CR1_Fclk_Div8                 = 2u<<3,
	CK_SPIx_CR1_Fclk_Div16                = 3u<<3,
	CK_SPIx_CR1_Fclk_Div32                = 4u<<3,
	CK_SPIx_CR1_Fclk_Div64                = 5u<<3,
	CK_SPIx_CR1_Fclk_Div128               = 6u<<3,
	CK_SPIx_CR1_Fclk_Div256               = 7u<<3

}CK_SPIx_CR1_Fclk_Div;

void CK_SPI_Init(SPI_TypeDef* spi_n);

void CK_SPI_Enable(SPI_TypeDef* spi_n);

void CK_SPI_Disable(SPI_TypeDef* spi_n);

void CK_SPI_EnableDMA(SPI_TypeDef* spi_n);

void CK_SPI_DisableDMA(SPI_TypeDef* spi_n);

void CK_SPI_ChangeClock(SPI_TypeDef* spi_n, CK_SPIx_CR1_Fclk_Div clk);

uint8_t CK_SPI_WriteRegister(uint8_t reg, uint8_t data, SPI_TypeDef* SPIn, GPIO_TypeDef* GPIOx_CS, uint16_t cs_pin);

void CK_SPI_ReadRegisterMulti(uint8_t reg, SPI_TypeDef* SPIn, GPIO_TypeDef* GPIOx_CS, uint16_t cs_pin, uint8_t* dataIn, int count);

uint8_t CK_SPI_Transfer(SPI_TypeDef* SPIn, uint8_t data);

uint8_t CK_SPI_WaitTransfer(SPI_TypeDef* SPIn);

int CK_SPI_CheckInitialized(SPI_TypeDef* SPIn);

void CK_SPI_TimeOutCounter(SPI_TypeDef* spi);

uint32_t CK_SPI_GetTimeOut(SPI_TypeDef* spi);

void CK_SPI_ResetTimeOut(SPI_TypeDef* spi);

#endif /* CK_SPI_H_ */
