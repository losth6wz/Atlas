#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "main.h"

typedef enum
{
    PROTOCOL_COMMAND_NONE = 0,
    PROTOCOL_COMMAND_CALIBRATE,
    PROTOCOL_COMMAND_PING
} Protocol_Command;

void Protocol_Init(UART_HandleTypeDef *huart);

void Protocol_StartReception(void);

Protocol_Command Protocol_GetCommand(void);

#endif /* INC_PROTOCOL_H_ */
