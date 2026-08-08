#ifndef INC_ATLAS_H_
#define INC_ATLAS_H_

#include "main.h"

typedef enum
{
    ATLAS_STATE_BOOT = 0,
    ATLAS_STATE_READY,
    ATLAS_STATE_CALIBRATING,
    ATLAS_STATE_RUNNING,
    ATLAS_STATE_ERROR

} Atlas_State;

HAL_StatusTypeDef Atlas_Init(
    I2C_HandleTypeDef *hi2c,
    UART_HandleTypeDef *huart
);

void Atlas_Update(void);

Atlas_State Atlas_GetState(void);

#endif /* INC_ATLAS_H_ */
