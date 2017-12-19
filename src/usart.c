/*
 * usart.c
 *
 *  Created on: 18.12.2017
 *      Author: Tomek
 */
#include "usart.h"

static void USART_CDC();
static void USART_RN52();

static LL_USART_InitTypeDef USART_InitStruct;

void USART_Conf()
{
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

	USART_CDC();
	USART_RN52();
}

void USART_CDC()
{
	USART_InitStruct.BaudRate = 9600;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_9B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_EVEN;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART1, &USART_InitStruct);

	LL_USART_ConfigAsyncMode(USART1);

	NVIC_SetPriority(USART1_IRQn, 1);
	NVIC_EnableIRQ(USART1_IRQn);

	LL_USART_EnableIT_RXNE(USART1);

	LL_USART_Enable(USART1);
}

void USART_RN52()
{
	USART_InitStruct.BaudRate = 9600;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART2, &USART_InitStruct);

	LL_USART_ConfigAsyncMode(USART2);

	NVIC_SetPriority(USART2_IRQn, 2);
	NVIC_EnableIRQ(USART2_IRQn);

	LL_USART_EnableIT_RXNE(USART2);

	LL_USART_Enable(USART2);
}

uint8_t USART_CDC_checksum(const uint8_t *data, uint8_t length)
{
	uint8_t res = 0;
	for(uint8_t i=0; i<length; i++)
	{
		res = res ^ data[i];
	}
	return res;
}

static volatile uint8_t USART_CDC_TX_buffer[32];

uint8_t USART_CDC_SendPacket(uint8_t *data, uint8_t length, uint8_t retries)
{
	if(length <= 28)
	{
		for(int ret=0; ret<retries;ret++)
		{
			USART_CDC_TX_buffer[0] = 0x3D;
			USART_CDC_TX_buffer[1] = USART_CDC_SendSequence;
			USART_CDC_TX_buffer[2] = length;
			for(uint8_t i=0; i<length; i++)
			{
				USART_CDC_TX_buffer[3+i] = data[i];
			}
			USART_CDC_TX_buffer[3+length] = USART_CDC_checksum(USART_CDC_TX_buffer, length+3);

			while(USART_CDC_TxLocked);
			USART_CDC_TxLocked = 1;

			for(int i = 0; i<length+4; i++)
			{
				while(!LL_USART_IsActiveFlag_TXE(USART1));
				LL_USART_TransmitData8(USART1, USART_CDC_TX_buffer[i]);
				while(!LL_USART_IsActiveFlag_TC(USART1));
			}

			USART_CDC_TxLocked = 0;

			USART_CDC_Wait = WAITING;
			LL_TIM_SetCounter(TIM2, 0);
			LL_TIM_EnableCounter(TIM2);
			while(USART_CDC_Wait == WAITING);
			LL_TIM_DisableCounter(TIM2);

			if(USART_CDC_Wait == CONFIRMED)
			{
				USART_CDC_SendSequence++;
				return 1;
			}
		}
	}

	return 0;
}

static volatile uint8_t USART_CDC_RX_buffer[32];
static volatile uint8_t USART_CDC_RX_Ptr = 0;

void USART1_IRQHandler(void)
{
	while(LL_USART_IsActiveFlag_RXNE(USART1))
	{
		USART_CDC_RX_buffer[USART_CDC_RX_Ptr] = LL_USART_ReceiveData8(USART1);
		USART_CDC_RX_Ptr++;

		if(USART_CDC_RX_buffer[0] == 0xC5) // confirmation
		{
			USART_CDC_RX_Ptr = 0; //start transimison over
			USART_CDC_Wait = CONFIRMED; // release waiting for confirmation

		}else if(USART_CDC_RX_buffer[0] != 0x3D) //if not a new incoming packet header drop transmission
		{
			USART_CDC_RX_Ptr = 0;
		}

		if(USART_CDC_RX_Ptr >= 4) //handle incoming transmission
		{
			if(USART_CDC_RX_Ptr == USART_CDC_RX_buffer[2] + 4)
			{
				if(USART_CDC_RX_buffer[USART_CDC_RX_Ptr-1] == USART_CDC_checksum(USART_CDC_RX_buffer, USART_CDC_RX_buffer[2]+3))
				{
					while(USART_CDC_TxLocked);
					USART_CDC_TxLocked = 1;

					while(!LL_USART_IsActiveFlag_TXE(USART1));
					LL_USART_TransmitData8(USART1, 0xC5); // confirm packet
					while(!LL_USART_IsActiveFlag_TC(USART1));

					if(USART_CDC_RX_buffer[2] == 7 && USART_CDC_RX_buffer[3] == 0x31)
					{
						CDC_CurrentState = CONFIRM_HU_VERSION;
					}

					if(USART_CDC_RX_buffer[2] == 1)
					{
						if(USART_CDC_RX_buffer[3] == 0x13)
						{
							CDC_CurrentState = RECEIVED_PLAY;
						}

						if(USART_CDC_RX_buffer[3] == 0x02)
						{
							CDC_CurrentState = RECEIVED_PAUSE;
						}

						if(USART_CDC_RX_buffer[3] == 0x19)
						{
							CDC_CurrentState = RECEIVED_STANDBY;
						}

						if(USART_CDC_RX_buffer[3] == 0x24)
						{
							CDC_CurrentState = RECEIVED_CD_CHANGE;
						}
					}

					if(USART_CDC_RX_buffer[2] == 2)
					{
						if(USART_CDC_RX_buffer[3] == 0x17 && USART_CDC_RX_buffer[4] == 0x01)
						{
							CDC_CurrentState = RECEIVED_NEXT;
						}

						if(USART_CDC_RX_buffer[3] == 0x2C && USART_CDC_RX_buffer[4] == 0xFF)
						{
							//ENTERING CDC MODE
						}

						if(USART_CDC_RX_buffer[3] == 0x2C && USART_CDC_RX_buffer[4] == 0x00)
						{
							//EXITING CDC MODE
						}
					}

					if(USART_CDC_RX_buffer[2] == 3)
					{
						if(USART_CDC_RX_buffer[3] == 0x22 && USART_CDC_RX_buffer[4] == 0x01 && USART_CDC_RX_buffer[5] == 0x02)
						{
							CDC_CurrentState = RECEIVED_PREV;
						}
					}

					USART_CDC_TxLocked = 0;
				}
				USART_CDC_RX_Ptr = 0;
			}else if(USART_CDC_RX_Ptr > USART_CDC_RX_buffer[2] + 4) //if received data to long, drop transmission
			{
				USART_CDC_RX_Ptr = 0;
			}
		}

		if(USART_CDC_RX_Ptr == 32)
		{
			USART_CDC_RX_Ptr = 0;
		}
	}
}

void USART_RN52_Send(uint8_t *data, uint8_t length)
{
	if(USART_RN52_CMD_Mode == RN52_CMD_MODE)
	{
		for(int i=0; i<length; i++)
		{
			while(!LL_USART_IsActiveFlag_TXE(USART2));
			LL_USART_TransmitData8(USART2, data[i]);
			while(!LL_USART_IsActiveFlag_TC(USART2));
		}

		while(!LL_USART_IsActiveFlag_TXE(USART2));
		LL_USART_TransmitData8(USART2, '\r');
		while(!LL_USART_IsActiveFlag_TC(USART2));
	}
}

static volatile uint8_t USART_RN52_RX_buffer[32];
static volatile uint8_t USART_RN52_RX_Ptr = 0;

const uint8_t RN52_Reconnect[1] = "B";
const uint8_t RN52_Discoverable[3] = "@,1";

void USART2_IRQHandler(void)
{
	while(LL_USART_IsActiveFlag_RXNE(USART2))
	{
		USART_RN52_RX_buffer[USART_RN52_RX_Ptr] = LL_USART_ReceiveData8(USART2);
		USART_RN52_RX_Ptr++;

		if(USART_RN52_RX_Ptr >= 2)
		{
			if(USART_RN52_RX_buffer[USART_RN52_RX_Ptr-2] == '\r' && USART_RN52_RX_buffer[USART_RN52_RX_Ptr-1] == '\n')
			{
				if(USART_RN52_RX_Ptr == 5)
				{
					if(USART_RN52_RX_buffer[0] == 'C' && USART_RN52_RX_buffer[1] == 'M' && USART_RN52_RX_buffer[2] == 'D')
					{
						USART_RN52_CMD_Mode = RN52_CMD_MODE;
						USART_RN52_Send(RN52_Discoverable, 3);
						USART_RN52_Send(RN52_Reconnect, 1);
					}

					if(USART_RN52_RX_buffer[0] == 'E' && USART_RN52_RX_buffer[1] == 'N' && USART_RN52_RX_buffer[2] == 'D')
					{
						USART_RN52_CMD_Mode = RN52_DATA_MODE;
					}
				}

				USART_RN52_RX_Ptr = 0;
			}
		}

		if(USART_RN52_RX_Ptr == 32)
		{
			USART_RN52_RX_Ptr = 0;
		}
	}
}
