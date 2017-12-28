/*
 * tim.h
 *
 *  Created on: 19.12.2017
 *      Author: Tomek
 */

#ifndef TIM_H_
#define TIM_H_

#include "system_vars.h"

void TIM_Conf();

void TIM2_IRQHandler();
void TIM3_IRQHandler();
void TIM14_IRQHandler();

#endif /* TIM_H_ */
