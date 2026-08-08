#include "gps.h"

static UART_HandleTypeDef *gps_uart = NULL;

HAL_StatusTypeDef GPS_Init(UART_HandleTypeDef *huart)
{
    if (huart == NULL)
    {
        return HAL_ERROR;
    }

    gps_uart = huart;

    return HAL_OK;
}

HAL_StatusTypeDef GPS_ReadLine(
    char *buffer,
    uint16_t buffer_size
)
{
    if ((gps_uart == NULL) ||
        (buffer == NULL) ||
        (buffer_size < 2))
    {
        return HAL_ERROR;
    }

    uint16_t index = 0;
    uint8_t ch;

    while (index < (buffer_size - 1))
    {
        if (HAL_UART_Receive(
                gps_uart,
                &ch,
                1,
                100
            ) != HAL_OK)
        {
            return HAL_TIMEOUT;
        }

        if (ch == '\n')
        {
            break;
        }

        if (ch != '\r')
        {
            buffer[index++] = (char)ch;
        }
    }

    buffer[index] = '\0';

    return HAL_OK;
}
