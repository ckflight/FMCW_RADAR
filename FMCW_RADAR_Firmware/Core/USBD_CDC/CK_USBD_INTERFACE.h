
#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "USBD_CDC/usbd_def.h"

void CK_USBD_Init(void);

void CK_USBD_Start(void);

void CK_USBD_Stop(void);

int CK_USBD_Transmit(void);

int CK_USBD_ReadData(uint8_t* data);

int CK_USBD_WriteRxCircularBuffer(uint8_t* Buf, uint32_t* Len);

int CK_USBD_WriteTxCircularBuffer(uint8_t data);

void CK_USBD_ClearBufferIndex(void);

void CK_USBD_IntPrint(int32_t num);

void CK_USBD_IntPrintln(int32_t num);

void CK_USBD_FloatPrintln(float num);

void CK_USBD_FloatPrint(float num);

void CK_USBD_StringPrintln(const char str[]);

void CK_USBD_StringPrint(const char str[]);

#endif
