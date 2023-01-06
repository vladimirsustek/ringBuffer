/*
 * ringBuffer.h
 *
 *  Created on: Jan 2, 2023
 *      Author: 42077
 */

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include <stdio.h>
#include <string.h>

#include "usart.h"

#define BUFF_SIZE 16
#define MESSAGE_BUFFER_SIZE (BUFF_SIZE / 3)
#define TERMINATION_LNG 2

#define FETCH_TIMEOUT (uint32_t)(-2)
#define BUFF_OK	0

#define CR '\r'
#define LF '\n'

typedef struct ringBuffPtr
{
	uint8_t* pBegin;
	uint8_t* pEnd;
} ringBuffPtr_t;

#endif /* INC_RINGBUFFER_H_ */
