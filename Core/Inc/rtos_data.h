#ifndef INC_RTOS_DATA_H_
#define INC_RTOS_DATA_H_

#include "imu.h"
#include "cmsis_os.h"

void RTOS_Data_Init(void);

void RTOS_Data_SetIMU(const IMU_Data *data);

uint8_t RTOS_Data_GetIMU(IMU_Data *data);

uint32_t RTOS_Log_GetDropCount(void);

void RTOS_Log_SetReady(uint8_t ready);
uint8_t RTOS_Log_IsReady(void);

typedef struct
{
    uint32_t time_ms;
    IMU_Data imu;

} LogPacket;

void RTOS_Log_Init(void);

uint8_t RTOS_Log_Push(const LogPacket *packet);

uint8_t RTOS_Log_Pop(LogPacket *packet);
#endif
