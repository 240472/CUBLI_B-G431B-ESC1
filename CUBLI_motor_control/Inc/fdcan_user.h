

#ifndef FDCAN_USER_H
#define FDCAN_USER_H


#define RX_BUFFER_SIZE 8
#define CAN_QUEUE_SIZE 16  // Number of messages in the ring buffer
#define CAN_DEVICE 0x003   //0x001, 0x002 or 0x003 depending which board is commanded


typedef struct {
    FDCAN_RxHeaderTypeDef header;
    uint8_t data[RX_BUFFER_SIZE];
} CAN_Message_t;

//Must define these before int main(void)!!
extern volatile uint16_t canWriteIndex;
extern volatile uint16_t canReadIndex;
extern CAN_Message_t canQueue[CAN_QUEUE_SIZE];


void CAN_ConfigFilters(FDCAN_HandleTypeDef *hfdcan);
void CAN_Start_and_Notify(FDCAN_HandleTypeDef *hfdcan);

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
uint8_t CAN_Procces_Received_Messages(void);



/*
  * @brief  Sends one CAN frame with specified identifier and data.
  *
  * @param  *hfdcan: CAN handle.
  * @param  data[]: Data of specified length.
  * @param  dataLength: Use FDCAN_DLC_BYTES_X; X = <1,8>.
  * @param  messageIdentifier: Message identfier = <0x000, 0x7FF>.
  */
void CAN_SendFrame(FDCAN_HandleTypeDef *hfdcan, uint8_t data[], uint32_t dataLength, uint32_t messageIdentifier);
void CAN_SendTestFrame(FDCAN_HandleTypeDef *hfdcan);


#endif
