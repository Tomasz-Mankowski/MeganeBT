/*
 * can.h
 *
 *  Created on: 28.12.2017
 *      Author: Tomek
 */

#ifndef CAN_H_
#define CAN_H_

#include "system_vars.h"

void CAN_Conf();
void AFFA_DisplayText(char TextShort[8], char TextLong[12]);

void CEC_CAN_IRQHandler(void);

#endif /* CAN_H_ */
