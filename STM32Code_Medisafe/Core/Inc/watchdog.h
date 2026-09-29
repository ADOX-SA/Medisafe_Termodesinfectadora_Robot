/*
 * watchdog.h
 *
 *  Created on: Jan 7, 2025
 *      Author: ADOX
 */

#ifndef INC_WATCHDOG_H_
#define INC_WATCHDOG_H_

#include "stm32f1xx_hal.h"

// Variable asociada a cuantas veces salta la interrupcion del Watchdog.
extern uint8_t wwdg_counter;

/**
 * @brief Funcion que refresca el watchdog para usar en main()->while(1){}
 * @param none
 * @return none
 */
void watchdog_refresh();

// Interrupcion:
//void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *);

#endif /* INC_WATCHDOG_H_ */
