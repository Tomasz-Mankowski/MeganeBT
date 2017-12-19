/*
 * tim.c
 *
 *  Created on: 19.12.2017
 *      Author: Tomek
 */
#include "tim.h"

static void TIM_CDC_Timeout();
static void TIM_CDC_FakePlay();

static LL_TIM_InitTypeDef TIM_InitStruct;

void TIM_Conf()
{
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);


	TIM_CDC_Timeout();
	TIM_CDC_FakePlay();
}

static void TIM_CDC_Timeout()
{
	TIM_InitStruct.Prescaler = 191; //Inc 250 000 Hz
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 4999; // Rel 4999 = 20 ms = 50 Hz
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM2, &TIM_InitStruct);

	NVIC_SetPriority(TIM2_IRQn, 0);
	NVIC_EnableIRQ(TIM2_IRQn);

	LL_TIM_ClearFlag_UPDATE(TIM2);
	LL_TIM_EnableIT_UPDATE(TIM2);

	LL_TIM_DisableCounter(TIM2);
}

void TIM_CDC_FakePlay()
{
	TIM_InitStruct.Prescaler = 1919; //Inc 25 000 Hz
	TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	TIM_InitStruct.Autoreload = 24999; // Rel 24999 = 1 Hz
	TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_Init(TIM3, &TIM_InitStruct);

	NVIC_SetPriority(TIM3_IRQn, 2);
	NVIC_EnableIRQ(TIM3_IRQn);

	LL_TIM_ClearFlag_UPDATE(TIM3);
	LL_TIM_EnableIT_UPDATE(TIM3);

	LL_TIM_DisableCounter(TIM3);
}

void TIM2_IRQHandler()
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM2))
	{
		LL_TIM_DisableCounter(TIM2);

		USART_CDC_Wait = TIMEOUT;

		LL_TIM_ClearFlag_UPDATE(TIM2);
	}
}

const uint8_t CDC_Payload_OperateStandby[6] = {0x20, 0x01, 0x03, 0x09, 0x15, 0x01};
const uint8_t CDC_Payload_OperatePreperePlay[6] = {0x20, 0x05, 0x03, 0x09, 0x15, 0x01};
const uint8_t CDC_Payload_OperatePaused[6] = {0x20, 0x03, 0x03, 0x09, 0x15, 0x01};
uint8_t CDC_Payload_OperatePlaying[11] = {0x47, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};

void TIM3_IRQHandler()
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM3))
	{
		switch(CDC_CurrentState)
		{
			case OPERATE_STANDBY:
				USART_CDC_SendPacket(CDC_Payload_OperateStandby, 6, 5);
				break;

			case OPERATE_PREPARE_PLAY:
				USART_CDC_SendPacket(CDC_Payload_OperatePreperePlay, 6, 5);
				break;

			case OPERATE_PLAYING:
				CDC_Payload_OperatePlaying[9] = USART_CDC_PlaySequence;
				USART_CDC_SendPacket(CDC_Payload_OperatePlaying, 11, 5);
				USART_CDC_PlaySequence++;
				break;

			case OPERATE_PAUSED:
				USART_CDC_SendPacket(CDC_Payload_OperatePaused, 6, 5);
				break;

			default:
				break;
		}

		LL_TIM_ClearFlag_UPDATE(TIM3);
	}
}
