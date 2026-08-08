/*
 * imu.c
 *
 *  Created on: Aug 6, 2026
 *      Author: PC
 */

#include "imu.h"
#include <math.h>

#define IMU_ADDRESS          (0x68 << 1)

#define REG_PWR_MGMT_1       0x6B
#define REG_ACCEL_XOUT_H     0x3B

#define ACCEL_SCALE_FACTOR   16384.0f
#define RAD_TO_DEG           57.2958f
#define CALIBRATION_SAMPLES  500

static I2C_HandleTypeDef *imu_i2c = NULL;

static int32_t offset_x = 0;
static int32_t offset_y = 0;
static int32_t offset_z = 0;


HAL_StatusTypeDef IMU_Init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
    {
        return HAL_ERROR;
    }

    imu_i2c = hi2c;

    uint8_t wake_command = 0x00;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        imu_i2c,
        IMU_ADDRESS,
        REG_PWR_MGMT_1,
        I2C_MEMADD_SIZE_8BIT,
        &wake_command,
        1,
        100
    );

    HAL_Delay(100);

    return status;
}


HAL_StatusTypeDef IMU_Read(IMU_Data *data)
{
    if ((imu_i2c == NULL) || (data == NULL))
    {
        return HAL_ERROR;
    }

    uint8_t accel_data[6];

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        imu_i2c,
        IMU_ADDRESS,
        REG_ACCEL_XOUT_H,
        I2C_MEMADD_SIZE_8BIT,
        accel_data,
        6,
        100
    );

    if (status != HAL_OK)
    {
        return status;
    }

    int16_t measured_x =
        (int16_t)((accel_data[0] << 8) | accel_data[1]);

    int16_t measured_y =
        (int16_t)((accel_data[2] << 8) | accel_data[3]);

    int16_t measured_z =
        (int16_t)((accel_data[4] << 8) | accel_data[5]);

    data->raw_x = measured_x - offset_x;
    data->raw_y = measured_y - offset_y;
    data->raw_z = measured_z - offset_z;

    data->accel_x_g = data->raw_x / ACCEL_SCALE_FACTOR;
    data->accel_y_g = data->raw_y / ACCEL_SCALE_FACTOR;
    data->accel_z_g = data->raw_z / ACCEL_SCALE_FACTOR;

    data->roll_deg =
        atan2f(data->accel_y_g, data->accel_z_g)
        * RAD_TO_DEG;

    data->pitch_deg =
        atan2f(
            -data->accel_x_g,
            sqrtf(
                data->accel_y_g * data->accel_y_g +
                data->accel_z_g * data->accel_z_g
            )
        ) * RAD_TO_DEG;

    return HAL_OK;
}


HAL_StatusTypeDef IMU_Calibrate(void)
{
    if (imu_i2c == NULL)
    {
        return HAL_ERROR;
    }

    int64_t sum_x = 0;
    int64_t sum_y = 0;
    int64_t sum_z = 0;

    uint8_t accel_data[6];
    uint32_t valid_samples = 0;

    for (uint32_t i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
            imu_i2c,
            IMU_ADDRESS,
            REG_ACCEL_XOUT_H,
            I2C_MEMADD_SIZE_8BIT,
            accel_data,
            6,
            100
        );

        if (status == HAL_OK)
        {
            int16_t x =
                (int16_t)((accel_data[0] << 8) | accel_data[1]);

            int16_t y =
                (int16_t)((accel_data[2] << 8) | accel_data[3]);

            int16_t z =
                (int16_t)((accel_data[4] << 8) | accel_data[5]);

            sum_x += x;
            sum_y += y;
            sum_z += z;

            valid_samples++;
        }

        HAL_Delay(5);
    }

    if (valid_samples == 0)
    {
        return HAL_ERROR;
    }

    offset_x = (int32_t)(sum_x / valid_samples);
    offset_y = (int32_t)(sum_y / valid_samples);

    /*
     * When the sensor is level, Z should measure +1 g.
     * For the default ±2 g range, +1 g is 16384.
     */
    offset_z =
        (int32_t)(sum_z / valid_samples) - 16384;

    return HAL_OK;
}


int32_t IMU_GetOffsetX(void)
{
    return offset_x;
}


int32_t IMU_GetOffsetY(void)
{
    return offset_y;
}


int32_t IMU_GetOffsetZ(void)
{
    return offset_z;
}
