/*
 * user_gpio.h
 *
 *  Created on: Jan 2, 2025
 *      Author: momo
 */

#ifndef INC_USER_GPIO_H_
#define INC_USER_GPIO_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>

void buzzer_init();
void L1_Open_Door();
void L1_Close_Door();
void L2_Open_Door();
void L2_Close_Door();

void user_gpio_init(void);
void user_gpio_loop(void);

void gpio_RelesOff(void);
void fsm_toggleBuzzer(void);
void fsm_on_buzzer(void);
void fsm_off_buzzer(void);

void fsm_on_solenoide(void);
void fsm_off_solenoide(void);
void fsm_on_BombaValvula(void);
void fsm_on_BombaAire(void);
void fsm_off_BombaAire(void);
void fsm_on_ValvulaAire(void);
void fsm_off_ValvulaAire(void);
void fsm_off_Estanqueidad(void);

#endif /* INC_USER_GPIO_H_ */
