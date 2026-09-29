#include "main.h"
#include "relay_module.h"
#include "global_variables.h"
#include "nextion.h"
#include "ticket.h"
#include "user_gpio.h"
#include "fsm_reprocesadora1.h"
#include "ENDOCLEAN2000.h"
#include <stdio.h>

void fsm_off_sistema(void);

#define EC2000_SENSOR_NIVEL_DESINF_1 RM1_IN1
#define EC2000_SENSOR_NIVEL_DESINF_2 RM1_IN2
#define EC2000_SENSOR_NIVEL_DESINF_3 RM1_IN3
#define EC2000_SENSOR_NIVEL_DESINF_4 RM1_IN4

#define EC2000_NIVEL_BACHA_MEDIO RM1_IN9
#define EC2000_NIVEL_BACHA_ALTO (!RM1_IN10) //----> Usa logica invertida (el flotante esta dado vuelta)
#define EC2000_NIVEL_BACHA_BAJO RM1_IN6
#define EC2000_SENSOR_IMAN_TAPA RM1_IN8
#define EC2000_PEDAL RM1_IN7


#define COMP RM1_OUT1
#define ACOL RM1_OUT2
#define SELF RM1_OUT3
#define AIR_S RM1_OUT4
#define DET RM1_OUT5
#define WAT_1 RM1_OUT6
#define WAT_2 RM1_OUT7
#define LEA_A RM1_OUT8
#define LEA_W RM1_OUT9
#define AIR RM1_OUT10
#define DRAIN RM1_OUT11
#define DIS RM1_OUT12
#define KLD_1 RM1_OUT13
#define KLD_2 RM1_OUT14
#define SOLE RM1_OUT15
#define VACIO RM1_OUT16

#define COMP_ON RM1_OUT1=1;
#define ACOL_ON RM1_OUT2=1;
#define SELF_ON RM1_OUT3=1;
#define AIR_S_ON RM1_OUT4=1;
#define DET_ON RM1_OUT5=1;
#define WAT_1_ON RM1_OUT6=1;
#define WAT_2_ON RM1_OUT7=1;
#define LEA_A_ON RM1_OUT8=1;
#define LEA_W_ON RM1_OUT9=1;
#define AIR_ON RM1_OUT10=1;
#define DRAIN_ON RM1_OUT11=1; //Bomba Drenaje
#define DIS_ON RM1_OUT12=1;
#define KLD_DRENAR {RM1_OUT13=0;RM1_OUT14=1;}
#define KLD_BIDON {RM1_OUT13=1;RM1_OUT14=0;}
#define SOLE_ON RM1_OUT15=1;
#define VACIO_ON RM1_OUT16=1;

#define COMP_OFF RM1_OUT1=0;
#define ACOL_OFF RM1_OUT2=0;
#define SELF_OFF RM1_OUT3=0;
#define AIR_S_OFF RM1_OUT4=0;
#define DET_OFF RM1_OUT5=0;
#define WAT_1_OFF RM1_OUT6=0;
#define WAT_2_OFF RM1_OUT7=0;
#define LEA_A_OFF RM1_OUT8=0;
#define LEA_W_OFF RM1_OUT9=0;
#define AIR_OFF RM1_OUT10=0;
#define DRAIN_OFF RM1_OUT11=0;
#define DIS_OFF RM1_OUT12=0;
#define KLD_CERRAR {RM1_OUT13=1;RM1_OUT14=1;}
#define SOLE_OFF RM1_OUT15=0;
#define VACIO_OFF RM1_OUT16=0;

/*--------------------------------------------------------------------------------------------*/
/*          ESTADOS                                                                           */
/*--------------------------------------------------------------------------------------------*/

typedef enum
{
	L_IDLE,
	L_CORROBORAR_BACHA_VACIA,
	L_LAVAR_CON_DRENAJE_ABIERTO,
	L_ESPERAR_NIVEL_BAJO,
	L_ESPERAR_NIVEL_MEDIO,
	L_DRENAR_BACHA,
	L_ERROR_FALTA_AGUA,
	L_ERROR_TAPA_ABIERTA
} L_FSM_LAVADO_STATES_T;

typedef enum
{
	DET_IDLE,
	DET_INICIO,
	DET_DOSIFICANDO,
	DET_DRENAR_BACHA,
	DET_ESPERAR_VACIADO,
	DET_ERROR_TAPA_ABIERTA
} DET_FSM_STATES_T;

typedef enum
{
	A_IDLE,
	A_INICIO,
	A_SECANDO,
	A_DRENAR_FINAL
} A_FSM_STATES_T;

typedef enum
{
	DES_IDLE,
	DES_INICIO,
	DES_DRENAJE_INICIAL,
	DES_LLENAR_BACHA,
	DES_ESPERAR_TIEMPO,
	DES_RETORNAR_BIDON
} DES_FSM_STATES_T;

typedef enum
{
	ALC_IDLE,
	ALC_DOSIFICANDO
} ALC_FSM_STATES_T;

typedef enum
{
	C_IDLE,
	C_INICIO,
	C_LAVADO_1,
	C_DETERGENTE,
	C_LAVADO_2,
	C_AIRE_1,
	C_DESINFECTANTE,
	C_AIRE_2,
	C_LAVADO_3,
	C_ALCOHOL,
	C_AIRE_3,
	C_FIN
} C_FSM_STATES_T;

typedef enum
{
	EC2000_BIDON_EMPTY,
	EC2000_BIDON_LOW,
	EC2000_BIDON_MEDIUM,
	EC2000_BIDON_HIGH,
	EC2000_BIDON_FULL,
	EC2000_BIDON_NIVEL_ERROR
} EC2000_NIVEL_BIDON_T;

typedef enum
{
	EC2000_BACHA_EMPTY,
	EC2000_BACHA_LOW,
	EC2000_BACHA_MEDIUM,
	EC2000_BACHA_HIGH,
	EC2000_BACHA_FULL,
	EC2000_BACHA_NIVEL_ERROR
} EC2000_NIVEL_BACHA_T;

typedef enum
{
	EC2000_DRS_INICIO,
	EC2000_DRS_ESPERAR_INICIO,
	EC2000_DRS_DESAGUE,
	EC2000_DRS_POST_DRENAJE,
	EC2000_DRS_TAPA_ABIERTA
} EC2000_DRAIN_START_T;

typedef enum
{
	EC2000_DOS_INICIO,
	EC2000_DOS_ESPERAR_INICIO,
	EC2000_DOS_BIDON_NO_HIGH,
	EC2000_DOS_CONFIRMACION,
	EC2000_DOS_BIDON_HIGH,
	EC2000_DOS_POST_DRENAJE
} EC2000_DOSING_START_T;

typedef enum
{
	EC2000_CRB_INICIO,
	EC2000_CRB_TAPA_ABIERTA,
	EC2000_CRB_DESINFECTANTE,
	EC2000_CRB_ESPERAR_NUEVO_LAVADO,
	EC2000_CRB_LLENAR_BACHA,
	EC2000_CRB_FALTA_AGUA,
	EC2000_CRB_ESPERAR_BIDON,
	EC2000_CRB_BIDON,
	EC2000_CRB_POST_BIDON,
	EC2000_CRB_ESPERAR_DESAGUE,
	EC2000_CRB_DESAGUE,
	EC2000_CRB_POST_DESAGUE,
	EC2000_CRB_STOP,
	EC2000_CRB_VACIAR
} EC2000_CLEANING_RECYCLE_BOX_T;

typedef enum
{
	EC2000_IDLE,
	EC2000_PREPARAR_BACHA,
	EC2000_LAVADO,
	EC2000_DETERGENTE,
	EC2000_AIRE,
	EC2000_DESINFECTANTE,
	EC2000_ALCOHOL,
	EC2000_COMPLETO,
	EC2000_PISTOLA_AIRE,
	EC2000_DRENAJE_MANUAL,
	EC2000_ESTANQUEIDAD,
	EC2000_DRAIN_START,
	EC2000_DOSING_START,
	EC2000_CLEANING_RECYCLE_BOX,
	EC2000_ERROR_FALTA_DESINFECTANTE,
	EC2000_ERROR_SENSOR_NIVEL_BACHA,
	EC2000_ERROR_NIVEL_MAXIMO,
	EC2000_FIN_PROCESO,
	EC2000_FINALIZAR_MANUAL
} EC2000_FSM_STATES_T;

typedef enum
{
	EST_INICIO,
	EST_ESPERAR_NIVEL_BAJO,
	EST_ESPERAR_NIVEL_MEDIO,
	EST_ESPERAR_TIEMPO,
	EST_DRENAR_BACHA,
	EST_ERROR_FALTA_AGUA
} EST_FSM_STATES_T;

/*--------------------------------------------------------------------------------------------*/
/*          VARIABLES Y TIMER                                                                 */
/*--------------------------------------------------------------------------------------------*/

static L_FSM_LAVADO_STATES_T L_state;
static DET_FSM_STATES_T DET_state;
static A_FSM_STATES_T A_state;
static DES_FSM_STATES_T DES_state;
static ALC_FSM_STATES_T ALC_state;
static C_FSM_STATES_T C_state;
static EC2000_FSM_STATES_T EC2000_state;
static EC2000_FSM_STATES_T EC2000_proximo_state;
static EST_FSM_STATES_T EST_state;
static EC2000_NIVEL_BIDON_T EC2000_nivel_bidon;
static EC2000_NIVEL_BACHA_T EC2000_nivel_bacha;
static EC2000_DRAIN_START_T EC2000_drain_state;
static EC2000_DOSING_START_T EC2000_dosing_state;
static EC2000_CLEANING_RECYCLE_BOX_T EC2000_clean_box_state;

uint8_t EC2000_lavado_init;
uint8_t EC2000_detergente_init;
uint8_t EC2000_aire_init;
uint8_t EC2000_desinfectante_init;
uint8_t EC2000_completo_init;
uint8_t EC2000_alcohol_init;
uint8_t EC2000_fsm_enable;
uint8_t EC2000_fsm_paused;

static uint8_t EC2000_salidas_pausa[16];
static uint32_t EC2000_lavado_tic_pausa;
static uint32_t EC2000_llenado_tic_pausa;
static uint32_t EC2000_buzzer_tic_pausa;
static uint32_t EC2000_aire_tic_pausa;
static uint32_t EC2000_pistola_aire_tic_pausa;
static uint32_t EC2000_drenaje_manual_tic_pausa;
static uint32_t EC2000_estanq_tic_pausa;
static uint32_t EC2000_detergente_tic_pausa;
static uint32_t EC2000_desinfectante_tic_pausa;
static uint32_t EC2000_alcohol_tic_pausa;
static uint32_t EC2000_mantenimiento_tic_pausa;
static uint32_t EC2000_mantenimiento_timeout_tic_pausa;
static uint32_t EC2000_vaciado_bacha_tic_pausa;
static L_FSM_LAVADO_STATES_T L_state_antes_error_tapa;
static DET_FSM_STATES_T DET_state_antes_error_tapa;
static uint32_t L_lavado_tic_antes_error_tapa;
static uint32_t L_llenado_tic_antes_error_tapa;
static uint32_t DET_tic_antes_error_tapa;

uint32_t EC2000_lavado_tic;
uint32_t EC2000_llenado_tic;
uint32_t EC2000_buzzer_tic;
uint32_t EC2000_aire_tic;
uint32_t EC2000_pistola_aire_tic;
uint32_t EC2000_drenaje_manual_tic;
uint32_t EC2000_estanq_tic;
uint32_t EC2000_puerta_pedal_tic;
uint32_t EC2000_detergente_tic;
uint32_t EC2000_desinfectante_tic;
uint32_t EC2000_alcohol_tic;
uint32_t EC2000_mantenimiento_tic;
uint32_t EC2000_mantenimiento_timeout_tic;
uint32_t EC2000_vaciado_bacha_tic;
uint32_t EC2000_sendstate_tic;
static uint8_t L_buzzer_pulsos;
static uint8_t EC2000_fin_pitidos;
static uint8_t EC2000_fin_buzzer_on;
static uint8_t EC2000_clean_box_ciclo;
static uint8_t EC2000_clean_box_nivel_bajo_detectado;
static uint8_t EC2000_vaciado_bacha_activo;
static uint8_t EC2000_post_vaciado_bacha_iniciado;
static uint8_t EC2000_completo_en_ejecucion;
static uint8_t EC2000_completo_etapa_finalizada;
static uint8_t EC2000_completo_alcohol_iniciado;
static uint8_t EC2000_vaciado_error_nivel_maximo;
static uint8_t EC2000_error_tapa_activo;
static uint8_t EC2000_error_inicio_sin_vaciado;
static uint8_t DES_nivel_inicial_evaluado;
static uint8_t EC2000_puerta_pedal_activa;
static uint8_t EC2000_pedal_bloqueado;
static uint8_t EC2000_ticket_activo;
static const char *EC2000_ticket_proceso;
static uint8_t EC2000_salidas_antes_error_tapa[16];
static uint32_t EC2000_lavado_tic_antes_error_tapa;
static uint32_t EC2000_llenado_tic_antes_error_tapa;
static uint32_t EC2000_aire_tic_antes_error_tapa;
static uint32_t EC2000_detergente_tic_antes_error_tapa;
static uint32_t EC2000_desinfectante_tic_antes_error_tapa;
static uint32_t EC2000_alcohol_tic_antes_error_tapa;
static uint32_t EC2000_estanq_tic_antes_error_tapa;
static uint32_t EC2000_vaciado_tic_antes_error_tapa;

static void fsm_on_puerta_sole(void);
static void fsm_off_puerta_sole(void);

static void EC2000_pedal_runcycle(void)
{
	uint8_t pedal_habilitado = (EC2000_state == EC2000_IDLE) ||
			(EC2000_state == EC2000_ESTANQUEIDAD) ||
			((EC2000_state == EC2000_PREPARAR_BACHA) &&
					(EC2000_proximo_state == EC2000_ESTANQUEIDAD));

	if (!pedal_habilitado || EC2000_lavado_init ||
			EC2000_detergente_init || EC2000_aire_init ||
			EC2000_desinfectante_init || EC2000_alcohol_init ||
			EC2000_completo_init || L1_botonDRS || L1_botonDOS || L1_botonCRB)
	{
		if (EC2000_puerta_pedal_activa)
			fsm_off_puerta_sole();
		EC2000_puerta_pedal_activa = 0;
		EC2000_puerta_pedal_tic = 0;
		return;
	}

	if (EC2000_puerta_pedal_activa)
	{
		if (!EC2000_puerta_pedal_tic)
		{
			fsm_off_puerta_sole();
			EC2000_puerta_pedal_activa = 0;
		}
		return;
	}

	if (!EC2000_PEDAL)
	{
		EC2000_pedal_bloqueado = 0;
		return;
	}

	if (!EC2000_pedal_bloqueado)
	{
		fsm_on_puerta_sole();
		EC2000_puerta_pedal_tic = EC2000_TIME_PUERTA_PEDAL;
		EC2000_puerta_pedal_activa = 1;
		EC2000_pedal_bloqueado = 1;
	}
}

static const char *EC2000_ticket_nombre_proceso(EC2000_FSM_STATES_T proceso)
{
	switch (proceso)
	{
	case EC2000_LAVADO: return "Lavado";
	case EC2000_DETERGENTE: return "Detergente";
	case EC2000_AIRE: return "Aire";
	case EC2000_DESINFECTANTE: return "Desinfeccion";
	case EC2000_ALCOHOL: return "Alcohol";
	case EC2000_COMPLETO: return "Lavado completo";
	case EC2000_ESTANQUEIDAD: return "Estanqueidad";
	default: return "ENDOCLEAN2000";
	}
}

static void EC2000_impresora_iniciar_proceso(EC2000_FSM_STATES_T proceso)
{
	EC2000_ticket_proceso = EC2000_ticket_nombre_proceso(proceso);
	Ticket_EnqHeader(REPRO1, "", "", "", "", EC2000_ticket_proceso);
	Ticket_EnqStepStartCached(REPRO1, EC2000_ticket_proceso);
	EC2000_ticket_activo = 1;
}

static void EC2000_impresora_finalizar_proceso(const char *resultado)
{
	if (!EC2000_ticket_activo)
		return;

	Ticket_EnqStepEndRtc(REPRO1, EC2000_ticket_proceso);
	Ticket_EnqFinish(REPRO1, resultado, "");
	Ticket_EnqPrint(REPRO1);
	EC2000_ticket_activo = 0;
}

static const char *EC2000_get_substate(void)
{
	switch (EC2000_state)
	{
	case EC2000_IDLE: return "IDLE";
	case EC2000_PREPARAR_BACHA: return "CORROBORAR_BACHA_VACIA";
	case EC2000_LAVADO:
		switch (L_state)
		{
		case L_IDLE: return "L_IDLE";
		case L_CORROBORAR_BACHA_VACIA: return "L_CORROBORAR_BACHA_VACIA";
		case L_LAVAR_CON_DRENAJE_ABIERTO: return "L_LAVAR_CON_DRENAJE_ABIERTO";
		case L_ESPERAR_NIVEL_BAJO: return "L_ESPERAR_NIVEL_BAJO";
		case L_ESPERAR_NIVEL_MEDIO: return "L_ESPERAR_NIVEL_MEDIO";
		case L_DRENAR_BACHA: return "L_DRENAR_BACHA";
		case L_ERROR_FALTA_AGUA: return "L_ERROR_FALTA_AGUA";
		case L_ERROR_TAPA_ABIERTA: return "L_ERROR_TAPA_ABIERTA";
		default: return "L_ESTADO_INVALIDO";
		}
		case EC2000_DETERGENTE:
			switch (DET_state)
			{
			case DET_IDLE: return "DET_IDLE";
			case DET_INICIO: return "DET_INICIO";
			case DET_DOSIFICANDO: return "DET_DOSIFICANDO";
			case DET_DRENAR_BACHA: return "DET_DRENAR_BACHA";
			case DET_ESPERAR_VACIADO: return "DET_ESPERAR_VACIADO";
			case DET_ERROR_TAPA_ABIERTA: return "DET_ERROR_TAPA_ABIERTA";
			default: return "DET_ESTADO_INVALIDO";
			}
			case EC2000_AIRE:
				switch (A_state)
				{
				case A_IDLE: return "A_IDLE";
				case A_INICIO: return "A_INICIO";
				case A_SECANDO: return "A_SECANDO";
				case A_DRENAR_FINAL: return "A_DRENAR_FINAL";
				default: return "A_ESTADO_INVALIDO";
				}
				case EC2000_DESINFECTANTE:
					switch (DES_state)
					{
					case DES_IDLE: return "DES_IDLE";
					case DES_INICIO: return "DES_INICIO";
					case DES_DRENAJE_INICIAL: return "DES_DRENAJE_INICIAL";
					case DES_LLENAR_BACHA: return "DES_LLENAR_BACHA";
					case DES_ESPERAR_TIEMPO: return "DES_ESPERAR_TIEMPO";
					case DES_RETORNAR_BIDON: return "DES_RETORNAR_BIDON";
					default: return "DES_ESTADO_INVALIDO";
					}
					case EC2000_ALCOHOL:
						switch (ALC_state)
						{
						case ALC_IDLE: return "ALC_INICIO";
						case ALC_DOSIFICANDO: return "ALC_DOSIFICANDO";
						default: return "ALC_ESTADO_INVALIDO";
						}
						case EC2000_COMPLETO:
							switch (C_state)
							{
							case C_IDLE: return "C_IDLE";
							case C_INICIO: return "C_INICIO";
							case C_LAVADO_1:
							case C_LAVADO_2:
							case C_LAVADO_3:
								switch (L_state)
								{
								case L_IDLE: return "L_IDLE";
								case L_CORROBORAR_BACHA_VACIA: return "L_CORROBORAR_BACHA_VACIA";
								case L_LAVAR_CON_DRENAJE_ABIERTO: return "L_LAVAR_CON_DRENAJE_ABIERTO";
								case L_ESPERAR_NIVEL_BAJO: return "L_ESPERAR_NIVEL_BAJO";
								case L_ESPERAR_NIVEL_MEDIO: return "L_ESPERAR_NIVEL_MEDIO";
								case L_DRENAR_BACHA: return "L_DRENAR_BACHA";
								case L_ERROR_FALTA_AGUA: return "L_ERROR_FALTA_AGUA";
								case L_ERROR_TAPA_ABIERTA: return "L_ERROR_TAPA_ABIERTA";
								default: return "L_ESTADO_INVALIDO";
								}
								case C_DETERGENTE:
									switch (DET_state)
									{
									case DET_IDLE: return "DET_IDLE";
									case DET_INICIO: return "DET_INICIO";
									case DET_DOSIFICANDO: return "DET_DOSIFICANDO";
									case DET_DRENAR_BACHA: return "DET_DRENAR_BACHA";
									case DET_ESPERAR_VACIADO: return "DET_ESPERAR_VACIADO";
									case DET_ERROR_TAPA_ABIERTA: return "DET_ERROR_TAPA_ABIERTA";
									default: return "DET_ESTADO_INVALIDO";
									}
									case C_AIRE_1:
									case C_AIRE_2:
									case C_AIRE_3:
										switch (A_state)
										{
										case A_IDLE: return "A_IDLE";
										case A_INICIO: return "A_INICIO";
										case A_SECANDO: return "A_SECANDO";
										case A_DRENAR_FINAL: return "A_DRENAR_FINAL";
										default: return "A_ESTADO_INVALIDO";
										}
										case C_DESINFECTANTE:
											switch (DES_state)
											{
											case DES_IDLE: return "DES_IDLE";
											case DES_INICIO: return "DES_INICIO";
											case DES_DRENAJE_INICIAL: return "DES_DRENAJE_INICIAL";
											case DES_LLENAR_BACHA: return "DES_LLENAR_BACHA";
											case DES_ESPERAR_TIEMPO: return "DES_ESPERAR_TIEMPO";
											case DES_RETORNAR_BIDON: return "DES_RETORNAR_BIDON";
											default: return "DES_ESTADO_INVALIDO";
											}
											case C_ALCOHOL: return "DOSIFICANDO_ALCOHOL";
											case C_FIN: return "C_FIN";
											default: return "C_ESTADO_INVALIDO";
							}
							case EC2000_PISTOLA_AIRE: return "COMPRESOR_ENCENDIDO";
							case EC2000_DRENAJE_MANUAL: return "DRENAJE_MANUAL";
							case EC2000_ESTANQUEIDAD:
								switch (EST_state)
								{
								case EST_INICIO: return "EST_INICIO";
								case EST_ESPERAR_NIVEL_BAJO: return "EST_ESPERAR_NIVEL_BAJO";
								case EST_ESPERAR_NIVEL_MEDIO: return "EST_ESPERAR_NIVEL_MEDIO";
								case EST_ESPERAR_TIEMPO: return "EST_ESPERAR_TIEMPO";
								case EST_DRENAR_BACHA: return "EST_DRENAR_BACHA";
								case EST_ERROR_FALTA_AGUA: return "EST_ERROR_FALTA_AGUA";
								default: return "EST_ESTADO_INVALIDO";
								}
								case EC2000_DRAIN_START:
									switch (EC2000_drain_state)
									{
									case EC2000_DRS_INICIO: return "DRS_INICIO";
									case EC2000_DRS_ESPERAR_INICIO: return "DRS_ESPERAR_INICIO";
									case EC2000_DRS_DESAGUE: return "DRS_DESAGUE";
									case EC2000_DRS_POST_DRENAJE: return "DRS_POST_DRENAJE";
									case EC2000_DRS_TAPA_ABIERTA: return "DRS_TAPA_ABIERTA";
									default: return "DRS_ESTADO_INVALIDO";
									}
									case EC2000_DOSING_START:
										switch (EC2000_dosing_state)
										{
										case EC2000_DOS_INICIO: return "DOS_INICIO";
										case EC2000_DOS_ESPERAR_INICIO: return "DOS_ESPERAR_INICIO";
										case EC2000_DOS_BIDON_NO_HIGH: return "DOS_BIDON_NO_HIGH";
										case EC2000_DOS_CONFIRMACION: return "DOS_CONFIRMACION";
										case EC2000_DOS_BIDON_HIGH: return "DOS_BIDON_HIGH";
										case EC2000_DOS_POST_DRENAJE: return "DOS_POST_DRENAJE";
										default: return "DOS_ESTADO_INVALIDO";
										}
										case EC2000_CLEANING_RECYCLE_BOX:
											switch (EC2000_clean_box_state)
											{
											case EC2000_CRB_INICIO: return "CRB_INICIO";
											case EC2000_CRB_TAPA_ABIERTA: return "CRB_TAPA_ABIERTA";
											case EC2000_CRB_DESINFECTANTE: return "CRB_DESINFECTANTE";
											case EC2000_CRB_ESPERAR_NUEVO_LAVADO: return "CRB_ESPERAR_NUEVO_LAVADO";
											case EC2000_CRB_LLENAR_BACHA: return "CRB_LLENAR_BACHA";
											case EC2000_CRB_FALTA_AGUA: return "CRB_FALTA_AGUA";
											case EC2000_CRB_ESPERAR_BIDON: return "CRB_ESPERAR_BIDON";
											case EC2000_CRB_BIDON: return "CRB_BIDON";
											case EC2000_CRB_POST_BIDON: return "CRB_POST_BIDON";
											case EC2000_CRB_ESPERAR_DESAGUE: return "CRB_ESPERAR_DESAGUE";
											case EC2000_CRB_DESAGUE: return "CRB_DESAGUE";
											case EC2000_CRB_POST_DESAGUE: return "CRB_POST_DESAGUE";
											case EC2000_CRB_STOP: return "CRB_STOP";
											case EC2000_CRB_VACIAR: return "CRB_VACIAR";
											default: return "CRB_ESTADO_INVALIDO";
											}
											case EC2000_ERROR_FALTA_DESINFECTANTE: return "FALTA_DESINFECTANTE";
											case EC2000_ERROR_SENSOR_NIVEL_BACHA: return "FALLA_SENSOR_NIVEL";
											case EC2000_ERROR_NIVEL_MAXIMO: return "SENSOR_NIVEL_MAXIMO";
											case EC2000_FIN_PROCESO: return "PITIDOS_FIN_PROCESO";
											case EC2000_FINALIZAR_MANUAL: return "VACIAR_BACHA";
											default: return "ESTADO_INVALIDO";
	}
}

static const char *EC2000_get_state(void)
{
	switch (EC2000_state)
	{
	case EC2000_IDLE: return "IDLE";
	case EC2000_PREPARAR_BACHA: return "PREPARAR_BACHA";
	case EC2000_LAVADO: return "LAVADO";
	case EC2000_DETERGENTE: return "DETERGENTE";
	case EC2000_AIRE: return "AIRE";
	case EC2000_DESINFECTANTE: return "DESINFECTANTE";
	case EC2000_ALCOHOL: return "ALCOHOL";
	case EC2000_PISTOLA_AIRE: return "PISTOLA_AIRE";
	case EC2000_DRENAJE_MANUAL: return "DRENAJE_MANUAL";
	case EC2000_ESTANQUEIDAD: return "ESTANQUEIDAD";
	case EC2000_COMPLETO:
		switch (C_state)
		{
		case C_IDLE: return "C_IDLE";
		case C_INICIO: return "C_INICIO";
		case C_LAVADO_1: return "C_LAVADO1";
		case C_DETERGENTE: return "C_DETERGENTE";
		case C_LAVADO_2: return "C_LAVADO2";
		case C_AIRE_1: return "C_AIRE1";
		case C_DESINFECTANTE: return "C_DESINFECTANTE";
		case C_AIRE_2: return "C_AIRE2";
		case C_LAVADO_3: return "C_LAVADO3";
		case C_ALCOHOL: return "C_ALCOHOL";
		case C_AIRE_3: return "C_AIRE3";
		case C_FIN: return "C_FIN";
		default: return "C_ESTADO_INVALIDO";
		}
		case EC2000_DRAIN_START: return "DRAIN_START";
		case EC2000_DOSING_START: return "DOSING_START";
		case EC2000_CLEANING_RECYCLE_BOX: return "CLEANING_RECYCLE_BOX";
		case EC2000_ERROR_FALTA_DESINFECTANTE: return "ERROR";
		case EC2000_ERROR_SENSOR_NIVEL_BACHA: return "ERROR";
		case EC2000_ERROR_NIVEL_MAXIMO: return "ERROR";
		case EC2000_FIN_PROCESO: return "FINALIZANDO";
		case EC2000_FINALIZAR_MANUAL: return "FINALIZANDO_MANUAL";
		default: return "ESTADO_INVALIDO";
	}
}


/*--------------------------------------------------------------------------------------------*/
/*          FUNCIONES INTERNAS                                                                */
/*--------------------------------------------------------------------------------------------*/
static void EC2000_actualizar_nivel_bacha(void)
{
	if (EC2000_NIVEL_BACHA_ALTO)
	{
		/* Sensor de emergencia: detiene inmediatamente el proceso actual. */
		EC2000_nivel_bacha = EC2000_BACHA_HIGH;
		/* El drenaje manual informa el nivel, pero no toma decisiones por sensores. */
		if (EC2000_state == EC2000_DRENAJE_MANUAL)
			return;
		/* Evita volver a notificar el mismo error en cada vuelta del main. */
		if (EC2000_state == EC2000_ERROR_NIVEL_MAXIMO)
			return;
		/* L1_b_finalizar autoriza solamente el vaciado seguro posterior al error. */
		if (EC2000_vaciado_error_nivel_maximo)
			return;
		fsm_off_sistema();
		fsm_off_buzzer();
		EC2000_lavado_init = 0;
		EC2000_detergente_init = 0;
		EC2000_aire_init = 0;
		EC2000_desinfectante_init = 0;
		EC2000_completo_init = 0;
		EC2000_alcohol_init = 0;
		L_state = L_IDLE;
		DET_state = DET_IDLE;
		A_state = A_IDLE;
		DES_state = DES_IDLE;
		ALC_state = ALC_IDLE;
		C_state = C_IDLE;
		EC2000_completo_en_ejecucion = 0;
		EC2000_completo_etapa_finalizada = 0;
		EC2000_lavado_tic = 0;
		EC2000_llenado_tic = 0;
		EC2000_aire_tic = 0;
		EC2000_detergente_tic = 0;
		EC2000_desinfectante_tic = 0;
		EC2000_alcohol_tic = 0;
		EC2000_estanq_tic = 0;
		EC2000_vaciado_bacha_tic = 0;
		EC2000_vaciado_bacha_activo = 0;
		EC2000_post_vaciado_bacha_iniciado = 0;
		L_buzzer_pulsos = 0;
		EC2000_buzzer_tic = 0;
		EC2000_fsm_paused = 0;
		EC2000_state = EC2000_ERROR_NIVEL_MAXIMO;
		EC2000_fsm_enable = 0;
		Nextion_Error(REPRO1, "Sensor nivel MAXIMO"); // ERROR Nextion: sensor alto de emergencia activado
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		Ticket_EnqEventRtc(REPRO1, "ERROR: SENSOR NIVEL MAXIMO"); // Ticket: registra nivel maximo de emergencia
	}
	else if (!EC2000_NIVEL_BACHA_BAJO && !EC2000_NIVEL_BACHA_MEDIO &&
			!EC2000_NIVEL_BACHA_ALTO)
		EC2000_nivel_bacha = EC2000_BACHA_EMPTY;
	else if (EC2000_NIVEL_BACHA_BAJO && (!EC2000_NIVEL_BACHA_MEDIO) && (!EC2000_NIVEL_BACHA_ALTO))
		EC2000_nivel_bacha = EC2000_BACHA_LOW;
	else if (EC2000_NIVEL_BACHA_BAJO && EC2000_NIVEL_BACHA_MEDIO && (!EC2000_NIVEL_BACHA_ALTO))
		EC2000_nivel_bacha = EC2000_BACHA_MEDIUM;
	else
		EC2000_nivel_bacha = EC2000_BACHA_NIVEL_ERROR;
}

static void EC2000_actualizar_nivel_bidon(void)
{
	if (!EC2000_SENSOR_NIVEL_DESINF_1 && !EC2000_SENSOR_NIVEL_DESINF_2 &&
			!EC2000_SENSOR_NIVEL_DESINF_3 && !EC2000_SENSOR_NIVEL_DESINF_4)
		EC2000_nivel_bidon = EC2000_BIDON_EMPTY;
	else if (EC2000_SENSOR_NIVEL_DESINF_1 && !EC2000_SENSOR_NIVEL_DESINF_2 &&
			!EC2000_SENSOR_NIVEL_DESINF_3 && !EC2000_SENSOR_NIVEL_DESINF_4)
		EC2000_nivel_bidon = EC2000_BIDON_LOW;
	else if (EC2000_SENSOR_NIVEL_DESINF_1 && EC2000_SENSOR_NIVEL_DESINF_2 &&
			!EC2000_SENSOR_NIVEL_DESINF_3 && !EC2000_SENSOR_NIVEL_DESINF_4)
		EC2000_nivel_bidon = EC2000_BIDON_MEDIUM;
	else if (EC2000_SENSOR_NIVEL_DESINF_1 && EC2000_SENSOR_NIVEL_DESINF_2 &&
			EC2000_SENSOR_NIVEL_DESINF_3 && !EC2000_SENSOR_NIVEL_DESINF_4)
		EC2000_nivel_bidon = EC2000_BIDON_HIGH;
	else if (EC2000_SENSOR_NIVEL_DESINF_1 && EC2000_SENSOR_NIVEL_DESINF_2 &&
			EC2000_SENSOR_NIVEL_DESINF_3 && EC2000_SENSOR_NIVEL_DESINF_4)
		EC2000_nivel_bidon = EC2000_BIDON_FULL;
	else
		EC2000_nivel_bidon = EC2000_BIDON_NIVEL_ERROR;
}

static void EC2000_buzzer_error_runcycle(void)
{
	if (L_buzzer_pulsos >= 3)
	{
		fsm_off_buzzer();
		return;
	}

	if (EC2000_buzzer_tic)
		return;

	if (HAL_GPIO_ReadPin(BUZZER_GPIO_Port, BUZZER_Pin) == GPIO_PIN_SET)
	{
		fsm_off_buzzer();
		L_buzzer_pulsos++;
		if (L_buzzer_pulsos < 3)
			EC2000_buzzer_tic = TIME_BUZZER_ERROR;
	}
	else
	{
		fsm_on_buzzer();
		EC2000_buzzer_tic = TIME_BUZZER_ERROR;
	}
}

static void EC2000_iniciar_control_vaciado_bacha(void)
{
	EC2000_vaciado_bacha_activo = 1;
	EC2000_post_vaciado_bacha_iniciado = 0;
	EC2000_vaciado_bacha_tic = 0;
}

static uint8_t EC2000_vaciado_bacha_completo(void)
{
	if (!EC2000_vaciado_bacha_activo)
		return 1;

	if (EC2000_nivel_bacha != EC2000_BACHA_EMPTY)
	{
		EC2000_post_vaciado_bacha_iniciado = 0;
		EC2000_vaciado_bacha_tic = 0;
		return 0;
	}

	if (!EC2000_post_vaciado_bacha_iniciado)
	{
		EC2000_post_vaciado_bacha_iniciado = 1;
		EC2000_vaciado_bacha_tic = EC2000_TIME_POST_DRENAJE_BACHA;
		return 0;
	}

	if (EC2000_vaciado_bacha_tic)
		return 0;

	EC2000_vaciado_bacha_activo = 0;
	EC2000_post_vaciado_bacha_iniciado = 0;
	return 1;
}

void fsm_off_sistema()
{
	COMP_OFF
	ACOL_OFF
	SELF_OFF
	AIR_S_OFF
	DET_OFF
	WAT_1_OFF
	WAT_2_OFF
	LEA_A_OFF
	LEA_W_OFF
	AIR_OFF
	DRAIN_OFF
	DIS_OFF
	KLD_CERRAR
	SOLE_OFF
	VACIO_OFF
}
static void fsm_on_puerta_sole(void)
{
	SOLE_ON
}
static void fsm_off_puerta_sole(void)
{
	SOLE_OFF
}
static void fsm_on_opa_bacha(void)
{
	DIS_ON
}
static void fsm_off_opa_bacha(void)
{
	DIS_OFF
}

static void fsm_on_opa_bidon(void)
{
	KLD_BIDON
}
static void fsm_off_opa_bidon(void)
{
	KLD_CERRAR
}

static void fsm_on_aire(void)
{
	COMP_ON
	AIR_S_ON
	AIR_ON

}
static void fsm_off_aire(void)
{
	COMP_OFF
	AIR_S_OFF
	AIR_OFF
}
static void fsm_on_estanq_bomba(void)
{
	LEA_A_ON
}
static void fsm_off_estanq_bomba(void)
{
	LEA_A_OFF
}
static void fsm_on_detergente(void)
{
	DET_ON
}
static void fsm_off_detergente(void)
{
	DET_OFF
}
static void fsm_on_alcohol(void)
{
	ACOL_ON
}
static void fsm_off_alcohol(void)
{
	ACOL_OFF
}
static void fsm_on_llenar_bacha(void)
{
	LEA_W_ON
}

static void fsm_off_llenar_bacha(void)
{
	LEA_W_OFF
}

static void fsm_on_lavado_bomba(void)
{
	WAT_1_ON
	WAT_2_ON
}

static void fsm_off_lavado_bomba(void)
{
	WAT_1_OFF
	WAT_2_OFF
}

static void fsm_on_drenar_bacha(void)
{
	DRAIN_ON //Enciendo la bomba de drenaje
	KLD_DRENAR //Abro la KLD para el lado del drenaje
}

static void fsm_off_drenar_bacha(void)
{
	DRAIN_OFF //Apago la bomba de drenaje
	KLD_CERRAR //Cierro la KLD
}

static uint8_t EC2000_corroborar_bacha_runcycle(void)
{
	if (EC2000_nivel_bacha != EC2000_BACHA_EMPTY)
	{
		if (!EC2000_vaciado_bacha_activo)
			EC2000_iniciar_control_vaciado_bacha();

		if (L1_opa_bacha)
		{
			fsm_off_drenar_bacha();
			fsm_on_opa_bidon();
		}
		else
		{
			fsm_off_opa_bidon();
			fsm_on_drenar_bacha();
		}
		return 0;
	}

	if (!EC2000_vaciado_bacha_activo)
		return 1;

	/* Mantiene la ruta seleccionada durante todo el tiempo posterior a EMPTY. */
	if (L1_opa_bacha)
	{
		fsm_off_drenar_bacha();
		fsm_on_opa_bidon();
	}
	else
	{
		fsm_off_opa_bidon();
		fsm_on_drenar_bacha();
	}

	if (!EC2000_vaciado_bacha_completo())
		return 0;

	fsm_off_opa_bidon();
	fsm_off_drenar_bacha();
	if (L1_opa_bacha)
		Nextion_Send_OPA_OFF(REPRO1);
	return 1;
}

static uint32_t EC2000_tiempo_proceso_individual(uint32_t tiempo_configurado)
{
	if ((!tiempo_configurado) && (!EC2000_completo_en_ejecucion))
		return EC2000_TIME_PROCESO_INDIVIDUAL;

	return tiempo_configurado;
}

static uint8_t EC2000_verificar_nivel_bacha_antes_de_llenar(void)
{
	if (EC2000_nivel_bacha != EC2000_BACHA_NIVEL_ERROR)
		return 1;

	fsm_off_sistema();
	fsm_off_buzzer();
	L_buzzer_pulsos = 0;
	EC2000_buzzer_tic = TIME_BUZZER_ERROR;
	Nextion_Error(REPRO1, "Falla sensor nivel"); // ERROR Nextion: combinacion invalida de sensores de nivel de bacha
	Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
	Ticket_EnqEventRtc(REPRO1, "ERROR: FALLA SENSOR NIVEL BACHA"); // Ticket: registra falla de sensores de nivel
	EC2000_state = EC2000_ERROR_SENSOR_NIVEL_BACHA;
	return 0;
}

static uint8_t EC2000_control_tapa_runcycle(void)
{
	uint8_t i;
	uint8_t proceso_activo = ((EC2000_state == EC2000_PREPARAR_BACHA) &&
			(EC2000_proximo_state != EC2000_ESTANQUEIDAD)) ||
					(EC2000_state == EC2000_LAVADO) ||
					(EC2000_state == EC2000_DETERGENTE) ||
					(EC2000_state == EC2000_AIRE) ||
					(EC2000_state == EC2000_DESINFECTANTE) ||
					(EC2000_state == EC2000_ALCOHOL) ||
					(EC2000_state == EC2000_COMPLETO);

	if (!EC2000_error_tapa_activo)
	{
		if (!proceso_activo || EC2000_SENSOR_IMAN_TAPA)
			return 0;

		for (i = 0; i < 16; i++)
			EC2000_salidas_antes_error_tapa[i] = rm_output1[i];

		EC2000_lavado_tic_antes_error_tapa = EC2000_lavado_tic;
		EC2000_llenado_tic_antes_error_tapa = EC2000_llenado_tic;
		EC2000_aire_tic_antes_error_tapa = EC2000_aire_tic;
		EC2000_detergente_tic_antes_error_tapa = EC2000_detergente_tic;
		EC2000_desinfectante_tic_antes_error_tapa = EC2000_desinfectante_tic;
		EC2000_alcohol_tic_antes_error_tapa = EC2000_alcohol_tic;
		EC2000_estanq_tic_antes_error_tapa = EC2000_estanq_tic;
		EC2000_vaciado_tic_antes_error_tapa = EC2000_vaciado_bacha_tic;
		EC2000_lavado_tic = 0;
		EC2000_llenado_tic = 0;
		EC2000_aire_tic = 0;
		EC2000_detergente_tic = 0;
		EC2000_desinfectante_tic = 0;
		EC2000_alcohol_tic = 0;
		EC2000_estanq_tic = 0;
		EC2000_vaciado_bacha_tic = 0;
		fsm_off_sistema();
		fsm_off_buzzer();
		L_buzzer_pulsos = 0;
		EC2000_buzzer_tic = TIME_BUZZER_ERROR;
		EC2000_error_tapa_activo = 1;
		/* Si la tapa ya estaba abierta durante la preparacion, el proceso no llego a comenzar. */
		if (EC2000_state == EC2000_PREPARAR_BACHA)
			EC2000_error_inicio_sin_vaciado = 1;
		Nextion_Error(REPRO1, "Tapa abierta"); // ERROR Nextion: detiene cualquier proceso por tapa abierta
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		Ticket_EnqEventRtc(REPRO1, "ERROR: TAPA ABIERTA"); // Ticket: registra apertura de tapa durante el proceso
		return 1;
	}

	/* Un error de tapa detectado antes de comenzar queda finalizado y no permite reanudar. */
	if (EC2000_error_inicio_sin_vaciado)
	{
		EC2000_buzzer_error_runcycle();
		return 1;
	}

	if (!EC2000_SENSOR_IMAN_TAPA)
	{
		EC2000_buzzer_error_runcycle();
		return 1;
	}

	fsm_off_buzzer();
	L_buzzer_pulsos = 0;
	EC2000_buzzer_tic = 0;
	for (i = 0; i < 16; i++)
		rm_output1[i] = EC2000_salidas_antes_error_tapa[i];
	EC2000_lavado_tic = EC2000_lavado_tic_antes_error_tapa;
	EC2000_llenado_tic = EC2000_llenado_tic_antes_error_tapa;
	EC2000_aire_tic = EC2000_aire_tic_antes_error_tapa;
	EC2000_detergente_tic = EC2000_detergente_tic_antes_error_tapa;
	EC2000_desinfectante_tic = EC2000_desinfectante_tic_antes_error_tapa;
	EC2000_alcohol_tic = EC2000_alcohol_tic_antes_error_tapa;
	EC2000_estanq_tic = EC2000_estanq_tic_antes_error_tapa;
	EC2000_vaciado_bacha_tic = EC2000_vaciado_tic_antes_error_tapa;
	EC2000_error_tapa_activo = 0;
	enqueue_nextion_command("data.L1_mensaje.txt=\"\""); // Limpia el error al cerrar la tapa
	Nextion_ChangeState(REPRO1, Lavando_NextionState); // Informa que el proceso continua
	return 0;
}

static void EC2000_iniciar_fin_proceso(void)
{
	fsm_off_sistema();
	fsm_off_buzzer();

	if (EC2000_completo_en_ejecucion)
	{
		EC2000_completo_etapa_finalizada = 1;
		return;
	}

	EC2000_impresora_finalizar_proceso("OK");

	Nextion_ChangeState(REPRO1, Finalizado_NextionState);
	Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Ocultar_NextionButtons);
	EC2000_fin_pitidos = 0;
	EC2000_fin_buzzer_on = 1;
	fsm_on_buzzer();
	EC2000_buzzer_tic = TIME_BUZZER_FIN;
	EC2000_state = EC2000_FIN_PROCESO;
}

/*--------------------------------------------------------------------------------------------*/
/*          INICIALIZACION                                                                    */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_lavado_init(void)
{
	L_state = L_IDLE;
	EC2000_lavado_init = 0;
	EC2000_lavado_tic = 0;
	EC2000_llenado_tic = 0;
	EC2000_buzzer_tic = 0;
	L_buzzer_pulsos = 0;

	fsm_off_sistema();
	fsm_off_buzzer();
}

void EC2000_fsm_lavado_start(void)
{
	EC2000_LAVADO_INIT;
}

void EC2000_fsm_lavado_stop(void)
{
	EC2000_fsm_finish();
}

/*--------------------------------------------------------------------------------------------*/
/*          RUNCYCLE                                                                          */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_lavado_runcycle(void)
{
	/* La tapa se controla durante todo el proceso de lavado. */
	if ((L_state != L_IDLE) &&
			(L_state != L_ERROR_FALTA_AGUA) &&
			(L_state != L_ERROR_TAPA_ABIERTA) &&
			(!EC2000_SENSOR_IMAN_TAPA))
	{
		L_state_antes_error_tapa = L_state;
		L_lavado_tic_antes_error_tapa = EC2000_lavado_tic;
		L_llenado_tic_antes_error_tapa = EC2000_llenado_tic;
		fsm_off_sistema();
		EC2000_lavado_tic = 0;
		EC2000_llenado_tic = 0;
		fsm_off_buzzer();
		L_buzzer_pulsos = 0;
		EC2000_buzzer_tic = TIME_BUZZER_ERROR;
		Nextion_Error(REPRO1, "Tapa abierta"); // ERROR Nextion: tapa abierta
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		Ticket_EnqEventRtc(REPRO1, "ERROR: TAPA ABIERTA"); // Ticket: registra el error de tapa abierta
		L_state = L_ERROR_TAPA_ABIERTA;
	}

	switch (L_state)
	{
	case L_IDLE:
		if (EC2000_lavado_init)
		{
			EC2000_lavado_init = 0;
			EC2000_lavado_tic = 0;
			EC2000_llenado_tic = 0;
			EC2000_buzzer_tic = 0;
			L_buzzer_pulsos = 0;
			fsm_off_sistema();
			fsm_off_buzzer();
			Nextion_ChangeState(REPRO1, Lavando_NextionState);
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
			L_state = L_CORROBORAR_BACHA_VACIA;
		}
		break;
	case L_CORROBORAR_BACHA_VACIA:
		/* Los sensores medio y alto deben estar apagados. */
		if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
		{
			if (EC2000_vaciado_bacha_activo)
			{
				if (L1_opa_bacha)
				{
					fsm_off_drenar_bacha();
					fsm_on_opa_bidon();
				}
				else
				{
					fsm_off_opa_bidon();
					fsm_on_drenar_bacha();
				}
				if (!EC2000_vaciado_bacha_completo())
					break;
				fsm_off_opa_bidon();
				fsm_off_drenar_bacha();
				if (L1_opa_bacha)
					Nextion_Send_OPA_OFF(REPRO1);
			}
			fsm_on_lavado_bomba();
			fsm_on_drenar_bacha();
			EC2000_lavado_tic = EC2000_tiempo_proceso_individual(L1_TIME_RECYCLE_FLUSH);
			EC2000_llenado_tic = 0;
			L_state = L_LAVAR_CON_DRENAJE_ABIERTO;
		}
		else
		{
			/* Devuelve OPA al bidon o envia agua al drenaje hasta completar el post-vaciado. */
			if (!EC2000_vaciado_bacha_activo)
				EC2000_iniciar_control_vaciado_bacha();
			fsm_off_lavado_bomba();
			fsm_off_llenar_bacha();
			if (L1_opa_bacha)
			{
				fsm_off_drenar_bacha();
				fsm_on_opa_bidon();
			}
			else
			{
				fsm_off_opa_bidon();
				fsm_on_drenar_bacha();
			}
		}
		break;

	case L_LAVAR_CON_DRENAJE_ABIERTO:
		/* En este estado solo se espera L1_TIME_RECYCLE_FLUSH. */
		if (!EC2000_lavado_tic)
		{
			fsm_off_drenar_bacha();
			fsm_off_lavado_bomba();
			if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
				break;
			fsm_on_llenar_bacha();

			if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) || (EC2000_nivel_bacha == EC2000_BACHA_HIGH))
			{
				fsm_off_llenar_bacha();
				fsm_on_drenar_bacha();
				EC2000_iniciar_control_vaciado_bacha();
				EC2000_llenado_tic = 0;
				L_state = L_DRENAR_BACHA;
			}
			else if (EC2000_nivel_bacha == EC2000_BACHA_LOW)
			{
				EC2000_llenado_tic = TIMEOUT_LLENADO_2;
				L_state = L_ESPERAR_NIVEL_MEDIO;
			}
			else
			{
				EC2000_llenado_tic = TIMEOUT_LLENADO_1;
				L_state = L_ESPERAR_NIVEL_BAJO;
			}
		}
		break;

	case L_ESPERAR_NIVEL_BAJO:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		else if (EC2000_nivel_bacha == EC2000_BACHA_LOW)
		{
			EC2000_llenado_tic = TIMEOUT_LLENADO_2;
			L_state = L_ESPERAR_NIVEL_MEDIO;
		}
		else if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) || (EC2000_nivel_bacha == EC2000_BACHA_HIGH))
		{
			fsm_off_llenar_bacha();
			fsm_on_drenar_bacha();
			EC2000_iniciar_control_vaciado_bacha();
			EC2000_llenado_tic = 0;
			L_state = L_DRENAR_BACHA;
		}
		else if (!EC2000_llenado_tic)
		{
			fsm_off_sistema();
			fsm_off_buzzer();
			L_buzzer_pulsos = 0;
			EC2000_buzzer_tic = TIME_BUZZER_ERROR;
			Nextion_Error(REPRO1, "Falta agua"); // ERROR Nextion: no alcanzo el nivel bajo
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
			Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA AGUA NIVEL BAJO"); // Ticket: registra timeout de nivel bajo
			L_state = L_ERROR_FALTA_AGUA;
		}
		break;

	case L_ESPERAR_NIVEL_MEDIO:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		else if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) || (EC2000_nivel_bacha == EC2000_BACHA_HIGH))
		{
			fsm_off_llenar_bacha();
			fsm_on_drenar_bacha();
			EC2000_iniciar_control_vaciado_bacha();
			EC2000_llenado_tic = 0;
			L_state = L_DRENAR_BACHA;
		}
		else if (!EC2000_llenado_tic)
		{
			fsm_off_sistema();
			fsm_off_buzzer();
			L_buzzer_pulsos = 0;
			EC2000_buzzer_tic = TIME_BUZZER_ERROR;
			Nextion_Error(REPRO1, "Falta agua"); // ERROR Nextion: no alcanzo el nivel medio
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
			Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA AGUA NIVEL MEDIO"); // Ticket: registra timeout de nivel medio
			L_state = L_ERROR_FALTA_AGUA;
		}
		break;

	case L_DRENAR_BACHA:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_drenar_bacha();
			L_state = L_IDLE;
			EC2000_iniciar_fin_proceso();
		}
		break;

	case L_ERROR_FALTA_AGUA:
		EC2000_buzzer_error_runcycle();
		break;

	case L_ERROR_TAPA_ABIERTA:
		if (EC2000_SENSOR_IMAN_TAPA)
		{
			fsm_off_buzzer();
			EC2000_buzzer_tic = 0;
			EC2000_lavado_tic = L_lavado_tic_antes_error_tapa;
			EC2000_llenado_tic = L_llenado_tic_antes_error_tapa;
			L_state = L_state_antes_error_tapa;

			if (L_state == L_LAVAR_CON_DRENAJE_ABIERTO)
			{
				fsm_on_lavado_bomba();
				fsm_on_drenar_bacha();
			}
			else if ((L_state == L_ESPERAR_NIVEL_BAJO) ||
					(L_state == L_ESPERAR_NIVEL_MEDIO))
			{
				fsm_off_drenar_bacha();
				if (EC2000_verificar_nivel_bacha_antes_de_llenar())
					fsm_on_llenar_bacha();
			}
			else if (L_state == L_DRENAR_BACHA)
				fsm_on_drenar_bacha();

			enqueue_nextion_command("data.L1_mensaje.txt=\"\""); // Limpia el mensaje de error de tapa
			Nextion_ChangeState(REPRO1, Lavando_NextionState); // Informa a Nextion que el lavado continua
		}
		else
			EC2000_buzzer_error_runcycle();
		break;

	default:
		EC2000_fsm_lavado_init();
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          FSM DETERGENTE                                                                    */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_detergente_init(void)
{
	DET_state = DET_IDLE;
	EC2000_detergente_init = 0;
	EC2000_detergente_tic = 0;
}

void EC2000_fsm_detergente_runcycle(void)
{
	if ((DET_state != DET_IDLE) &&
			(DET_state != DET_ERROR_TAPA_ABIERTA) &&
			(!EC2000_SENSOR_IMAN_TAPA))
	{
		DET_state_antes_error_tapa = DET_state;
		DET_tic_antes_error_tapa = EC2000_detergente_tic;
		fsm_off_sistema();
		EC2000_detergente_tic = 0;
		fsm_off_buzzer();
		L_buzzer_pulsos = 0;
		EC2000_buzzer_tic = TIME_BUZZER_ERROR;
		Nextion_Error(REPRO1, "Tapa abierta"); // ERROR Nextion: tapa abierta durante detergente
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		Ticket_EnqEventRtc(REPRO1, "ERROR: TAPA ABIERTA DETERGENTE"); // Ticket: registra el error de tapa abierta
		DET_state = DET_ERROR_TAPA_ABIERTA;
	}

	switch (DET_state)
	{
	case DET_IDLE:
		if (EC2000_detergente_init)
		{
			EC2000_detergente_init = 0;
			DET_state = DET_INICIO;
		}
		break;

	case DET_INICIO:
		if (!EC2000_corroborar_bacha_runcycle())
			break;
		fsm_on_lavado_bomba();
		fsm_on_detergente();
		fsm_on_drenar_bacha();
		EC2000_detergente_tic = EC2000_tiempo_proceso_individual(L1_TIME_RECYCLE_WASH);
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		DET_state = DET_DOSIFICANDO;
		break;

	case DET_DOSIFICANDO:
		if (!EC2000_detergente_tic)
		{
			fsm_off_lavado_bomba();
			fsm_off_detergente();
			fsm_on_drenar_bacha();
			EC2000_iniciar_control_vaciado_bacha();
			DET_state = DET_DRENAR_BACHA;
		}
		break;

	case DET_DRENAR_BACHA:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_drenar_bacha();
			/* Espera 10 segundos con el drenaje apagado antes de verificar nuevamente. */
			EC2000_detergente_tic = 10000;
			DET_state = DET_ESPERAR_VACIADO;
		}
		break;

	case DET_ERROR_TAPA_ABIERTA:
		if (EC2000_SENSOR_IMAN_TAPA)
		{
			fsm_off_buzzer();
			EC2000_buzzer_tic = 0;
			EC2000_detergente_tic = DET_tic_antes_error_tapa;
			DET_state = DET_state_antes_error_tapa;

			if (DET_state == DET_DOSIFICANDO)
			{
				fsm_on_lavado_bomba();
				fsm_on_detergente();
				fsm_on_drenar_bacha();
			}
			else if (DET_state == DET_DRENAR_BACHA)
				fsm_on_drenar_bacha();
			else if (DET_state == DET_ESPERAR_VACIADO)
				fsm_off_drenar_bacha();

			enqueue_nextion_command("data.L1_mensaje.txt=\"\""); // Limpia el mensaje de error de tapa
			Nextion_ChangeState(REPRO1, Lavando_NextionState); // Informa a Nextion que detergente continua
		}
		else
			EC2000_buzzer_error_runcycle();
		break;

	case DET_ESPERAR_VACIADO:
		if (!EC2000_detergente_tic)
		{
			if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
			{
				fsm_off_drenar_bacha();
				DET_state = DET_IDLE;
				EC2000_iniciar_fin_proceso();
			}
			else
			{
				fsm_on_drenar_bacha();
				EC2000_iniciar_control_vaciado_bacha();
				DET_state = DET_DRENAR_BACHA;
			}
		}
		break;

	default:
		EC2000_fsm_detergente_init();
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          FSM AIRE                                                                          */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_aire_init(void)
{
	A_state = A_IDLE;
	EC2000_aire_init = 0;
	EC2000_aire_tic = 0;
}

void EC2000_fsm_aire_runcycle(void)
{
	switch (A_state)
	{
	case A_IDLE:
		if (EC2000_aire_init)
		{
			EC2000_aire_init = 0;
			A_state = A_INICIO;
		}
		break;

	case A_INICIO:
		if (!EC2000_corroborar_bacha_runcycle())
			break;
		fsm_on_aire();
		fsm_off_drenar_bacha();
		EC2000_aire_tic = EC2000_tiempo_proceso_individual(L1_TIME_DRYING);
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		A_state = A_SECANDO;
		break;

	case A_SECANDO:
		if (EC2000_nivel_bacha != EC2000_BACHA_EMPTY)
		{
			if (!EC2000_vaciado_bacha_activo)
				EC2000_iniciar_control_vaciado_bacha();
			fsm_on_drenar_bacha();
		}
		else if (EC2000_vaciado_bacha_activo)
		{
			fsm_on_drenar_bacha();
			if (EC2000_vaciado_bacha_completo())
				fsm_off_drenar_bacha();
		}
		else
			fsm_off_drenar_bacha();

		if (!EC2000_aire_tic)
		{
			fsm_off_aire();
			/* Siempre realiza el post-drenaje final, aunque la bacha ya este vacia. */
			if (!EC2000_vaciado_bacha_activo)
				EC2000_iniciar_control_vaciado_bacha();
			fsm_on_drenar_bacha();
			A_state = A_DRENAR_FINAL;
		}
		break;

	case A_DRENAR_FINAL:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_drenar_bacha();
			A_state = A_IDLE;
			EC2000_iniciar_fin_proceso();
		}
		break;

	default:
		EC2000_fsm_aire_init();
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          FSM DESINFECTANTE                                                                 */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_desinfectante_init(void)
{
	DES_state = DES_IDLE;
	DES_nivel_inicial_evaluado = 0;
	EC2000_desinfectante_init = 0;
	EC2000_desinfectante_tic = 0;
}

void EC2000_fsm_desinfectante_runcycle(void)
{
	switch (DES_state)
	{
	case DES_IDLE:
		if (EC2000_desinfectante_init)
		{
			EC2000_desinfectante_init = 0;
			DES_state = DES_INICIO;
		}
		break;

	case DES_INICIO:
		/* Si la bacha ya estaba vacia al iniciar, realiza igualmente un drenaje preventivo. */
		if (!DES_nivel_inicial_evaluado)
		{
			DES_nivel_inicial_evaluado = 1;
			if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
			{
				fsm_on_drenar_bacha();
				EC2000_desinfectante_tic = EC2000_TIME_DRENAJE_INICIAL_DESINFECTANTE;
				DES_state = DES_DRENAJE_INICIAL;
				break;
			}
		}
		if (!EC2000_corroborar_bacha_runcycle())
			break;
		if (!EC2000_completo_en_ejecucion)
			enqueue_nextion_command("data.L1_on_modo.txt=\"Desinfeccion\"");
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		fsm_off_opa_bidon();
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		fsm_on_opa_bacha();
		Nextion_Send_OPA_ON(REPRO1);
		DES_state = DES_LLENAR_BACHA;
		break;

	case DES_DRENAJE_INICIAL:
		if (!EC2000_desinfectante_tic)
		{
			fsm_off_drenar_bacha();
			DES_state = DES_INICIO;
		}
		break;

	case DES_LLENAR_BACHA:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		else if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) || (EC2000_nivel_bacha == EC2000_BACHA_HIGH) ||
				(EC2000_nivel_bidon == EC2000_BIDON_EMPTY))
		{
			fsm_off_opa_bacha();
			EC2000_desinfectante_tic = EC2000_tiempo_proceso_individual(L1_TIME_RECYCLE_DESINFECT);
			DES_state = DES_ESPERAR_TIEMPO;
		}
		break;

	case DES_ESPERAR_TIEMPO:
		if (!EC2000_desinfectante_tic)
		{
			EC2000_iniciar_control_vaciado_bacha();
			fsm_on_opa_bidon();
			fsm_on_aire();
			DES_state = DES_RETORNAR_BIDON;
		}
		break;

	case DES_RETORNAR_BIDON:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_opa_bidon();
			fsm_off_aire();
			Nextion_Send_OPA_OFF(REPRO1);
			DES_state = DES_IDLE;
			EC2000_iniciar_fin_proceso();
		}
		break;

	default:
		EC2000_fsm_desinfectante_init();
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          FSM ALCOHOL                                                                       */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_alcohol_init(void)
{
	ALC_state = ALC_IDLE;
	EC2000_alcohol_init = 0;
	EC2000_alcohol_tic = 0;
	fsm_off_alcohol();
}

void EC2000_fsm_alcohol_runcycle(void)
{
	switch (ALC_state)
	{
	case ALC_IDLE:
		if (EC2000_alcohol_init)
		{
			EC2000_alcohol_init = 0;
			fsm_on_alcohol();
			EC2000_alcohol_tic = EC2000_tiempo_proceso_individual(L1_TIME_ALCOHOL);
			ALC_state = ALC_DOSIFICANDO;
		}
		break;

	case ALC_DOSIFICANDO:
		if (!EC2000_alcohol_tic)
		{
			fsm_off_alcohol();
			ALC_state = ALC_IDLE;
			EC2000_iniciar_fin_proceso();
		}
		break;

	default:
		EC2000_fsm_alcohol_init();
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          FSM LAVADO COMPLETO                                                                */
/*--------------------------------------------------------------------------------------------*/

static void EC2000_completo_iniciar_lavado(void)
{
	EC2000_fsm_lavado_init();
	EC2000_lavado_init = 1;
	EC2000_completo_etapa_finalizada = 0;
}

static void EC2000_completo_iniciar_detergente(void)
{
	EC2000_fsm_detergente_init();
	EC2000_detergente_init = 1;
	EC2000_completo_etapa_finalizada = 0;
}

static void EC2000_completo_iniciar_aire(void)
{
	EC2000_fsm_aire_init();
	EC2000_aire_init = 1;
	EC2000_completo_etapa_finalizada = 0;
}

static void EC2000_completo_iniciar_desinfectante(void)
{
	EC2000_fsm_desinfectante_init();
	EC2000_desinfectante_init = 1;
	EC2000_completo_etapa_finalizada = 0;
}

static void EC2000_completo_avanzar_desde(C_FSM_STATES_T siguiente_estado)
{
	while (1)
	{
		switch (siguiente_estado)
		{
		case C_LAVADO_1:
			if (!L1_TIME_RECYCLE_FLUSH)
			{
				siguiente_estado = C_DETERGENTE;
				break;
			}
			EC2000_completo_iniciar_lavado();
			C_state = C_LAVADO_1;
			return;

		case C_DETERGENTE:
			if (!L1_TIME_RECYCLE_WASH)
			{
				siguiente_estado = C_LAVADO_2;
				break;
			}
			EC2000_completo_iniciar_detergente();
			C_state = C_DETERGENTE;
			return;

		case C_LAVADO_2:
			if (!L1_TIME_RECYCLE_FLUSH)
			{
				siguiente_estado = C_AIRE_1;
				break;
			}
			EC2000_completo_iniciar_lavado();
			C_state = C_LAVADO_2;
			return;

		case C_AIRE_1:
			if (!L1_TIME_DRYING)
			{
				siguiente_estado = C_DESINFECTANTE;
				break;
			}
			EC2000_completo_iniciar_aire();
			C_state = C_AIRE_1;
			return;

		case C_DESINFECTANTE:
			if (!L1_TIME_RECYCLE_DESINFECT)
			{
				siguiente_estado = C_AIRE_2;
				break;
			}
			if (L1_TIME_RECYCLE_DESINFECT &&
					(EC2000_nivel_bidon == EC2000_BIDON_EMPTY))
			{
				EC2000_completo_en_ejecucion = 0;
				fsm_off_sistema();
				fsm_off_buzzer();
				L_buzzer_pulsos = 0;
				EC2000_buzzer_tic = TIME_BUZZER_ERROR;
				Nextion_Error(REPRO1, "Falta desinfectante"); // ERROR Nextion: bidon vacio durante lavado completo
				Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA DESINFECTANTE"); // Ticket: registra falta de desinfectante
				EC2000_state = EC2000_ERROR_FALTA_DESINFECTANTE;
				return;
			}
			EC2000_completo_iniciar_desinfectante();
			C_state = C_DESINFECTANTE;
			return;

		case C_AIRE_2:
			if (!L1_TIME_DRYING)
			{
				siguiente_estado = C_LAVADO_3;
				break;
			}
			EC2000_completo_iniciar_aire();
			C_state = C_AIRE_2;
			return;

		case C_LAVADO_3:
			if (!L1_TIME_RECYCLE_FLUSH)
			{
				siguiente_estado = C_ALCOHOL;
				break;
			}
			EC2000_completo_iniciar_lavado();
			C_state = C_LAVADO_3;
			return;

		case C_ALCOHOL:
			if (!L1_TIME_ALCOHOL)
			{
				siguiente_estado = C_AIRE_3;
				break;
			}
			EC2000_completo_etapa_finalizada = 0;
			EC2000_completo_alcohol_iniciado = 0;
			C_state = C_ALCOHOL;
			return;

		case C_AIRE_3:
			if (!L1_TIME_DRYING)
			{
				siguiente_estado = C_FIN;
				break;
			}
			EC2000_completo_iniciar_aire();
			C_state = C_AIRE_3;
			return;

		case C_FIN:
		default:
			EC2000_completo_etapa_finalizada = 0;
			C_state = C_FIN;
			return;
		}
	}
}

void EC2000_fsm_completo_init(void)
{
	C_state = C_IDLE;
	EC2000_completo_init = 0;
	EC2000_alcohol_tic = 0;
	EC2000_completo_en_ejecucion = 0;
	EC2000_completo_etapa_finalizada = 0;
	EC2000_completo_alcohol_iniciado = 0;
	fsm_off_alcohol();
}

void EC2000_fsm_completo_runcycle(void)
{
	switch (C_state)
	{
	case C_IDLE:
		if (EC2000_completo_init)
		{
			EC2000_completo_init = 0;
			C_state = C_INICIO;
		}
		break;

	case C_INICIO:
		if (!EC2000_corroborar_bacha_runcycle())
			break;
		EC2000_completo_en_ejecucion = 1;
		enqueue_nextion_command("data.L1_on_modo.txt=\"Lavado completo\"");
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
		EC2000_completo_avanzar_desde(C_LAVADO_1);
		break;

	case C_LAVADO_1:
		EC2000_fsm_lavado_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_DETERGENTE);
		break;

	case C_DETERGENTE:
		EC2000_fsm_detergente_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_LAVADO_2);
		break;

	case C_LAVADO_2:
		EC2000_fsm_lavado_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_AIRE_1);
		break;

	case C_AIRE_1:
		EC2000_fsm_aire_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_DESINFECTANTE);
		break;

	case C_DESINFECTANTE:
		EC2000_fsm_desinfectante_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_AIRE_2);
		break;

	case C_AIRE_2:
		EC2000_fsm_aire_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_LAVADO_3);
		break;

	case C_LAVADO_3:
		EC2000_fsm_lavado_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_ALCOHOL);
		break;

	case C_ALCOHOL:
		if (!EC2000_completo_alcohol_iniciado)
		{
			if (!EC2000_corroborar_bacha_runcycle())
				break;
			EC2000_alcohol_tic = L1_TIME_ALCOHOL;
			fsm_on_alcohol();
			EC2000_completo_alcohol_iniciado = 1;
			break;
		}
		if (!EC2000_alcohol_tic)
		{
			fsm_off_alcohol();
			EC2000_completo_alcohol_iniciado = 0;
			EC2000_completo_avanzar_desde(C_AIRE_3);
		}
		break;

	case C_AIRE_3:
		EC2000_fsm_aire_runcycle();
		if (EC2000_completo_etapa_finalizada)
			EC2000_completo_avanzar_desde(C_FIN);
		break;

	case C_FIN:
		EC2000_completo_en_ejecucion = 0;
		C_state = C_IDLE;
		EC2000_iniciar_fin_proceso();
		break;

	default:
		EC2000_fsm_completo_init();
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          MANTENIMIENTO DEL BIDON DE DESINFECTANTE                                           */
/*--------------------------------------------------------------------------------------------*/

static void EC2000_mantenimiento_error_buzzer_init(void)
{
	fsm_off_sistema();
	fsm_off_buzzer();
	L_buzzer_pulsos = 0;
	EC2000_buzzer_tic = TIME_BUZZER_ERROR;
}

static void EC2000_drain_start_runcycle(void)
{
	switch (EC2000_drain_state)
	{
	case EC2000_DRS_INICIO:
		if (EC2000_nivel_bidon == EC2000_BIDON_EMPTY)
		{
			Nextion_FinDrain();
			EC2000_state = EC2000_IDLE;
		}
		else if (!EC2000_SENSOR_IMAN_TAPA)
		{
			EC2000_mantenimiento_error_buzzer_init();
			Nextion_Drain_ErrorTapa();
			EC2000_drain_state = EC2000_DRS_TAPA_ABIERTA;
		}
		else
		{
			fsm_on_drenar_bacha();
			EC2000_iniciar_control_vaciado_bacha();
			EC2000_mantenimiento_tic = EC2000_TIME_MANTENIMIENTO_INICIO;
			EC2000_drain_state = EC2000_DRS_ESPERAR_INICIO;
		}
		break;

	case EC2000_DRS_ESPERAR_INICIO:
		if (!EC2000_mantenimiento_tic)
		{
			fsm_on_opa_bacha();
			EC2000_drain_state = EC2000_DRS_DESAGUE;
		}
		break;

	case EC2000_DRS_DESAGUE:
		if (EC2000_nivel_bidon == EC2000_BIDON_EMPTY)
		{
			EC2000_mantenimiento_tic = L1_TIME_DRAINAGE;
			EC2000_drain_state = EC2000_DRS_POST_DRENAJE;
		}
		break;

	case EC2000_DRS_POST_DRENAJE:
		if (!EC2000_mantenimiento_tic && EC2000_vaciado_bacha_completo())
		{
			fsm_off_opa_bacha();
			fsm_off_drenar_bacha();
			Nextion_FinDrain();
			EC2000_state = EC2000_IDLE;
		}
		break;

	case EC2000_DRS_TAPA_ABIERTA:
		if (EC2000_SENSOR_IMAN_TAPA)
		{
			fsm_off_buzzer();
			EC2000_buzzer_tic = 0;
			Nextion_Drain_Reset_ErrorTapa();
			EC2000_drain_state = EC2000_DRS_INICIO;
		}
		else
			EC2000_buzzer_error_runcycle();
		break;

	default:
		EC2000_drain_state = EC2000_DRS_INICIO;
		break;
	}
}

static void EC2000_dosing_start_runcycle(void)
{
	/* El nivel FULL tiene prioridad y detiene el agregado desde cualquier subestado. */
	if (EC2000_nivel_bidon == EC2000_BIDON_FULL)
	{
		fsm_off_aire();
		fsm_off_opa_bidon();
		EC2000_mantenimiento_tic = 0;
		EC2000_vaciado_bacha_tic = 0;
		EC2000_vaciado_bacha_activo = 0;
		EC2000_post_vaciado_bacha_iniciado = 0;
		L1_botonDOS = 0;
		L1_botonAceptar = 0;
		Nextion_FinDosing();
		EC2000_dosing_state = EC2000_DOS_INICIO;
		EC2000_state = EC2000_IDLE;
		return;
	}

	switch (EC2000_dosing_state)
	{
	case EC2000_DOS_INICIO:
		if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
		{
			Nextion_FinDosing();
			EC2000_state = EC2000_IDLE;
		}
		else if (EC2000_nivel_bidon == EC2000_BIDON_HIGH)
		{
			L1_botonAceptar = 0;
			enqueue_nextion_command("page RDos_Confirm");
			EC2000_dosing_state = EC2000_DOS_CONFIRMACION;
		}
		else
		{
			L1_botonAceptar = 0;
			fsm_on_opa_bidon();
			EC2000_mantenimiento_tic = EC2000_TIME_MANTENIMIENTO_INICIO;
			EC2000_dosing_state = EC2000_DOS_ESPERAR_INICIO;
		}
		break;

	case EC2000_DOS_ESPERAR_INICIO:
		if (!EC2000_mantenimiento_tic)
		{
			fsm_on_aire();
			EC2000_dosing_state = (EC2000_nivel_bidon == EC2000_BIDON_HIGH) ?
					EC2000_DOS_BIDON_HIGH : EC2000_DOS_BIDON_NO_HIGH;
		}
		break;

	case EC2000_DOS_BIDON_NO_HIGH:
	case EC2000_DOS_BIDON_HIGH:
		if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
		{
			EC2000_iniciar_control_vaciado_bacha();
			EC2000_dosing_state = EC2000_DOS_POST_DRENAJE;
		}
		break;

	case EC2000_DOS_CONFIRMACION:
		if (L1_botonIdle)
		{
			L1_botonIdle = 0;
			Nextion_FinDosing();
			EC2000_state = EC2000_IDLE;
		}
		else if (L1_botonAceptar && (EC2000_nivel_bidon != EC2000_BIDON_FULL))
		{
			L1_botonDOS = 0;
			L1_botonAceptar = 0;
			fsm_on_opa_bidon();
			EC2000_mantenimiento_tic = EC2000_TIME_MANTENIMIENTO_INICIO;
			EC2000_dosing_state = EC2000_DOS_ESPERAR_INICIO;
		}
		break;

	case EC2000_DOS_POST_DRENAJE:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_aire();
			fsm_off_opa_bidon();
			Nextion_FinDosing();
			EC2000_state = EC2000_IDLE;
		}
		break;

	default:
		EC2000_dosing_state = EC2000_DOS_INICIO;
		break;
	}
}

static void EC2000_cleaning_recycle_box_runcycle(void)
{
	switch (EC2000_clean_box_state)
	{
	case EC2000_CRB_INICIO:
		if (!EC2000_SENSOR_IMAN_TAPA)
		{
			EC2000_mantenimiento_error_buzzer_init();
			Nextion_CleanRecBox_ErrorTapa();
			EC2000_clean_box_state = EC2000_CRB_TAPA_ABIERTA;
		}
		else if (EC2000_nivel_bidon != EC2000_BIDON_EMPTY)
		{
			EC2000_mantenimiento_error_buzzer_init();
			Nextion_CleanRecBox_ErrorNivel();
			EC2000_clean_box_state = EC2000_CRB_DESINFECTANTE;
		}
		else
		{
			if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
				break;
			EC2000_clean_box_ciclo = 1;
			EC2000_clean_box_nivel_bajo_detectado = 0;
			EC2000_mantenimiento_timeout_tic = TIMEOUT_LLENADO_1;
			fsm_on_llenar_bacha();
			Nextion_CleanRecBox_n_lavado1();
			EC2000_clean_box_state = EC2000_CRB_LLENAR_BACHA;
		}
		break;

	case EC2000_CRB_TAPA_ABIERTA:
		if (EC2000_SENSOR_IMAN_TAPA)
		{
			fsm_off_buzzer();
			EC2000_buzzer_tic = 0;
			Nextion_CleanRecBox_Reset_ErrorAgua();
			EC2000_clean_box_state = EC2000_CRB_INICIO;
		}
		else
			EC2000_buzzer_error_runcycle();
		break;

	case EC2000_CRB_DESINFECTANTE:
		if (L1_botonIdle)
		{
			L1_botonIdle = 0;
			fsm_off_buzzer();
			EC2000_state = EC2000_IDLE;
		}
		else
			EC2000_buzzer_error_runcycle();
		break;

	case EC2000_CRB_ESPERAR_NUEVO_LAVADO:
		if (!EC2000_mantenimiento_tic)
		{
			if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
				break;
			EC2000_mantenimiento_timeout_tic = TIMEOUT_LLENADO_1;
			fsm_on_llenar_bacha();
			EC2000_clean_box_state = EC2000_CRB_LLENAR_BACHA;
		}
		break;

	case EC2000_CRB_LLENAR_BACHA:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		else if (L1_botonStop)
		{
			L1_botonStop = 0;
			fsm_off_llenar_bacha();
			EC2000_clean_box_state = EC2000_CRB_STOP;
		}
		else if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) ||
				(EC2000_nivel_bacha == EC2000_BACHA_HIGH))
		{
			fsm_off_llenar_bacha();
			EC2000_mantenimiento_timeout_tic = 0;
			EC2000_mantenimiento_tic = EC2000_TIME_LAVADO_BIDON;
			EC2000_clean_box_state = EC2000_CRB_ESPERAR_BIDON;
		}
		else if (!EC2000_clean_box_nivel_bajo_detectado &&
				(EC2000_nivel_bacha == EC2000_BACHA_LOW))
		{
			EC2000_clean_box_nivel_bajo_detectado = 1;
			EC2000_mantenimiento_timeout_tic = TIMEOUT_LLENADO_2;
		}
		else if (!EC2000_mantenimiento_timeout_tic)
		{
			fsm_off_llenar_bacha();
			EC2000_mantenimiento_error_buzzer_init();
			Nextion_CleanRecBox_ErrorAgua();
			EC2000_clean_box_state = EC2000_CRB_FALTA_AGUA;
		}
		break;

	case EC2000_CRB_FALTA_AGUA:
		if (L1_botonContinuar)
		{
			L1_botonContinuar = 0;
			fsm_off_buzzer();
			EC2000_buzzer_tic = 0;
			if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
				break;
			EC2000_clean_box_nivel_bajo_detectado = 0;
			EC2000_mantenimiento_timeout_tic = TIMEOUT_LLENADO_1;
			fsm_on_llenar_bacha();
			Nextion_CleanRecBox_Reset_ErrorAgua();
			EC2000_clean_box_state = EC2000_CRB_LLENAR_BACHA;
		}
		else
			EC2000_buzzer_error_runcycle();
		break;

	case EC2000_CRB_ESPERAR_BIDON:
		if (!EC2000_mantenimiento_tic)
		{
			fsm_on_opa_bidon();
			fsm_on_aire();
			EC2000_clean_box_state = EC2000_CRB_BIDON;
		}
		break;

	case EC2000_CRB_BIDON:
		if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
		{
			EC2000_iniciar_control_vaciado_bacha();
			EC2000_clean_box_state = EC2000_CRB_POST_BIDON;
		}
		break;

	case EC2000_CRB_POST_BIDON:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_aire();
			fsm_off_opa_bidon();
			EC2000_mantenimiento_tic = EC2000_TIME_LAVADO_BIDON;
			EC2000_clean_box_state = EC2000_CRB_ESPERAR_DESAGUE;
		}
		break;

	case EC2000_CRB_ESPERAR_DESAGUE:
		if (!EC2000_mantenimiento_tic)
		{
			fsm_on_drenar_bacha();
			fsm_on_opa_bacha();
			EC2000_iniciar_control_vaciado_bacha();
			EC2000_clean_box_state = EC2000_CRB_DESAGUE;
		}
		break;

	case EC2000_CRB_DESAGUE:
		if (EC2000_nivel_bidon == EC2000_BIDON_EMPTY)
		{
			EC2000_mantenimiento_tic = L1_TIME_DRAINAGE + (L1_TIME_DRAINAGE / 2);
			EC2000_clean_box_state = EC2000_CRB_POST_DESAGUE;
		}
		break;

	case EC2000_CRB_POST_DESAGUE:
		if (!EC2000_mantenimiento_tic && EC2000_vaciado_bacha_completo())
		{
			fsm_off_opa_bacha();
			fsm_off_drenar_bacha();
			if (EC2000_clean_box_ciclo < 2)
			{
				EC2000_clean_box_ciclo++;
				EC2000_clean_box_nivel_bajo_detectado = 0;
				EC2000_mantenimiento_timeout_tic = 0;
				EC2000_mantenimiento_tic = EC2000_TIME_ENTRE_LAVADOS_BIDON;
				Nextion_CleanRecBox_n_lavado2();
				EC2000_clean_box_state = EC2000_CRB_ESPERAR_NUEVO_LAVADO;
			}
			else
			{
				Nextion_CleanRecBox_LavComplete();
				EC2000_state = EC2000_IDLE;
			}
		}
		break;

	case EC2000_CRB_STOP:
		fsm_off_llenar_bacha();
		EC2000_clean_box_state = EC2000_CRB_VACIAR;
		break;

	case EC2000_CRB_VACIAR:
		if ((EC2000_nivel_bacha == EC2000_BACHA_EMPTY) &&
				(EC2000_nivel_bidon == EC2000_BIDON_EMPTY))
		{
			if (!EC2000_vaciado_bacha_completo())
				break;

			fsm_off_opa_bacha();
			fsm_off_drenar_bacha();
			Nextion_CleanRecBox_LavComplete();
			EC2000_state = EC2000_IDLE;
		}
		else
		{
			EC2000_iniciar_control_vaciado_bacha();
			fsm_on_drenar_bacha();
			if (EC2000_nivel_bidon != EC2000_BIDON_EMPTY)
				fsm_on_opa_bacha();
			else
				fsm_off_opa_bacha();
		}
		break;

	default:
		EC2000_clean_box_state = EC2000_CRB_INICIO;
		break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*          FSM SUPERVISORA ENDOCLEAN2000                                                      */
/*--------------------------------------------------------------------------------------------*/

void EC2000_fsm_init(void)
{
	EC2000_state = EC2000_IDLE;
	EC2000_proximo_state = EC2000_IDLE;
	EC2000_fin_pitidos = 0;
	EC2000_fin_buzzer_on = 0;
	EC2000_fsm_enable = 1;
	EC2000_fsm_paused = 0;
	EC2000_sendstate_tic = 0;
	EC2000_mantenimiento_tic = 0;
	EC2000_mantenimiento_timeout_tic = 0;
	EC2000_vaciado_bacha_tic = 0;
	EC2000_pistola_aire_tic = 0;
	EC2000_drenaje_manual_tic = 0;
	EC2000_estanq_tic = 0;
	EC2000_vaciado_bacha_activo = 0;
	EC2000_post_vaciado_bacha_iniciado = 0;
	EC2000_error_tapa_activo = 0;
	EC2000_error_inicio_sin_vaciado = 0;
	EC2000_ticket_activo = 0;
	EC2000_ticket_proceso = "ENDOCLEAN2000";
	EC2000_vaciado_error_nivel_maximo = 0;
	EC2000_drain_state = EC2000_DRS_INICIO;
	EC2000_dosing_state = EC2000_DOS_INICIO;
	EC2000_clean_box_state = EC2000_CRB_INICIO;
	EC2000_clean_box_ciclo = 0;
	EC2000_clean_box_nivel_bajo_detectado = 0;
	EST_state = EST_INICIO;
	EC2000_fsm_lavado_init();
	EC2000_fsm_detergente_init();
	EC2000_fsm_aire_init();
	EC2000_fsm_desinfectante_init();
	EC2000_fsm_alcohol_init();
	EC2000_fsm_completo_init();
}

void EC2000_fsm_pistola_aire_start(void)
{
	EC2000_fsm_init();
	fsm_off_sistema();
	COMP_ON
	EC2000_pistola_aire_tic = EC2000_TIME_PISTOLA_AIRE;
	EC2000_state = EC2000_PISTOLA_AIRE;
}

void EC2000_fsm_pistola_aire_stop(void)
{
	COMP_OFF
	EC2000_pistola_aire_tic = 0;

	if (EC2000_state == EC2000_PISTOLA_AIRE)
		EC2000_state = EC2000_IDLE;
}

void EC2000_fsm_drenaje_manual_start(void)
{
	EC2000_fsm_init();
	fsm_off_sistema();
	fsm_on_drenar_bacha();
	EC2000_drenaje_manual_tic = EC2000_TIME_DRENAJE_MANUAL;
	EC2000_state = EC2000_DRENAJE_MANUAL;
}

void EC2000_fsm_drenaje_manual_stop(void)
{
	fsm_off_drenar_bacha();
	EC2000_drenaje_manual_tic = 0;

	if (EC2000_state == EC2000_DRENAJE_MANUAL)
		EC2000_state = EC2000_IDLE;
}

void EC2000_fsm_estanq_start(void)
{
	EC2000_fsm_init();
	fsm_off_sistema();
	EST_state = EST_INICIO;
	EC2000_proximo_state = EC2000_ESTANQUEIDAD;
	EC2000_state = EC2000_PREPARAR_BACHA;
	Nextion_ChangeState(REPRO1, Lavando_NextionState);
	enqueue_nextion_command("data.L1_on_modo.txt=\"Estanqueidad\"");
}

static void EC2000_fsm_estanq_runcycle(void)
{
	switch (EST_state)
	{
	case EST_INICIO:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		fsm_on_estanq_bomba();
		fsm_on_llenar_bacha();
		if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) ||
				(EC2000_nivel_bacha == EC2000_BACHA_HIGH))
		{
			fsm_off_llenar_bacha();
			EC2000_estanq_tic = EC2000_TIME_ESTANQUEIDAD;
			EST_state = EST_ESPERAR_TIEMPO;
		}
		else if (EC2000_nivel_bacha == EC2000_BACHA_LOW)
		{
			EC2000_llenado_tic = TIMEOUT_LLENADO_2;
			EST_state = EST_ESPERAR_NIVEL_MEDIO;
		}
		else
		{
			EC2000_llenado_tic = TIMEOUT_LLENADO_1;
			EST_state = EST_ESPERAR_NIVEL_BAJO;
		}
		break;

	case EST_ESPERAR_NIVEL_BAJO:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) ||
				(EC2000_nivel_bacha == EC2000_BACHA_HIGH))
		{
			fsm_off_llenar_bacha();
			EC2000_llenado_tic = 0;
			EC2000_estanq_tic = EC2000_TIME_ESTANQUEIDAD;
			EST_state = EST_ESPERAR_TIEMPO;
		}
		else if (EC2000_nivel_bacha == EC2000_BACHA_LOW)
		{
			EC2000_llenado_tic = TIMEOUT_LLENADO_2;
			EST_state = EST_ESPERAR_NIVEL_MEDIO;
		}
		else if (!EC2000_llenado_tic)
		{
			fsm_off_sistema();
			Nextion_Error(REPRO1, "Falta agua");
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
			Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA AGUA ESTANQUEIDAD");
			L_buzzer_pulsos = 0;
			EC2000_buzzer_tic = TIME_BUZZER_ERROR;
			EST_state = EST_ERROR_FALTA_AGUA;
		}
		break;

	case EST_ESPERAR_NIVEL_MEDIO:
		if (!EC2000_verificar_nivel_bacha_antes_de_llenar())
			break;
		if ((EC2000_nivel_bacha == EC2000_BACHA_MEDIUM) ||
				(EC2000_nivel_bacha == EC2000_BACHA_HIGH))
		{
			fsm_off_llenar_bacha();
			EC2000_llenado_tic = 0;
			EC2000_estanq_tic = EC2000_TIME_ESTANQUEIDAD;
			EST_state = EST_ESPERAR_TIEMPO;
		}
		else if (!EC2000_llenado_tic)
		{
			fsm_off_sistema();
			Nextion_Error(REPRO1, "Falta agua");
			Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
			Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA AGUA ESTANQUEIDAD");
			L_buzzer_pulsos = 0;
			EC2000_buzzer_tic = TIME_BUZZER_ERROR;
			EST_state = EST_ERROR_FALTA_AGUA;
		}
		break;

	case EST_ESPERAR_TIEMPO:
		if (!EC2000_estanq_tic)
		{
			fsm_on_drenar_bacha();
			fsm_off_estanq_bomba();
			EC2000_iniciar_control_vaciado_bacha();
			EST_state = EST_DRENAR_BACHA;
		}
		break;

	case EST_DRENAR_BACHA:
		if (EC2000_vaciado_bacha_completo())
		{
			fsm_off_drenar_bacha();
			EC2000_iniciar_fin_proceso();
		}
		break;

	case EST_ERROR_FALTA_AGUA:
		EC2000_buzzer_error_runcycle();
		break;

	default:
		EST_state = EST_INICIO;
		break;
	}
}

void EC2000_sendstate_runcycle(void)
{
	char buffer[100];

	if (EC2000_sendstate_tic)
		return;

	EC2000_sendstate_tic = TIME_SENDSTATE;

	snprintf(buffer, sizeof(buffer), "data.L1_state.txt=\"%s\"", EC2000_get_state());
	enqueue_nextion_command(buffer);

	snprintf(buffer, sizeof(buffer), "data.L1_substate.txt=\"%s\"", EC2000_get_substate());
	enqueue_nextion_command(buffer);
}

void EC2000_Nextion_Send_NivelBidon(void)
{
	switch (EC2000_nivel_bidon)
	{
	case EC2000_BIDON_EMPTY:
		enqueue_nextion_command("data.L1_nivel_bidon.txt=\"empty\"");
		break;
	case EC2000_BIDON_LOW:
		enqueue_nextion_command("data.L1_nivel_bidon.txt=\"low\"");
		break;
	case EC2000_BIDON_MEDIUM:
		enqueue_nextion_command("data.L1_nivel_bidon.txt=\"medium\"");
		break;
	case EC2000_BIDON_HIGH:
		enqueue_nextion_command("data.L1_nivel_bidon.txt=\"high\"");
		break;
	case EC2000_BIDON_FULL:
		enqueue_nextion_command("data.L1_nivel_bidon.txt=\"full\"");
		break;
	case EC2000_BIDON_NIVEL_ERROR:
		enqueue_nextion_command("data.L1_nivel_bidon.txt=\"error\"");
		break;
	default:
		break;
	}
}

void EC2000_Nextion_Send_NivelBacha(void)
{
	switch (EC2000_nivel_bacha)
	{
	case EC2000_BACHA_EMPTY:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"empty\"");
		break;
	case EC2000_BACHA_LOW:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"low\"");
		break;
	case EC2000_BACHA_MEDIUM:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"medium\"");
		break;
	case EC2000_BACHA_HIGH:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"high\"");
		break;
	case EC2000_BACHA_FULL:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"full\"");
		break;
	case EC2000_BACHA_NIVEL_ERROR:
		enqueue_nextion_command("data.L1_nivel_bacha.txt=\"error\"");
		break;
	default:
		break;
	}
}

void EC2000_fsm_pause(void)
{
	uint8_t i;

	if (EC2000_fsm_enable && (EC2000_state != EC2000_IDLE))
	{
		for (i = 0; i < 16; i++)
			EC2000_salidas_pausa[i] = rm_output1[i];

		EC2000_lavado_tic_pausa = EC2000_lavado_tic;
		EC2000_llenado_tic_pausa = EC2000_llenado_tic;
		EC2000_buzzer_tic_pausa = EC2000_buzzer_tic;
		EC2000_aire_tic_pausa = EC2000_aire_tic;
		EC2000_pistola_aire_tic_pausa = EC2000_pistola_aire_tic;
		EC2000_drenaje_manual_tic_pausa = EC2000_drenaje_manual_tic;
		EC2000_estanq_tic_pausa = EC2000_estanq_tic;
		EC2000_detergente_tic_pausa = EC2000_detergente_tic;
		EC2000_desinfectante_tic_pausa = EC2000_desinfectante_tic;
		EC2000_alcohol_tic_pausa = EC2000_alcohol_tic;
		EC2000_mantenimiento_tic_pausa = EC2000_mantenimiento_tic;
		EC2000_mantenimiento_timeout_tic_pausa = EC2000_mantenimiento_timeout_tic;
		EC2000_vaciado_bacha_tic_pausa = EC2000_vaciado_bacha_tic;
		EC2000_lavado_tic = 0;
		EC2000_llenado_tic = 0;
		EC2000_buzzer_tic = 0;
		EC2000_aire_tic = 0;
		EC2000_pistola_aire_tic = 0;
		EC2000_drenaje_manual_tic = 0;
		EC2000_estanq_tic = 0;
		EC2000_detergente_tic = 0;
		EC2000_desinfectante_tic = 0;
		EC2000_alcohol_tic = 0;
		EC2000_mantenimiento_tic = 0;
		EC2000_mantenimiento_timeout_tic = 0;
		EC2000_vaciado_bacha_tic = 0;
		EC2000_fsm_enable = 0;
		EC2000_fsm_paused = 1;
		fsm_off_sistema();
		fsm_off_buzzer();
		Nextion_ChangeState(REPRO1, Pausado_NextionState);
	}
}

void EC2000_fsm_continue(void)
{
	uint8_t i;

	if (EC2000_fsm_paused)
	{
		for (i = 0; i < 16; i++)
			rm_output1[i] = EC2000_salidas_pausa[i];

		EC2000_lavado_tic = EC2000_lavado_tic_pausa;
		EC2000_llenado_tic = EC2000_llenado_tic_pausa;
		EC2000_buzzer_tic = EC2000_buzzer_tic_pausa;
		EC2000_aire_tic = EC2000_aire_tic_pausa;
		EC2000_pistola_aire_tic = EC2000_pistola_aire_tic_pausa;
		EC2000_drenaje_manual_tic = EC2000_drenaje_manual_tic_pausa;
		EC2000_estanq_tic = EC2000_estanq_tic_pausa;
		EC2000_detergente_tic = EC2000_detergente_tic_pausa;
		EC2000_desinfectante_tic = EC2000_desinfectante_tic_pausa;
		EC2000_alcohol_tic = EC2000_alcohol_tic_pausa;
		EC2000_mantenimiento_tic = EC2000_mantenimiento_tic_pausa;
		EC2000_mantenimiento_timeout_tic = EC2000_mantenimiento_timeout_tic_pausa;
		EC2000_vaciado_bacha_tic = EC2000_vaciado_bacha_tic_pausa;
		EC2000_fsm_paused = 0;
		EC2000_fsm_enable = 1;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
	}
}

void EC2000_fsm_finish(void)
{
	uint8_t finalizar_sin_vaciado = EC2000_error_inicio_sin_vaciado;

	EC2000_vaciado_error_nivel_maximo = (EC2000_state == EC2000_ERROR_NIVEL_MAXIMO);
	EC2000_impresora_finalizar_proceso("STOP");
	fsm_off_sistema();
	fsm_off_buzzer();
	EC2000_fsm_lavado_init();
	EC2000_fsm_detergente_init();
	EC2000_fsm_aire_init();
	EC2000_fsm_desinfectante_init();
	EC2000_fsm_alcohol_init();
	EC2000_fsm_completo_init();
	EC2000_proximo_state = EC2000_IDLE;
	EC2000_mantenimiento_tic = 0;
	EC2000_mantenimiento_timeout_tic = 0;
	EC2000_pistola_aire_tic = 0;
	EC2000_drenaje_manual_tic = 0;
	EC2000_estanq_tic = 0;
	EC2000_vaciado_bacha_tic = 0;
	EC2000_vaciado_bacha_activo = 0;
	EC2000_post_vaciado_bacha_iniciado = 0;
	EC2000_error_inicio_sin_vaciado = 0;

	/* Los errores de validacion inicial terminan sin drenar ni devolver liquido al bidon. */
	if (finalizar_sin_vaciado)
	{
		EC2000_error_tapa_activo = 0;
		EC2000_fsm_paused = 0;
		EC2000_fsm_enable = 1;
		EC2000_state = EC2000_IDLE;
		Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Ocultar_NextionButtons);
		Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
		return;
	}

	EC2000_iniciar_control_vaciado_bacha();
	EC2000_error_tapa_activo = 0;
	EC2000_fsm_paused = 0;
	EC2000_fsm_enable = 1;
	EC2000_state = EC2000_FINALIZAR_MANUAL;
}

void EC2000_mantenimiento_stop(void)
{
	fsm_off_sistema();
	fsm_off_buzzer();
	EC2000_mantenimiento_tic = 0;
	EC2000_mantenimiento_timeout_tic = 0;
	EC2000_vaciado_bacha_tic = 0;
	EC2000_vaciado_bacha_activo = 0;
	EC2000_post_vaciado_bacha_iniciado = 0;
	EC2000_fsm_paused = 0;
	EC2000_fsm_enable = 1;
	L1_botonDRS = 0;
	L1_botonDOS = 0;
	L1_botonCRB = 0;
	L1_botonAceptar = 0;
	EC2000_state = EC2000_IDLE;
}

void EC2000_fsm_runcycle(void)
{
	/* El drenaje manual termina solo por tiempo o comando stop e ignora los sensores. */
	if (EC2000_state == EC2000_DRENAJE_MANUAL)
	{
		if (rm_input1_valid)
		{
			EC2000_actualizar_nivel_bacha();
			EC2000_actualizar_nivel_bidon();
		}

		if (!EC2000_fsm_enable)
			return;

		if (!EC2000_drenaje_manual_tic)
			EC2000_fsm_drenaje_manual_stop();
		return;
	}

	/* No evalua sensores ni ejecuta procesos hasta recibir la primera lectura RS485 valida. */
	if (!rm_input1_valid)
	{
		/* Al arrancar, todas las salidas del modulo de reles deben permanecer en cero. */
		for (uint8_t i = 0; i < 16; i++)
			rm_output1[i] = 0;
		fsm_off_buzzer();
		return;
	}

	/* Los niveles se recalculan siempre, incluso con la FSM pausada o detenida por error. */
	EC2000_actualizar_nivel_bacha();
	EC2000_actualizar_nivel_bidon();

	/* La emergencia de nivel maximo deshabilita la FSM durante la actualizacion. */
	if (!EC2000_fsm_enable)
	{
		if (EC2000_state == EC2000_ERROR_NIVEL_MAXIMO)
			EC2000_buzzer_error_runcycle();
		return;
	}

	if (EC2000_control_tapa_runcycle())
		return;

	EC2000_pedal_runcycle();

	switch (EC2000_state)
	{
	case EC2000_IDLE:
		/* En reposo, el modulo de reles debe conservar todas sus salidas en cero.
		 * Se respeta solamente el pulso temporizado de apertura accionado por pedal. */
		if (!EC2000_puerta_pedal_activa)
		{
			for (uint8_t i = 0; i < 16; i++)
				rm_output1[i] = 0;
		}

		if (L1_botonDRS)
		{
			L1_botonDRS = 0;
			L1_botonStop = 0;
			fsm_off_sistema();
			EC2000_drain_state = EC2000_DRS_INICIO;
			EC2000_state = EC2000_DRAIN_START;
		}
		else if (L1_botonDOS)
		{
			L1_botonDOS = 0;
			L1_botonStop = 0;
			fsm_off_sistema();
			EC2000_dosing_state = EC2000_DOS_INICIO;
			EC2000_state = EC2000_DOSING_START;
		}
		else if (L1_botonCRB)
		{
			L1_botonCRB = 0;
			L1_botonStop = 0;
			fsm_off_sistema();
			EC2000_clean_box_state = EC2000_CRB_INICIO;
			EC2000_state = EC2000_CLEANING_RECYCLE_BOX;
		}
		else if (EC2000_lavado_init)
		{
			EC2000_proximo_state = EC2000_LAVADO;
			EC2000_state = EC2000_PREPARAR_BACHA;
		}
		else if (EC2000_detergente_init)
		{
			EC2000_proximo_state = EC2000_DETERGENTE;
			EC2000_state = EC2000_PREPARAR_BACHA;
		}
		else if (EC2000_aire_init)
		{
			EC2000_proximo_state = EC2000_AIRE;
			EC2000_state = EC2000_PREPARAR_BACHA;
		}
		else if (EC2000_alcohol_init)
		{
			EC2000_proximo_state = EC2000_ALCOHOL;
			EC2000_state = EC2000_PREPARAR_BACHA;
		}
		else if (EC2000_desinfectante_init)
		{
			if (EC2000_nivel_bidon == EC2000_BIDON_EMPTY)
			{
				EC2000_desinfectante_init = 0;
				fsm_off_sistema();
				fsm_off_buzzer();
				L_buzzer_pulsos = 0;
				EC2000_buzzer_tic = TIME_BUZZER_ERROR;
				EC2000_error_inicio_sin_vaciado = 1;
				Nextion_Error(REPRO1, "Falta desinfectante"); // ERROR Nextion: bidon de desinfectante vacio
				Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
				Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA DESINFECTANTE"); // Ticket: registra falta de desinfectante
				EC2000_state = EC2000_ERROR_FALTA_DESINFECTANTE;
			}
			else
			{
				EC2000_proximo_state = EC2000_DESINFECTANTE;
				EC2000_state = EC2000_PREPARAR_BACHA;
			}
		}
		else if (EC2000_completo_init)
		{
			if (L1_TIME_RECYCLE_DESINFECT &&
					(EC2000_nivel_bidon == EC2000_BIDON_EMPTY))
			{
				EC2000_completo_init = 0;
				fsm_off_sistema();
				fsm_off_buzzer();
				L_buzzer_pulsos = 0;
				EC2000_buzzer_tic = TIME_BUZZER_ERROR;
				EC2000_error_inicio_sin_vaciado = 1;
				Nextion_Error(REPRO1, "Falta desinfectante"); // ERROR Nextion: el lavado completo requiere desinfectante
				Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
				Ticket_EnqEventRtc(REPRO1, "ERROR: FALTA DESINFECTANTE"); // Ticket: registra falta de desinfectante
				EC2000_state = EC2000_ERROR_FALTA_DESINFECTANTE;
			}
			else
			{
				EC2000_proximo_state = EC2000_COMPLETO;
				EC2000_state = EC2000_PREPARAR_BACHA;
			}
		}
		break;

	case EC2000_PREPARAR_BACHA:
		if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
		{
			if (!EC2000_vaciado_bacha_completo())
				break;

			fsm_off_opa_bidon();
			fsm_off_drenar_bacha();

			if (L1_opa_bacha)
				Nextion_Send_OPA_OFF(REPRO1);

			EC2000_impresora_iniciar_proceso(EC2000_proximo_state);

			fsm_on_buzzer();
			HAL_Delay(200);
			fsm_off_buzzer();

			EC2000_state = EC2000_proximo_state;
		}
		else if (L1_opa_bacha)
		{
			EC2000_iniciar_control_vaciado_bacha();
			fsm_off_drenar_bacha();
			fsm_on_opa_bidon();
		}
		else
		{
			EC2000_iniciar_control_vaciado_bacha();
			fsm_off_opa_bidon();
			fsm_on_drenar_bacha();
		}
		break;

	case EC2000_LAVADO:
		EC2000_fsm_lavado_runcycle();
		break;

	case EC2000_DETERGENTE:
		EC2000_fsm_detergente_runcycle();
		break;

	case EC2000_AIRE:
		EC2000_fsm_aire_runcycle();
		break;

	case EC2000_DESINFECTANTE:
		EC2000_fsm_desinfectante_runcycle();
		break;

	case EC2000_ALCOHOL:
		EC2000_fsm_alcohol_runcycle();
		break;

	case EC2000_COMPLETO:
		EC2000_fsm_completo_runcycle();
		break;

	case EC2000_PISTOLA_AIRE:
		if (!EC2000_pistola_aire_tic)
		{
			COMP_OFF
			EC2000_state = EC2000_IDLE;
			Nextion_ChangeState(REPRO1, Preparado_NextionState);
		}
		break;

	case EC2000_DRENAJE_MANUAL:
		break;

	case EC2000_ESTANQUEIDAD:
		EC2000_fsm_estanq_runcycle();
		break;

	case EC2000_DRAIN_START:
		EC2000_drain_start_runcycle();
		break;

	case EC2000_DOSING_START:
		EC2000_dosing_start_runcycle();
		break;

	case EC2000_CLEANING_RECYCLE_BOX:
		EC2000_cleaning_recycle_box_runcycle();
		break;

	case EC2000_ERROR_FALTA_DESINFECTANTE:
	case EC2000_ERROR_SENSOR_NIVEL_BACHA:
		EC2000_buzzer_error_runcycle();
		break;

	case EC2000_ERROR_NIVEL_MAXIMO:
		fsm_off_buzzer();
		break;

	case EC2000_FIN_PROCESO:
		if (!EC2000_buzzer_tic)
		{
			if (EC2000_fin_buzzer_on)
			{
				fsm_off_buzzer();
				EC2000_fin_buzzer_on = 0;
				EC2000_fin_pitidos++;

				if (EC2000_fin_pitidos == 3)
				{
					EC2000_state = EC2000_IDLE;
				}
				else
					EC2000_buzzer_tic = TIME_BUZZER_FIN;
			}
			else
			{
				fsm_on_buzzer();
				EC2000_fin_buzzer_on = 1;
				EC2000_buzzer_tic = TIME_BUZZER_FIN;
			}
		}
		break;

	case EC2000_FINALIZAR_MANUAL:
		if (EC2000_nivel_bacha == EC2000_BACHA_EMPTY)
		{
			if (L1_opa_bacha)
			{
				fsm_off_drenar_bacha();
				fsm_on_opa_bidon();
			}
			else
			{
				fsm_off_opa_bidon();
				fsm_on_drenar_bacha();
			}
			if (!EC2000_vaciado_bacha_completo())
				break;

			fsm_off_opa_bidon();
			fsm_off_drenar_bacha();

			if (L1_opa_bacha)
				Nextion_Send_OPA_OFF(REPRO1);

			EC2000_vaciado_error_nivel_maximo = 0;
			EC2000_state = EC2000_IDLE;
			Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
		}
		else if (L1_opa_bacha)
		{
			EC2000_iniciar_control_vaciado_bacha();
			fsm_off_drenar_bacha();
			fsm_on_opa_bidon();
		}
		else
		{
			EC2000_iniciar_control_vaciado_bacha();
			fsm_off_opa_bidon();
			fsm_on_drenar_bacha();
		}
		break;

	default:
		EC2000_fsm_init();
		break;
	}
}
