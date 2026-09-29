#ifndef MAX6675_H_
#define MAX6675_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MAX6675_OK = 0,
    MAX6675_ERR_TC_OPEN,
    MAX6675_ERR_SPI
} max6675_status_t;

typedef struct {
    float tempC;
    GPIO_TypeDef* CS_PORT;
    uint16_t CS_PIN;

    uint32_t period_ms;
    uint32_t next_ms;

    max6675_status_t status;
} max6675_tc;

extern max6675_tc tc1;
extern max6675_tc tc2;

void MAX6675_TC_Init(max6675_tc* tc,
                     GPIO_TypeDef* port, uint16_t pin,
                     uint32_t period_ms);

void MAX6675_TC_Service(max6675_tc* tc, uint32_t now_ms);

#endif
