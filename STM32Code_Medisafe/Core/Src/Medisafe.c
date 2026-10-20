/*
 * Medisafe.c
 *
 * Maquina de estados supervisora de Medisafe.
 * La estructura toma como referencia la FSM de ENDOCLEAN2000.
 */

#include "Medisafe.h"
#include "relay_module.h"
#include "Medisafe_Gpio_Config.h"
#include "nextion.h"
#include "global_variables.h"
#include "user_gpio.h"
#include "ticket.h"
#include <stdio.h>

typedef enum
{
	MEDSAF_BUZZER_APAGADO = 0,
	MEDSAF_BUZZER_ERROR,
	MEDSAF_BUZZER_FIN
} MEDSAF_BUZZER_STATES_T;

static MEDSAF_FSM_STATES_T medsaf_state;
static MEDSAF_FSM_STATES_T medsaf_proximo_state;
static MEDSAF_PREPARE_TANK_STATES_T medsaf_prepare_tank_state;
static MEDSAF_PRE_WASH_STATES_T medsaf_pre_wash_state;
static MEDSAF_WASH_STATES_T medsaf_wash_state;
static MEDSAF_RINSE_STATES_T medsaf_rinse_state;
static MEDSAF_FINAL_RINSE_STATES_T medsaf_final_rinse_state;
static MEDSAF_COOLING_STATES_T medsaf_cooling_state;
static MEDSAF_FINISH_STATES_T medsaf_finish_state;

uint8_t medsaf_fsm_enable;
uint8_t medsaf_fsm_paused;
uint8_t medsaf_error_puerta;
uint8_t medsaf_error_nivel_maximo;
uint8_t medsaf_error_nivel_bacha;
uint8_t medsaf_error_llenado_timeout;
uint8_t medsaf_error_drenaje_timeout;
uint8_t medsaf_error_calefaccion_timeout;
uint8_t medsaf_error_calefactor;
uint8_t medsaf_error_termocuplas;
uint8_t medsaf_error_sobretemperatura;
uint8_t medsaf_error_temperatura;
MEDSAF_MODO_T medsaf_modo = MEDSAF_MODO_ESTANDAR;
MEDSAF_PROGRAMA_T medsaf_programa = MEDSAF_PROGRAMA_P1;
MEDSAF_NIVEL_BACHA_T medsaf_nivel_bacha;

uint32_t medsaf_pre_wash_tic;
uint32_t medsaf_tapa_tic;
uint32_t medsaf_drenaje_extra_tic;
uint32_t medsaf_sendstate_tic;
uint32_t medsaf_llenado_timeout_tic;
uint32_t medsaf_drenaje_timeout_tic;
uint32_t medsaf_detergente_tic;
uint32_t medsaf_wash_tic;
uint32_t medsaf_ultrasonido_total_tic;
uint32_t medsaf_ultrasonido_pulso_tic;
uint32_t medsaf_calefaccion_timeout_tic;
uint32_t medsaf_calefactor_control_tic;
uint32_t medsaf_calefactor_muestra_tic;
uint32_t medsaf_rinse_tic;
uint32_t medsaf_final_rinse_tic;
uint32_t medsaf_cooling_tic;
uint32_t medsaf_buzzer_tic;

static MEDSAF_BUZZER_STATES_T medsaf_buzzer_state;
static uint8_t medsaf_buzzer_pulsos;
static uint8_t medsaf_buzzer_on;

static uint8_t medsaf_salidas_pausa[16];
static uint32_t medsaf_pre_wash_tic_pausa;
static uint32_t medsaf_tapa_tic_pausa;
static uint32_t medsaf_drenaje_extra_tic_pausa;
static uint32_t medsaf_llenado_timeout_tic_pausa;
static uint32_t medsaf_drenaje_timeout_tic_pausa;
static uint32_t medsaf_detergente_tic_pausa;
static uint32_t medsaf_wash_tic_pausa;
static uint32_t medsaf_ultrasonido_total_tic_pausa;
static uint32_t medsaf_ultrasonido_pulso_tic_pausa;
static uint32_t medsaf_calefaccion_timeout_tic_pausa;
static uint32_t medsaf_calefactor_control_tic_pausa;
static uint32_t medsaf_calefactor_muestra_tic_pausa;
static uint32_t medsaf_rinse_tic_pausa;
static uint32_t medsaf_final_rinse_tic_pausa;
static uint32_t medsaf_cooling_tic_pausa;
static uint8_t medsaf_ticket_activo;
static char medsaf_ticket_proceso[16];

static const MEDSAF_CALEFACTOR_CONTROL_T medsaf_calefactor_control =
{
	60000,
	2.0f
};

typedef struct
{
	float temperatura_inicial_acumulada;
	float temperatura_final_acumulada;
	uint8_t muestras_iniciales;
	uint8_t muestras_finales;
	float termocupla_1_acumulada;
	float termocupla_2_acumulada;
	uint8_t muestras_termocuplas;
	uint8_t activo;
} MEDSAF_CALEFACTOR_MONITOR_T;

static MEDSAF_CALEFACTOR_MONITOR_T medsaf_calefactor_monitor;

static void Medsaf_cambiar_estado(MEDSAF_FSM_STATES_T medsaf_nuevo_estado);
static void Medsaf_detener_por_error(const char *medsaf_mensaje);
static uint8_t Medsaf_control_timeout_drenaje(void);
static void Medsaf_fsm_pre_wash_init(void);
static void Medsaf_fsm_wash_init(void);
static void Medsaf_fsm_rinse_init(void);
static void Medsaf_fsm_final_rinse_init(void);
static void Medsaf_fsm_cooling_init(void);
static void Medsaf_buzzer_fin_init(void);
static void Medsaf_ticket_iniciar(void);
static void Medsaf_ticket_finalizar(const char *medsaf_resultado);

static void Medsaf_calefactor_monitor_stop(void)
{
	medsaf_calefactor_monitor.activo = 0;
	medsaf_calefactor_control_tic = 0;
	medsaf_calefactor_muestra_tic = 0;
}

static void Medsaf_calefactor_monitor_init(void)
{
	medsaf_calefactor_monitor.temperatura_inicial_acumulada = 0.0f;
	medsaf_calefactor_monitor.temperatura_final_acumulada = 0.0f;
	medsaf_calefactor_monitor.muestras_iniciales = 0;
	medsaf_calefactor_monitor.muestras_finales = 0;
	medsaf_calefactor_monitor.termocupla_1_acumulada = 0.0f;
	medsaf_calefactor_monitor.termocupla_2_acumulada = 0.0f;
	medsaf_calefactor_monitor.muestras_termocuplas = 0;
	medsaf_calefactor_monitor.activo = 1;
	medsaf_calefactor_control_tic = medsaf_calefactor_control.timeout;
	medsaf_calefactor_muestra_tic = 0;
}

static uint8_t Medsaf_calefactor_monitor_runcycle(void)
{
	float medsaf_temperatura_promedio;
	float medsaf_temperatura_inicial;
	float medsaf_temperatura_final;
	float medsaf_temperatura_1_promedio;
	float medsaf_temperatura_2_promedio;
	float medsaf_diferencia_temperaturas;
	uint32_t medsaf_diferencia_decimas;
	uint32_t medsaf_ventana_final;
	char medsaf_mensaje[60];

	if (!medsaf_calefactor_monitor.activo)
		return 1;

	medsaf_ventana_final = MEDSAF_TIME_MUESTRA_CALEFACTOR *
			MEDSAF_MUESTRAS_PROMEDIO_CALEFACTOR;

	if (!medsaf_calefactor_muestra_tic)
	{
		medsaf_temperatura_promedio = (L1_realTemp + L2_realTemp) / 2.0f;
		medsaf_calefactor_monitor.termocupla_1_acumulada += L1_realTemp;
		medsaf_calefactor_monitor.termocupla_2_acumulada += L2_realTemp;
		medsaf_calefactor_monitor.muestras_termocuplas++;

		if (medsaf_calefactor_monitor.muestras_termocuplas >=
			MEDSAF_MUESTRAS_PROMEDIO_CALEFACTOR)
		{
			medsaf_temperatura_1_promedio =
					medsaf_calefactor_monitor.termocupla_1_acumulada /
					medsaf_calefactor_monitor.muestras_termocuplas;
			medsaf_temperatura_2_promedio =
					medsaf_calefactor_monitor.termocupla_2_acumulada /
					medsaf_calefactor_monitor.muestras_termocuplas;
			medsaf_diferencia_temperaturas = medsaf_temperatura_1_promedio -
					medsaf_temperatura_2_promedio;
			if (medsaf_diferencia_temperaturas < 0.0f)
				medsaf_diferencia_temperaturas = -medsaf_diferencia_temperaturas;

			if (medsaf_diferencia_temperaturas >=
				MEDSAF_DIFERENCIA_MAX_TERMOCUPLAS)
			{
				medsaf_diferencia_decimas =
						(uint32_t)(medsaf_diferencia_temperaturas * 10.0f);
				snprintf(medsaf_mensaje, sizeof(medsaf_mensaje),
						 "Falla termocuplas Dif=%lu.%lu C",
						 (unsigned long)(medsaf_diferencia_decimas / 10),
						 (unsigned long)(medsaf_diferencia_decimas % 10));
				medsaf_error_termocuplas = 1;
				Medsaf_detener_por_error(medsaf_mensaje);
				Medsaf_calefactor_monitor_stop();
				return 0;
			}

			medsaf_calefactor_monitor.termocupla_1_acumulada = 0.0f;
			medsaf_calefactor_monitor.termocupla_2_acumulada = 0.0f;
			medsaf_calefactor_monitor.muestras_termocuplas = 0;
		}

		if (medsaf_calefactor_monitor.muestras_iniciales <
			MEDSAF_MUESTRAS_PROMEDIO_CALEFACTOR)
		{
			medsaf_calefactor_monitor.temperatura_inicial_acumulada +=
					medsaf_temperatura_promedio;
			medsaf_calefactor_monitor.muestras_iniciales++;
		}

		if ((medsaf_calefactor_control_tic <= medsaf_ventana_final) &&
			(medsaf_calefactor_monitor.muestras_finales <
			 MEDSAF_MUESTRAS_PROMEDIO_CALEFACTOR))
		{
			medsaf_calefactor_monitor.temperatura_final_acumulada +=
					medsaf_temperatura_promedio;
			medsaf_calefactor_monitor.muestras_finales++;
		}

		medsaf_calefactor_muestra_tic = MEDSAF_TIME_MUESTRA_CALEFACTOR;
	}

	if (medsaf_calefactor_control_tic)
		return 1;

	if (!medsaf_calefactor_monitor.muestras_iniciales ||
		!medsaf_calefactor_monitor.muestras_finales)
	{
		medsaf_error_calefactor = 1;
		Medsaf_detener_por_error("Falla medicion calefactor");
		Medsaf_calefactor_monitor_stop();
		return 0;
	}

	medsaf_temperatura_inicial =
			medsaf_calefactor_monitor.temperatura_inicial_acumulada /
			medsaf_calefactor_monitor.muestras_iniciales;
	medsaf_temperatura_final =
			medsaf_calefactor_monitor.temperatura_final_acumulada /
			medsaf_calefactor_monitor.muestras_finales;

	if ((medsaf_temperatura_final - medsaf_temperatura_inicial) <
		medsaf_calefactor_control.step_temperatura)
	{
		medsaf_error_calefactor = 1;
		Medsaf_detener_por_error("Falla calefactor");
		Medsaf_calefactor_monitor_stop();
		return 0;
	}

	/* Si aun no alcanzo el objetivo, comienza una nueva ventana de control. */
	Medsaf_calefactor_monitor_init();
	return 1;
}

static void Medsaf_ticket_iniciar(void)
{
	if (medsaf_ticket_activo)
		return;

	snprintf(medsaf_ticket_proceso, sizeof(medsaf_ticket_proceso),
			 "P%d %s", (int)medsaf_programa,
			 (medsaf_modo == MEDSAF_MODO_ROBOT) ? "ROBOT" : "ESTANDAR");
	Ticket_EnqHeader(REPRO1, "", "", "", "", medsaf_ticket_proceso);
	Ticket_EnqStepStartCached(REPRO1, medsaf_ticket_proceso);
	medsaf_ticket_activo = 1;
}

static void Medsaf_ticket_error(const char *medsaf_mensaje)
{
	char medsaf_evento[50];

	if (!medsaf_ticket_activo)
		return;

	snprintf(medsaf_evento, sizeof(medsaf_evento),
			 "ERROR: %s", medsaf_mensaje);
	Ticket_EnqEventRtc(REPRO1, medsaf_evento);
}

static void Medsaf_ticket_finalizar(const char *medsaf_resultado)
{
	if (!medsaf_ticket_activo)
		return;

	Ticket_EnqStepEndRtc(REPRO1, medsaf_ticket_proceso);
	Ticket_EnqFinish(REPRO1, medsaf_resultado, "");
	Ticket_EnqPrint(REPRO1);
	medsaf_ticket_activo = 0;
}

static void Medsaf_fsm_finish_runcycle(void)
{
	switch (medsaf_finish_state)
	{
	case MEDSAF_FINISH_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_timeout_tic = 0;
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_finish_state = MEDSAF_FINISH_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_FINISH_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_finish_state = MEDSAF_FINISH_FIN;
		}
		break;

	case MEDSAF_FINISH_FIN:
		MEDSAF_fsm_off_sistema();
		Medsaf_ticket_finalizar("STOP");
		Medsaf_cambiar_estado(MEDSAF_END);
		Nextion_ChangeState(REPRO1, Finalizado_NextionState);
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Ocultar_NextionButtons);
		Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
		enqueue_nextion_command("data.L1_state.txt=\"END\"");
		enqueue_nextion_command("data.L1_substate.txt=\"FINALIZADO_MANUAL\"");
		Medsaf_buzzer_fin_init();
		medsaf_fsm_enable = 0;
		break;

	default:
		Medsaf_detener_por_error("Estado FINALIZAR invalido");
		break;
	}
}

static void Medsaf_fsm_prepare_tank_init(void)
{
	medsaf_prepare_tank_state = MEDSAF_PREPARE_TANK_INICIO;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_drenaje_extra_tic = 0;
}

static void Medsaf_fsm_prepare_tank_runcycle(void)
{
	switch (medsaf_prepare_tank_state)
	{
	case MEDSAF_PREPARE_TANK_INICIO:
		MEDSAF_fsm_off_sistema();
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_prepare_tank_state = MEDSAF_PREPARE_TANK_FIN;
		}
		else
		{
			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			medsaf_prepare_tank_state = MEDSAF_PREPARE_TANK_DRENAR;
		}
		break;

	case MEDSAF_PREPARE_TANK_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_timeout_tic = 0;
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_prepare_tank_state = MEDSAF_PREPARE_TANK_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_PREPARE_TANK_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_prepare_tank_state = MEDSAF_PREPARE_TANK_FIN;
		}
		break;

	case MEDSAF_PREPARE_TANK_FIN:
		Medsaf_fsm_pre_wash_init();
		Medsaf_cambiar_estado(MEDSAF_PRE_WASH);
		break;

	default:
		Medsaf_detener_por_error("Estado PREPARE_TANK invalido");
		break;
	}
}

static void Medsaf_buzzer_off(void)
{
	fsm_off_buzzer();
	medsaf_buzzer_state = MEDSAF_BUZZER_APAGADO;
	medsaf_buzzer_pulsos = 0;
	medsaf_buzzer_on = 0;
	medsaf_buzzer_tic = 0;
}

static void Medsaf_buzzer_error_init(void)
{
	fsm_off_buzzer();
	medsaf_buzzer_state = MEDSAF_BUZZER_ERROR;
	medsaf_buzzer_pulsos = 0;
	medsaf_buzzer_on = 0;
	medsaf_buzzer_tic = MEDSAF_TIME_BUZZER_ERROR;
}

static void Medsaf_buzzer_fin_init(void)
{
	medsaf_buzzer_state = MEDSAF_BUZZER_FIN;
	medsaf_buzzer_pulsos = 0;
	medsaf_buzzer_on = 1;
	fsm_on_buzzer();
	medsaf_buzzer_tic = MEDSAF_TIME_BUZZER_FIN;
}

static void Medsaf_buzzer_runcycle(void)
{
	uint32_t medsaf_tiempo_buzzer;

	if (medsaf_buzzer_state == MEDSAF_BUZZER_APAGADO)
		return;

	if (medsaf_buzzer_pulsos >= 3)
	{
		Medsaf_buzzer_off();
		return;
	}

	if (medsaf_buzzer_tic)
		return;

	medsaf_tiempo_buzzer = (medsaf_buzzer_state == MEDSAF_BUZZER_ERROR) ?
			MEDSAF_TIME_BUZZER_ERROR : MEDSAF_TIME_BUZZER_FIN;

	if (medsaf_buzzer_on)
	{
		fsm_off_buzzer();
		medsaf_buzzer_on = 0;
		medsaf_buzzer_pulsos++;
		if (medsaf_buzzer_pulsos < 3)
			medsaf_buzzer_tic = medsaf_tiempo_buzzer;
	}
	else
	{
		fsm_on_buzzer();
		medsaf_buzzer_on = 1;
		medsaf_buzzer_tic = medsaf_tiempo_buzzer;
	}
}

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
		if (!medsaf_error_puerta)
		{
			Medsaf_buzzer_error_init();
			Medsaf_ticket_error("PUERTA ABIERTA");
			Nextion_Error(REPRO1, "Puerta abierta");
			Nextion_ChangeState(REPRO1, Error_NextionState);
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		}
		medsaf_error_puerta = 1;
		medsaf_fsm_enable = 0;
		return 0;
	}

	return 1;
}

static uint8_t Medsaf_control_nivel_maximo(void)
{
	if (MEDSAF_NIVEL_BACHA_MAXIMO)
	{
		MEDSAF_fsm_off_sistema();
		if (!medsaf_error_nivel_maximo)
		{
			Medsaf_buzzer_error_init();
			Medsaf_ticket_error("SENSOR NIVEL MAXIMO");
			Nextion_Error(REPRO1, "Sensor nivel MAXIMO");
			Nextion_ChangeState(REPRO1, Error_NextionState);
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		}
		medsaf_error_nivel_maximo = 1;
		medsaf_fsm_enable = 0;
		return 0;
	}

	return 1;
}

static void Medsaf_detener_por_error(const char *medsaf_mensaje)
{
	MEDSAF_fsm_off_sistema();
	medsaf_fsm_enable = 0;
	Medsaf_buzzer_error_init();
	Medsaf_ticket_error(medsaf_mensaje);
	Nextion_Error(REPRO1, medsaf_mensaje);
	Nextion_ChangeState(REPRO1, Error_NextionState);
	Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
}

static uint8_t Medsaf_control_nivel_invalido(void)
{
	if (medsaf_nivel_bacha == MEDSAF_BACHA_NIVEL_ERROR)
	{
		if (!medsaf_error_nivel_bacha)
			Medsaf_detener_por_error("Error sensores nivel bacha");

		medsaf_error_nivel_bacha = 1;
		return 0;
	}

	return 1;
}

static uint8_t Medsaf_control_timeout_llenado(void)
{
	if (medsaf_llenado_timeout_tic)
		return 1;

	if (!medsaf_error_llenado_timeout)
		Medsaf_detener_por_error("Timeout llenado bacha");

	medsaf_error_llenado_timeout = 1;
	return 0;
}

static uint8_t Medsaf_control_timeout_drenaje(void)
{
	if (medsaf_drenaje_timeout_tic)
		return 1;

	if (!medsaf_error_drenaje_timeout)
		Medsaf_detener_por_error("Timeout drenaje bacha");

	medsaf_error_drenaje_timeout = 1;
	return 0;
}

static uint8_t Medsaf_control_calefaccion(void)
{
	if (L1_realTemp > (L1_TEMP_WASHING_MAX + MEDSAF_MARGEN_SOBRETEMPERATURA))
	{
		if (!medsaf_error_sobretemperatura)
			Medsaf_detener_por_error("Sobretemperatura bacha");

		medsaf_error_sobretemperatura = 1;
		return 0;
	}

	if (!medsaf_calefaccion_timeout_tic)
	{
		if (!medsaf_error_calefaccion_timeout)
			Medsaf_detener_por_error("Timeout calefaccion");

		medsaf_error_calefaccion_timeout = 1;
		return 0;
	}

	if (!Medsaf_calefactor_monitor_runcycle())
		return 0;

	return 1;
}

static void Medsaf_fsm_pre_wash_init(void)
{
	medsaf_pre_wash_state = MEDSAF_PRE_WASH_INICIO;
	medsaf_pre_wash_tic = 0;
	medsaf_tapa_tic = 0;
	medsaf_drenaje_extra_tic = 0;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
}

static void Medsaf_fsm_pre_wash_runcycle(void)
{
	switch (medsaf_pre_wash_state)
	{
	case MEDSAF_PRE_WASH_INICIO:
		MEDSAF_fsm_off_sistema();
		MEDSAF_fsm_on_cold_water();
		medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
		medsaf_pre_wash_state = MEDSAF_PRE_WASH_LLENAR_FRIA;
		break;

	case MEDSAF_PRE_WASH_LLENAR_FRIA:
		if ((medsaf_nivel_bacha == MEDSAF_BACHA_LOW) ||
			(medsaf_nivel_bacha == MEDSAF_BACHA_MEDIUM) ||
			(medsaf_nivel_bacha == MEDSAF_BACHA_HIGH))
		{
			MEDSAF_fsm_on_lavado_bomba();
			MEDSAF_SOLIN_TAPA_ON
			medsaf_tapa_tic = MEDSAF_TIME_ALTERNAR_TAPA;
			medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
			medsaf_pre_wash_state = MEDSAF_PRE_WASH_COMPLETAR_LLENADO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_PRE_WASH_COMPLETAR_LLENADO:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_HIGH)
		{
			MEDSAF_fsm_off_llenar_bacha();
			medsaf_pre_wash_tic = MEDSAF_TIME_PRE_WASH;
			medsaf_llenado_timeout_tic = 0;
			medsaf_pre_wash_state = MEDSAF_PRE_WASH_LAVANDO;
		}
		else if (!medsaf_tapa_tic)
		{
			if (MEDSAF_SOLIN_TAPA)
			{
				MEDSAF_SOLIN_TAPA_OFF
			}
			else
			{
				MEDSAF_SOLIN_TAPA_ON
			}

			medsaf_tapa_tic = MEDSAF_TIME_ALTERNAR_TAPA;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_PRE_WASH_LAVANDO:
		if (!medsaf_pre_wash_tic)
		{
			MEDSAF_fsm_off_lavado_bomba();
			MEDSAF_SOLIN_TAPA_OFF
			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			medsaf_pre_wash_state = MEDSAF_PRE_WASH_DRENAR;
		}
		else if (!medsaf_tapa_tic)
		{
			if (MEDSAF_SOLIN_TAPA)
			{
				MEDSAF_SOLIN_TAPA_OFF
			}
			else
			{
				MEDSAF_SOLIN_TAPA_ON
			}

			medsaf_tapa_tic = MEDSAF_TIME_ALTERNAR_TAPA;
		}
		break;

	case MEDSAF_PRE_WASH_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_drenaje_timeout_tic = 0;
			medsaf_pre_wash_state = MEDSAF_PRE_WASH_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_PRE_WASH_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_pre_wash_state = MEDSAF_PRE_WASH_FIN;
		}
		break;

	case MEDSAF_PRE_WASH_FIN:
		Medsaf_fsm_wash_init();
		Medsaf_cambiar_estado(MEDSAF_WASH);
		break;

	default:
		MEDSAF_fsm_off_sistema();
		medsaf_fsm_enable = 0;
		break;
	}
}

static void Medsaf_fsm_wash_init(void)
{
	medsaf_wash_state = MEDSAF_WASH_INICIO;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_detergente_tic = 0;
	medsaf_wash_tic = 0;
	medsaf_ultrasonido_total_tic = 0;
	medsaf_ultrasonido_pulso_tic = 0;
	medsaf_drenaje_extra_tic = 0;
}

static void Medsaf_fsm_wash_runcycle(void)
{
	switch (medsaf_wash_state)
	{
	case MEDSAF_WASH_INICIO:
		MEDSAF_fsm_off_sistema();
		MEDSAF_fsm_on_hot_water();
		medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
		medsaf_wash_state = MEDSAF_WASH_LLENAR_MEDIUM;
		break;

	case MEDSAF_WASH_LLENAR_MEDIUM:
		if ((medsaf_nivel_bacha == MEDSAF_BACHA_MEDIUM) ||
			(medsaf_nivel_bacha == MEDSAF_BACHA_HIGH))
		{
			MEDSAF_fsm_off_llenar_bacha();
			MEDSAF_fsm_on_detergente();
			medsaf_llenado_timeout_tic = 0;
			medsaf_detergente_tic = MEDSAF_TIME_DETERGENTE;
			medsaf_wash_state = MEDSAF_WASH_DOSIFICAR_DETERGENTE;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_WASH_DOSIFICAR_DETERGENTE:
		if (!medsaf_detergente_tic)
		{
			MEDSAF_fsm_off_detergente();
			MEDSAF_fsm_on_hot_water();
			medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
			medsaf_wash_state = MEDSAF_WASH_COMPLETAR_LLENADO;
		}
		break;

	case MEDSAF_WASH_COMPLETAR_LLENADO:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_HIGH)
		{
			MEDSAF_fsm_off_llenar_bacha();
			MEDSAF_fsm_on_lavado_bomba();
			MEDSAF_SOLIN_TAPA_ON
			medsaf_llenado_timeout_tic = 0;
			medsaf_wash_tic = MEDSAF_TIME_WASH;
			medsaf_wash_state = MEDSAF_WASH_LAVANDO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_WASH_LAVANDO:
		if (!medsaf_wash_tic)
		{
			MEDSAF_fsm_on_ultrasonido();
			medsaf_ultrasonido_total_tic = MEDSAF_TIME_ULTRASONIDO_TOTAL;
			medsaf_ultrasonido_pulso_tic = MEDSAF_TIME_ULTRASONIDO_ON;
			medsaf_wash_state = MEDSAF_WASH_ULTRASONIDO_ON;
		}
		break;

	case MEDSAF_WASH_ULTRASONIDO_ON:
		if (!medsaf_ultrasonido_total_tic)
		{
			MEDSAF_fsm_off_ultrasonido();
			MEDSAF_fsm_off_lavado_bomba();
			MEDSAF_SOLIN_TAPA_OFF
			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			medsaf_wash_state = MEDSAF_WASH_DRENAR;
		}
		else if (!medsaf_ultrasonido_pulso_tic)
		{
			MEDSAF_fsm_off_ultrasonido();
			medsaf_ultrasonido_pulso_tic = MEDSAF_TIME_ULTRASONIDO_OFF;
			medsaf_wash_state = MEDSAF_WASH_ULTRASONIDO_OFF;
		}
		break;

	case MEDSAF_WASH_ULTRASONIDO_OFF:
		if (!medsaf_ultrasonido_total_tic)
		{
			MEDSAF_fsm_off_ultrasonido();
			MEDSAF_fsm_off_lavado_bomba();
			MEDSAF_SOLIN_TAPA_OFF
			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			medsaf_wash_state = MEDSAF_WASH_DRENAR;
		}
		else if (!medsaf_ultrasonido_pulso_tic)
		{
			MEDSAF_fsm_on_ultrasonido();
			medsaf_ultrasonido_pulso_tic = MEDSAF_TIME_ULTRASONIDO_ON;
			medsaf_wash_state = MEDSAF_WASH_ULTRASONIDO_ON;
		}
		break;

	case MEDSAF_WASH_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_timeout_tic = 0;
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_wash_state = MEDSAF_WASH_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_WASH_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_wash_state = MEDSAF_WASH_FIN;
		}
		break;

	case MEDSAF_WASH_FIN:
		Medsaf_fsm_rinse_init();
		Medsaf_cambiar_estado(MEDSAF_RINSE);
		break;

	default:
		Medsaf_detener_por_error("Estado WASH invalido");
		break;
	}
}

static void Medsaf_fsm_rinse_init(void)
{
	medsaf_rinse_state = MEDSAF_RINSE_INICIO;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_calefaccion_timeout_tic = 0;
	Medsaf_calefactor_monitor_stop();
	medsaf_rinse_tic = 0;
	medsaf_tapa_tic = 0;
	medsaf_drenaje_extra_tic = 0;
}

static void Medsaf_fsm_rinse_runcycle(void)
{
	switch (medsaf_rinse_state)
	{
	case MEDSAF_RINSE_INICIO:
		MEDSAF_fsm_off_sistema();
		MEDSAF_fsm_on_hot_water();
		medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
		medsaf_rinse_state = MEDSAF_RINSE_LLENAR_MEDIUM;
		break;

	case MEDSAF_RINSE_LLENAR_MEDIUM:
		if ((medsaf_nivel_bacha == MEDSAF_BACHA_MEDIUM) ||
			(medsaf_nivel_bacha == MEDSAF_BACHA_HIGH))
		{
			MEDSAF_fsm_on_calefactor();
			Medsaf_calefactor_monitor_init();
			medsaf_calefaccion_timeout_tic = MEDSAF_TIMEOUT_CALEFACCION;
			medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
			medsaf_rinse_state = MEDSAF_RINSE_COMPLETAR_LLENADO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_RINSE_COMPLETAR_LLENADO:
		if (!Medsaf_control_calefaccion())
			break;

		if (medsaf_nivel_bacha == MEDSAF_BACHA_HIGH)
		{
			MEDSAF_fsm_off_llenar_bacha();
			medsaf_llenado_timeout_tic = 0;
			medsaf_rinse_state = MEDSAF_RINSE_CALENTANDO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_RINSE_CALENTANDO:
		if (L1_realTemp >= L1_TEMP_WASHING_MAX)
		{
			MEDSAF_fsm_off_calefactor();
			Medsaf_calefactor_monitor_stop();
			MEDSAF_fsm_on_lavado_bomba();
			MEDSAF_SOLIN_TAPA_ON
			medsaf_calefaccion_timeout_tic = 0;
			medsaf_tapa_tic = MEDSAF_TIME_ALTERNAR_TAPA_RINSE;
			medsaf_rinse_tic = MEDSAF_TIME_RINSE;
			medsaf_rinse_state = MEDSAF_RINSE_LAVANDO;
		}
		else
			Medsaf_control_calefaccion();
		break;

	case MEDSAF_RINSE_LAVANDO:
		if (!medsaf_rinse_tic)
		{
			MEDSAF_fsm_off_lavado_bomba();
			MEDSAF_SOLIN_TAPA_OFF
			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			medsaf_rinse_state = MEDSAF_RINSE_DRENAR;
		}
		else if (!medsaf_tapa_tic)
		{
			if (MEDSAF_SOLIN_TAPA)
			{
				MEDSAF_SOLIN_TAPA_OFF
			}
			else
			{
				MEDSAF_SOLIN_TAPA_ON
			}

			medsaf_tapa_tic = MEDSAF_TIME_ALTERNAR_TAPA_RINSE;
		}
		break;

	case MEDSAF_RINSE_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_timeout_tic = 0;
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_rinse_state = MEDSAF_RINSE_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_RINSE_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_rinse_state = MEDSAF_RINSE_FIN;
		}
		break;

	case MEDSAF_RINSE_FIN:
		Medsaf_fsm_final_rinse_init();
		Medsaf_cambiar_estado(MEDSAF_FINAL_RINSE);
		break;

	default:
		Medsaf_detener_por_error("Estado RINSE invalido");
		break;
	}
}

static void Medsaf_fsm_final_rinse_init(void)
{
	medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_INICIO;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_final_rinse_tic = 0;
	medsaf_drenaje_extra_tic = 0;
}

static void Medsaf_fsm_final_rinse_runcycle(void)
{
	switch (medsaf_final_rinse_state)
	{
	case MEDSAF_FINAL_RINSE_INICIO:
		MEDSAF_fsm_off_sistema();
		MEDSAF_fsm_on_hot_water();
		medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
		medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_LLENAR_MEDIUM;
		break;

	case MEDSAF_FINAL_RINSE_LLENAR_MEDIUM:
		if ((medsaf_nivel_bacha == MEDSAF_BACHA_MEDIUM) ||
			(medsaf_nivel_bacha == MEDSAF_BACHA_HIGH))
		{
			MEDSAF_fsm_on_lavado_bomba();
			MEDSAF_SOLIN_TAPA_ON
			medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
			medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_COMPLETAR_LLENADO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_FINAL_RINSE_COMPLETAR_LLENADO:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_HIGH)
		{
			MEDSAF_fsm_off_llenar_bacha();
			medsaf_llenado_timeout_tic = 0;
			medsaf_final_rinse_tic = MEDSAF_TIME_FINAL_RINSE;
			medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_RECIRCULANDO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_FINAL_RINSE_RECIRCULANDO:
		if (!medsaf_final_rinse_tic)
		{
			MEDSAF_fsm_off_lavado_bomba();
			MEDSAF_SOLIN_TAPA_OFF
			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_DRENAR;
		}
		break;

	case MEDSAF_FINAL_RINSE_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_timeout_tic = 0;
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_FINAL_RINSE_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_final_rinse_state = MEDSAF_FINAL_RINSE_FIN;
		}
		break;

	case MEDSAF_FINAL_RINSE_FIN:
		Medsaf_fsm_cooling_init();
		Medsaf_cambiar_estado(MEDSAF_COOLING);
		break;

	default:
		Medsaf_detener_por_error("Estado FINAL_RINSE invalido");
		break;
	}
}

static void Medsaf_fsm_cooling_init(void)
{
	medsaf_cooling_state = MEDSAF_COOLING_INICIO;
	medsaf_cooling_tic = 0;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_drenaje_extra_tic = 0;
}

static void Medsaf_fsm_cooling_runcycle(void)
{
	switch (medsaf_cooling_state)
	{
	case MEDSAF_COOLING_INICIO:
		MEDSAF_fsm_off_sistema();
		MEDSAF_fsm_on_cold_water();
		medsaf_llenado_timeout_tic = MEDSAF_TIMEOUT_LLENADO;
		medsaf_cooling_state = MEDSAF_COOLING_LLENAR_MEDIUM;
		break;

	case MEDSAF_COOLING_LLENAR_MEDIUM:
		if ((medsaf_nivel_bacha == MEDSAF_BACHA_MEDIUM) ||
			(medsaf_nivel_bacha == MEDSAF_BACHA_HIGH))
		{
			MEDSAF_fsm_off_llenar_bacha();
			medsaf_llenado_timeout_tic = 0;
			medsaf_cooling_tic = MEDSAF_TIME_COOLING;
			medsaf_cooling_state = MEDSAF_COOLING_ESPERANDO;
		}
		else
			Medsaf_control_timeout_llenado();
		break;

	case MEDSAF_COOLING_ESPERANDO:
		if (!medsaf_cooling_tic)
		{
			if (!L1_realTemp)
			{
				medsaf_error_temperatura = 1;
				Medsaf_detener_por_error("Falla sensor temperatura");
				break;
			}

			MEDSAF_fsm_on_drenar_bacha();
			medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
			if (L1_realTemp <= MEDSAF_TEMP_COOLING)
				medsaf_cooling_state = MEDSAF_COOLING_DRENAJE_FINAL;
			else
				medsaf_cooling_state = MEDSAF_COOLING_DRENAR;
		}
		break;

	case MEDSAF_COOLING_DRENAR:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_drenaje_timeout_tic = 0;
			medsaf_cooling_state = MEDSAF_COOLING_INICIO;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_COOLING_DRENAJE_FINAL:
		if (medsaf_nivel_bacha == MEDSAF_BACHA_EMPTY)
		{
			medsaf_drenaje_timeout_tic = 0;
			medsaf_drenaje_extra_tic = MEDSAF_TIME_DRENAJE_EXTRA;
			medsaf_cooling_state = MEDSAF_COOLING_DRENAJE_EXTRA;
		}
		else
			Medsaf_control_timeout_drenaje();
		break;

	case MEDSAF_COOLING_DRENAJE_EXTRA:
		if (!medsaf_drenaje_extra_tic)
		{
			MEDSAF_fsm_off_drenar_bacha();
			medsaf_cooling_state = MEDSAF_COOLING_FIN;
		}
		break;

	case MEDSAF_COOLING_FIN:
		MEDSAF_fsm_off_sistema();
		Medsaf_ticket_finalizar("OK");
		Medsaf_cambiar_estado(MEDSAF_END);
		Nextion_ChangeState(REPRO1, Finalizado_NextionState);
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Ocultar_NextionButtons);
		Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
		enqueue_nextion_command("data.L1_state.txt=\"END\"");
		enqueue_nextion_command("data.L1_substate.txt=\"FINALIZADO\"");
		Medsaf_buzzer_fin_init();
		medsaf_fsm_enable = 0;
		break;

	default:
		Medsaf_detener_por_error("Estado COOLING invalido");
		break;
	}
}

static const char *Medsaf_get_state_text(void)
{
	switch (medsaf_state)
	{
	case MEDSAF_INIT: return "INIT";
	case MEDSAF_PREPARE_TANK: return "PREPARE_TANK";
	case MEDSAF_PRE_WASH: return "PRE_WASH";
	case MEDSAF_WASH: return "WASH";
	case MEDSAF_RINSE: return "RINSE";
	case MEDSAF_FINAL_RINSE: return "FINAL_RINSE";
	case MEDSAF_COOLING: return "COOLING";
	case MEDSAF_FINALIZAR_MANUAL: return "FINALIZANDO_MANUAL";
	case MEDSAF_END: return "END";
	default: return "ESTADO_INVALIDO";
	}
}

static const char *Medsaf_get_nextion_modo_text(void)
{
	switch (medsaf_state)
	{
	case MEDSAF_INIT: return "Iniciando lavado...";
	case MEDSAF_PREPARE_TANK: return "Preparando bacha...";
	case MEDSAF_PRE_WASH: return "Prelavado en curso...";
	case MEDSAF_WASH: return "Lavado en curso...";
	case MEDSAF_RINSE: return "Enjuagando...";
	case MEDSAF_FINAL_RINSE: return "Enjuague final...";
	case MEDSAF_COOLING: return "Enfriando...";
	case MEDSAF_FINALIZAR_MANUAL: return "Finalizando...";
	case MEDSAF_END: return "Finalizado";
	default: return "Estado invalido";
	}
}

static const char *Medsaf_get_substate_text(void)
{
	if (medsaf_error_puerta)
		return "ERROR_PUERTA_ABIERTA";
	if (medsaf_error_nivel_maximo)
		return "ERROR_NIVEL_MAXIMO";
	if (medsaf_error_nivel_bacha)
		return "ERROR_SENSOR_NIVEL";
	if (medsaf_error_llenado_timeout)
		return "ERROR_TIMEOUT_LLENADO";
	if (medsaf_error_drenaje_timeout)
		return "ERROR_TIMEOUT_DRENAJE";
	if (medsaf_error_calefaccion_timeout)
		return "ERROR_TIMEOUT_CALEFACCION";
	if (medsaf_error_calefactor)
		return "ERROR_CALEFACTOR";
	if (medsaf_error_termocuplas)
		return "ERROR_TERMOCUPLAS";
	if (medsaf_error_sobretemperatura)
		return "ERROR_SOBRETEMPERATURA";
	if (medsaf_error_temperatura)
		return "ERROR_SENSOR_TEMPERATURA";
	if (medsaf_fsm_paused)
		return "PAUSADO";
	if (medsaf_state == MEDSAF_END)
		return "FINALIZADO";
	if (medsaf_state == MEDSAF_FINALIZAR_MANUAL)
	{
		switch (medsaf_finish_state)
		{
		case MEDSAF_FINISH_DRENAR: return "FINALIZAR_DRENAR";
		case MEDSAF_FINISH_DRENAJE_EXTRA: return "FINALIZAR_DRENAJE_EXTRA";
		case MEDSAF_FINISH_FIN: return "FINALIZAR_FIN";
		default: return "FINALIZAR_ESTADO_INVALIDO";
		}
	}

	if (medsaf_state == MEDSAF_PRE_WASH)
	{
		switch (medsaf_pre_wash_state)
		{
		case MEDSAF_PRE_WASH_INICIO: return "PRE_WASH_INICIO";
		case MEDSAF_PRE_WASH_LLENAR_FRIA: return "PRE_WASH_LLENAR_FRIA";
		case MEDSAF_PRE_WASH_COMPLETAR_LLENADO: return "PRE_WASH_COMPLETAR_LLENADO";
		case MEDSAF_PRE_WASH_LAVANDO: return "PRE_WASH_LAVANDO";
		case MEDSAF_PRE_WASH_DRENAR: return "PRE_WASH_DRENAR";
		case MEDSAF_PRE_WASH_DRENAJE_EXTRA: return "PRE_WASH_DRENAJE_EXTRA";
		case MEDSAF_PRE_WASH_FIN: return "PRE_WASH_FIN";
		default: return "PRE_WASH_ESTADO_INVALIDO";
		}
	}

	if (medsaf_state == MEDSAF_PREPARE_TANK)
	{
		switch (medsaf_prepare_tank_state)
		{
		case MEDSAF_PREPARE_TANK_INICIO: return "PREPARE_TANK_INICIO";
		case MEDSAF_PREPARE_TANK_DRENAR: return "PREPARE_TANK_DRENAR";
		case MEDSAF_PREPARE_TANK_DRENAJE_EXTRA: return "PREPARE_TANK_DRENAJE_EXTRA";
		case MEDSAF_PREPARE_TANK_FIN: return "PREPARE_TANK_FIN";
		default: return "PREPARE_TANK_ESTADO_INVALIDO";
		}
	}

	if (medsaf_state == MEDSAF_WASH)
	{
		switch (medsaf_wash_state)
		{
		case MEDSAF_WASH_INICIO: return "WASH_INICIO";
		case MEDSAF_WASH_LLENAR_MEDIUM: return "WASH_LLENAR_MEDIUM";
		case MEDSAF_WASH_DOSIFICAR_DETERGENTE: return "WASH_DOSIFICAR_DETERGENTE";
		case MEDSAF_WASH_COMPLETAR_LLENADO: return "WASH_COMPLETAR_LLENADO";
		case MEDSAF_WASH_LAVANDO: return "WASH_LAVANDO";
		case MEDSAF_WASH_ULTRASONIDO_ON: return "WASH_ULTRASONIDO_ON";
		case MEDSAF_WASH_ULTRASONIDO_OFF: return "WASH_ULTRASONIDO_OFF";
		case MEDSAF_WASH_DRENAR: return "WASH_DRENAR";
		case MEDSAF_WASH_DRENAJE_EXTRA: return "WASH_DRENAJE_EXTRA";
		case MEDSAF_WASH_FIN: return "WASH_FIN";
		default: return "WASH_ESTADO_INVALIDO";
		}
	}

	if (medsaf_state == MEDSAF_RINSE)
	{
		switch (medsaf_rinse_state)
		{
		case MEDSAF_RINSE_INICIO: return "RINSE_INICIO";
		case MEDSAF_RINSE_LLENAR_MEDIUM: return "RINSE_LLENAR_MEDIUM";
		case MEDSAF_RINSE_COMPLETAR_LLENADO: return "RINSE_COMPLETAR_LLENADO";
		case MEDSAF_RINSE_CALENTANDO: return "RINSE_CALENTANDO";
		case MEDSAF_RINSE_LAVANDO: return "RINSE_LAVANDO";
		case MEDSAF_RINSE_DRENAR: return "RINSE_DRENAR";
		case MEDSAF_RINSE_DRENAJE_EXTRA: return "RINSE_DRENAJE_EXTRA";
		case MEDSAF_RINSE_FIN: return "RINSE_FIN";
		default: return "RINSE_ESTADO_INVALIDO";
		}
	}

	if (medsaf_state == MEDSAF_FINAL_RINSE)
	{
		switch (medsaf_final_rinse_state)
		{
		case MEDSAF_FINAL_RINSE_INICIO: return "FINAL_RINSE_INICIO";
		case MEDSAF_FINAL_RINSE_LLENAR_MEDIUM: return "FINAL_RINSE_LLENAR_MEDIUM";
		case MEDSAF_FINAL_RINSE_COMPLETAR_LLENADO: return "FINAL_RINSE_COMPLETAR_LLENADO";
		case MEDSAF_FINAL_RINSE_RECIRCULANDO: return "FINAL_RINSE_RECIRCULANDO";
		case MEDSAF_FINAL_RINSE_DRENAR: return "FINAL_RINSE_DRENAR";
		case MEDSAF_FINAL_RINSE_DRENAJE_EXTRA: return "FINAL_RINSE_DRENAJE_EXTRA";
		case MEDSAF_FINAL_RINSE_FIN: return "FINAL_RINSE_FIN";
		default: return "FINAL_RINSE_ESTADO_INVALIDO";
		}
	}

	if (medsaf_state == MEDSAF_COOLING)
	{
		switch (medsaf_cooling_state)
		{
		case MEDSAF_COOLING_INICIO: return "COOLING_INICIO";
		case MEDSAF_COOLING_LLENAR_MEDIUM: return "COOLING_LLENAR_MEDIUM";
		case MEDSAF_COOLING_ESPERANDO: return "COOLING_ESPERANDO";
		case MEDSAF_COOLING_DRENAR: return "COOLING_DRENAR";
		case MEDSAF_COOLING_DRENAJE_FINAL: return "COOLING_DRENAJE_FINAL";
		case MEDSAF_COOLING_DRENAJE_EXTRA: return "COOLING_DRENAJE_EXTRA";
		case MEDSAF_COOLING_FIN: return "COOLING_FIN";
		default: return "COOLING_ESTADO_INVALIDO";
		}
	}

	return Medsaf_get_state_text();
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
	medsaf_error_nivel_maximo = 0;
	medsaf_error_nivel_bacha = 0;
	medsaf_error_llenado_timeout = 0;
	medsaf_error_drenaje_timeout = 0;
	medsaf_error_calefaccion_timeout = 0;
	medsaf_error_calefactor = 0;
	medsaf_error_termocuplas = 0;
	medsaf_error_sobretemperatura = 0;
	medsaf_error_temperatura = 0;
	medsaf_sendstate_tic = 0;
	Medsaf_buzzer_off();
	Medsaf_fsm_prepare_tank_init();
	Medsaf_fsm_pre_wash_init();
	Medsaf_fsm_wash_init();
	Medsaf_fsm_rinse_init();
	Medsaf_fsm_final_rinse_init();
	Medsaf_fsm_cooling_init();

	MEDSAF_fsm_off_sistema();
	Medsaf_actualizar_nivel_bacha();
	Medsaf_control_puerta();
	Medsaf_control_nivel_maximo();
	if (medsaf_fsm_enable)
		Medsaf_ticket_iniciar();
}

MEDSAF_FSM_STATES_T Medsaf_fsm_get_state(void)
{
	return medsaf_state;
}

MEDSAF_PREPARE_TANK_STATES_T Medsaf_fsm_get_prepare_tank_state(void)
{
	return medsaf_prepare_tank_state;
}

MEDSAF_PRE_WASH_STATES_T Medsaf_fsm_get_pre_wash_state(void)
{
	return medsaf_pre_wash_state;
}

MEDSAF_WASH_STATES_T Medsaf_fsm_get_wash_state(void)
{
	return medsaf_wash_state;
}

MEDSAF_RINSE_STATES_T Medsaf_fsm_get_rinse_state(void)
{
	return medsaf_rinse_state;
}

MEDSAF_FINAL_RINSE_STATES_T Medsaf_fsm_get_final_rinse_state(void)
{
	return medsaf_final_rinse_state;
}

MEDSAF_COOLING_STATES_T Medsaf_fsm_get_cooling_state(void)
{
	return medsaf_cooling_state;
}

void Medsaf_fsm_pause(void)
{
	uint8_t medsaf_i;

	if (!medsaf_fsm_enable || medsaf_fsm_paused ||
		(medsaf_state == MEDSAF_END))
		return;

	for (medsaf_i = 0; medsaf_i < 16; medsaf_i++)
		medsaf_salidas_pausa[medsaf_i] = rm_output1[medsaf_i];

	medsaf_pre_wash_tic_pausa = medsaf_pre_wash_tic;
	medsaf_tapa_tic_pausa = medsaf_tapa_tic;
	medsaf_drenaje_extra_tic_pausa = medsaf_drenaje_extra_tic;
	medsaf_llenado_timeout_tic_pausa = medsaf_llenado_timeout_tic;
	medsaf_drenaje_timeout_tic_pausa = medsaf_drenaje_timeout_tic;
	medsaf_detergente_tic_pausa = medsaf_detergente_tic;
	medsaf_wash_tic_pausa = medsaf_wash_tic;
	medsaf_ultrasonido_total_tic_pausa = medsaf_ultrasonido_total_tic;
	medsaf_ultrasonido_pulso_tic_pausa = medsaf_ultrasonido_pulso_tic;
	medsaf_calefaccion_timeout_tic_pausa = medsaf_calefaccion_timeout_tic;
	medsaf_calefactor_control_tic_pausa = medsaf_calefactor_control_tic;
	medsaf_calefactor_muestra_tic_pausa = medsaf_calefactor_muestra_tic;
	medsaf_rinse_tic_pausa = medsaf_rinse_tic;
	medsaf_final_rinse_tic_pausa = medsaf_final_rinse_tic;
	medsaf_cooling_tic_pausa = medsaf_cooling_tic;
	medsaf_pre_wash_tic = 0;
	medsaf_tapa_tic = 0;
	medsaf_drenaje_extra_tic = 0;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_detergente_tic = 0;
	medsaf_wash_tic = 0;
	medsaf_ultrasonido_total_tic = 0;
	medsaf_ultrasonido_pulso_tic = 0;
	medsaf_calefaccion_timeout_tic = 0;
	medsaf_calefactor_control_tic = 0;
	medsaf_calefactor_muestra_tic = 0;
	medsaf_rinse_tic = 0;
	medsaf_final_rinse_tic = 0;
	medsaf_cooling_tic = 0;
	medsaf_fsm_enable = 0;
	medsaf_fsm_paused = 1;
	MEDSAF_fsm_off_sistema();
	fsm_off_buzzer();
	Nextion_ChangeState(REPRO1, Pausado_NextionState);
}

void Medsaf_fsm_continue(void)
{
	uint8_t medsaf_i;

	if (!medsaf_fsm_paused || medsaf_error_puerta ||
		medsaf_error_nivel_maximo || medsaf_error_nivel_bacha ||
		medsaf_error_llenado_timeout || medsaf_error_drenaje_timeout ||
		medsaf_error_calefaccion_timeout || medsaf_error_sobretemperatura ||
		medsaf_error_calefactor ||
		medsaf_error_termocuplas ||
		medsaf_error_temperatura ||
		!MEDSAF_SENSOR_PUERTA ||
		MEDSAF_NIVEL_BACHA_MAXIMO)
		return;

	for (medsaf_i = 0; medsaf_i < 16; medsaf_i++)
		rm_output1[medsaf_i] = medsaf_salidas_pausa[medsaf_i];

	medsaf_pre_wash_tic = medsaf_pre_wash_tic_pausa;
	medsaf_tapa_tic = medsaf_tapa_tic_pausa;
	medsaf_drenaje_extra_tic = medsaf_drenaje_extra_tic_pausa;
	medsaf_llenado_timeout_tic = medsaf_llenado_timeout_tic_pausa;
	medsaf_drenaje_timeout_tic = medsaf_drenaje_timeout_tic_pausa;
	medsaf_detergente_tic = medsaf_detergente_tic_pausa;
	medsaf_wash_tic = medsaf_wash_tic_pausa;
	medsaf_ultrasonido_total_tic = medsaf_ultrasonido_total_tic_pausa;
	medsaf_ultrasonido_pulso_tic = medsaf_ultrasonido_pulso_tic_pausa;
	medsaf_calefaccion_timeout_tic = medsaf_calefaccion_timeout_tic_pausa;
	medsaf_calefactor_control_tic = medsaf_calefactor_control_tic_pausa;
	medsaf_calefactor_muestra_tic = medsaf_calefactor_muestra_tic_pausa;
	medsaf_rinse_tic = medsaf_rinse_tic_pausa;
	medsaf_final_rinse_tic = medsaf_final_rinse_tic_pausa;
	medsaf_cooling_tic = medsaf_cooling_tic_pausa;
	medsaf_fsm_paused = 0;
	medsaf_fsm_enable = 1;
	Nextion_ChangeState(REPRO1, Lavando_NextionState);
}

void Medsaf_fsm_finish(void)
{
	if ((!medsaf_ticket_activo && !medsaf_fsm_enable && !medsaf_fsm_paused) ||
		(medsaf_state == MEDSAF_END) ||
		(medsaf_state == MEDSAF_FINALIZAR_MANUAL))
		return;

	MEDSAF_fsm_off_sistema();
	Medsaf_buzzer_off();
	medsaf_pre_wash_tic = 0;
	medsaf_tapa_tic = 0;
	medsaf_drenaje_extra_tic = 0;
	medsaf_llenado_timeout_tic = 0;
	medsaf_drenaje_timeout_tic = 0;
	medsaf_detergente_tic = 0;
	medsaf_wash_tic = 0;
	medsaf_ultrasonido_total_tic = 0;
	medsaf_ultrasonido_pulso_tic = 0;
	medsaf_calefaccion_timeout_tic = 0;
	Medsaf_calefactor_monitor_stop();
	medsaf_rinse_tic = 0;
	medsaf_final_rinse_tic = 0;
	medsaf_cooling_tic = 0;
	medsaf_fsm_paused = 0;
	medsaf_fsm_enable = 1;
	medsaf_finish_state = MEDSAF_FINISH_DRENAR;
	medsaf_drenaje_timeout_tic = MEDSAF_TIMEOUT_DRENAJE;
	MEDSAF_fsm_on_drenar_bacha();
	Medsaf_cambiar_estado(MEDSAF_FINALIZAR_MANUAL);
	enqueue_nextion_command("data.L1_on_modo.txt=\"Finalizando...\"");
	Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Ocultar_NextionButtons);
}

void Medsaf_sendstate_runcycle(void)
{
	char medsaf_buffer[100];

	if (medsaf_sendstate_tic)
		return;

	medsaf_sendstate_tic = MEDSAF_TIME_SENDSTATE;
	snprintf(medsaf_buffer, sizeof(medsaf_buffer),
			 "data.L1_on_modo.txt=\"%s\"", Medsaf_get_nextion_modo_text());
	enqueue_nextion_command(medsaf_buffer);
	snprintf(medsaf_buffer, sizeof(medsaf_buffer),
			 "data.L1_state.txt=\"%s\"", Medsaf_get_state_text());
	enqueue_nextion_command(medsaf_buffer);
	snprintf(medsaf_buffer, sizeof(medsaf_buffer),
			 "data.L1_substate.txt=\"%s\"", Medsaf_get_substate_text());
	enqueue_nextion_command(medsaf_buffer);
}

void Medsaf_Nextion_Send_NivelBacha(void)
{
	switch (medsaf_nivel_bacha)
	{
	case MEDSAF_BACHA_EMPTY:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"empty\"");
		break;
	case MEDSAF_BACHA_LOW:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"low\"");
		break;
	case MEDSAF_BACHA_MEDIUM:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"medium\"");
		break;
	case MEDSAF_BACHA_HIGH:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"high\"");
		break;
	case MEDSAF_BACHA_FULL:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"full\"");
		break;
	case MEDSAF_BACHA_NIVEL_ERROR:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"error\"");
		break;
	default:
		break;
	}
}

void Medsaf_fsm_runcycle(void)
{
	Medsaf_buzzer_runcycle();
	Medsaf_actualizar_nivel_bacha();

	/* En END o luego de un error solo queda activo el servicio del buzzer. */
	if (!medsaf_fsm_enable && !medsaf_fsm_paused)
		return;

	/* La puerta se controla antes de ejecutar cualquier estado del proceso. */
	if (!Medsaf_control_puerta())
		return;

	/* FULL es un nivel de seguridad y nunca un nivel normal de llenado. */
	if (!Medsaf_control_nivel_maximo())
		return;

	if (!Medsaf_control_nivel_invalido())
		return;

	if (!medsaf_fsm_enable || medsaf_fsm_paused)
		return;

	switch (medsaf_state)
	{
	case MEDSAF_INIT:
		MEDSAF_fsm_off_sistema();
		Medsaf_fsm_prepare_tank_init();
		Medsaf_cambiar_estado(MEDSAF_PREPARE_TANK);
		break;

	case MEDSAF_PREPARE_TANK:
		Medsaf_fsm_prepare_tank_runcycle();
		break;

	case MEDSAF_PRE_WASH:
		Medsaf_fsm_pre_wash_runcycle();
		break;

	case MEDSAF_WASH:
		Medsaf_fsm_wash_runcycle();
		break;

	case MEDSAF_RINSE:
		Medsaf_fsm_rinse_runcycle();
		break;

	case MEDSAF_FINAL_RINSE:
		Medsaf_fsm_final_rinse_runcycle();
		break;

	case MEDSAF_COOLING:
		Medsaf_fsm_cooling_runcycle();
		break;

	case MEDSAF_FINALIZAR_MANUAL:
		Medsaf_fsm_finish_runcycle();
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
