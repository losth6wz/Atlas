#include "rtos_data.h"
#include <string.h>
#define LOG_QUEUE_LENGTH 64

static osMessageQueueId_t log_queue;

static uint32_t log_drop_count = 0;

static IMU_Data latest_imu;

static osMutexId_t imu_mutex;

static uint8_t logger_ready = 0;

static const osMutexAttr_t imu_mutex_attributes =
{
    .name = "IMUMutex"
};

void RTOS_Log_SetReady(uint8_t ready)
{
    logger_ready = ready;
}

uint8_t RTOS_Log_IsReady(void)
{
    return logger_ready;
}

void RTOS_Log_Init(void)
{
    log_queue = osMessageQueueNew(
        LOG_QUEUE_LENGTH,
        sizeof(LogPacket),
        NULL
    );
}

uint8_t RTOS_Log_Push(const LogPacket *packet)
{
    if ((packet == NULL) || (log_queue == NULL))
    {
        return 0;
    }

    if (osMessageQueuePut(
            log_queue,
            packet,
            0,
            0
        ) != osOK)
    {
        log_drop_count++;
        return 0;
    }

    return 1;
}

uint32_t RTOS_Log_GetDropCount(void)
{
    return log_drop_count;
}

uint8_t RTOS_Log_Pop(LogPacket *packet)
{
    if ((packet == NULL) || (log_queue == NULL))
    {
        return 0;
    }

    return osMessageQueueGet(
        log_queue,
        packet,
        NULL,
        osWaitForever
    ) == osOK;
}

void RTOS_Data_Init(void)
{
    imu_mutex = osMutexNew(&imu_mutex_attributes);
}

void RTOS_Data_SetIMU(const IMU_Data *data)
{
    if ((data == NULL) || (imu_mutex == NULL))
    {
        return;
    }

    if (osMutexAcquire(imu_mutex, osWaitForever) == osOK)
    {
        memcpy(&latest_imu, data, sizeof(IMU_Data));

        osMutexRelease(imu_mutex);
    }
}

uint8_t RTOS_Data_GetIMU(IMU_Data *data)
{
    if ((data == NULL) || (imu_mutex == NULL))
    {
        return 0;
    }

    if (osMutexAcquire(imu_mutex, osWaitForever) == osOK)
    {
        memcpy(data, &latest_imu, sizeof(IMU_Data));

        osMutexRelease(imu_mutex);

        return 1;
    }

    return 0;
}
