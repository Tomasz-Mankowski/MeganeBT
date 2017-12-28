/*
 * gpio.c
 *
 *  Created on: 18.12.2017
 *      Author: Tomek
 */
#include "gpio.h"

static void GPIO_LED();
static void GPIO_RN52();
static void GPIO_CDC();

static LL_GPIO_InitTypeDef GPIO_InitStruct;
static LL_EXTI_InitTypeDef EXTI_InitStruct;

void GPIO_Conf()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	GPIO_LED();
	GPIO_RN52();
	GPIO_CDC();
}

void GPIO_LED()
{
	GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void GPIO_RN52()
{
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE4);

	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_4;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
	LL_EXTI_Init(&EXTI_InitStruct);

	NVIC_SetPriority(EXTI4_15_IRQn, 3);
	NVIC_EnableIRQ(EXTI4_15_IRQn);

	LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_4);*/
}

static void GPIO_CDC()
{
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void EXTI4_15_IRQHandler(void)//not used
{
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_4))
	{
		//USART_RN52_Send("Q", 1);

		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_4);
	}
}
