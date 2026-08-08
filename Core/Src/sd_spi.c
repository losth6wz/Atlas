#include "sd_spi.h"

extern SPI_HandleTypeDef hspi1;

#define CMD0 0
#define CMD1 1
#define CMD8 8
#define CMD9 9
#define CMD12 12
#define CMD16 16
#define CMD17 17
#define CMD18 18
#define CMD24 24
#define CMD25 25
#define CMD55 55
#define CMD58 58

#define ACMD23 (0x80U + 23U)
#define ACMD41 (0x80U + 41U)

#define SD_TYPE_NONE  0U
#define SD_TYPE_MMC   1U
#define SD_TYPE_SD1   2U
#define SD_TYPE_SD2   4U
#define SD_TYPE_BLOCK 8U

static volatile DSTATUS sd_stat = STA_NOINIT;
static uint8_t sd_type = SD_TYPE_NONE;
static volatile uint8_t sd_debug_stage = 0;

static void SD_Select(void)
{
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static uint8_t SD_Transfer(uint8_t tx)
{
    uint8_t rx = 0xFF;
    if (HAL_SPI_TransmitReceive(&hspi1, &tx, &rx, 1, 100) != HAL_OK)
        return 0xFF;
    return rx;
}

static void SD_Deselect(void)
{
    uint8_t dummy = 0xFF;
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
    (void)HAL_SPI_Transmit(&hspi1, &dummy, 1, 100);
}

static uint8_t SD_WaitReady(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    do {
        if (SD_Transfer(0xFF) == 0xFF) return 1U;
    } while ((HAL_GetTick() - start) < timeout_ms);
    return 0U;
}

static uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t response = 0xFF;
    uint8_t crc;

    if ((cmd & 0x80U) != 0U)
    {
        cmd &= 0x7FU;
        response = SD_SendCommand(CMD55, 0);
        if (response > 1U) return response;
    }

    SD_Deselect();
    SD_Select();

    if (!SD_WaitReady(500))
    {
        SD_Deselect();
        return 0xFF;
    }

    SD_Transfer((uint8_t)(0x40U | cmd));
    SD_Transfer((uint8_t)(arg >> 24));
    SD_Transfer((uint8_t)(arg >> 16));
    SD_Transfer((uint8_t)(arg >> 8));
    SD_Transfer((uint8_t)arg);

    crc = (cmd == CMD0) ? 0x95 : ((cmd == CMD8) ? 0x87 : 0x01);
    SD_Transfer(crc);

    if (cmd == CMD12) (void)SD_Transfer(0xFF);

    for (uint8_t i = 0; i < 10U; i++)
    {
        response = SD_Transfer(0xFF);
        if ((response & 0x80U) == 0U) return response;
    }

    return 0xFF;
}

static uint8_t SD_ReadBlock(uint8_t *buffer, uint32_t length)
{
    uint8_t token = 0xFF;
    uint32_t start = HAL_GetTick();

    do {
        token = SD_Transfer(0xFF);
        if (token == 0xFE) break;
    } while ((HAL_GetTick() - start) < 500U);

    if (token != 0xFE) return 0U;

    while (length-- > 0U) *buffer++ = SD_Transfer(0xFF);

    (void)SD_Transfer(0xFF);
    (void)SD_Transfer(0xFF);
    return 1U;
}

static uint8_t SD_WriteBlock(const uint8_t *buffer, uint8_t token)
{
    if (!SD_WaitReady(500)) return 0U;

    (void)SD_Transfer(token);

    if (token == 0xFD)
        return SD_WaitReady(1000);

    if (buffer == NULL) return 0U;

    for (uint32_t i = 0; i < 512U; i++)
        (void)SD_Transfer(buffer[i]);

    (void)SD_Transfer(0xFF);
    (void)SD_Transfer(0xFF);

    uint8_t response = SD_Transfer(0xFF);
    if ((response & 0x1FU) != 0x05U) return 0U;

    return SD_WaitReady(1000);
}

static uint32_t SD_GetSectorCount(void)
{
    uint8_t csd[16];

    if (SD_SendCommand(CMD9, 0) != 0)
    {
        SD_Deselect();
        return 0U;
    }

    if (!SD_ReadBlock(csd, sizeof(csd)))
    {
        SD_Deselect();
        return 0U;
    }

    SD_Deselect();

    if ((csd[0] >> 6) == 1U)
    {
        uint32_t c_size =
            ((uint32_t)(csd[7] & 0x3FU) << 16) |
            ((uint32_t)csd[8] << 8) |
            (uint32_t)csd[9];

        return (c_size + 1U) << 10;
    }

    uint32_t read_bl_len = (uint32_t)(csd[5] & 0x0FU);
    uint32_t c_size =
        ((uint32_t)(csd[6] & 0x03U) << 10) |
        ((uint32_t)csd[7] << 2) |
        ((uint32_t)(csd[8] & 0xC0U) >> 6);
    uint32_t c_size_mult =
        ((uint32_t)(csd[9] & 0x03U) << 1) |
        ((uint32_t)(csd[10] & 0x80U) >> 7);
    uint32_t block_count = (c_size + 1U) << (c_size_mult + 2U);
    uint32_t block_size = 1UL << read_bl_len;

    return (block_count * block_size) / 512U;
}

DSTATUS SD_SPI_Initialize(BYTE pdrv)
{
    uint8_t response = 0xFF;
    uint8_t ocr[4];
    uint32_t start;

    sd_debug_stage = 1U;
    if (pdrv != 0U) return STA_NOINIT;

    HAL_Delay(250);

    SD_Deselect();
    for (uint8_t i = 0; i < 10U; i++) (void)SD_Transfer(0xFF);

    sd_debug_stage = 2U;
    sd_type = SD_TYPE_NONE;

    for (uint8_t attempt = 0; attempt < 20U; attempt++)
    {
        response = SD_SendCommand(CMD0, 0);
        if (response == 1U) break;
        HAL_Delay(10);
    }

    if (response == 1U)
    {
        sd_debug_stage = 3U;

        if (SD_SendCommand(CMD8, 0x1AAU) == 1U)
        {
            sd_debug_stage = 4U;

            for (uint8_t i = 0; i < 4U; i++)
                ocr[i] = SD_Transfer(0xFF);

            if ((ocr[2] == 0x01U) && (ocr[3] == 0xAAU))
            {
                sd_debug_stage = 5U;
                start = HAL_GetTick();

                do {
                    response = SD_SendCommand(ACMD41, 1UL << 30);
                    if ((HAL_GetTick() - start) > 3000U) break;
                } while (response != 0U);

                if (response == 0U)
                {
                    sd_debug_stage = 6U;

                    if (SD_SendCommand(CMD58, 0) == 0U)
                    {
                        sd_debug_stage = 7U;

                        for (uint8_t i = 0; i < 4U; i++)
                            ocr[i] = SD_Transfer(0xFF);

                        sd_type = SD_TYPE_SD2;
                        if ((ocr[0] & 0x40U) != 0U)
                            sd_type |= SD_TYPE_BLOCK;

                        sd_debug_stage = 8U;
                    }
                }
            }
        }
        else
        {
            uint8_t init_cmd;

            if (SD_SendCommand(ACMD41, 0) <= 1U)
            {
                sd_type = SD_TYPE_SD1;
                init_cmd = ACMD41;
            }
            else
            {
                sd_type = SD_TYPE_MMC;
                init_cmd = CMD1;
            }

            start = HAL_GetTick();

            do {
                response = SD_SendCommand(init_cmd, 0);
                if ((HAL_GetTick() - start) > 3000U) break;
            } while (response != 0U);

            if ((response != 0U) || (SD_SendCommand(CMD16, 512U) != 0U))
                sd_type = SD_TYPE_NONE;
        }
    }

    SD_Deselect();

    if (sd_type != SD_TYPE_NONE)
    {
        sd_stat &= (DSTATUS)~STA_NOINIT;
        sd_debug_stage = 9U;
    }
    else
    {
        sd_stat = STA_NOINIT;
    }

    return sd_stat;
}

DSTATUS SD_SPI_Status(BYTE pdrv)
{
    if (pdrv != 0U) return STA_NOINIT;
    return sd_stat;
}

DRESULT SD_SPI_Read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if ((pdrv != 0U) || (buff == NULL) || (count == 0U))
        return RES_PARERR;

    if ((sd_stat & STA_NOINIT) != 0U)
        return RES_NOTRDY;

    if ((sd_type & SD_TYPE_BLOCK) == 0U)
        sector *= 512U;

    if (count == 1U)
    {
        if ((SD_SendCommand(CMD17, sector) == 0U) &&
            SD_ReadBlock(buff, 512U))
            count = 0U;
    }
    else if (SD_SendCommand(CMD18, sector) == 0U)
    {
        do {
            if (!SD_ReadBlock(buff, 512U)) break;
            buff += 512U;
        } while (--count > 0U);

        (void)SD_SendCommand(CMD12, 0);
    }

    SD_Deselect();
    return (count == 0U) ? RES_OK : RES_ERROR;
}

#if _USE_WRITE == 1
DRESULT SD_SPI_Write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if ((pdrv != 0U) || (buff == NULL) || (count == 0U))
        return RES_PARERR;

    if ((sd_stat & STA_NOINIT) != 0U)
        return RES_NOTRDY;

    if ((sd_type & SD_TYPE_BLOCK) == 0U)
        sector *= 512U;

    if (count == 1U)
    {
        if ((SD_SendCommand(CMD24, sector) == 0U) &&
            SD_WriteBlock(buff, 0xFE))
            count = 0U;
    }
    else
    {
        if ((sd_type & (SD_TYPE_SD1 | SD_TYPE_SD2)) != 0U)
            (void)SD_SendCommand(ACMD23, count);

        if (SD_SendCommand(CMD25, sector) == 0U)
        {
            do {
                if (!SD_WriteBlock(buff, 0xFC)) break;
                buff += 512U;
            } while (--count > 0U);

            if (!SD_WriteBlock(NULL, 0xFD))
                count = 1U;
        }
    }

    SD_Deselect();
    return (count == 0U) ? RES_OK : RES_ERROR;
}
#endif

#if _USE_IOCTL == 1
DRESULT SD_SPI_Ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    if (pdrv != 0U) return RES_PARERR;
    if ((sd_stat & STA_NOINIT) != 0U) return RES_NOTRDY;

    switch (cmd)
    {
        case CTRL_SYNC:
            SD_Select();
            if (SD_WaitReady(1000))
            {
                SD_Deselect();
                return RES_OK;
            }
            SD_Deselect();
            return RES_ERROR;

        case GET_SECTOR_COUNT:
        {
            if (buff == NULL) return RES_PARERR;
            uint32_t sectors = SD_GetSectorCount();
            if (sectors == 0U) return RES_ERROR;
            *(DWORD *)buff = sectors;
            return RES_OK;
        }

        case GET_SECTOR_SIZE:
            if (buff == NULL) return RES_PARERR;
            *(WORD *)buff = 512U;
            return RES_OK;

        case GET_BLOCK_SIZE:
            if (buff == NULL) return RES_PARERR;
            *(DWORD *)buff = 1U;
            return RES_OK;

        default:
            return RES_PARERR;
    }
}
#endif

uint8_t SD_SPI_GetDebugStage(void)
{
    return sd_debug_stage;
}
