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
#define RECEIVED_TOO_LESS_BYTES      (RX_CNT - readPtr < 2)

typedef struct terminationWord
{
	uint8_t cr;
	uint8_t lf;
	uint32_t readPtr;
	uint32_t readPtr_1;
} terminationWord_t;

static void buff_RemoveElement(uint32_t idx);

terminationWord_t debugStr[128] = {0};
uint32_t debugStrIdx = 0;

uint32_t buff_RXfetch(void)
{
	uint32_t recvBytes = 0;
	static uint8_t* auxBegin = intBuffer;
	uint8_t* auxEnd = NULL;

	while(readPtr != RX_CNT && messageIndex < MESSAGE_BUFFER_SIZE)
	{
		if((INDEX_ALREADY_WRAPPED && WRAPPED_BYTE_NOT_RECEIVED) || RECEIVED_TOO_LESS_BYTES)
		{
			return 0;
		}
		if(recvBytes > BUFF_SIZE)
		{
			break;
		}
		recvBytes++;

		if((CR == intBuffer[readPtr]) && (LF == intBuffer[(readPtr + 1) % BUFF_SIZE]))
		{

			auxEnd = intBuffer + readPtr;
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

	return recvBytes;
}

uint32_t buff_RXcompare(char* keyWord, uint32_t lng)
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
					buff_RemoveElement(auxIdx);
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
					buff_RemoveElement(auxIdx);
					break;
				}
			}

		}
	}

	return match;
}


uint32_t buff_RXextractUI32(char* keyWord, uint32_t keyLng, uint32_t *num)
{
	uint32_t match = 0;
	uint32_t number = 0;
	uint8_t strNum[11] = {0};
	const uint32_t min_lng = 1;

	for(uint32_t auxIdx = 0; auxIdx  < messageIndex; auxIdx++)
	{
		uint8_t* pBegin = messagePointers[auxIdx].pBegin;
		uint8_t* pEnd = messagePointers[auxIdx].pEnd;

		if(pEnd > pBegin)
		{
			if(keyLng + min_lng <= pEnd - pBegin)
			{
				if(0 == memcmp(keyWord, pBegin, keyLng))
				{

					for(uint32_t didx = 0; didx < (uint32_t)(pEnd - pBegin - keyLng); didx++)
					{
						if((pBegin[keyLng + didx] < '0') || (pBegin[keyLng + didx] > '9') || (didx > 10))
						{
							break;
						}
						else
						{
							match = 1;
							strNum[didx] = pBegin[keyLng + didx];
						}
					}

					buff_RemoveElement(auxIdx);

					break;
				}
			}
		}
		else
		{
			uint32_t firstPartLng = BUFF_SIZE - ((uint32_t)pBegin - (uint32_t)intBuffer);
			if(keyLng  + min_lng <= firstPartLng + (pEnd - intBuffer))
			{
				if(0 == memcmp(keyWord, pBegin, keyLng) ||
				  (0 == memcmp(keyWord, pBegin, firstPartLng) &&
				   0 == memcmp(keyWord + firstPartLng, intBuffer, keyLng-firstPartLng)))
				{
					uint32_t didx = 0;

					uint32_t wrapKeyWordLng = keyLng - firstPartLng;

					/* Number starts in "first part" and continues to the second part*/
					if(keyLng < firstPartLng)
					{
						for(didx = 0; didx < firstPartLng - keyLng; didx++)
						{
							if((pBegin[keyLng + didx] < '0') || (pBegin[keyLng + didx] > '9') || (didx > 10))
							{
								break;
							}
							else
							{
								match = 1;
								strNum[didx] = pBegin[keyLng + didx];
							}
						}

						wrapKeyWordLng = 0;
					}
					for(uint32_t didx2 = 0; didx2 < (uint32_t)(pEnd - intBuffer); didx2++)
					{
						if((intBuffer[wrapKeyWordLng + didx2] < '0') ||
								(intBuffer[wrapKeyWordLng + didx2] > '9') ||
								(didx + didx2 > 10))
						{
							break;
						}
						else
						{
							match = 1;
							strNum[didx2 + didx] = intBuffer[wrapKeyWordLng + didx2];
						}
					}
					buff_RemoveElement(auxIdx);
					break;
				}
			}

		}
	}

	if(strlen((char*)strNum))
	{
		uint32_t maxDec = 1;
		for(uint32_t dec = 1; dec < strlen((char*)strNum); dec++)
		{
			maxDec *= 10;
		}

		for(uint32_t digit = 0; digit < strlen((char*)strNum); digit++)
		{
			number += maxDec * (strNum[digit] - '0');
			maxDec /= 10;
		}

		*num = number;
	}

	return number;
}


static void buff_RemoveElement(uint32_t idx)
{
    uint32_t outdated = 0;
    uint32_t writeIdx = 0;
	uint32_t readIdx = 0;

    for(readIdx = 0; readIdx < messageIndex; readIdx++)
	 {
	     if(readIdx != idx)
	     {
	    	 messagePointers[writeIdx] = messagePointers[readIdx];
	         writeIdx++;
	     }
	     else
	     {
	         outdated++;
	     }
	 }

	 for(readIdx = (messageIndex - 1);
			 readIdx > messageIndex - 1 - outdated;
			 readIdx--)
	 {
		 messagePointers[readIdx].pBegin = NULL;
		 messagePointers[readIdx].pEnd = NULL;
	 }

	 messageIndex -= outdated;
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
