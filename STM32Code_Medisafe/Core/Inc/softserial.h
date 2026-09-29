/*
 * softserial.h
 *
 *  Created on: Jan 8, 2026
 *      Author: migue
 */

#ifndef INC_SOFTSERIAL_H_
#define INC_SOFTSERIAL_H_

#pragma once
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stddef.h>

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t baud;
    uint32_t bit_cycles;   // ciclos de CPU por bit
} SoftUartTx_t;

void SoftUartTx_Begin(SoftUartTx_t *su, GPIO_TypeDef *port, uint16_t pin, uint32_t baud);
void SoftUartTx_WriteByte(SoftUartTx_t *su, uint8_t b);
void SoftUartTx_WriteBuf(SoftUartTx_t *su, const uint8_t *buf, size_t len);
void SoftUartTx_Print(SoftUartTx_t *su, const char *s);


#endif /* INC_SOFTSERIAL_H_ */
