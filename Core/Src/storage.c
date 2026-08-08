#include "storage.h"

#include "fatfs.h"
#include "ff.h"
#include "user_diskio.h"
#include <string.h>

static FATFS storage_fs;
static uint8_t storage_ready = 0;
static FRESULT last_fatfs_result = FR_OK;
static uint8_t mkfs_workbuf[4096];
static FIL log_file;
static uint8_t log_file_open = 0;

Storage_Status Storage_OpenLog(const char *filename)
{
    if (!storage_ready)
    {
        return STORAGE_NOT_READY;
    }

    if (filename == NULL)
    {
        return STORAGE_OPEN_FAILED;
    }

    FRESULT result = f_open(
        &log_file,
        filename,
        FA_OPEN_APPEND | FA_WRITE
    );

    if (result != FR_OK)
    {
        log_file_open = 0;
        return STORAGE_OPEN_FAILED;
    }

    log_file_open = 1;

    return STORAGE_OK;
}
Storage_Status Storage_WriteLog(
    const char *text,
    uint32_t length
)
{
    if (!storage_ready || !log_file_open)
    {
        return STORAGE_NOT_READY;
    }

    UINT bytes_written = 0;

    FRESULT result = f_write(
        &log_file,
        text,
        length,
        &bytes_written
    );

    if ((result != FR_OK) ||
        (bytes_written != length))
    {
        return STORAGE_WRITE_FAILED;
    }

    return STORAGE_OK;
}
Storage_Status Storage_SyncLog(void)
{
    if (!storage_ready || !log_file_open)
    {
        return STORAGE_NOT_READY;
    }

    if (f_sync(&log_file) != FR_OK)
    {
        return STORAGE_WRITE_FAILED;
    }

    return STORAGE_OK;
}
Storage_Status Storage_WriteHeaderIfEmpty(void)
{
    if (!storage_ready || !log_file_open)
    {
        return STORAGE_NOT_READY;
    }

    if (f_size(&log_file) == 0)
    {
        const char *header =
            "time_ms,roll_deg,pitch_deg\r\n";

        return Storage_WriteLog(
            header,
            strlen(header)
        );
    }

    return STORAGE_OK;
}
Storage_Status Storage_Init(void)
{
    HAL_Delay(500);

    last_fatfs_result = f_mount(
        &storage_fs,
        "",
        1
    );

    if (last_fatfs_result == FR_NO_FILESYSTEM)
    {
        last_fatfs_result = f_mkfs(
            "",
            FM_FAT32,
            0,
            mkfs_workbuf,
            sizeof(mkfs_workbuf)
        );

        if (last_fatfs_result != FR_OK)
        {
            storage_ready = 0;
            return STORAGE_MOUNT_FAILED;
        }

        /* Mount again after formatting */
        last_fatfs_result = f_mount(
            &storage_fs,
            "",
            1
        );
    }

    if (last_fatfs_result != FR_OK)
    {
        storage_ready = 0;
        return STORAGE_MOUNT_FAILED;
    }

    storage_ready = 1;
    return STORAGE_OK;
}


Storage_Status Storage_WriteTest(void)
{
    if (!storage_ready)
    {
        return STORAGE_NOT_READY;
    }

    FIL file;
    FRESULT result;
    UINT bytes_written;

    const char *message =
        "ATLAS STORAGE ONLINE\r\n";

    result = f_open(
        &file,
        "ATLAS.TXT",
        FA_CREATE_ALWAYS | FA_WRITE
    );

    if (result != FR_OK)
    {
        return STORAGE_OPEN_FAILED;
    }

    result = f_write(
        &file,
        message,
        strlen(message),
        &bytes_written
    );

    f_close(&file);

    if ((result != FR_OK) ||
        (bytes_written != strlen(message)))
    {
        return STORAGE_WRITE_FAILED;
    }

    return STORAGE_OK;
}


uint8_t Storage_IsReady(void)
{
    return storage_ready;
}

int Storage_GetLastError(void)
{
    return (int)last_fatfs_result;
}

Storage_Status Storage_AppendLine(
    const char *filename,
    const char *text,
    uint32_t length
)
{
    if (!storage_ready)
    {
        return STORAGE_NOT_READY;
    }

    FIL file;
    UINT bytes_written;

    FRESULT result = f_open(
        &file,
        filename,
        FA_OPEN_APPEND | FA_WRITE
    );

    if (result != FR_OK)
    {
        return STORAGE_OPEN_FAILED;
    }

    result = f_write(
        &file,
        text,
        length,
        &bytes_written
    );

    f_close(&file);

    if ((result != FR_OK) ||
        (bytes_written != length))
    {
        return STORAGE_WRITE_FAILED;
    }

    return STORAGE_OK;
}
