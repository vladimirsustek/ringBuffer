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

static uint8_t       intBuffer[BUFF_SIZE];
static ringBuffPtr_t messagePointers[MESSAGE_BUFFER_SIZE] = {0};
static uint32_t      messageIndex = 0;
static uint32_t      readPtr = 0;
static uint32_t      bufferWrapped = 0;

#define INDEX_ALREADY_WRAPPED        ((readPtr + 1) % BUFF_SIZE == 0)
#define WRAPPED_BYTE_NOT_RECEIVED    (RX_CNT == 0)
#define RECEIVED_TOO_LESS            (RX_CNT - readPtr < 2)

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
	static uint8_t* auxBegin = intBuffer;
	uint8_t* auxEnd = NULL;

	while(readPtr != RX_CNT)
	{
		if((INDEX_ALREADY_WRAPPED && WRAPPED_BYTE_NOT_RECEIVED) || RECEIVED_TOO_LESS)
		{
			return 0;
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
		acc++;

		if((CR == intBuffer[readPtr]) && (LF == intBuffer[(readPtr + 1) % BUFF_SIZE]))
		{

			//messagePointers[messageIndex].pEnd = intBuffer + readPtr;
			auxEnd = intBuffer + readPtr;

			//uint32_t beginNext = (readPtr + 2) % BUFF_SIZE;
			//messagePointers[(messageIndex + 1) % MESSAGE_BUFFER_SIZE].pBegin = (uint8_t*)(intBuffer + beginNext);

			readPtr = (readPtr + 2) % BUFF_SIZE;

			messagePointers[messageIndex].pBegin = auxBegin;
			messagePointers[messageIndex].pEnd = auxEnd;

			auxBegin = (uint8_t*)(intBuffer + readPtr);

			messageIndex = (messageIndex + 1) % MESSAGE_BUFFER_SIZE;

			printf("OK\r\n");

			break;
		}

		readPtr = (readPtr + 1) % BUFF_SIZE;
	}

	return acc;
}

uint32_t buff_RXcheck(char* keyWord, uint32_t lng)
{
	uint32_t match = 0;

	for(uint32_t auxIdx = 0; auxIdx  < messageIndex; auxIdx++)
	{
		uint8_t* pBegin = messagePointers[auxIdx].pBegin;
		uint8_t* pEnd = messagePointers[auxIdx].pEnd;

		if(pEnd > pBegin)
		{
			if(lng <= pEnd - pBegin)
			{
				if(0 == memcmp(keyWord, pBegin, lng))
				{
					match = 1;
					break;
				}
			}
		}
		else
		{
			uint32_t firstPartLng = BUFF_SIZE - ((uint32_t)pBegin - (uint32_t)intBuffer);
			if(lng <= firstPartLng + (pEnd - intBuffer))
			{
				if(0 == memcmp(keyWord, pBegin, firstPartLng) &&
				   0 == memcmp(keyWord + firstPartLng, intBuffer, lng-firstPartLng))
				{
					match = 1;
					break;
				}
			}

		}
	}

	return match;
}

uint32_t buff_RXstart(void)
{
	HAL_UART_Receive_DMA(&huart3, intBuffer, BUFF_SIZE);
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
		bufferWrapped = 1;
	}
}
