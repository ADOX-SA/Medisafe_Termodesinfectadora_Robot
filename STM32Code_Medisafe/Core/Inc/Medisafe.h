/*
 * Medisafe.h
 *
 * Maquina de estados supervisora de Medisafe.
 */

#ifndef INC_MEDISAFE_H_
#define INC_MEDISAFE_H_

#include <stdint.h>
#include <stdbool.h>
#include "Medisafe_Mode_Config.h"

/* Estados principales de la FSM Medisafe. */
typedef enum
{
	MEDSAF_INIT = 0,
	MEDSAF_PREPARE_TANK,
	MEDSAF_PRE_WASH,
	MEDSAF_WASH,
	MEDSAF_RINSE,
	MEDSAF_FINAL_RINSE,
	MEDSAF_DISINFECTION,
	MEDSAF_COOLING,
	MEDSAF_FINALIZAR_MANUAL,
	MEDSAF_END
} MEDSAF_FSM_STATES_T;

/* Subestados usados al finalizar manualmente desde la pantalla. */
typedef enum
{
	MEDSAF_FINISH_DRENAR = 0,
	MEDSAF_FINISH_DRENAJE_EXTRA,
	MEDSAF_FINISH_FIN
} MEDSAF_FINISH_STATES_T;

/* Subestados comunes previos al inicio de cualquier programa P1...P6. */
typedef enum
{
	MEDSAF_PREPARE_TANK_INICIO = 0,
	MEDSAF_PREPARE_TANK_DRENAR,
	MEDSAF_PREPARE_TANK_DRENAJE_EXTRA,
	MEDSAF_PREPARE_TANK_FIN
} MEDSAF_PREPARE_TANK_STATES_T;

/* Subestados del proceso PRE_WASH. */
typedef enum
{
	MEDSAF_PRE_WASH_INICIO = 0,
	MEDSAF_PRE_WASH_LLENAR_FRIA,
	MEDSAF_PRE_WASH_COMPLETAR_LLENADO,
	MEDSAF_PRE_WASH_CALENTANDO,
	MEDSAF_PRE_WASH_LAVANDO,
	MEDSAF_PRE_WASH_DRENAR,
	MEDSAF_PRE_WASH_DRENAJE_EXTRA,
	MEDSAF_PRE_WASH_FIN
} MEDSAF_PRE_WASH_STATES_T;

/* Subestados del proceso WASH. */
typedef enum
{
	MEDSAF_WASH_INICIO = 0,
	MEDSAF_WASH_LLENAR_MEDIUM,
	MEDSAF_WASH_DOSIFICAR_DETERGENTE,
	MEDSAF_WASH_COMPLETAR_LLENADO,
	MEDSAF_WASH_CALENTANDO,
	MEDSAF_WASH_LAVANDO,
	MEDSAF_WASH_DEGAS_ON,
	MEDSAF_WASH_DEGAS_OFF,
	MEDSAF_WASH_ULTRASONIDO_ON,
	MEDSAF_WASH_ULTRASONIDO_OFF,
	MEDSAF_WASH_DRENAR,
	MEDSAF_WASH_DRENAJE_EXTRA,
	MEDSAF_WASH_FIN
} MEDSAF_WASH_STATES_T;

/* Subestados del proceso RINSE. */
typedef enum
{
	MEDSAF_RINSE_INICIO = 0,
	MEDSAF_RINSE_LLENAR_MEDIUM,
	MEDSAF_RINSE_COMPLETAR_LLENADO,
	MEDSAF_RINSE_CALENTANDO,
	MEDSAF_RINSE_LAVANDO,
	MEDSAF_RINSE_DRENAR,
	MEDSAF_RINSE_DRENAJE_EXTRA,
	MEDSAF_RINSE_FIN
} MEDSAF_RINSE_STATES_T;

/* Subestados del proceso FINAL_RINSE. */
typedef enum
{
	MEDSAF_FINAL_RINSE_INICIO = 0,
	MEDSAF_FINAL_RINSE_LLENAR_MEDIUM,
	MEDSAF_FINAL_RINSE_COMPLETAR_LLENADO,
	MEDSAF_FINAL_RINSE_RECIRCULANDO,
	MEDSAF_FINAL_RINSE_DRENAR,
	MEDSAF_FINAL_RINSE_DRENAJE_EXTRA,
	MEDSAF_FINAL_RINSE_FIN
} MEDSAF_FINAL_RINSE_STATES_T;

/* Subestados del proceso DISINFECTION. */
typedef enum
{
	MEDSAF_DISINFECTION_INICIO = 0,
	MEDSAF_DISINFECTION_LLENAR,
	MEDSAF_DISINFECTION_CALENTANDO,
	MEDSAF_DISINFECTION_RECIRCULANDO,
	MEDSAF_DISINFECTION_DRENAR,
	MEDSAF_DISINFECTION_DRENAJE_EXTRA,
	MEDSAF_DISINFECTION_FIN
} MEDSAF_DISINFECTION_STATES_T;

/* Subestados del proceso COOLING. */
typedef enum
{
	MEDSAF_COOLING_INICIO = 0,
	MEDSAF_COOLING_LLENAR_MEDIUM,
	MEDSAF_COOLING_ESPERANDO,
	MEDSAF_COOLING_DRENAR,
	MEDSAF_COOLING_DRENAJE_FINAL,
	MEDSAF_COOLING_DRENAJE_EXTRA,
	MEDSAF_COOLING_FIN
} MEDSAF_COOLING_STATES_T;

/*
 * Nivel calculado de la bacha.
 * Los sensores deben activarse en orden: bajo, medio, alto y maximo.
 */
typedef enum
{
	MEDSAF_BACHA_EMPTY = 0,
	MEDSAF_BACHA_LOW,
	MEDSAF_BACHA_MEDIUM,
	MEDSAF_BACHA_HIGH,
	MEDSAF_BACHA_FULL,
	MEDSAF_BACHA_NIVEL_ERROR
} MEDSAF_NIVEL_BACHA_T;

extern uint8_t medsaf_fsm_enable;
extern uint8_t medsaf_fsm_paused;
extern uint8_t medsaf_error_puerta;
extern uint8_t medsaf_error_nivel_maximo;
extern uint8_t medsaf_error_nivel_bacha;
extern uint8_t medsaf_error_llenado_timeout;
extern uint8_t medsaf_error_drenaje_timeout;
extern uint8_t medsaf_error_calefaccion_timeout;
extern uint8_t medsaf_error_calefactor;
extern uint8_t medsaf_error_termocuplas;
extern uint8_t medsaf_error_sobretemperatura;
extern uint8_t medsaf_error_temperatura;
extern MEDSAF_MODO_T medsaf_modo;
extern MEDSAF_PROGRAMA_T medsaf_programa;
extern MEDSAF_NIVEL_BACHA_T medsaf_nivel_bacha;
extern bool medsaf_buzzer_transiciones_enable;

/* Temporizadores descendentes atendidos cada 1 ms por SystemTicks_1ms(). */
extern uint32_t medsaf_pre_wash_tic;
extern uint32_t medsaf_tapa_tic;
extern uint32_t medsaf_drenaje_extra_tic;
extern uint32_t medsaf_sendstate_tic;
extern uint32_t medsaf_llenado_timeout_tic;
extern uint32_t medsaf_drenaje_timeout_tic;
extern uint32_t medsaf_detergente_tic;
extern uint32_t medsaf_wash_tic;
extern uint32_t medsaf_degas_tic;
extern uint32_t medsaf_ultrasonido_total_tic;
extern uint32_t medsaf_ultrasonido_pulso_tic;
extern uint32_t medsaf_calefaccion_timeout_tic;
extern uint32_t medsaf_calefactor_control_tic;
extern uint32_t medsaf_calefactor_muestra_tic;
extern uint32_t medsaf_rinse_tic;
extern uint32_t medsaf_final_rinse_tic;
extern uint32_t medsaf_disinfection_tic;
extern uint32_t medsaf_cooling_tic;
extern uint32_t medsaf_cooling_timeout_tic;
extern uint32_t medsaf_buzzer_tic;

void Medsaf_fsm_init(void);
void Medsaf_fsm_runcycle(void);
void Medsaf_fsm_pause(void);
void Medsaf_fsm_continue(void);
void Medsaf_fsm_finish(void);
void Medsaf_actualizar_nivel_bacha(void);
MEDSAF_NIVEL_BACHA_T Medsaf_get_nivel_bacha(void);
MEDSAF_FSM_STATES_T Medsaf_fsm_get_state(void);
MEDSAF_PREPARE_TANK_STATES_T Medsaf_fsm_get_prepare_tank_state(void);
MEDSAF_PRE_WASH_STATES_T Medsaf_fsm_get_pre_wash_state(void);
MEDSAF_WASH_STATES_T Medsaf_fsm_get_wash_state(void);
MEDSAF_RINSE_STATES_T Medsaf_fsm_get_rinse_state(void);
MEDSAF_FINAL_RINSE_STATES_T Medsaf_fsm_get_final_rinse_state(void);
MEDSAF_DISINFECTION_STATES_T Medsaf_fsm_get_disinfection_state(void);
MEDSAF_COOLING_STATES_T Medsaf_fsm_get_cooling_state(void);
void Medsaf_sendstate_runcycle(void);
void Medsaf_Nextion_Send_NivelBacha(void);

#endif /* INC_MEDISAFE_H_ */
