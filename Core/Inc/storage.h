#ifndef INC_STORAGE_H_
#define INC_STORAGE_H_

#include "main.h"

typedef enum
{
    STORAGE_OK = 0,
    STORAGE_NOT_READY,
    STORAGE_MOUNT_FAILED,
    STORAGE_OPEN_FAILED,
    STORAGE_WRITE_FAILED

} Storage_Status;

Storage_Status Storage_Init(void);

Storage_Status Storage_WriteTest(void);

uint8_t Storage_IsReady(void);

int Storage_GetLastError(void);

Storage_Status Storage_AppendLine(
    const char *filename,
    const char *text,
    uint32_t length
);

Storage_Status Storage_OpenLog(const char *filename);

Storage_Status Storage_WriteHeaderIfEmpty(void);

Storage_Status Storage_WriteLog(
    const char *text,
    uint32_t length
);

Storage_Status Storage_SyncLog(void);
#endif /* INC_STORAGE_H_ */
