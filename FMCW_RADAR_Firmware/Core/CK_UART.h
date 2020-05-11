
#ifndef CK_UART_H_
#define CK_UART_H_

#include "stm32f4xx.h"
//#include "CK_CIRCULARBUFFER.h"
#include "CK_GPIO.h"
//#include "CK_SBUS.h"

//circularBuffer_t cb;

#define CK_RCC_APB2ENR_USART1            1u<<4    //USART1 CLK ENABLE
#define CK_RCC_APB1ENR_USART3            1u<<18   //USART3 CLK ENABLE
#define CK_RCC_APB1ENR_UART5             1u<<20   //UART5 CLK ENABLE

#define CK_USART_CR1_TE         		 1u<<3    //TX ENABLE
#define CK_USART_CR1_RE         		 1u<<2    //RX ENABLE
#define CK_USART_CR1_RXNEIE				 1u<<5    //Receive Interrupt Enable
#define CK_USART_CR1_UE          		 1u<<13   //USART ENABLE
#define CK_USART_CR1_OVER8      		 1u<<15   //OVER SAMPLING 8
#define CK_USART_CR1_PCE      		     1u<<10   //PARITY ENABLED
#define CK_USART_CR1_PS      		     1u<<9    //PARITY SELECT 0=EVEN,1=ODD

#define CK_USART_CR2_STOP_2Bit      	 2u<<12   //2 Stop Bits

#define CK_USART_SR_TXE          		 1u<<7    //TX BUFFER EMPTY
#define CK_USART_SR_TC          		 1u<<6    //TRANSFER COMPLETE
#define CK_USART_SR_RXNE          		 1u<<5    //RECEIVE COMPLETE

#define CK_BufferSize 32

typedef enum{

	SBUS_INACTIVE			=0,
	SBUS_ACTIVE				=1

}SBUS_MODE;

uint8_t CK_TXBuffer_int16[CK_BufferSize];
uint8_t CK_TXBuffer_int[CK_BufferSize];
uint8_t CK_TXBuffer_float[CK_BufferSize];

void CK_UART_Init1(int baudRate);

void CK_USART1Send(uint8_t chr[],int size);

void CK_IntPrintln(int num);
void CK_IntPrint(int num);

void CK_Int16Println(int16_t num);
void CK_Int16Print(int16_t num);

void CK_FloatPrintln(float num);
void CK_FloatPrint(float num);

void CK_StringPrintln(const char str[]);
void CK_StringPrint(const char str[]);

int CK_getArraySize(const char arry[]);

#endif /* CK_UART_H_ */
