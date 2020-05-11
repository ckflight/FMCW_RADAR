
#include "stdint.h"		    // uint8_t type
#include "stdlib.h"		    // malloc
#include "string.h"			// memcpy

#include "USBD_CDC/CK_USBD_BUFFER.h"

void CK_USBD_BUFFER_Init(circularBuffer_t* cb, int size){

	cb->buffer = malloc(sizeof(uint8_t) * size);
	cb->head = 0;
	cb->tail = 0;
	cb->size = size;

}

int CK_USBD_BUFFER_BufferWrite(circularBuffer_t* c, uint8_t data){

	// next is where head will point to after this write.
	int next = c->head + 1;
	if (next >= c->size)
		next = 0;				//revolve to start location of buffer

	if (next == c->tail) 		// check if circular buffer is full
		return -1;      		// and return with an error.

	c->buffer[c->head] = data; 	// Load data and then move
	c->head = next;            	// head to next data offset.

	return 0;  					// return success to indicate successful push.
}

int CK_USBD_BUFFER_BufferWriteMulti(circularBuffer_t* c, uint8_t* buff, uint32_t length){

	// When using this method the checking should be done outside
	// by using CK_CIRCULARBUFFER_GetAvailable.

	// The buff content is copied over circular buffer array.
	memcpy(c->buffer, buff, length);

	// Move head numberOfElement times since that much data is written at once.
	c->head += length;

	return 0;  					// return success to indicate successful push.

}

int CK_USBD_BUFFER_BufferRead(circularBuffer_t* c, uint8_t* data){

    // if the head isn't ahead of the tail, we don't have any characters
    if (c->head == c->tail) 	// check if circular buffer is empty
        return -1;         		// and return with an error

    // next is where tail will point to after this read.
    int next = c->tail + 1;
    if(next >= c->size)
        next = 0;				//revolve to start location of buffer

    *data = c->buffer[c->tail]; // Read data and then move
    c->tail = next;             // tail to next data offset.

    return 0;  					// return success to indicate successful push.

}

/*
 * Get all elements in the buffer.
 */
void CK_USBD_BUFFER_GetBuffer(circularBuffer_t* c, uint8_t* buff, uint32_t* numOfElement){

	uint32_t len = c->head - c->tail;
	*numOfElement = len;

	// The circular buffer content is copied over buf array.
	memcpy(buff, c->buffer, len);

	// Now since all the data is taken reset.
	c->head = 0;

	c->tail = 0;

}

/*
 * Return number of element available to write
 */
int CK_USBD_BUFFER_GetAvailable(circularBuffer_t* c){

	return c->size - (c->head - c->tail);

}

bool CK_USBD_BUFFER_IsBufferEmpty(circularBuffer_t* c){
	return c->head == c->tail;
}

bool CK_USBD_BUFFER_IsBufferFull(circularBuffer_t* c){
	return ((c->head + 1) % c->size) == c->tail;
}
