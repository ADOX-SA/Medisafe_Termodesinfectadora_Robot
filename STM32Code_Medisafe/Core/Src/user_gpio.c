/*
 * user_gpio.c
 *
 *  Created on: Jan 2, 2025
 *      Author: momo
 */
#include <stdio.h>
#include "user_gpio.h"
#include "i2c_pressure.h"
#include "main.h"
#include "fsm_reprocesadora1.h"
#include "nextion.h"

extern bool test;

void buzzer_init()
{
	for (uint8_t x = 0;  x < 3; x++)
	{
		fsm_on_buzzer();
		HAL_Delay(25);
		fsm_off_buzzer();
		HAL_Delay(25);
	}
}

void L1_Open_Door()
{
	HAL_GPIO_WritePin(R_IN2_GPIO_Port, R_IN2_Pin, GPIO_PIN_RESET);
}

void L1_Close_Door()
{
	HAL_GPIO_WritePin(R_IN2_GPIO_Port, R_IN2_Pin, GPIO_PIN_SET);
}

void L2_Open_Door()
{
	HAL_GPIO_WritePin(R_IN3_GPIO_Port, R_IN3_Pin, GPIO_PIN_RESET);
}

void L2_Close_Door()
{
	HAL_GPIO_WritePin(R_IN3_GPIO_Port, R_IN3_Pin, GPIO_PIN_SET);
}


void user_gpio_init(void)
{
	gpio_RelesOff();
	fsm_off_buzzer();
}

void user_gpio_loop()
{
	/*
	// --- Sensor de nivel magnético con antirrebote no bloqueante ---

	static uint32_t debounce_time_lvl = 0;
	static GPIO_PinState last_lvl_state = GPIO_PIN_SET;
	static uint8_t lvl_detected = 0;

	GPIO_PinState lvl_state = HAL_GPIO_ReadPin(SENS_LVL_DET_GPIO_Port, SENS_LVL_DET_Pin);

	if (lvl_state != last_lvl_state)
	{
		debounce_time_lvl = HAL_GetTick();
		last_lvl_state = lvl_state;
	}

	if ((HAL_GetTick() - debounce_time_lvl) > 3000)
	{
		if (lvl_state == GPIO_PIN_RESET && !lvl_detected)
		{
			lvl_detected = 1;
			fsm_ev_senslvlDet();
			lvl_det = 1;
		}
		else if (lvl_state == GPIO_PIN_SET && lvl_detected)
		{
			lvl_detected = 0;
			fsm_ev_off_senslvlDet();
			lvl_det = 0;
		}
	}
	 */
/*
	if(test)	// flag seteado en hardware test estanqueidad
	{
		sensadoPresion();
	}
*/
}

void gpio_RelesOff(void)
{
	HAL_GPIO_WritePin(R_IN1_GPIO_Port, R_IN1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(R_IN2_GPIO_Port, R_IN2_Pin, GPIO_PIN_SET);	// L1 Puerta
	HAL_GPIO_WritePin(R_IN3_GPIO_Port, R_IN3_Pin, GPIO_PIN_SET);	// L2 Puerta

	// Estanqueidad:
	HAL_GPIO_WritePin(R_IN4_GPIO_Port, R_IN4_Pin, GPIO_PIN_SET);	// Rele IN4: solenoide para abrir para un circuito u otro
	HAL_GPIO_WritePin(R_IN5_GPIO_Port, R_IN5_Pin, GPIO_PIN_SET);	// bomba vacio (5 V)
	HAL_GPIO_WritePin(R_IN6_GPIO_Port, R_IN6_Pin, GPIO_PIN_SET);	// 12v de valvula de aire
}

void fsm_toggleBuzzer(void)
{
	HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
}

void fsm_on_buzzer(void)
{
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

void fsm_off_buzzer(void)
{
	HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

/*		estanqueidad	*/

void fsm_on_solenoide(void)
{
	HAL_GPIO_WritePin(R_IN4_GPIO_Port, R_IN4_Pin, GPIO_PIN_RESET);	// Rele IN4: solenoide para abrir para LAV2
}

void fsm_off_solenoide(void)
{
	HAL_GPIO_WritePin(R_IN4_GPIO_Port, R_IN4_Pin, GPIO_PIN_SET);	// Rele IN4: solenoide para abrir para LAV2
}

void fsm_on_BombaValvula(void)
{
	HAL_GPIO_WritePin(R_IN5_GPIO_Port, R_IN5_Pin, GPIO_PIN_RESET);						// bomba vacio (5 V)
	HAL_Delay(100);
	HAL_GPIO_WritePin(R_IN6_GPIO_Port, R_IN6_Pin, GPIO_PIN_RESET);						// 12v de valvula de aire
}

void fsm_on_BombaAire(void)
{
	HAL_GPIO_WritePin(R_IN5_GPIO_Port, R_IN5_Pin, GPIO_PIN_RESET);						// bomba vacio (5 V)
}

void fsm_off_BombaAire(void)
{
	HAL_GPIO_WritePin(R_IN5_GPIO_Port, R_IN5_Pin, GPIO_PIN_SET);						// bomba vacio (5 V)
}

void fsm_on_ValvulaAire(void)
{
	HAL_GPIO_WritePin(R_IN6_GPIO_Port, R_IN6_Pin, GPIO_PIN_RESET);						// 12v de valvula de aire
}

void fsm_off_ValvulaAire(void)
{
	HAL_GPIO_WritePin(R_IN6_GPIO_Port, R_IN6_Pin, GPIO_PIN_SET);						// 12v de valvula de aire
}

void fsm_off_Estanqueidad(void)
{
	//HAL_GPIO_WritePin(R_IN4_GPIO_Port, R_IN4_Pin, GPIO_PIN_SET);						// Rele IN4
	HAL_GPIO_WritePin(R_IN5_GPIO_Port, R_IN5_Pin, GPIO_PIN_SET);						// bomba vacio (5 V)
	HAL_GPIO_WritePin(R_IN6_GPIO_Port, R_IN6_Pin, GPIO_PIN_SET);						// 12v de valvula de aire
}
