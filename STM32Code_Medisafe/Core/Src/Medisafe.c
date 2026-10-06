/*
 * Medisafe.c
 *
 * Maquina de estados supervisora de Medisafe.
 * La estructura toma como referencia la FSM de ENDOCLEAN2000.
 */

#include "Medisafe.h"
#include "relay_module.h"
#include "Medisafe_Gpio_Config.h"

static MEDSAF_FSM_STATES_T medsaf_state;
static MEDSAF_FSM_STATES_T medsaf_proximo_state;

uint8_t medsaf_fsm_enable;
uint8_t medsaf_fsm_paused;
uint8_t medsaf_error_puerta;
MEDSAF_NIVEL_BACHA_T medsaf_nivel_bacha;

static void Medsaf_cambiar_estado(MEDSAF_FSM_STATES_T medsaf_nuevo_estado)
{
	medsaf_proximo_state = medsaf_nuevo_estado;
	medsaf_state = medsaf_proximo_state;
}

static uint8_t Medsaf_control_puerta(void)
{
	if (!MEDSAF_SENSOR_PUERTA)
	{
		MEDSAF_fsm_off_sistema();
		medsaf_error_puerta = 1;
		medsaf_fsm_enable = 0;
		return 0;
	}

	return 1;
}

void Medsaf_actualizar_nivel_bacha(void)
{
	if (!MEDSAF_NIVEL_BACHA_BAJO && !MEDSAF_NIVEL_BACHA_MEDIO &&
		!MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		medsaf_nivel_bacha = MEDSAF_BACHA_EMPTY;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && !MEDSAF_NIVEL_BACHA_MEDIO &&
			 !MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		medsaf_nivel_bacha = MEDSAF_BACHA_LOW;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && MEDSAF_NIVEL_BACHA_MEDIO &&
			 !MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		medsaf_nivel_bacha = MEDSAF_BACHA_MEDIUM;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && MEDSAF_NIVEL_BACHA_MEDIO &&
			 MEDSAF_NIVEL_BACHA_ALTO && !MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		medsaf_nivel_bacha = MEDSAF_BACHA_HIGH;
	}
	else if (MEDSAF_NIVEL_BACHA_BAJO && MEDSAF_NIVEL_BACHA_MEDIO &&
			 MEDSAF_NIVEL_BACHA_ALTO && MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		medsaf_nivel_bacha = MEDSAF_BACHA_FULL;
	}
	else
	{
		medsaf_nivel_bacha = MEDSAF_BACHA_NIVEL_ERROR;
	}
}

MEDSAF_NIVEL_BACHA_T Medsaf_get_nivel_bacha(void)
{
	return medsaf_nivel_bacha;
}

void Medsaf_fsm_init(void)
{
	medsaf_state = MEDSAF_INIT;
	medsaf_proximo_state = MEDSAF_INIT;
	medsaf_fsm_enable = 1;
	medsaf_fsm_paused = 0;
	medsaf_error_puerta = 0;

	MEDSAF_fsm_off_sistema();
	Medsaf_actualizar_nivel_bacha();
	Medsaf_control_puerta();
}

MEDSAF_FSM_STATES_T Medsaf_fsm_get_state(void)
{
	return medsaf_state;
}

void Medsaf_fsm_runcycle(void)
{
	Medsaf_actualizar_nivel_bacha();

	/* La puerta se controla antes de ejecutar cualquier estado del proceso. */
	if (!Medsaf_control_puerta())
		return;

	if (!medsaf_fsm_enable || medsaf_fsm_paused)
		return;

	switch (medsaf_state)
	{
	case MEDSAF_INIT:
		MEDSAF_fsm_off_sistema();
		Medsaf_cambiar_estado(MEDSAF_PRE_WASH);
		break;

	case MEDSAF_PRE_WASH:
		/* La logica y condicion de transicion se implementaran a continuacion. */
		break;

	case MEDSAF_WASH:
		/* La logica y condicion de transicion se implementaran a continuacion. */
		break;

	case MEDSAF_RINSE:
		/* La logica y condicion de transicion se implementaran a continuacion. */
		break;

	case MEDSAF_FINAL_RINSE:
		/* La logica y condicion de transicion se implementaran a continuacion. */
		break;

	case MEDSAF_COOLING:
		/* La logica y condicion de transicion se implementaran a continuacion. */
		break;

	case MEDSAF_END:
		MEDSAF_fsm_off_sistema();
		break;

	default:
		MEDSAF_fsm_off_sistema();
		medsaf_fsm_enable = 0;
		break;
	}
}
