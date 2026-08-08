/*
 * imu.h
 *
 *  Created on: Aug 6, 2026
 *      Author: PC
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include "main.h"

typedef struct
{
    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;

    float accel_x_g;
    float accel_y_g;
    float accel_z_g;

    float roll_deg;
    float pitch_deg;

} IMU_Data;

HAL_StatusTypeDef IMU_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef IMU_Read(IMU_Data *data);
HAL_StatusTypeDef IMU_Calibrate(void);

int32_t IMU_GetOffsetX(void);
int32_t IMU_GetOffsetY(void);
int32_t IMU_GetOffsetZ(void);

#endif /* INC_IMU_H_ */
