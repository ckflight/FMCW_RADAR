
#include "CK_GPIO.h"
#include "CK_SPI.h"

#define CK_SPIx_CR1_MSTR               1u<<2
#define CK_SPIx_CR1_CPOL               1u<<1
#define CK_SPIx_CR1_CPHA               1u<<0
#define CK_SPIx_CR1_SPE                1u<<6
#define CK_SPIx_CR1_SSI                1u<<8
#define CK_SPIx_CR1_SSM                1u<<9

#define CK_SPIx_CR2_TXDMAEN            1u<<1
#define CK_SPIx_CR2_RXDMAEN            1u<<0

#define CK_SPIx_SR_TXE                 1u<<1
#define CK_SPIx_SR_RXNE                1u<<0
#define CK_SPIx_SR_BSY                 1u<<7

#define CK_RCC_SPI1_EN                 1u<<12
#define CK_RCC_SPI2_EN                 1u<<14
#define CK_RCC_SPI3_EN                 1u<<15
#define CK_RCC_SPI5_EN                 1u<<20


#define SPI_TIMEOUT                    250 // 250 makes around 25 usec which is enough.

typedef struct{

    uint32_t timeout;

    uint32_t spi1_timeout;
    int spi1_init;

    uint32_t spi2_timeout;
    int spi2_init;

    uint32_t spi3_timeout;
    int spi3_init;

    uint32_t spi5_timeout;
    int spi5_init;

}SPI_t;

SPI_t spi_variables = {

    .timeout      = 0,

    .spi1_timeout = 0,
    .spi1_init    = 0,

    .spi2_timeout = 0,
    .spi2_init    = 0,

    .spi3_timeout = 0,
    .spi3_init    = 0,

	.spi5_timeout = 0,
	.spi5_init    = 0,

};

void CK_SPI_Init(SPI_TypeDef* spi_n){

	SPI_TypeDef* SPIx = spi_n;
	GPIO_TypeDef* GPIOx;
	CK_GPIOx_AFx AFx;
	uint16_t miso_pin, mosi_pin, sck_pin;

	/*
	 *	SPI1,4,5 have APB2 = 90MHz
	 *	SPI2 have APB1 = 45MHz
	 *	so prescaler must be selected to meet 10MHZ max spi clock speed
	 *
	 */

	if(SPIx == SPI1){

		GPIOx = GPIOA;

		sck_pin     = 5;
		miso_pin    = 6;
		mosi_pin    = 7;

		AFx = CK_GPIO_AF5;
		RCC->APB2ENR |= CK_RCC_SPI1_EN; // Enable related SPI clock

		spi_variables.spi1_init    = 1;
		spi_variables.spi1_timeout = 0;
	}
	else if(SPIx == SPI2){

		GPIOx = GPIOB;

		sck_pin     = 13;
		miso_pin    = 14;
		mosi_pin    = 15;

		AFx = CK_GPIO_AF5;
		RCC->APB1ENR |= CK_RCC_SPI2_EN; // Enable related SPI clock

		spi_variables.spi2_init    = 1;
		spi_variables.spi2_timeout = 0;
	}
	else if(SPIx == SPI3){

		GPIOx = GPIOC;

		sck_pin     = 10;
		miso_pin    = 11;
		mosi_pin    = 12;

		AFx = CK_GPIO_AF6;
		RCC->APB1ENR |= CK_RCC_SPI3_EN; // Enable related SPI clock

		spi_variables.spi3_init    = 1;
		spi_variables.spi3_timeout = 0;
	}

	CK_GPIO_ClockEnable(GPIOx);
	CK_GPIO_Init(GPIOx, sck_pin,  CK_GPIO_AF, AFx, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(GPIOx, miso_pin, CK_GPIO_AF, AFx, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);
	CK_GPIO_Init(GPIOx, mosi_pin, CK_GPIO_AF, AFx, CK_GPIO_PUSHPULL, CK_GPIO_VERYHIGH, CK_GPIO_NOPUPD);

	/*
	 * Default: full duplex, (cpha=0,cpol=0),8bit data,No CRC,MSB First
	 */
	if(SPIx == SPI2){
		SPIx->CR1 |= CK_SPIx_CR1_MSTR | CK_SPIx_CR1_SSM | CK_SPIx_CR1_SSI | CK_SPIx_CR1_Fclk_Div128;
		SPIx->CR1 |= CK_SPIx_CR1_SPE; // SPI Enable

	}
	else if(SPIx == SPI3){
		SPIx->CR1 |= CK_SPIx_CR1_MSTR | CK_SPIx_CR1_SSM | CK_SPIx_CR1_SSI | CK_SPIx_CR1_Fclk_Div2;
		SPIx->CR1 |= CK_SPIx_CR1_SPE; // SPI Enable
	}
	else{
		SPIx->CR1 |= CK_SPIx_CR1_MSTR | CK_SPIx_CR1_SSM | CK_SPIx_CR1_SSI | CK_SPIx_CR1_Fclk_Div4;
		SPIx->CR1 |= CK_SPIx_CR1_SPE; // SPI Enable
	}



}

void CK_SPI_Enable(SPI_TypeDef* spi_n){

	spi_n->CR1 |= CK_SPIx_CR1_SPE;//SPI Enable

}
void CK_SPI_Disable(SPI_TypeDef* spi_n){

	spi_n->CR1 &= ~CK_SPIx_CR1_SPE;//SPI Disable

}

void CK_SPI_EnableDMA(SPI_TypeDef* spi_n){

	spi_n ->CR2 |= CK_SPIx_CR2_TXDMAEN;// | CK_SPIx_CR2_RXDMAEN;

}

void CK_SPI_DisableDMA(SPI_TypeDef* spi_n){

	spi_n ->CR2 &= ~(CK_SPIx_CR2_TXDMAEN);// | CK_SPIx_CR2_RXDMAEN);

}

void CK_SPI_ChangeClock(SPI_TypeDef* spi_n, CK_SPIx_CR1_Fclk_Div clk){

	spi_n->CR1 &= ~(7u<<3);
	spi_n->CR1 |= clk;

}

uint8_t CK_SPI_WriteRegister(uint8_t reg, uint8_t data, SPI_TypeDef* SPIn, GPIO_TypeDef* GPIOx_CS, uint16_t cs_pin){
	uint8_t val = 0;
	CK_GPIO_ClearPin(GPIOx_CS, cs_pin);

	CK_SPI_Transfer(SPIn, reg);
	val = CK_SPI_Transfer(SPIn, data);

	CK_GPIO_SetPin(GPIOx_CS, cs_pin);
	return val;
}

void CK_SPI_ReadRegisterMulti(uint8_t reg, SPI_TypeDef* SPIn, GPIO_TypeDef* GPIOx_CS, uint16_t cs_pin, uint8_t* dataIn, int count){

    CK_GPIO_ClearPin(GPIOx_CS, cs_pin);

    CK_SPI_Transfer(SPIn, reg);

    while (count--) {

        *dataIn++ = CK_SPI_Transfer(SPIn, 0);
    }

    CK_GPIO_SetPin(GPIOx_CS, cs_pin);

}

uint8_t CK_SPI_Transfer(SPI_TypeDef* SPIn, uint8_t data){

    spi_variables.timeout = SPI_TIMEOUT;
	while(((SPIn)->SR & (CK_SPIx_SR_TXE | CK_SPIx_SR_RXNE)) == 0 || ((SPIn)->SR & CK_SPIx_SR_BSY)){

		if(--spi_variables.timeout == 0x00){
		    CK_SPI_TimeOutCounter(SPIn);
		    return 1;
		}
	}

	SPIn->DR = data;

	spi_variables.timeout = SPI_TIMEOUT;
	while(((SPIn)->SR & (CK_SPIx_SR_TXE | CK_SPIx_SR_RXNE)) == 0 || ((SPIn)->SR & CK_SPIx_SR_BSY)){

		if(--spi_variables.timeout == 0x00){
		    CK_SPI_TimeOutCounter(SPIn);
		    return 1;
		}
	}
	return SPIn->DR;
}

uint8_t CK_SPI_WaitTransfer(SPI_TypeDef* SPIn){

    spi_variables.timeout = SPI_TIMEOUT;
    while(((SPIn)->SR & (CK_SPIx_SR_TXE | CK_SPIx_SR_RXNE)) == 0 || ((SPIn)->SR & CK_SPIx_SR_BSY)){
        if(--spi_variables.timeout == 0x00){
            CK_SPI_TimeOutCounter(SPIn);
            return 1;
        }
    }

    return 0;
}

int CK_SPI_CheckInitialized(SPI_TypeDef* SPIn){

    int res;
    if(SPIn == SPI1){
        res = spi_variables.spi1_init;
    }
    else if(SPIn == SPI2){
        res = spi_variables.spi2_init;
    }
    else if(SPIn == SPI3){
        res = spi_variables.spi3_init;
    }
    else{
        res = 2; // Error
    }

	return res;

}

void CK_SPI_TimeOutCounter(SPI_TypeDef* spi){
    if(spi == SPI1){
        spi_variables.spi1_timeout++;
    }
    else if(spi == SPI2){
        spi_variables.spi2_timeout++;
    }
    else if(spi == SPI3){
        spi_variables.spi3_timeout++;
    }
}

uint32_t CK_SPI_GetTimeOut(SPI_TypeDef* spi){

    uint32_t res;

    if(spi == SPI1){
        res = spi_variables.spi1_timeout;
    }
    else if(spi == SPI2){
        res = spi_variables.spi2_timeout;
    }
    else if(spi == SPI3){
        res = spi_variables.spi3_timeout;
    }

    return res;
}

void CK_SPI_ResetTimeOut(SPI_TypeDef* spi){

    if(spi == SPI1){
        spi_variables.spi1_timeout = 0;
    }
    else if(spi == SPI2){
        spi_variables.spi2_timeout = 0;
    }
    else if(spi == SPI3){
        spi_variables.spi3_timeout = 0;
    }

}
