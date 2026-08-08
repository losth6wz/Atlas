#include "atlas.h"

#include <stdio.h>
#include "imu.h"
#include "protocol.h"
#include "telemetry.h"
#include "storage.h"

static Atlas_State atlas_state = ATLAS_STATE_BOOT;

static IMU_Data imu_data;

static uint8_t calibration_requested = 0;

static void Atlas_HandleCommands(void)
{
    Protocol_Command command = Protocol_GetCommand();

    if (command == PROTOCOL_COMMAND_CALIBRATE)
    {
        calibration_requested = 1;
    }
    else if (command == PROTOCOL_COMMAND_PING)
    {
        Telemetry_SendText("PONG\r\n");
    }
}


static void Atlas_HandleButton(void)
{
    if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
    {
        calibration_requested = 1;

        while (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(10);
        }
    }
}


static void Atlas_RunCalibration(void)
{
    atlas_state = ATLAS_STATE_CALIBRATING;

    Telemetry_SendText("CALIBRATION STARTED\r\n");


    if (IMU_Calibrate() == HAL_OK)
    {
        Telemetry_SendCalibration(
            IMU_GetOffsetX(),
            IMU_GetOffsetY(),
            IMU_GetOffsetZ()
        );


        atlas_state = ATLAS_STATE_RUNNING;
    }
    else
    {
        Telemetry_SendText("CALIBRATION FAILED\r\n");


        atlas_state = ATLAS_STATE_ERROR;
    }
}


static void Atlas_RunMission(void)
{
    if (IMU_Read(&imu_data) == HAL_OK)
    {
        Telemetry_SendOrientation(&imu_data);

    }
    else
    {
        Telemetry_SendText("IMU OFFLINE\r\n");


        atlas_state = ATLAS_STATE_ERROR;
    }
}


HAL_StatusTypeDef Atlas_Init(
    I2C_HandleTypeDef *hi2c,
    UART_HandleTypeDef *huart
)
{
    if ((hi2c == NULL) || (huart == NULL))
    {
        atlas_state = ATLAS_STATE_ERROR;
        return HAL_ERROR;
    }

    Telemetry_Init(huart);

    Protocol_Init(huart);
    Protocol_StartReception();

    Telemetry_SendText(
        "\r\n"
        "ATLAS v0.8 | Mission Computer\r\n"
        "--------------------------------\r\n"
    );

    if (IMU_Init(hi2c) != HAL_OK)
    {
        Telemetry_SendText("IMU: OFFLINE\r\n");
        atlas_state = ATLAS_STATE_ERROR;

        return HAL_ERROR;
    }

    Telemetry_SendText(
        "IMU: ONLINE\r\n"
        "Press B1 or send CALIBRATE\r\n"
    );


    atlas_state = ATLAS_STATE_READY;

    return HAL_OK;
}


void Atlas_Update(void)
{
    Atlas_HandleCommands();
    Atlas_HandleButton();

    if (calibration_requested)
    {
        calibration_requested = 0;
        Atlas_RunCalibration();
    }

    switch (atlas_state)
    {
        case ATLAS_STATE_READY:
            /*
             * Waiting for calibration.
             * No mission telemetry yet.
             */
            break;

        case ATLAS_STATE_RUNNING:
            Atlas_RunMission();
            break;

        case ATLAS_STATE_CALIBRATING:
            /*
             * Calibration currently runs synchronously,
             * so this state is temporary.
             */
            break;

        case ATLAS_STATE_ERROR:

            HAL_Delay(150);
            break;

        case ATLAS_STATE_BOOT:
        default:
            break;
    }
}


Atlas_State Atlas_GetState(void)
{
    return atlas_state;
}
