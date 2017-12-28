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
	LL_USART_DisableOverrunDetect(USART1);

	NVIC_SetPriority(USART1_IRQn, 2);
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
	LL_USART_DisableOverrunDetect(USART2);

	NVIC_SetPriority(USART2_IRQn, 3);
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

			LL_USART_DisableIT_RXNE(USART1);

			for(int i = 0; i<length+4; i++)
			{
				while(!LL_USART_IsActiveFlag_TXE(USART1));
				LL_USART_TransmitData8(USART1, USART_CDC_TX_buffer[i]);
				while(!LL_USART_IsActiveFlag_TC(USART1));
			}

			LL_USART_EnableIT_RXNE(USART1);

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

						if(USART_CDC_RX_buffer[3] == 0x1C)
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

static volatile uint8_t USART_RN52_RX_buffer[100];
static volatile uint8_t USART_RN52_RX_Ptr = 0;

const uint8_t RN52_Setup1[7] = "SP,1236";

volatile uint32_t flags;

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

						//UNCOMMENT TO RUN RN52 Configuration
						//DO ONLY ONCE, THEN REPROGRAM FLASH FOR NORMAL OPERATION

						/*LL_TIM_DisableIT_UPDATE(TIM3);
						USART_RN52_Send("S|,02", 5); //S/PDIF output
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("S-,Megane", 9); //module name
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("SA,04", 5); // pin required
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("SC,200428", 9); //CoD class
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("SD,04", 5); //A2DP detection class
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("SK,04", 5); //A2DP connection class
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("SN,Megane", 9); //serial name
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("SP,1234", 7); //set pin
						for(int i=0; i<0xFFFF; i++);
						USART_RN52_Send("S%,1026", 7); //extended futures, auto-discover, auto-reconnect, mute up/down vol tones
						for(int i=0; i<0xFFFF; i++);
						LL_TIM_EnableIT_UPDATE(TIM3);*/
					}

					if(USART_RN52_RX_buffer[0] == 'E' && USART_RN52_RX_buffer[1] == 'N' && USART_RN52_RX_buffer[2] == 'D')
					{
						USART_RN52_CMD_Mode = RN52_DATA_MODE;
					}
				}

				if(USART_RN52_RX_Ptr == 6) //Q status
				{
					if(USART_RN52_RX_buffer[3] >= 48 && USART_RN52_RX_buffer[3] <= 57) // if number 0-9
					{
						USART_RN52_RX_buffer[3] -= 48; // set to range 0-9
					}else if(USART_RN52_RX_buffer[3] >= 65 && USART_RN52_RX_buffer[3] <= 70) // if letter A-F
					{
						USART_RN52_RX_buffer[3] -= 55; // set to range 10-16
					}

					uint8_t RN_Status = USART_RN52_RX_buffer[3] & 0x0F; //extract bits 0-3

					if(RN_Status >= 0 && RN_Status <= 2)
					{
						RN52_State = RN52_State_NotConnected;
					}else if(RN_Status == 3)
					{
						RN52_State = RN52_State_Paused;
					}else if(RN_Status == 13)
					{
						RN52_State = RN52_State_Playing;
					}
				}

				if(USART_RN52_RX_Ptr > 9) //Title info
				{
					if(USART_RN52_RX_buffer[5] == '=' && USART_RN52_RX_buffer[0] == 'T' && USART_RN52_RX_buffer[1] == 'i')
					{
						for(uint8_t i = 6; i < USART_RN52_RX_Ptr - 2; i++)
						{
							RN52_Title[i-6] = USART_RN52_RX_buffer[i];
						}
					}
				}

				if(USART_RN52_RX_Ptr > 10) //Artist info
				{
					if(USART_RN52_RX_buffer[6] == '=' && USART_RN52_RX_buffer[0] == 'A' && USART_RN52_RX_buffer[1] == 'r')
					{
						for(uint8_t i = 7; i < USART_RN52_RX_Ptr - 2; i++)
						{
							RN52_Artist[i-7] = USART_RN52_RX_buffer[i];
						}

						/*uint8_t DestPtr=0;
						uint8_t SrcPtr=0;

						while(RN52_Artist[SrcPtr] != '\n' || SrcPtr > 81)
						{
							CAN_AFFA_Text[DestPtr] = RN52_Artist[SrcPtr];
							DestPtr++;
							SrcPtr++;
						}

						CAN_AFFA_Text[DestPtr] = ' '; DestPtr++;
						CAN_AFFA_Text[DestPtr] = '-'; DestPtr++;
						CAN_AFFA_Text[DestPtr] = ' '; DestPtr++;

						SrcPtr = 0;

						while(RN52_Title[SrcPtr] != '\n' || SrcPtr > 81)
						{
							CAN_AFFA_Text[DestPtr] = RN52_Artist[SrcPtr];
							DestPtr++;
							SrcPtr++;
						}

						CAN_AFFA_Text_Lenght = DestPtr+1;*/

						CAN_AFFA_isRefrNeeded = CAN_AFFA_Refresh;
					}
				}

				USART_RN52_RX_Ptr = 0;
			}
		}

		if(USART_RN52_RX_Ptr == 100)
		{
			USART_RN52_RX_Ptr = 0;
		}
	}
}
