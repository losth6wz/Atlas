#ifndef INC_SD_SPI_H_
#define INC_SD_SPI_H_

#include "main.h"
#include "ff_gen_drv.h"

DSTATUS SD_SPI_Initialize(BYTE pdrv);
DSTATUS SD_SPI_Status(BYTE pdrv);
DRESULT SD_SPI_Read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count);

#if _USE_WRITE == 1
DRESULT SD_SPI_Write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif

#if _USE_IOCTL == 1
DRESULT SD_SPI_Ioctl(BYTE pdrv, BYTE cmd, void *buff);
#endif

uint8_t SD_SPI_GetDebugStage(void);

#endif
