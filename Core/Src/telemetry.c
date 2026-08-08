/*
 * telemetry.c
 *
 *  Created on: Aug 6, 2026
 *      Author: PC
 */

#include "telemetry.h"

#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"

#define TELEMETRY_BUFFER_SIZE 100

static UART_HandleTypeDef *telemetry_uart = NULL;

static osMutexId_t telemetry_mutex = NULL;

static char telemetry_buffer[TELEMETRY_BUFFER_SIZE];


void Telemetry_Init(UART_HandleTypeDef *huart)
{
    telemetry_uart = huart;
}

void Telemetry_RTOS_Init(void)
{
    telemetry_mutex = osMutexNew(NULL);
}

static HAL_StatusTypeDef Telemetry_Transmit(
    const uint8_t *data,
    uint16_t length
)
{
    HAL_StatusTypeDef status;

    if ((telemetry_uart == NULL) || (data == NULL))
    {
        return HAL_ERROR;
    }

    if (telemetry_mutex != NULL)
    {
        if (osMutexAcquire(telemetry_mutex, osWaitForever) != osOK)
        {
            return HAL_ERROR;
        }
    }

    status = HAL_UART_Transmit(
        telemetry_uart,
        (uint8_t *)data,
        length,
        100
    );

    if (telemetry_mutex != NULL)
    {
        osMutexRelease(telemetry_mutex);
    }

    return status;
}


HAL_StatusTypeDef Telemetry_SendText(const char *text)
{
    if (text == NULL)
    {
        return HAL_ERROR;
    }

    return Telemetry_Transmit(
        (const uint8_t *)text,
        (uint16_t)strlen(text)
    );
}


HAL_StatusTypeDef Telemetry_SendOrientation(
    const IMU_Data *imu_data
)
{
    if ((telemetry_uart == NULL) || (imu_data == NULL))
    {
        return HAL_ERROR;
    }

    int roll_int = (int)imu_data->roll_deg;
    int pitch_int = (int)imu_data->pitch_deg;

    int length = snprintf(
        telemetry_buffer,
        sizeof(telemetry_buffer),
        "ROLL:%d PITCH:%d\r\n",
        roll_int,
        pitch_int
    );

    if ((length <= 0) || (length >= (int)sizeof(telemetry_buffer)))
    {
        return HAL_ERROR;
    }

    return Telemetry_Transmit(
        (const uint8_t *)telemetry_buffer,
        (uint16_t)length
    );
}


HAL_StatusTypeDef Telemetry_SendCalibration(
    int32_t offset_x,
    int32_t offset_y,
    int32_t offset_z
)
{
    if (telemetry_uart == NULL)
    {
        return HAL_ERROR;
    }

    int length = snprintf(
        telemetry_buffer,
        sizeof(telemetry_buffer),
        "CALIBRATION COMPLETE X:%ld Y:%ld Z:%ld\r\n",
        (long)offset_x,
        (long)offset_y,
        (long)offset_z
    );

    if ((length <= 0) || (length >= (int)sizeof(telemetry_buffer)))
    {
        return HAL_ERROR;
    }

    return Telemetry_Transmit(
        (const uint8_t *)telemetry_buffer,
        (uint16_t)length
    );
}
