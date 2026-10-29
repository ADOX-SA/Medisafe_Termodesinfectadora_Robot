/*
 * Medisafe.h
 *
 * Maquina de estados supervisora de Medisafe.
 */

#ifndef INC_MEDISAFE_H_
#define INC_MEDISAFE_H_

#include <stdint.h>

/* Duracion total del lavado durante PRE_WASH, en milisegundos. */
#define MEDSAF_TIME_PRE_WASH 30000
#define MEDSAF_TEMP_PRE_WASH 35.0f

/* Periodo de alternancia del solenoide de entrada de agua a la tapa. */
#define MEDSAF_TIME_TAPA_SPRAY 10000
#define MEDSAF_TIME_LUMEN_FLUSH 5000

/* Drenaje adicional luego de detectar la bacha vacia. */
#define MEDSAF_TIME_DRENAJE_EXTRA 30000

/* Periodo de envio de estado y subestado a Nextion. */
#define MEDSAF_TIME_SENDSTATE 1000

/* Tiempos del buzzer, siguiendo el patron de ENDOCLEAN2000. */
#define MEDSAF_TIME_BUZZER_ERROR 1000
#define MEDSAF_TIME_BUZZER_FIN 2000

/* Tiempos del proceso WASH, en milisegundos. */
#define MEDSAF_TIME_DETERGENTE 10000
#define MEDSAF_TEMP_WASH 40.0f
#define MEDSAF_TIME_DEGAS 30000
#define MEDSAF_TIME_WASH 30000
#define MEDSAF_TIME_ULTRASONIDO_ON 10000
#define MEDSAF_TIME_ULTRASONIDO_OFF 5000
#define MEDSAF_TIME_WASH_ULTRASONIDO 10000
#define MEDSAF_TIME_WASH_LUMEN 5000

/* Tiempos maximos de seguridad para movimientos de liquido. */
#define MEDSAF_TIMEOUT_LLENADO 300000
#define MEDSAF_TIMEOUT_DRENAJE 300000

/* Tiempos y limites del proceso RINSE. */
#define MEDSAF_TEMP_RINSE 60.0f
#define MEDSAF_TIME_RINSE 30000
#define MEDSAF_TIMEOUT_CALEFACCION 600000
#define MEDSAF_MARGEN_SOBRETEMPERATURA 5
#define MEDSAF_TIME_MUESTRA_CALEFACTOR 1000
#define MEDSAF_MUESTRAS_PROMEDIO_CALEFACTOR 5
#define MEDSAF_DIFERENCIA_MAX_TERMOCUPLAS 5.0f
#define MEDSAF_TEMP_CAMBIO_CONTROL_CALEFACTOR 70.0f
#define MEDSAF_TIMEOUT_CALEFACTOR_BAJA_TEMP 240000
#define MEDSAF_STEP_CALEFACTOR_BAJA_TEMP 1.0f
#define MEDSAF_TIMEOUT_CALEFACTOR_ALTA_TEMP 360000
#define MEDSAF_STEP_CALEFACTOR_ALTA_TEMP 0.5f

/* Requisitos para comprobar que el calefactor aumenta la temperatura. */
typedef struct
{
	uint32_t timeout;
	float step_temperatura;
} MEDSAF_CALEFACTOR_CONTROL_T;

/* Duracion de la recirculacion durante FINAL_RINSE. */
#define MEDSAF_TIME_FINAL_RINSE 30000

/* Parametros P6 usados temporalmente para probar DISINFECTION. */
#define MEDSAF_TEMP_DISINFECTION_STD 90.0f
#define MEDSAF_TEMP_DISINFECTION_RBT 91.0f
#define MEDSAF_TIME_DISINFECTION_STD 30000
#define MEDSAF_TIME_DISINFECTION_RBT 30000
#define MEDSAF_TIMEOUT_DISINFECTION_HEATING 1800000
#define MEDSAF_HISTERESIS_DISINFECTION 1.0f

/* Tiempo de espera por ciclo y temperatura objetivo de enfriamiento. */
#define MEDSAF_TIME_COOLING 30000
#define MEDSAF_TEMP_COOLING 60
#define MEDSAF_TIMEOUT_COOLING 600000

/* Variante seleccionada para los parametros del programa P1...P6. */
typedef enum
{
	MEDSAF_MODO_ESTANDAR = 0,
	MEDSAF_MODO_ROBOT
} MEDSAF_MODO_T;

/* Programa seleccionado desde la pantalla Nextion. */
typedef enum
{
	MEDSAF_PROGRAMA_P1 = 1,
	MEDSAF_PROGRAMA_P2,
	MEDSAF_PROGRAMA_P3,
	MEDSAF_PROGRAMA_P4,
	MEDSAF_PROGRAMA_P5,
	MEDSAF_PROGRAMA_P6
} MEDSAF_PROGRAMA_T;

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
