/*
 * ringBuffer.c
 *
 *  Created on: Jan 2, 2023
 *      Author: Vladimir Sustek, MSc.
 */

#include "ringBuffer.h"

extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

#define RX_CNT	(BUFF_SIZE - hdma_usart3_rx.Instance->NDTR)

static uint8_t intBuffer[BUFF_SIZE];
static ringBuffPtr_t messagePointers[MESSAGE_BUFFER_SIZE] = {0};
static uint32_t messageIndex = 0;
static uint32_t readPtr = 0;
static uint32_t wrapFlag = 0;
static uint8_t oldByte = 0;

typedef struct terminationWord
{
	uint8_t cr;
	uint8_t lf;
	uint32_t readPtr;
	uint32_t readPtr_1;
} terminationWord_t;

terminationWord_t debugStr[128] = {0};
uint32_t debugStrIdx = 0;

uint32_t buff_RXfetch(void)
{
	uint32_t acc = 0;

	while(readPtr != RX_CNT)
	{
		/* When readPtr+1 indexes wrapped byte,
		 * but wrapped 1th (index 0) byte is not received yet*/
		if((readPtr + 1) % BUFF_SIZE == 0 && RX_CNT == 0)
		{
			/* Return because the last byte must be received*/
			return 0;
		}
		/* Receive must be minimally 2-byte long (to detect CR-LF)*/
		if(RX_CNT - readPtr < 2)
		{
			/* When wrapped byte is received, zero the wrap flag*/
			if(RX_CNT == 1 && wrapFlag == 1)
			{
				wrapFlag = 0;
			}
			else
			{
				/* Return always when not enough new bytes received*/
				return 0;
			}
		}

		/* Safety when loop "gets stuck" - impossible to receive more
		 * than BUFF_SIZE at once but calculation may get wrong */
		if(acc > BUFF_SIZE)
		{
			break;
		}

		//__ISB();

		//debugStr[debugStrIdx].cr = intBuffer[readPtr];

		//debugStr[debugStrIdx].lf = intBuffer[(readPtr + 1) % BUFF_SIZE];

		//debugStr[debugStrIdx].readPtr = readPtr;

		//debugStr[debugStrIdx].readPtr_1 = (readPtr + 1) % BUFF_SIZE;

		//debugStrIdx++;

		if((CR == intBuffer[readPtr]) && (LF == intBuffer[(readPtr + 1) % BUFF_SIZE]))
		{
			messagePointers[messageIndex].pEnd = intBuffer + readPtr;
			uint32_t beginNext = (readPtr + 2) % BUFF_SIZE;
			messagePointers[(messageIndex + 1) % MESSAGE_BUFFER_SIZE].pBegin =
			(uint8_t*)(intBuffer + beginNext);
			readPtr = (readPtr + 2) % BUFF_SIZE;
			messageIndex = (messageIndex + 1) % MESSAGE_BUFFER_SIZE;
			printf("OK\r\n");
			break;
		}

		readPtr = (readPtr + 1) % BUFF_SIZE;
		acc++;
	}

	return acc;
}

uint32_t buff_RXcheck(uint32_t moveOn)
{
	return 0;
}

uint32_t buff_RXstart(void)
{
	HAL_UART_Receive_DMA(&huart3, intBuffer, BUFF_SIZE);
	messagePointers[messageIndex].pBegin = intBuffer;
	return 0;
}


int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart3, (uint8_t*)ptr, len, HAL_MAX_DELAY);
	return len;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{

	}
}

