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

/* Buffer size may be increased/decreased */
#define BUFF_SIZE 16

#define HARD_ERROR	(uint32_t)(-1)
#define RX_OK       (uint32_t)(0)
#define RX_PENDING  (uint32_t)(-2)

#define MESSAGE_BUFFER_SIZE (BUFF_SIZE / 3)

#define TERMINATION_LNG 2
#define CR '\r'
#define LF '\n'

typedef struct ringBuffPtr
{
	uint8_t* pBegin;
	uint8_t* pEnd;
} ringBuffPtr_t;


/**
  * @brief Check buffer content received by UART (DMA or ISR mode).
  * @param None.
  * @detail   Function expects <CR><LF> termination of the string.
  *         Function shall be called periodically in order to allow
  *         further processing functions to operate:
  *         buff_RXcompare()
  *         buff_RXextractUI32()
  *         buff_RXextractString()
  *         buff_RXcopyString()
  *         In principle function goes through the internal UART buffer
  *         and checks received bytes. When <CR><LF> termination is
  *         received, pointer to the string begin and the end (<CR><LF>
  *         termination) is recorded. Next new begin pointer begin starts
  *         after the <CR><LF> of the previous. Maximally, the amount of
  *         MESSAGE_BUFFER_SIZE may be recorded. The processing functions
  *         remove the record when meets parameter requirements or
  *         buff_RXflush() may complete remove all records.
  * @retval RX_OK - when less than 2-bytes received -> pending state
  *         HARD_ERR -> when check loop got stuck
  *         number between 1 up to (BUFF_SIZE - 1) -> bytes received
  */
uint32_t buff_RXfetch(void);


/**
  * @brief Check whether is keyword has been received.
  * @param keyWord - keyWord string (pointer)
  * @param lng - length of the keyWord
  * @detail   Function goes through the recorded strings and compares them
  *         to the keyword. When keyWord matches, record is deleted
  *         and function returns 1. When keyWord has not been received
  *         0 is received (no match).
  * @retval 1 when keyWord is received 0 when not or HARD_ERR.
  */
uint32_t buff_RXcompare(char* keyWord, uint32_t lng);


/**
  * @brief Extract uint32_t number after the string keyWord.
  * @param keyWord - keyWord string (pointer)
  * @param keyLng - length of the keyWord
  * @param num - extracted number
  * @detail   Function goes through the recorded strings and compares them
  *         to the keyword. When keyWord matches, the function checks
  *         whether digits are located behind the keyWord and converts
  *         them into the number of the uint32_t type, the record is deleted
  *         and function returns 1. When keyWord has not been received
  *         0 is received (no match).
  * @retval 1 when keyWord is received 0 when not or HARD_ERR.
  */
uint32_t buff_RXextractUI32(char* keyWord, uint32_t keyLng, uint32_t *num);


/**
  * @brief Extract string after the string keyWord.
  * @param keyWord - keyWord string (pointer)
  * @param keyLng - length of the keyWord
  * @param buff - extracted string
  * @param buffLng - maximal length to be "extracted"
  * @detail   Function goes through the recorded strings and compares them
  *         to the keyword. When keyWord matches, the function copies all
  *         bytes within the record behind the keyword in the buff,
  *         the record is deleted and function returns 1. When keyWord has not
  *         been received 0 is received (no match).
  * @retval length of the extracted string (0 when keyWord does not match) or HARD_ERR.
  */
uint32_t buff_RXextractString(char* keyWord, uint32_t keyLng, uint8_t *buff, uint32_t buffLng);


/**
  * @brief Copy first record in the record queue.
  * @param buff - destination buffer
  * @param buffLng - maximal length to be copied
  * @detail   Function picks first record in queue, copies into the
  *           buff and deletes the record.
  * @retval length of the copied content or HARD_ERR.
  */
uint32_t buff_RXcopyString(uint8_t* buff, uint32_t buffLng);


/**
  * @brief Delete all received records (restores internal buffer state).
  */
void buff_RXflush(void);


/**
  * @brief Starts to listen and initializes the internal buffer.
  * @retval RX_OK when succeeds.
  */
uint32_t buff_RXstart(void);

#endif /* INC_RINGBUFFER_H_ */
