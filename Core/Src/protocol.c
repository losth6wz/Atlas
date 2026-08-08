#include "protocol.h"
#include "telemetry.h"

#include <string.h>

#define COMMAND_BUFFER_SIZE 32

static UART_HandleTypeDef *protocol_uart = NULL;

static uint8_t rx_byte;
static char command_buffer[COMMAND_BUFFER_SIZE];
static uint8_t command_index = 0;

static volatile Protocol_Command pending_command = PROTOCOL_COMMAND_NONE;


static void Protocol_ProcessCommand(const char *command)
{
    if (strcmp(command, "CALIBRATE") == 0)
    {
        pending_command = PROTOCOL_COMMAND_CALIBRATE;
    }
    else if (strcmp(command, "PING") == 0)
    {
        pending_command = PROTOCOL_COMMAND_PING;
    }
    else
    {
        Telemetry_SendText("ERROR:UNKNOWN_COMMAND\r\n");
    }
}


void Protocol_Init(UART_HandleTypeDef *huart)
{
    protocol_uart = huart;
}


void Protocol_StartReception(void)
{
    if (protocol_uart != NULL)
    {
        HAL_UART_Receive_IT(protocol_uart, &rx_byte, 1);
    }
}


Protocol_Command Protocol_GetCommand(void)
{
    Protocol_Command command = pending_command;
    pending_command = PROTOCOL_COMMAND_NONE;

    return command;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((protocol_uart != NULL) &&
        (huart->Instance == protocol_uart->Instance))
    {
        if ((rx_byte == '\r') || (rx_byte == '\n'))
        {
            if (command_index > 0)
            {
                command_buffer[command_index] = '\0';
                Protocol_ProcessCommand(command_buffer);
                command_index = 0;
            }
        }
        else if (command_index < (COMMAND_BUFFER_SIZE - 1))
        {
            command_buffer[command_index++] = (char)rx_byte;
        }
        else
        {
            command_index = 0;
            Telemetry_SendText("ERROR:COMMAND_TOO_LONG\r\n");
        }

        HAL_UART_Receive_IT(protocol_uart, &rx_byte, 1);
    }
}
