/*
 * can.c
 *
 *  Created on: 28.12.2017
 *      Author: Tomek
 */
#include "can.h"

static CAN_HandleTypeDef CanHandle;
static CAN_FilterConfTypeDef  sFilterConfig;

static CanTxMsgTypeDef TxMessage[4];
static CanRxMsgTypeDef RxMessage;

void CAN_Conf()
{
	CanHandle.Instance = CAN;
	CanHandle.pTxMsg = &(TxMessage[0]);
	CanHandle.pRxMsg = &RxMessage;

	CanHandle.Init.Prescaler = 6;
	CanHandle.Init.Mode = CAN_MODE_NORMAL;
	CanHandle.Init.SJW = CAN_SJW_1TQ;
	CanHandle.Init.BS1 = CAN_BS1_7TQ;
	CanHandle.Init.BS2 = CAN_BS2_8TQ;
	CanHandle.Init.TTCM = DISABLE;
	CanHandle.Init.ABOM = DISABLE;
	CanHandle.Init.AWUM = DISABLE;
	CanHandle.Init.NART = DISABLE;
	CanHandle.Init.RFLM = ENABLE;
	CanHandle.Init.TXFP = DISABLE;

	HAL_CAN_Init(&CanHandle);

	sFilterConfig.FilterNumber = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = 0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.BankNumber = 14;

	HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig);

	CanHandle.pTxMsg->StdId = 0x0121;
	CanHandle.pTxMsg->ExtId = 0x0;
	CanHandle.pTxMsg->RTR = CAN_RTR_DATA;
	CanHandle.pTxMsg->IDE = CAN_ID_STD;
	CanHandle.pTxMsg->DLC = 8;

    HAL_CAN_Receive_IT(&CanHandle, CAN_FIFO0);

	for(uint8_t i=0; i<4; i++)
	{
		TxMessage[i].StdId = 0x0121;
		TxMessage[i].ExtId = 0x0;
		TxMessage[i].DLC = 8;
		TxMessage[i].IDE = CAN_ID_STD;
		TxMessage[i].RTR = CAN_RTR_DATA;
	}

	TxMessage[0].Data[0] = 0x10;
	TxMessage[0].Data[1] = 0x19;
	TxMessage[0].Data[2] = 0x7E;
	TxMessage[0].Data[3] = 0x60;
	TxMessage[0].Data[4] = 0x01;

	TxMessage[1].Data[0] = 0x21;
	TxMessage[1].Data[6] = 0x10;

	TxMessage[2].Data[0] = 0x22;

	TxMessage[3].Data[0] = 0x23;
	TxMessage[3].Data[5] = 0x00;
	TxMessage[3].Data[6] = 0x81;
	TxMessage[3].Data[7] = 0x81;
}

void AFFA_DisplayText(char* TextShort, char* TextLong)
{
	TxMessage[0].Data[5] = TextShort[0];
	TxMessage[0].Data[6] = TextShort[1];
	TxMessage[0].Data[7] = TextShort[2];
	TxMessage[1].Data[1] = TextShort[3];
	TxMessage[1].Data[2] = TextShort[4];
	TxMessage[1].Data[3] = TextShort[5];
	TxMessage[1].Data[4] = TextShort[6];
	TxMessage[1].Data[5] = TextShort[7];

	TxMessage[1].Data[7] = TextLong[0];
	TxMessage[2].Data[1] = TextLong[1];
	TxMessage[2].Data[2] = TextLong[2];
	TxMessage[2].Data[3] = TextLong[3];
	TxMessage[2].Data[4] = TextLong[4];
	TxMessage[2].Data[5] = TextLong[5];
	TxMessage[2].Data[6] = TextLong[6];
	TxMessage[2].Data[7] = TextLong[7];
	TxMessage[3].Data[1] = TextLong[8];
	TxMessage[3].Data[2] = TextLong[9];
	TxMessage[3].Data[3] = TextLong[10];
	TxMessage[3].Data[4] = TextLong[11];


	for(uint8_t i=0; i<4; i++)
	{
		CanHandle.pTxMsg = &(TxMessage[i]);
		HAL_CAN_Transmit(&CanHandle, 100);
		for(int j=0;j<0xFFF;j++);
	}

	HAL_CAN_Receive_IT(&CanHandle, CAN_FIFO0);
}

char CAN_TextShort[9];
char CAN_TextLong[13];

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef *CanHandle)
{
	if((CanHandle->pRxMsg->IDE == CAN_ID_STD) && (CanHandle->pRxMsg->DLC == 8))
	{
		if(CanHandle->pRxMsg->StdId == 0x0121)//display messege
		{
			if(CanHandle->pRxMsg->Data[0] == 0x10)//if last
			{
				CAN_TextShort[0] = CanHandle->pRxMsg->Data[5];
				CAN_TextShort[1] = CanHandle->pRxMsg->Data[6];
				CAN_TextShort[2] = CanHandle->pRxMsg->Data[7];
			}

			if(CanHandle->pRxMsg->Data[0] == 0x21)//if last
			{
				CAN_TextShort[3] = CanHandle->pRxMsg->Data[1];
				CAN_TextShort[4] = CanHandle->pRxMsg->Data[2];
				CAN_TextShort[5] = CanHandle->pRxMsg->Data[3];
				CAN_TextShort[6] = CanHandle->pRxMsg->Data[4];
				CAN_TextShort[7] = CanHandle->pRxMsg->Data[5];

				CAN_TextLong[0] = CanHandle->pRxMsg->Data[7];
			}

			if(CanHandle->pRxMsg->Data[0] == 0x22)//if last
			{
				CAN_TextLong[1] = CanHandle->pRxMsg->Data[1];
				CAN_TextLong[2] = CanHandle->pRxMsg->Data[2];
				CAN_TextLong[3] = CanHandle->pRxMsg->Data[3];
				CAN_TextLong[4] = CanHandle->pRxMsg->Data[4];
				CAN_TextLong[5] = CanHandle->pRxMsg->Data[5];
				CAN_TextLong[6] = CanHandle->pRxMsg->Data[6];
				CAN_TextLong[7] = CanHandle->pRxMsg->Data[7];
			}

			if(CanHandle->pRxMsg->Data[0] == 0x23)//if last
			{
				CAN_TextLong[8] = CanHandle->pRxMsg->Data[1];
				CAN_TextLong[9] = CanHandle->pRxMsg->Data[2];
				CAN_TextLong[10] = CanHandle->pRxMsg->Data[3];
				CAN_TextLong[11] = CanHandle->pRxMsg->Data[4];

				CAN_TextShort[8] = 0;
				CAN_TextLong[12] = 0;

				if(strcmp(CAN_TextLong, "CD 1 TR 01  ") == 0 )
				{
					if(strcmp(CAN_TextShort, "TR 01 CD") == 0)
					{
						CAN_AFFA_Display_Shift = 0;
						CAN_AFFA_State = CAN_AFFA_Enabled;
						CAN_AFFA_isRefrNeeded = CAN_AFFA_Refresh;
					}else
					{
						CAN_AFFA_State = CAN_AFFA_Disabled;
					}
				}else
				{
					CAN_AFFA_State = CAN_AFFA_Disabled;
				}

				/*if(strcmp(CAN_TextShort, "TR 01 CD") == 0) && strcmp(CAN_TextLong, "CD 1 TR 01  ") == 0 ) // problems?! WHY?! Needs attention
				{
					CAN_AFFA_Display_Shift = 0;
					CAN_AFFA_State = CAN_AFFA_Enabled;
					CAN_AFFA_isRefrNeeded = CAN_AFFA_Refresh;
				}else
				{
					CAN_AFFA_State = CAN_AFFA_Disabled;
				}*/
			}
		}

		if(CanHandle->pRxMsg->StdId == 0x3DF)//synchro message
		{
			CAN_Sync = CAN_Synced;
		}else
		{
			CAN_Sync = CAN_NotSynced;
		}
	}

	HAL_CAN_Receive_IT(CanHandle, CAN_FIFO0);//recursive recive, if further data in buffer
}

void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  GPIO_AF4_CAN;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_11;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  GPIO_AF4_CAN;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
	__HAL_RCC_CAN1_FORCE_RESET();
	__HAL_RCC_CAN1_RELEASE_RESET();

	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);
	HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11);

	HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
}

void CEC_CAN_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&CanHandle);
}
