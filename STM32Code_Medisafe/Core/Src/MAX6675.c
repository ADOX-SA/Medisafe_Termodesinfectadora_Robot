#include "max6675.h"
#include "main.h"
#include "global_variables.h"

/*
 //PEGAR EN EL MAIN PARA USAR


		uint32_t now = HAL_GetTick();

		MAX6675_TC_Service(&tc1, now);
		if (tc1.status == MAX6675_OK)
			L1_realTempW = tc1.tempC;

		MAX6675_TC_Service(&tc2, now);
		if (tc2.status == MAX6675_OK)
			L2_realTempW = tc2.tempC;
		*/

extern SPI_HandleTypeDef hspi2;

#define MAX6675_SPI_TIMEOUT_MS  5U

max6675_tc tc1;
max6675_tc tc2;

static inline void cs_low(max6675_tc* tc)
{
    HAL_GPIO_WritePin(tc->CS_PORT, tc->CS_PIN, GPIO_PIN_RESET);
}

static inline void cs_high(max6675_tc* tc)
{
    HAL_GPIO_WritePin(tc->CS_PORT, tc->CS_PIN, GPIO_PIN_SET);
}

void MAX6675_TC_Init(max6675_tc* tc,
                     GPIO_TypeDef* port, uint16_t pin,
                     uint32_t period_ms)
{
    tc->CS_PORT = port;
    tc->CS_PIN  = pin;
    tc->period_ms = period_ms;
    tc->next_ms = 0;
    tc->tempC = 0.0f;
    tc->status = MAX6675_OK;

    cs_high(tc);
}

void MAX6675_TC_Service(max6675_tc* tc, uint32_t now_ms)
{
    if ((int32_t)(now_ms - tc->next_ms) < 0)
        return;

    uint8_t raw = 0;	// era uint16_t

    cs_low(tc);

    HAL_StatusTypeDef st =
        HAL_SPI_Receive(&hspi2, &raw, 1, MAX6675_SPI_TIMEOUT_MS);

    cs_high(tc);

    tc->next_ms = now_ms + tc->period_ms;

    if (st != HAL_OK) {
        tc->status = MAX6675_ERR_SPI;
        return;
    }

    // Bit D2 = 1 → termocupla abierta
    if (raw & (1U << 2)) {
        tc->status = MAX6675_ERR_TC_OPEN;
        return;
    }

    raw >>= 3;                  // D15..D3
    tc->tempC = (float)raw * 0.25f;
    tc->status = MAX6675_OK;
}
