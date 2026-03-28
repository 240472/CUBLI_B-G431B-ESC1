#include "main.h"
#include "fdcan_user.h"
#include <string.h>

void CAN_ConfigFilters(FDCAN_HandleTypeDef *hfdcan)
{
    FDCAN_FilterTypeDef filter;

    filter.IdType = FDCAN_STANDARD_ID;
    filter.FilterIndex = 0;
    filter.FilterType = FDCAN_FILTER_MASK;
    filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    filter.FilterID1 = 0x000 | CAN_DEVICE;
    filter.FilterID2 = 0x00F;   // Accept IDs only for the specified device

    HAL_FDCAN_ConfigFilter(hfdcan, &filter);
}




void CAN_Start_and_Notify(FDCAN_HandleTypeDef *hfdcan)
{
	HAL_FDCAN_Start(hfdcan);
	HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}




void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
    {
    	CAN_Message_t msg;
    	HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &msg.header, msg.data);

    	// Push to ring buffer
        uint16_t nextIndex = (canWriteIndex + 1) % CAN_QUEUE_SIZE;
    	if (nextIndex != canReadIndex)  // Check for overflow
    	{
			canQueue[canWriteIndex] = msg;
			canWriteIndex = nextIndex;
    	}
    	// If overflow occurs, message is dropped (or handle error)

    }
}

uint8_t CAN_Procces_Received_Messages(void)
{
	if (canReadIndex != canWriteIndex)		//maybe a while
    {
		CAN_Message_t msg = canQueue[canReadIndex];
		canReadIndex = (canReadIndex + 1) % CAN_QUEUE_SIZE;

		switch(msg.header.Identifier)
		{
			//Set Id and Iq reference currents
			case 0x010 | CAN_DEVICE:
				if(msg.header.DataLength == FDCAN_DLC_BYTES_8)
				{
					qd_f_t refCurrents;
					//refCurrents.d  = (float) (0x0000 | (msg.data[0] << 24) | (msg.data[1] << 16) | (msg.data[2] << 8) | (msg.data[3] << 0));
					memcpy(&refCurrents.d, &msg.data[0], sizeof(float));
					//refCurrents.q  = (float) (0x0000 | (msg.data[4] << 24) | (msg.data[5] << 16) | (msg.data[6] << 8) | (msg.data[7] << 0));
					memcpy(&refCurrents.q, &msg.data[4], sizeof(float));
					MC_SetCurrentReferenceMotor1_F(refCurrents);
					return 1;
				}
				break;

			//Set speed ramp
			case 0x020 | CAN_DEVICE:
					if(msg.header.DataLength == FDCAN_DLC_BYTES_8)
					{
						float speedRPM;
						uint16_t rampDuration;

						memcpy(&speedRPM, &msg.data[0], sizeof(float));
						memcpy(&rampDuration, &msg.data[4], sizeof(uint16_t));


						MC_ProgramSpeedRampMotor1_F(speedRPM, rampDuration);
						return 1;
					}
					break;

			//Start/Stop motor
			case 0x030 | CAN_DEVICE:
				if(msg.data[0] == 1)
				{
					MC_StartMotor1();
				}
				else
				{
					MC_StopMotor1();
				}
				break;


			//Set torque ramp
			case 0x040 | CAN_DEVICE:
				if(msg.header.DataLength == FDCAN_DLC_BYTES_8)
				{
					float torqueAMP;
					uint16_t rampDuration;

					memcpy(&torqueAMP, &msg.data[0], sizeof(float));
					memcpy(&rampDuration, &msg.data[4], sizeof(uint16_t));


					MC_ProgramTorqueRampMotor1_F(torqueAMP, rampDuration);
					return 1;
				}
				break;

			//Toggle LED
			case 0x7F0 | CAN_DEVICE:
				if(msg.data[0] == 1)
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
				}
				break;
		}
	}
	return 0;
}


void CAN_SendFrame(FDCAN_HandleTypeDef *hfdcan, uint8_t data[], uint32_t dataLength, uint32_t messageIdentifier)
{
    FDCAN_TxHeaderTypeDef TxHeader;

    TxHeader.Identifier = messageIdentifier;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = dataLength;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, data);
}


void CAN_SendTestFrame(FDCAN_HandleTypeDef *hfdcan)
{
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t data[8] = {1,2,3,4,5,6,7,8};

    TxHeader.Identifier = 0x123;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, data);
}





