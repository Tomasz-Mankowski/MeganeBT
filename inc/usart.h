/*
 * usart.h
 *
 *  Created on: 18.12.2017
 *      Author: Tomek
 */

#ifndef USART_H_
#define USART_H_

#include "system_vars.h"

void USART_Conf();

uint8_t USART_CDC_SendPacket(uint8_t *data, uint8_t length, uint8_t retries);
void USART_RN52_Send(uint8_t *data, uint8_t length);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);

#endif /* USART_H_ */
