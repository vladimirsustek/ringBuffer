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
#define HARD_ERROR	  (uint32_t)(-1)
#define BUFF_OK	0

#define CR '\r'
#define LF '\n'

typedef struct ringBuffPtr
{
	uint8_t* pBegin;
	uint8_t* pEnd;
} ringBuffPtr_t;

uint32_t buff_RXfetch(void);
uint32_t buff_RXcompare(char* keyWord, uint32_t lng);
uint32_t buff_RXextractUI32(char* keyWord, uint32_t keyLng, uint32_t *num);
uint32_t buff_RXextractString(char* keyWord, uint32_t keyLng, uint8_t *buff, uint32_t buffLng);
uint32_t buff_RXcopyString(uint8_t* buff, uint32_t buffLng);
void buff_RXflush(void);
uint32_t buff_RXstart(void);

#endif /* INC_RINGBUFFER_H_ */
