/*
 * telemetry.h
 *
 *  Created on: Aug 6, 2026
 *      Author: PC
 */

#ifndef INC_TELEMETRY_H_
#define INC_TELEMETRY_H_

#include "main.h"
#include "imu.h"

void Telemetry_Init(UART_HandleTypeDef *huart);

HAL_StatusTypeDef Telemetry_SendText(const char *text);

void Telemetry_RTOS_Init(void);

HAL_StatusTypeDef Telemetry_SendOrientation(
    const IMU_Data *imu_data
);

HAL_StatusTypeDef Telemetry_SendCalibration(
    int32_t offset_x,
    int32_t offset_y,
    int32_t offset_z
);

#endif /* INC_TELEMETRY_H_ */
