/*
 * softserial.c
 *
 *  Created on: Jan 8, 2026
 *      Author: migue
 */

#include "softserial.h"
#include <string.h>

/* --- DWT init --- */
static inline void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void wait_until(uint32_t target)
{
    while ((int32_t)(DWT->CYCCNT - target) < 0) { }
}

static inline void tx_high(SoftUartTx_t *su){ su->port->BSRR = su->pin; }
static inline void tx_low (SoftUartTx_t *su){ su->port->BRR  = su->pin; } // F1 tiene BRR

void SoftUartTx_Begin(SoftUartTx_t *su, GPIO_TypeDef *port, uint16_t pin, uint32_t baud)
{
    su->port = port;
    su->pin  = pin;
    su->baud = baud;

    // ciclos por bit (72MHz / 115200 = 625 exacto si clock es 72MHz)
    su->bit_cycles = (SystemCoreClock + baud/2u) / baud;

    DWT_Init();

    // idle UART = HIGH
    tx_high(su);
}

void SoftUartTx_WriteByte(SoftUartTx_t *su, uint8_t b)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t t = DWT->CYCCNT;

    // Start bit (0)
    tx_low(su);
    t += su->bit_cycles; wait_until(t);

    // 8 bits LSB first
    for (int i = 0; i < 8; i++) {
        if (b & 0x01) tx_high(su);
        else          tx_low(su);
        b >>= 1;
        t += su->bit_cycles; wait_until(t);
    }

    // Stop bit (1)
    tx_high(su);
    t += su->bit_cycles; wait_until(t);

    if (!primask) __enable_irq();
}

void SoftUartTx_WriteBuf(SoftUartTx_t *su, const uint8_t *buf, size_t len)
{
    for (size_t i = 0; i < len; i++) SoftUartTx_WriteByte(su, buf[i]);
}

void SoftUartTx_Print(SoftUartTx_t *su, const char *s)
{
    SoftUartTx_WriteBuf(su, (const uint8_t*)s, strlen(s));
}
