/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "main.h"

const uint8_t CDC_Payload_WaitBoot[3] = {0x11, 0x61, 0x06};
const uint8_t CDC_Payload_BootSequence_1[3] = {0x15, 0x00, 0x25};
const uint8_t CDC_Payload_BootSequence_2[6] = {0x20, 0x01, 0x03, 0x09, 0x15, 0x01};
const uint8_t CDC_Payload_BootSequence_3[2] = {0x25, 0x03};
const uint8_t CDC_Payload_BootSequence_4[5] = {0x26, 0x15, 0x00, 0x80, 0x80};
const uint8_t CDC_Payload_ConfirmHuVersion[2] = {0x62, 0x01};
const uint8_t CDC_Payload_ConfirmPlay[2] = {0x21, 0x05};
const uint8_t CDC_Payload_ConfirmPause[2] = {0x21, 0x03};
const uint8_t CDC_Payload_ConfirmStandby[2] = {0x21, 0x01};
const uint8_t CDC_Payload_ConfirmSongChange_1[4] = {0x27, 0x80, 0x01, 0x22};
const uint8_t CDC_Payload_ConfirmSongChange_2[2] = {0x21, 0x0A};
const uint8_t CDC_Payload_ConfirmSongChange_3[2] = {0x21, 0x05};
const uint8_t CDC_Payload_ConfirmSongChange_4[4] = {0x27, 0x15, 0x00, 0x22};

static void CDC_ConfirmSongChange();

const uint8_t RN52_NextTrack[3] = "AT+";
const uint8_t RN52_PrevTrack[3] = "AT-";
const uint8_t RN52_PlayPause[2] = "AP";

int main(void)
{
	USART_CDC_SendSequence = 0;
	USART_CDC_PlaySequence = 0;
	USART_CDC_TxLocked = 0;

	CDC_CurrentState = WAIT_BOOT;

	USART_RN52_CMD_Mode = RN52_DATA_MODE;
	RN52_State = RN52_State_NotConnected;
	RN52_SilentTime = 0;


	RN52_Title[0] = 'N';
	RN52_Title[1] = '/';
	RN52_Title[2] = 'A';
	RN52_Artist[0] = 'N';
	RN52_Artist[1] = '/';
	RN52_Artist[2] = 'A';

	CAN_AFFA_State = CAN_AFFA_Disabled;
	CAN_AFFA_isRefrNeeded = CAN_AFFA_Keep;
	CAN_Sync = CAN_NotSynced;

	CAN_AFFA_SongName[0] = 'N';
	CAN_AFFA_SongName[1] = '/';
	CAN_AFFA_SongName[2] = 'A';
	CAN_AFFA_SongName[3] = ' ';
	CAN_AFFA_SongName[4] = '-';
	CAN_AFFA_SongName[5] = ' ';
	CAN_AFFA_SongName[6] = 'N';
	CAN_AFFA_SongName[7] = '/';
	CAN_AFFA_SongName[8] = 'A';
	CAN_AFFA_SongName_Length = 9;
	CAN_AFFA_Display_Shift = 0;

	SystemClock_Config();

	GPIO_Conf();

	HAL_Init();
	CAN_Conf();

	USART_Conf();
	TIM_Conf();


	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);//RN52 CMD mode

	while(1)
	{
		switch(CDC_CurrentState)
		{
			case WAIT_BOOT:
				if(USART_CDC_SendPacket(CDC_Payload_WaitBoot, 3, 1))
				{
					CDC_CurrentState = BOOT_SEQUENCE;
				}
				break;

			case BOOT_SEQUENCE:
				USART_CDC_SendPacket(CDC_Payload_BootSequence_1, 3, 1);
				USART_CDC_SendPacket(CDC_Payload_BootSequence_2, 6, 1);
				USART_CDC_SendPacket(CDC_Payload_BootSequence_3, 2, 1);
				USART_CDC_SendPacket(CDC_Payload_BootSequence_4, 5, 1);
				CDC_CurrentState = WAIT_HU_VERSION;
				break;

			case WAIT_HU_VERSION:
				break;

			case CONFIRM_HU_VERSION:
				USART_CDC_SendPacket(CDC_Payload_ConfirmHuVersion, 2, 1);
				CDC_CurrentState = OPERATE_STANDBY;
				break;

			case RECEIVED_PLAY:
				LL_TIM_DisableCounter(TIM3);
				if(RN52_State == RN52_State_Paused) //Something wrong with RN52_State ??
				{
					RN52_SilentTime = 2;
					RN52_State = RN52_State_Playing;
					USART_RN52_Send(RN52_PlayPause, 2);
				}
				USART_CDC_SendPacket(CDC_Payload_ConfirmPlay, 2, 1);
				CDC_CurrentState = OPERATE_PREPARE_PLAY;
				break;

			case RECEIVED_PAUSE:
				LL_TIM_DisableCounter(TIM3);
				if(RN52_State == RN52_State_Playing)
				{
					RN52_SilentTime = 6;
					RN52_State = RN52_State_Paused;
					USART_RN52_Send(RN52_PlayPause, 2);
				}

				USART_CDC_SendPacket(CDC_Payload_ConfirmPause, 2, 1);
				CDC_CurrentState = OPERATE_PAUSED;
				break;

			case RECEIVED_CD_CHANGE:
				LL_TIM_DisableCounter(TIM3);
				CDC_CurrentState = OPERATE_PREPARE_PLAY;
				CDC_ConfirmSongChange();
				break;

			case RECEIVED_NEXT:
				LL_TIM_DisableCounter(TIM3);
				USART_RN52_Send(RN52_NextTrack, 3);
				CDC_CurrentState = OPERATE_PREPARE_PLAY;
				CDC_ConfirmSongChange();
				break;

			case RECEIVED_PREV:
				LL_TIM_DisableCounter(TIM3);
				USART_RN52_Send(RN52_PrevTrack, 3);
				CDC_CurrentState = OPERATE_PREPARE_PLAY;
				CDC_ConfirmSongChange();
				break;

			case RECEIVED_STANDBY:
				LL_TIM_DisableCounter(TIM3);
				if(RN52_State == RN52_State_Playing)
				{
					RN52_SilentTime = 6;
					RN52_State = RN52_State_Paused;
					USART_RN52_Send(RN52_PlayPause, 2);
				}
				USART_CDC_SendPacket(CDC_Payload_ConfirmStandby, 2, 1);
				CDC_CurrentState = OPERATE_STANDBY;
				break;

			case OPERATE_PREPARE_PLAY:
				LL_TIM_EnableCounter(TIM3); // start cyclic operation
				CDC_CurrentState = OPERATE_PLAYING;
				break;

			case OPERATE_PAUSED:
			case OPERATE_STANDBY:
			case OPERATE_PLAYING:
				LL_TIM_EnableCounter(TIM3); // start cyclic operation
				break;
		}
	}
}

void CDC_ConfirmSongChange()
{
	USART_CDC_SendPacket(CDC_Payload_ConfirmSongChange_1, 4, 1);
	USART_CDC_SendPacket(CDC_Payload_ConfirmSongChange_2, 2, 1);
	USART_CDC_SendPacket(CDC_Payload_ConfirmSongChange_3, 2, 1);
	USART_CDC_SendPacket(CDC_Payload_ConfirmSongChange_4, 4, 1);
	USART_CDC_PlaySequence = 0;
}

void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

	LL_RCC_HSE_Enable();

	/* Wait till HSE is ready */
	while(LL_RCC_HSE_IsReady() != 1);

	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_6, LL_RCC_PREDIV_DIV_1);

	LL_RCC_PLL_Enable();

	/* Wait till PLL is ready */
	while(LL_RCC_PLL_IsReady() != 1);

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

	/* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

	LL_Init1msTick(48000000);

	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);

	LL_SetSystemCoreClock(48000000);

	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
}
