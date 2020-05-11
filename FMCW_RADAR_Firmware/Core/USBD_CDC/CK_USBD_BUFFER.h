
#ifndef CK_CIRCULARBUFFER_H_
#define CK_CIRCULARBUFFER_H_

#include "stdbool.h"

typedef struct {

	uint8_t * buffer;
    int head;
    int tail;
    int size;

}circularBuffer_t;

void CK_USBD_BUFFER_Init(circularBuffer_t* cb, int size);

int CK_USBD_BUFFER_BufferWrite(circularBuffer_t* c, uint8_t data);

int CK_USBD_BUFFER_BufferWriteMulti(circularBuffer_t* c, uint8_t* buff, uint32_t length);

int CK_USBD_BUFFER_BufferRead(circularBuffer_t* c, uint8_t* data);

void CK_USBD_BUFFER_GetBuffer(circularBuffer_t* c, uint8_t* buff, uint32_t* numOfElement);

int CK_USBD_BUFFER_GetAvailable(circularBuffer_t* c);

bool CK_USBD_BUFFER_IsBufferEmpty(circularBuffer_t* c);

bool CK_USBD_BUFFER_IsBufferFull(circularBuffer_t* c);

#endif /* CK_CIRCULARBUFFER_H_ */
