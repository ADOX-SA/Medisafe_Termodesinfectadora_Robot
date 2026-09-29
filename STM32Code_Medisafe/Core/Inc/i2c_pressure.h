/*
 * i2c_pressure.h
 *
 *  Created on: Dec 16, 2024
 *      Author: momo
 */

#ifndef INC_I2C_PRESSURE_H_
#define INC_I2C_PRESSURE_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>

extern uint32_t i2c_tic, pressure_tic;

HAL_StatusTypeDef WriteOneByte(uint8_t, uint8_t);
HAL_StatusTypeDef ReadOneByte(uint8_t, uint8_t *);
int32_t ReadPressurePa(void);
float ReadTemperature(void);

void sensadoPresion(void);
void fsm_on_Estanqueidad(void);
void reset_estanqueidad(void);

#endif /* INC_I2C_PRESSURE_H_ */
