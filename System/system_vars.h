/*
 * system_vars.h
 *
 *  Created on: 18.12.2017
 *      Author: Tomek
 */

#ifndef SYSTEM_VARS_H_
#define SYSTEM_VARS_H_

#include "stm32f0xx.h"

#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"

#include "stm32f0xx_hal.h"

typedef enum{
	WAITING,
	CONFIRMED,
	TIMEOUT
}CDC_Wait;

volatile CDC_Wait USART_CDC_Wait;
volatile uint8_t USART_CDC_SendSequence;
volatile uint8_t USART_CDC_PlaySequence;
volatile uint8_t USART_CDC_TxLocked;

typedef enum{
	WAIT_BOOT,
	BOOT_SEQUENCE,
	WAIT_HU_VERSION,
	CONFIRM_HU_VERSION,

	RECEIVED_PLAY,
	RECEIVED_PAUSE,
	RECEIVED_STANDBY,
	RECEIVED_CD_CHANGE,
	RECEIVED_NEXT,
	RECEIVED_PREV,

	OPERATE_STANDBY,
	OPERATE_PAUSED,
	OPERATE_PREPARE_PLAY,
	OPERATE_PLAYING
}CDC_State;

volatile CDC_State CDC_CurrentState;


typedef enum{
	RN52_DATA_MODE,
	RN52_CMD_MODE
}RN52_Mode;

volatile RN52_Mode USART_RN52_CMD_Mode;

#endif /* SYSTEM_VARS_H_ */