/*
 * Medisafe.c
 *
 * Maquina de estados supervisora de Medisafe.
 * La estructura toma como referencia la FSM de ENDOCLEAN2000.
 */

#include "Medisafe.h"
#include "relay_module.h"
#include "Medisafe_Gpio_Config.h"

static Medsaf_FSM_STATES_T Medsaf_state;
static Medsaf_FSM_STATES_T Medsaf_proximo_state;

uint8_t Medsaf_fsm_enable;
uint8_t Medsaf_fsm_paused;
Medsaf_NIVEL_BACHA_T Medsaf_nivel_bacha;

void Medsaf_actualizar_nivel_bacha(void)
{
	if (!MEDSAF_NIVEL_BACHA_BAJO && !MEDSAF_NIVEL_BACHA_MEDIO &&
		!MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		Medsaf_nivel_bacha = Medsaf_BACHA_EMPTY;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && !MEDSAF_NIVEL_BACHA_MEDIO &&
			 !MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		Medsaf_nivel_bacha = Medsaf_BACHA_LOW;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && MEDSAF_NIVEL_BACHA_MEDIO &&
			 !MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		Medsaf_nivel_bacha = Medsaf_BACHA_MEDIUM;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && MEDSAF_NIVEL_BACHA_MEDIO &&
			 MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		Medsaf_nivel_bacha = Medsaf_BACHA_HIGH;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && MEDSAF_NIVEL_BACHA_MEDIO &&
			 MEDSAF_NIVEL_BACHA_ALTO && MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		Medsaf_nivel_bacha = Medsaf_BACHA_FULL;
	}
	else
	{
		Medsaf_nivel_bacha = Medsaf_BACHA_NIVEL_ERROR;
	}
}

Medsaf_NIVEL_BACHA_T Medsaf_get_nivel_bacha(void)
{
	return Medsaf_nivel_bacha;
}

void Medsaf_fsm_init(void)
{
	Medsaf_state = Medsaf_FSM_IDLE;
	Medsaf_proximo_state = Medsaf_FSM_IDLE;
	Medsaf_fsm_enable = 1U;
	Medsaf_fsm_paused = 0U;

	MEDSAF_fsm_off_sistema();
	Medsaf_actualizar_nivel_bacha();
}
