#ifndef INC_GPS_H_
#define INC_GPS_H_

#include "main.h"

HAL_StatusTypeDef GPS_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef GPS_ReadLine(char *buffer, uint16_t buffer_size);

#endif
