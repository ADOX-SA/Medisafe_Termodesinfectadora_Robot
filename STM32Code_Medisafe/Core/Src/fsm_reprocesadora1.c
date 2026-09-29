/*
 * fsm_reprocesadora1.c
 *
 *  Created on: Nov 20, 2025
 *      Author: momo
 */
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include "fsm_reprocesadora1.h"
#include "nextion.h"
#include "i2c_pressure.h"
#include "user_gpio.h"

#include "relay_module.h" 			// redefino como Xn las entradas del slave1
#include "global_variables.h"
#include "ticket.h"
#include "UltrasonicGenerator.h"

/*--------------------------------------------------------------------------------------------*/
/*		 	DEFINES 												  						  */
/*--------------------------------------------------------------------------------------------*/

#define WAIT_TIME_COUNT 1000

#define TIME_GASIFICACION 60000U //180000ms = 3mins
#define TIME_ULTSON_ON     10000U // tiempo en milisegundos
#define TIME_ULTSON_OFF    5000U // tiempo en milisegundos

#define FUGA_ESTANQUEIDAD "Fuga"
#define ESTANQUEIDAD_OK "Estanq_OK"

/*--------------------------------------------------------------------------------------------*/
/*		 	ESTADOS 												  						  */
/*--------------------------------------------------------------------------------------------*/

typedef enum {
	RUNNING,			// RUNCYCLE de REPROCESADORA
	STOPPED				// RUNCYCLE de STOP
} RUNCYCLE_T;

static RUNCYCLE_T runcycle = RUNNING;

typedef enum {	// estados del modo de lavado Standard, cada estado es una FSM
	IDLE,
	PRESET,
	PRUEBA_ESTANQ,
	FLUSH,
	WASHING,
	RISING,
	DESINFECT,
	FINAL_RISING,
	DRYING,
	DRAIN_START,
	DOSING_START,
	CLEANING_RECYCLE_BOX
} FSM1_LAVADORA_STATES_T;

typedef enum {	// estados del modo de lavado Standard, cada estado es una FSM
	_RECIRCULAR,
	_ENDOSC_DESCONEC
} RECIRCULADO_T;

static RECIRCULADO_T state_r = _RECIRCULAR;

/****************************/
/* SUBESTADOS MODO STANDARD */
/****************************/

typedef enum {			// Preset
	INICIO,
	DESAGUE_PURGA_LUMEN,
	TAPA_ABIERTA,		//
	FALTA_DESINF,		//
	FALTA_DETERG,		// errores
	FALTA_ALC,			//
	PRUEBA_BLOQUEO,
	ENDOSC_BLOQ_DESCON	// error
} FSM1_PRESET_T;

typedef enum {			// Prueba de Estanqueidad
	PRUEBA_ESTANQUEIDAD,
	FUGA_AIRE,			// error
	EXITO,
	EST_TAPA_ABIERTA
} FSM1_ESTANQUEIDAD_T;

typedef enum {			// Flush
	F_LLENAR_CUBA,
	F_FALTA_AGUA,			// error
	F_VACIAR_CUBA,
	F_RECIRCULAR,
	F_DESAGÜE_PURGA_LUMEN,
	F_TAPA_ABIERTA,			// error
} FSM1_FLUSH_T;

typedef enum {			// Washing
	W_LLENAR_CUBA_DET,
	W_FALTA_AGUA,			// error
	W_FALTA_DET,			// error
	W_CALENTAR,
	W_FALLA_CALEF,			// error
	W_VACIAR_CUBA,
	W_DESGASIFICAR,
	W_COMPLETAR_LLENADO,
	W_RECIRCULAR,
	W_DESAGUE_PURGA_LUMEN,
	W_TAPA_ABIERTA			// error
} FSM1_WASHING_T;

typedef enum {
	ULTSON_CYCLE_START,
	ULTSON_CYCLE_ON,
	ULTSON_CYCLE_OFF
} ULTSON_CYCLE_T;

typedef enum {			// Rising
	R_LLENAR_CUBA,
	R_FALTA_AGUA,			// error
	R_VACIAR_CUBA,
	R_RECIRCULAR,
	R_DESAGUE_PURGA_LUMEN,
	R_TAPA_ABIERTA			// error
} FSM1_RISING_T;

typedef enum {			// Desinfect
	D_LLENAR_CUBA_DESINF,
	D_CALENTAR,
	D_FALLA_CALEF,		// error
	D_VACIAR_CUBA,
	D_RECIRCULAR,
	D_BIDON_PURGA_LUMEN,
	D_TAPA_ABIERTA		// error
} FSM1_DESINF_T;

typedef enum {			// Final Rising
	FR_LLENAR_CUBA,
	FR_FALTA_AGUA,			// error
	FR_VACIAR_CUBA,
	FR_RECIRCULAR,
	FR_DESAGUE_PURGA_LUMEN,
	FR_TAPA_ABIERTA			// error
} FSM1_F_RISING_T;

typedef enum {			// Drying
	DR_ALCOHOL,
	DR_FALTA_ALC,			// error
	DR_VACIAR_CUBA,
	DR_SECADO,
	DR_FIN,
	DR_TAPA_ABIERTA			// error
} FSM1_F_DRYING_T;

/*-----------------------------------------------*/
/*												 */
/* 				Replace Desinfectant			 */
/*												 */
/*-----------------------------------------------*/

typedef enum {
	DRS_INICIO,
	DRS_TAPA_ABIERTA,		// error
	DRS_DESAGUE
} FSM1_DRAIN_START;

typedef enum {
	DOS_INICIO,
	DOS_BIDON_NO_HIGH,
	DOS_CONFIRMACION,
	DOS_BIDON_HIGH
} FSM1_DOSING_START;

typedef enum {
	CRB_INICIO,
	CRB_TAPA_ABIERTA,		// error
	CRB_DESINFECTANTE,
	CRB_LLENAR_CUBA,
	CRB_FALTA_AGUA,			// error
	CRB_BIDON,
	CRB_DESAGUE,
	CRB_STOP,
	CRB_VACIAR_CUBA
} FSM1_CLEANING_RECYCLE_BOX;

/****************************/
/****************************/

typedef struct
{
    uint8_t  active;          // 1 = hay delay en curso
    uint32_t start_tick;      // tick cuando se arrancó el delay
    uint32_t delay_ticks;     // duración del delay
    FSM1_LAVADORA_STATES_T next_state;      // estado al que quiero ir cuando termine
    int next_substate;        				// subestado al que quiero ir cuando termine
} fsm1_delay_ctx_t;

static FSM1_LAVADORA_STATES_T state;

static FSM1_PRESET_T substate_preset;
static FSM1_ESTANQUEIDAD_T substate_estanq;
static FSM1_FLUSH_T substate_flush;
static FSM1_WASHING_T substate_wash;
static FSM1_RISING_T substate_rising;
static FSM1_DESINF_T substate_desinf;
static FSM1_F_RISING_T substate_f_rise;
static FSM1_F_DRYING_T substate_dry;

static FSM1_DRAIN_START substate_drain;
static FSM1_DOSING_START substate_dosing;
static FSM1_CLEANING_RECYCLE_BOX substate_clean_box;

/*--------------------------------------------------------------------------------------------*/
/*		 	ENTRADAS 												  						  */
/*--------------------------------------------------------------------------------------------*/
/*
	X4: L1_SENSOR_CALEFACTOR 		-> 1: detecto liquido en bacha/tuberias
	X6: L1_SENSOR_NIVEL_TAPA 		-> 1: detecto bacha llena
	X7: L1_SENSOR_NIVEL_DET 		-> 1: detecto detergente en bidon
	X8: L1_SENSOR_NIVEL_ALC 		-> 1: detecto alcohol en bidon
	X9: L1_SENSOR_NIVEL_DESINF_1    ->    1: LOW level
	X10: L1_SENSOR_NIVEL_DESINF_2   ->  1: MEDIUM level
	X11: L1_SENSOR_NIVEL_DESINF_3   ->    1: HIGH level
	X13: L1_SENSOR_NIVEL_DESINF_4   ->    1: FULL level
	X14: L1_SENSOR_IMAN_TAPA 		-> 1: detecto tapa cerrada
	X15: L1_SENSOR_OPT_DET 		-> 1: detecto detergente en mangueras
	X16: L1_SENSOR_OPT_ALC 		-> 1: detecto alcohol en mangueras
 */
/*--------------------------------------------------------------------------------------------*/
/*		 	VARIABLES 												  						  */
/*--------------------------------------------------------------------------------------------*/

static bool tick_ms;
static uint32_t count_ms;		// CONTADOR SYSTICK
static uint32_t cont_agua,
cont_det,
cont_alc,
cont_air,
cont_buzz,
cont_puls,
cont_timeout,
cont_heating,
cont,
cont_desinf,
cont_press,
	cont_ultson,
flag_nextion;			// VARIABLES INTERNAS

static bool fuga, pruebaOK;		// estanqueidad
static bool recirculado;
static ULTSON_CYCLE_T ultson_cycle_state = ULTSON_CYCLE_START;

uint32_t i2c_tic = 200, pressure_tic = 100;

static fsm1_delay_ctx_t fsm1_delay_ctx = {0};

bool L1_botonIdle, L1_botonSTD_Mode_1, L1_botonContinuar, L1_botonVolver,
	 L1_botonDRS, L1_botonDOS, L1_botonCRB, L1_botonAceptar, L1_botonStop,
	 ready;


typedef enum {
    DET_IDLE = 0,
    DET_RUNNING,
    DET_DONE
} det_state_t;

static volatile det_state_t det_state = DET_IDLE;
static volatile uint32_t det_ms = 0;

static volatile uint8_t iniciarDetergente = 0;

/*--------------------------------------------------------------------------------------------*/
/*		 	FUNCIONES 												  						  */
/*--------------------------------------------------------------------------------------------*/

static void fsm_on_desagote(void);
static void fsm_on_recircular(void);
static void fsm_toggle_recircular(void);
static void fsm_off_recircular(void);
static void fsm_off_desagote(void);
static void fsm_on_llenar(void);
static void fsm_off_llenar(void);
static void fsm_on_desagote(void);
static void fsm_off_desagote(void);
static void fsm_on_llenar_desinf(void);
static void fsm_off_llenar_desinf(void);
static void fsm_on_bidon(void);
static void fsm_off_bidon(void);
static void fsm_on_desagote_sinbomba(void);
static void fsm_off_desagote_sinbomba(void);

static void reset_states_delay();

static void Detergente_Arm(void);
static void Detergente_Stop(void);

static void fsm_check_press(void);
static void fsm_check_pressure(void);
static void fsm_reset_lock_test(void);



void fsm_reprocesadora1_tick(void)
{
	count_ms++;

	if (count_ms >= 1)
	{
		count_ms = 0;
		tick_ms = 1;
	}

	static uint16_t contador_agua = 0;
	static uint8_t contador_control = 0;

	contador_agua++;
	contador_control++;

	if (contador_agua >= 1000)
	{
		contador_agua = 0;
		//		tick_agua_1s = true;
	}

	if (contador_control >= 100)
	{
		contador_control = 0;
		//		tick_control_rapido = true;
	}

	if(i2c_tic > 0)
		i2c_tic--;
	if(pressure_tic > 0)
		pressure_tic--;
}

void fsm_detergente1_tick(void)
{
    // Arranque: SOLO si está IDLE
    if (iniciarDetergente && det_state == DET_IDLE)
    {
        iniciarDetergente = 0;
        det_state = DET_RUNNING;
        det_ms = 0;
        L1_BPR05_ON
    }
    else if (iniciarDetergente) // si llega otro start mientras RUNNING/DONE lo ignoro
    {
        iniciarDetergente = 0;
    }

    // Conteo
    if (det_state == DET_RUNNING)
    {
        det_ms++;
        if (det_ms >= L1_TIME_DET)
        {
            det_state = DET_DONE;   // <- queda “terminado”, no rearma solo
            L1_BPR05_OFF
        }
    }
}

// Para permitir un nuevo disparo (llamar cuando arranca un nuevo ciclo/programa)
void Detergente_Arm(void)
{
    det_state = DET_IDLE;
    det_ms = 0;
    iniciarDetergente = 0;
}

// Stop de emergencia
void Detergente_Stop(void)
{
    det_state = DET_IDLE;
    det_ms = 0;
    iniciarDetergente = 0;
    L1_BPR05_OFF
}

static void fsm_check_pressure(void)
{
	if ( ( L1_pressure_1 < kPa_AIR_MIN ) ||
		 ( L1_pressure_2 < kPa_AIR_MIN ) )
	{
		L1_not_blocked = 0;
		L1_blocked = 1;			// endoscopio desconectado!
	}
	else if ( ( kPa_AIR_MIN < L1_pressure_1 && L1_pressure_1 < kPa_AIR_MAX ) ||
		 ( kPa_AIR_MIN < L1_pressure_2 && L1_pressure_2 < kPa_AIR_MAX ) )	// OK
	{
		L1_not_blocked = 1;		// endoscopio sin bloqueo!
		L1_blocked = 0;
	}
	else if ( ( L1_pressure_1 >= kPa_AIR_MAX ) ||
			  ( L1_pressure_2 >= kPa_AIR_MAX ) )
	{
		L1_not_blocked = 0;
		L1_blocked = 1;			// endoscopio bloqueado!
	}
}

static void fsm_check_press(void)
{
	if ( ( L1_pressure_1 < kPa_AG_MIN ) ||
	     ( L1_pressure_2 < kPa_AG_MIN ) )
	{
		L1_blocked = 1;			// endoscopio desconectado!
	}
}

static void fsm_reset_lock_test(void)
{
	L1_blocked = 0;
	L1_not_blocked = 0;
}

static void reset(void)
{
	tick_ms = 0;
	L1_botonSTD_Mode_1 = 0;	// inicio Modo Standard
	L1_botonVolver = 0;		// fin modo Standard
	L1_botonDRS = 0;		// inicio Drain Start
	L1_botonDOS = 0;		// inicio Dosing Start
	L1_botonCRB = 0;		// inicio Cleaning Recycle Box

	// AGREGAR FLAGS BOTONES SETEADOS DESDE NEXTION
}

void fsm_reprocesadora1_init(void)
{
	/*
	enqueue_nextion_command("wifi.ssid.txt=\"Sensor_XXXXXX\"");	// hardcodeo red modo AP node.
	enqueue_nextion_command("wifi.ip.txt=\"192.168.4.1\"");
	enqueue_nextion_command("wifi.state.txt=\"Modo AP\"");
	 */
	/*
	enqueue_nextion_command("thsp=300");	// seteo 5 min para dormir pantalla
	enqueue_nextion_command("thup=1");		// seteo que se pueda despertar con un toque al touch
	*/
	/*
	enqueue_nextion_command("thsp=0");	    // seteo 0 min para dormir pantalla (levanta)
	enqueue_nextion_command("thup=0");		// seteo que no se pueda despertar con un toque al touch
	*/

	state = IDLE;							// FSM PRINCIPAL

	substate_preset = INICIO;				// SUB-FSM's
	substate_estanq = PRUEBA_ESTANQUEIDAD;
	substate_flush = F_LLENAR_CUBA;;
	substate_wash = W_LLENAR_CUBA_DET;
	substate_rising = R_LLENAR_CUBA;
	substate_desinf = D_LLENAR_CUBA_DESINF;
	substate_f_rise = FR_LLENAR_CUBA;
	substate_dry = DR_ALCOHOL;

	substate_drain = DRS_INICIO;
	substate_dosing = DOS_INICIO;
	substate_clean_box = CRB_INICIO;

	// Estanqueidad //
	fuga = 0;
	pruebaOK = 0;
	cont_ultson = 0;
	ultson_cycle_state = ULTSON_CYCLE_START;

	L1_botonIdle = 0;
	L1_botonContinuar = 0;
	L1_botonAceptar = 0;
	L1_botonStop = 0;

	ready = 0; // STOP Estanqueidad

	recirculado = 0;		// chequeo de Presiones Recirculado

	reset();
}

/*--------------------------------------------------------------------------------------------*/
/*		 	CHEQUEO DURANTE RECIRCULADO  							  						  */
/*--------------------------------------------------------------------------------------------*/

void fsm1_recirculado(void)
{
	if ( L1_lock_test && recirculado )
	{
		switch (state_r)
		{
			case _RECIRCULAR:
				if ( tick_ms && L1_blocked )
				{
					cont_buzz = 0;
					cont_puls = 0;
					cont = 0;
					cont_press = 0;
					fsm_off_recircular();
					L1_CLF11_OFF
					// ERROR nextion:
					Nextion_Error(REPRO1, "Endoscopio Desconectado");
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					fsm1_enable = 0;

					state_r = _ENDOSC_DESCONEC;
				}
				else if ( tick_ms && !L1_blocked &&
						  cont_press == TIME_PRESS )
				{
					cont_press = 0;
					fsm_check_press();
				}
				else if ( tick_ms && !L1_blocked &&
						  cont_press < TIME_PRESS )
				{
					( L1_BOM02 ) ? cont_press ++ : (cont_press = 0);
				}
			break;

			case _ENDOSC_DESCONEC:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					fsm_reset_lock_test();
					cont_buzz = 0;
					cont_puls = 0;
					cont = 0; cont_press = 0;
					fsm_off_buzzer();
					// Reinicio ERROR Nextion:
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					fsm1_enable = 1;

					state_r = _RECIRCULAR;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
			break;
		}
	}
}

/*--------------------------------------------------------------------------------------------*/
/*		 	RUNCYCLE 												  						  */
/*--------------------------------------------------------------------------------------------*/

void fsm_reprocesadora1_runCycle(void)
{
	runcycle = RUNNING;				// indico para fsm1_delay que fsm deberá deshabilitar

	switch(state)
	{
		case IDLE:
			if( L1_botonSTD_Mode_1)
			{
				state = PRESET;
				substate_preset = INICIO;
				substate_estanq = PRUEBA_ESTANQUEIDAD;
				substate_flush = F_LLENAR_CUBA;;
				substate_wash = W_LLENAR_CUBA_DET;
				substate_rising = R_LLENAR_CUBA;
				substate_desinf = D_LLENAR_CUBA_DESINF;
				substate_f_rise = FR_LLENAR_CUBA;
				substate_dry = DR_ALCOHOL;

				L1_botonIdle = 0;
				L1_botonContinuar = 0;
				L1_botonAceptar = 0;
				L1_botonStop = 0;

				cont_agua =
				cont_det =
				cont_alc =
				cont_air =
				cont_buzz =
				cont_puls =
				cont_timeout =
				cont_heating =
				cont =
				cont_press =
				cont_desinf =
				cont_ultson = 0;
				ultson_cycle_state = ULTSON_CYCLE_START;

				Nextion_ChangeState(REPRO1, Lavando_NextionState);
			}
			else if ( L1_botonDRS )		// Drain Start
			{
				L1_botonStop = 0;

				state = DRAIN_START;
				substate_drain = DRS_INICIO;
				// Modificar texto boton de Drain Start a: DRAIN STOP
			}
			else if ( L1_botonDOS )		// Dosing Start
			{
				L1_botonStop = 0;

				state = DOSING_START;
				substate_dosing = DOS_INICIO;
				// Modificar texto boton de Dosing Start a: DOSING STOP
			}
			else if ( L1_botonCRB )		// Cleaning Recycle Box
			{
				L1_botonStop = 0;

				state = CLEANING_RECYCLE_BOX;
				substate_clean_box = CRB_INICIO;
			}
			break;
		/*-----------------------------------------------*/
		/*												 */
		/* 				Modo Standard					 */
		/*												 */
		/*-----------------------------------------------*/
		case PRESET:
			switch(substate_preset)
			{
			case INICIO:
				if( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR nextion: Tapa Abierta
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_preset = TAPA_ABIERTA;
				}
				else if ( tick_ms && ( L1_SENSOR_CALEFACTOR || L1_ignora_sensor ) )
				{
					cont_agua = 0;
					substate_preset = DESAGUE_PURGA_LUMEN;
				}
#if MODEL != MODEL_SIMPLE_SIN_OPA
				else if ( tick_ms && !SENSOR_NIVEL_DESINF_3 )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR nextion: Falta Desinfectante en Bidon
					Nextion_Error(REPRO1, "Falta desinfectante");
					//-----<
					substate_preset = FALTA_DESINF;
				}
#endif
				else if ( tick_ms && !SENSOR_NIVEL_DET )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR nextion: Falta Detergente en bidon
					Nextion_Error(REPRO1, "Falta detergente");
					//-----<
					substate_preset = FALTA_DETERG;
				}
				else if ( tick_ms && !SENSOR_NIVEL_ALC )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR nextion: Falta Alcohol en bidon
					Nextion_Error(REPRO1, "Falta alcohol");
					//-----<
					substate_preset = FALTA_ALC;
				}
				else if ( tick_ms && L1_SENSOR_IMAN_TAPA &&
						!L1_SENSOR_CALEFACTOR &&
						SENSOR_NIVEL_ALC &&
						SENSOR_NIVEL_DET )
				{
					L1_VAL08_OFF
					cont_air = 0;
					fsm1_delay(PRESET, PRUEBA_BLOQUEO, 5000);

					Ticket_EnqHeader(REPRO1, "", "", "", "", "Standard"); // requiere RTC   generacion del header del ticket: modelo, #serie, #personal, #endoscopio, modo lavado, dilucion
				}
				break;

			case DESAGUE_PURGA_LUMEN:
				if( L1_ignora_sensor )	// si el sensor de calefactor falla, dreno por timeout = 50 seg (esta calculado para descargar desde el sensor de nivel de seguridad)
				{
					if ( tick_ms && L1_opa_bacha && cont_agua < L1_TIME_DRAINAGE*2.5 )
					{
						fsm_on_bidon();
						cont_agua ++;
					}
					else if ( tick_ms && !L1_opa_bacha && cont_agua < L1_TIME_DRAINAGE*2.5 )
					{
						fsm_on_desagote();
						cont_agua ++;
					}
					else if ( tick_ms &&
							cont_agua == L1_TIME_DRAINAGE*2.5 )
					{
						L1_ignora_sensor = false;
						cont_agua = 0;
						fsm_off_desagote();
						fsm_off_bidon();
						substate_preset = INICIO;
						Nextion_Send_OPA_OFF(REPRO1); // reinicio flag de OPA en uso
					}
				}
				else if ( !L1_ignora_sensor )
				{
					if ( tick_ms && L1_SENSOR_CALEFACTOR && L1_opa_bacha && !SENSOR_NIVEL_DESINF_4 )
					{
						fsm_on_bidon();
					}
					else if ( tick_ms && L1_SENSOR_CALEFACTOR && !L1_opa_bacha )
					{
						fsm_on_desagote();
					}
					else if ( tick_ms &&
							!L1_SENSOR_CALEFACTOR &&
							cont_agua < L1_TIME_DRAINAGE )
						cont_agua ++;
					else if ( tick_ms &&
							!L1_SENSOR_CALEFACTOR &&
							cont_agua == L1_TIME_DRAINAGE )
					{
						cont_agua = 0;
						fsm_off_desagote();
						fsm_off_bidon();
						substate_preset = INICIO;
						Nextion_Send_OPA_OFF(REPRO1); // reinicio flag de OPA en uso
					}
				}
				break;

			case TAPA_ABIERTA:
				if ( tick_ms && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// REINICIO ERROR nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_preset = INICIO;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case FALTA_DESINF:
				if ( L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					// REINICIO ERROR nextion: Falta Desinfectante en bidon
					Nextion_ChangeState(REPRO1, Lavando_NextionState); //REVISAR
					fsm_off_buzzer();
					state = IDLE;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case FALTA_DETERG:
				if ( tick_ms && SENSOR_NIVEL_DET )
				{
					fsm_off_buzzer();
					// REINICIO ERROR nextion: Falta Detergente en bidon
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_preset = INICIO;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case FALTA_ALC:
				if ( tick_ms && SENSOR_NIVEL_ALC )
				{
					fsm_off_buzzer();
					// REINICIO ERROR nextion: Falta Alcohol en bidon
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_preset = INICIO;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case PRUEBA_BLOQUEO:
				if ( tick_ms && !L1_lock_test && !L1_blocked && !L1_not_blocked )
				{
					L1_blocked = 0;
					L1_not_blocked = 1;		// salgo directamente si se deshabilita lock test
				}
				else if ( tick_ms && L1_blocked && !L1_not_blocked )
				{
					cont_buzz = 0;
					cont_puls = 0;
					cont_air = 0;
					fsm_off_desagote_sinbomba();
					fsm_reset_lock_test();
					// ERROR nextion: Prueba de Bloqueo fallida
					Nextion_Error(REPRO1, "Endosc. Obstruido o Desconect.");
					//-----<

					Ticket_EnqLockTestResultadoRtc(REPRO1, "FALLO");
					Ticket_EnqFinish(REPRO1, "FALLO", "ENDOSC. DESC o BLOQ");
					Ticket_EnqPrint(REPRO1);								// imprimo y limpio el ticket

					substate_preset = ENDOSC_BLOQ_DESCON;
				}
				else if( tick_ms && !L1_blocked && !L1_not_blocked && cont_air == TIME_CHECK )
				{
					fsm_check_pressure(); // se encarga de censar rangos de presiones: OK u obstruido/desconectado
				}
				else if ( tick_ms && !L1_blocked && !L1_not_blocked && cont_air < TIME_LOCK_TEST )
				{
					cont_air++;
					fsm_on_desagote_sinbomba();
				}
				else if ( tick_ms && !L1_blocked && L1_not_blocked )
				{
					if ( !L1_estanq_en )
					{
						fsm_reset_lock_test();
						cont_buzz = 0;
						cont_air = 0;
						cont_timeout = 0;
						fsm_off_desagote_sinbomba();
						fsm1_delay(FLUSH, F_LLENAR_CUBA, 5000);

						( L1_lock_test ) ? Ticket_EnqLockTestResultadoRtc(REPRO1, "OK") : 1;
						Ticket_EnqStepStartCached(REPRO1, "Lavado Inicial");                    // NO requiere RTC (usa cache del header)
					}
					else
					{
						fsm_reset_lock_test();
						cont_buzz = 0;
						cont_air = 0;
						cont_timeout = 0;
						fsm_off_desagote_sinbomba();

						fsm1_delay(PRUEBA_ESTANQ, PRUEBA_ESTANQUEIDAD, 5000);

						( L1_lock_test ) ? Ticket_EnqLockTestResultadoRtc(REPRO1, "OK") : 1;
					}
				}
				break;

			case ENDOSC_BLOQ_DESCON:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					fsm_off_buzzer();
					// REINICIO ERROR nextion: Endoscopio Desconectado u Obstruido
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					state = IDLE;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case PRUEBA_ESTANQ:
			switch ( substate_estanq )
			{
			case PRUEBA_ESTANQUEIDAD:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					reset_estanqueidad();
					// ERROR nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_estanq = EST_TAPA_ABIERTA;
				}
				else if( tick_ms && fuga )
				{
					// ERROR nextion: Fuga en lumenes
					Nextion_Error(REPRO1, "Fuga de lumenes");
					//-----<
					fsm_off_Estanqueidad();
					reset_estanqueidad();
					cont_buzz = 0;
					cont_puls = 0;
					fuga = 0;
					substate_estanq = FUGA_AIRE;
				}
				else if( tick_ms && pruebaOK )
				{
					cont_buzz = 0;
					pruebaOK = 0;
					fsm_on_ValvulaAire();
					reset_estanqueidad();
					substate_estanq = EXITO;
				}
				else if( tick_ms )
				{
					fsm_on_Estanqueidad();
				}
				break;

			case FUGA_AIRE:
				if( L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					// REINICIO ERROR nextion: Fuga en lumenes
					Nextion_ChangeState(REPRO1, Lavando_NextionState);// REVISAR
					fsm_off_buzzer();
					state = IDLE;

					Ticket_EnqEstanqueidadResultadoRtc(REPRO1, "FALLO");
					Ticket_EnqFinish(REPRO1, "FALLO", "FUGA AIRE LUMEN");
					Ticket_EnqPrint(REPRO1);								// imprimo y limpio el ticket
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case EXITO:
				if( tick_ms && (cont_buzz < 2 * WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == 2 * WAIT_TIME_COUNT) )
				{
					fsm_off_ValvulaAire();
					fsm_off_solenoide();
					L1_VAL08_OFF
					cont_agua = 0;
					cont_timeout = 0;
					fsm1_delay(FLUSH, F_LLENAR_CUBA, 7000);

					Ticket_EnqEstanqueidadResultadoRtc(REPRO1, "OK");
					Ticket_EnqStepStartRtc(REPRO1, "Lavado Inicial");
				}
				break;
			case EST_TAPA_ABIERTA:
				if ( tick_ms && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_estanq = PRUEBA_ESTANQUEIDAD;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case FLUSH:
			switch ( substate_flush )
			{
			case F_LLENAR_CUBA:
				if ( !L1_TIME_RECYCLE_FLUSH )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					Detergente_Arm();		// inicializacion para Dosificar Detergente
					fsm1_delay(WASHING, W_LLENAR_CUBA_DET, 2000);
					Ticket_EnqStepStartRtc(REPRO1, "Lavado con Deterg.");
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 1;
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_flush = F_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						!L1_SENSOR_CALEFACTOR &&
						cont_agua == TIMEOUT_NOWATER )
				{
					cont_agua = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (nunca se lleno una sola gota)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_flush = F_FALTA_AGUA;

					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						L1_SENSOR_CALEFACTOR &&
						cont_timeout == TIMEOUT_NOWATER2 )
				{
					cont_timeout = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (lleno algo pero nunca por completo hasta la tapa)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_flush = F_FALTA_AGUA;

					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua++;
					cont_timeout++;
					fsm_on_llenar();
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua = 0;
					cont_timeout = 0;
					cont = 0;
					fsm_off_llenar();
					recirculado = 1;		// chequeo de Presiones Recirculado
					fsm1_delay(FLUSH, F_RECIRCULAR, 3000);
				}
				break;

			case F_FALTA_AGUA:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_agua = 0;
					cont_puls = 0;
					fsm_on_desagote();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_flush = F_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case F_VACIAR_CUBA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_agua = 0;
					cont_timeout = 0;
					// Reinicio ERROR Nextion: No hay Agua! (cualquiera sea)
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_flush = F_LLENAR_CUBA;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					fsm_off_desagote();
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					if ( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					}
				}
				break;

			case F_RECIRCULAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 2;
					cont = 0;
					cont_agua = 0;
					fsm_off_recircular();
					recirculado = 0;
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_flush = F_TAPA_ABIERTA;
				}
				else if ( tick_ms &&
						cont < L1_TIME_ON &&
						cont_agua < ( L1_TIME_RECYCLE_FLUSH + 10000 ) ) 		// agrego +10 s para asegurar 1 recirculado
				{
					cont++;
					cont_agua++;
				}
				else if ( tick_ms &&
						cont == L1_TIME_ON &&
						cont_agua < ( L1_TIME_RECYCLE_FLUSH + 10000 ) )
				{
					cont = 0;
					fsm_toggle_recircular();
				}
				else if ( tick_ms &&
						cont_agua == ( L1_TIME_RECYCLE_FLUSH + 10000 ) )
				{
					cont_agua = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					fsm1_delay(FLUSH, F_DESAGÜE_PURGA_LUMEN, 3000);
				}
				break;

			case F_DESAGÜE_PURGA_LUMEN:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 3;
					cont_agua = 0;
					fsm_off_desagote();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_flush = F_TAPA_ABIERTA;
				}
				else if ( tick_ms && L1_SENSOR_CALEFACTOR )
				{
					fsm_on_desagote();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
				{
					cont_agua ++;
					fsm_on_desagote();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					cont_agua = 0;
					cont_det = 0;
					cont_timeout = 0;
					fsm_off_desagote();
					Detergente_Arm();		// inicializacion para Dosificar Detergente
					fsm1_delay(WASHING, W_LLENAR_CUBA_DET, 7000);

					Ticket_EnqStepEndRtc(REPRO1, "Lavado Inicial");
					Ticket_EnqStepStartRtc(REPRO1, "Lavado con Deterg.");
				}
				break;
			case F_TAPA_ABIERTA:
				if ( tick_ms && L1_prev_state == 1 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_flush = F_LLENAR_CUBA;
				}
				else if ( tick_ms && L1_prev_state == 2 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_flush = F_RECIRCULAR;
				}
				else if ( tick_ms && L1_prev_state == 3 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_flush = F_DESAGÜE_PURGA_LUMEN;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case WASHING:
			switch ( substate_wash )
			{
			case W_LLENAR_CUBA_DET:
				if ( !L1_TIME_RECYCLE_WASH )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					fsm1_delay(RISING, R_LLENAR_CUBA, 2000);
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA ) //#2
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 1;
					cont_agua = 0;
					cont_timeout = 0;
					cont_det = 0;
					Detergente_Stop();
					fsm_off_llenar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_wash = W_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						!L1_SENSOR_CALEFACTOR &&
						cont_agua == TIMEOUT_NOWATER ) //#3
				{
					cont_agua = 0;
					fsm_off_llenar();
					Detergente_Stop();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (no hay una sola gota)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_wash = W_FALTA_AGUA;

					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						L1_SENSOR_CALEFACTOR &&
						cont_timeout == TIMEOUT_NOWATER2 ) //#4
				{
					cont_timeout = 0;
					cont_agua = 0;
					fsm_off_llenar();
					Detergente_Stop();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (algo llena pero no llega al tope)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_wash = W_FALTA_AGUA;

					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && ( !SENSOR_NIVEL_DET || !L1_SENSOR_OPT_DET ) &&
						cont_det == TIMEOUT_DET ) //#5
				{
					cont_det = 0;
					fsm_off_llenar();
					Detergente_Stop();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Falta Detergente en bidon y manguera! (me quede sin durante)
					Nextion_Error(REPRO1, "Falta detergente en bidón");
					//-----<
					substate_wash = W_FALTA_DET;

					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN DETERG.");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && ultson_enable == ULTSON_ENABLE_VALUE &&
						L1_SENSOR_CALEFACTOR ) //#6
				{
					cont_agua = 0;
					cont_det = 0;
					cont = 0;
					cont_timeout = 0;
					cont_ultson = 0;
					fsm_off_llenar();
					fsm_on_recircular(); //-----> Enciendo para realizar la mezcla de detergente
					recirculado = 0;	// no se controlan presiones durante la desgasificacion
					ultson_cycle_state = ULTSON_CYCLE_START;
					fsm1_delay(WASHING, W_DESGASIFICAR, 3000);
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA ) //#7
				{
					cont_agua ++;
					cont_timeout ++;
					cont_det++;
					fsm_on_llenar();
					iniciarDetergente = 1;		// arranca logica de dosificacion
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA &&
						L1_WASHING_HEATING ) //#8
				{
					cont_agua = 0;
					cont_det = 0;
					cont = 0;
					cont_timeout = 0;
					cont_heating = 0;
					L1_initialTemp = L1_realTemp;
					fsm_off_llenar();
					recirculado = 1;		// chequeo de Presiones Recirculado
					fsm1_delay(WASHING, W_CALENTAR, 3000);
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA &&
						!L1_WASHING_HEATING ) //#9
				{
					cont_agua = 0;
					cont_det = 0;
					cont = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					recirculado = 1;	// chequeo de Presiones Recirculado
					fsm1_delay(WASHING, W_RECIRCULAR, 3000);
				}
				break;

			case W_FALTA_AGUA:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_agua = 0;
					cont_puls = 0;
					fsm_on_desagote();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_wash = W_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case W_FALTA_DET:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_puls = 0;
					fsm_on_desagote();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_wash = W_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case W_CALENTAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA ) //1
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 2;
					cont = 0;
					cont_timeout = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_wash = W_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_WH_TIMEOUT && !L1_realTemp ) //2
				{
					cont = 0;
					cont_timeout = 0;
					cont_heating = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: El sensor de temperatura OneWire está defectuoso
					Nextion_Error(REPRO1, "Falla en sensor de temp");
					//-----<
					substate_wash = W_FALLA_CALEF;

					Ticket_EnqEventRtc(REPRO1, "ERROR: FALLA SENS TEMP");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_WH_TIMEOUT && ( L1_realTemp < L1_initialTemp + 3 ) &&
						cont_timeout == TIMEOUT_CALEFACTOR ) //3
				{
					cont = 0;
					cont_timeout = 0;
					cont_heating = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: El calefactor esta defectuoso
					Nextion_Error(REPRO1, "Falla en calefactor");
					//-----<
					substate_wash = W_FALLA_CALEF;

					Ticket_EnqEventRtc(REPRO1, "ERROR: FALLA CALEF");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_WH_TIMEOUT && ( L1_realTemp < L1_TEMP_WASHING_MAX ) &&
						  cont < L1_TIME_ON ) //4
				{
					cont ++;
					( cont_timeout < TIMEOUT_CALEFACTOR ) ? cont_timeout++ : 1;	// if ? true : false
					L1_CLF11_ON
				}
				else if ( tick_ms && L1_WH_TIMEOUT && cont < L1_TIME_ON &&
						  cont_heating < HEATING_TIME ) //5
				{
					cont ++;
					cont_heating++;
					L1_CLF11_ON
				}
				else if ( tick_ms && ( ( cont_heating == HEATING_TIME )
						|| ( L1_realTemp >= L1_TEMP_WASHING_MAX ) ) ) //6
				{
					cont = 0;
					cont_timeout = 0;
					cont_heating = 0;
					fsm_off_recircular();
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					L1_CLF11_OFF
					if ( ultson_enable == ULTSON_ENABLE_VALUE )
					{
						recirculado = 0;	// no se controlan presiones durante la desgasificacion
						fsm1_delay(WASHING, W_RECIRCULAR, 3000);
					}
					else
					{
						recirculado = 1;	// chequeo de Presiones Recirculado
						fsm1_delay(WASHING, W_RECIRCULAR, 1000);
					}
				}
				else if ( tick_ms && cont == L1_TIME_ON )
				{
					cont = 0;
					fsm_toggle_recircular();
				}
				break;

			case W_FALLA_CALEF:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_puls = 0;
					fsm_on_desagote();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_wash = W_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case W_VACIAR_CUBA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_agua = 0;
					cont_det = 0;
					cont_timeout = 0;
					// Reinicio ERROR Nextion: No hay Agua! (cualquiera sea) / Falla Calefactor / Falta Detergente
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					Detergente_Arm();		// inicializacion para Dosificar Detergente
					substate_wash = W_LLENAR_CUBA_DET;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					fsm_off_desagote();
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					if( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					}
				}
				break;

			case W_DESGASIFICAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA ) //1
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 5;
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					UltSon_Apagar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_wash = W_TAPA_ABIERTA;
				}
				else if ( tick_ms && ultson_enable != ULTSON_ENABLE_VALUE ) //2
				{
					cont_ultson = 0;
					cont_timeout = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					UltSon_Apagar();
					recirculado = 0;
					fsm1_delay(WASHING, W_COMPLETAR_LLENADO, 1000);
				}
				else if ( tick_ms && cont_ultson == 0 ) //3
				{
					UltSon_Encender();
					cont_ultson++;
				}
				else if ( tick_ms && cont_ultson < TIME_GASIFICACION )//4
				{
					cont_ultson++;
					if(cont_ultson > (TIME_GASIFICACION / 4)) //----> Mezcla de detergente
						fsm_off_recircular(); //---> apago la mezcla
				}
				else if ( tick_ms && cont_ultson == TIME_GASIFICACION ) //5
				{
					cont_ultson = 0;
					cont_timeout = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					UltSon_Apagar();
					recirculado = 0;
					fsm1_delay(WASHING, W_COMPLETAR_LLENADO, 1000);
				}
				break;

			case W_COMPLETAR_LLENADO:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA ) //1
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 6;
					cont_timeout = 0;
					fsm_off_llenar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_wash = W_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						cont_timeout == TIMEOUT_NOWATER2 ) //2
				{
					cont_timeout = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					Nextion_Error(REPRO1, "No hay agua");
					substate_wash = W_FALTA_AGUA;

					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA )
				{
					cont_timeout++;
					fsm_on_llenar();
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua = 0;
					cont = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					recirculado = 1;	// chequeo de Presiones Recirculado
					if (L1_WASHING_HEATING)
						fsm1_delay(WASHING, W_CALENTAR, 3000);
					else
						fsm1_delay(WASHING, W_RECIRCULAR, 3000);
				}
				break;

			case W_RECIRCULAR:
				if ( tick_ms && L1_SENSOR_IMAN_TAPA )
				{
					if ( ultson_enable == ULTSON_ENABLE_VALUE )
					{
						switch ( ultson_cycle_state )
						{
						case ULTSON_CYCLE_START:
							cont_ultson = 0;
							UltSon_Encender();
							ultson_cycle_state = ULTSON_CYCLE_ON;
							break;

						case ULTSON_CYCLE_ON:
							if ( cont_ultson < TIME_ULTSON_ON )
							{
								cont_ultson++;
							}
							else
							{
								cont_ultson = 0;
								UltSon_Apagar();
								ultson_cycle_state = ULTSON_CYCLE_OFF;
							}
							break;

						case ULTSON_CYCLE_OFF:
							if ( cont_ultson < TIME_ULTSON_OFF )
							{
								cont_ultson++;
							}
							else
							{
								cont_ultson = 0;
								UltSon_Encender();
								ultson_cycle_state = ULTSON_CYCLE_ON;
							}
							break;
						}
					}
					else if ( ultson_cycle_state != ULTSON_CYCLE_START )
					{
						cont_ultson = 0;
						UltSon_Apagar();
						ultson_cycle_state = ULTSON_CYCLE_START;
					}
				}

				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 3;
					cont = 0;
					cont_agua = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					UltSon_Apagar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_wash = W_TAPA_ABIERTA;
				}
				else if ( tick_ms && ( cont_agua < ( L1_TIME_RECYCLE_WASH + 10000 ) ) &&      // agrego + 10 s para garantizar 1 recirculado
						cont < L1_TIME_ON )
				{
					cont_agua ++;
					cont ++;
				}
				else if ( tick_ms && cont == L1_TIME_ON )
				{
					cont = 0;
					fsm_toggle_recircular();
				}
				else if ( tick_ms && ( cont_agua == ( L1_TIME_RECYCLE_WASH + 10000 ) ) )
				{
					cont_agua = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					UltSon_Apagar();
					fsm1_delay(WASHING, W_DESAGUE_PURGA_LUMEN, 3000);
				}
				break;

			case W_DESAGUE_PURGA_LUMEN:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 4;
					cont_agua = 0;
					fsm_off_desagote();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_wash = W_TAPA_ABIERTA;
				}
				else if ( tick_ms && L1_SENSOR_CALEFACTOR )
				{
					fsm_on_desagote();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && cont_agua < L1_TIME_DRAINAGE )
				{
					cont_agua ++;
					fsm_on_desagote();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && cont_agua == L1_TIME_DRAINAGE )
				{
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_desagote();
					fsm1_delay(RISING, R_LLENAR_CUBA, 7000);

					Ticket_EnqStepEndRtc(REPRO1, "Lavado con Deterg.");
					Ticket_EnqStepStartRtc(REPRO1, "Enjuague");
				}
				break;
			case W_TAPA_ABIERTA:
				if ( tick_ms && L1_prev_state == 1 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_wash = W_LLENAR_CUBA_DET;
				}
				else if ( tick_ms && L1_prev_state == 2 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;		// chequeo de Presiones Recirculado
					L1_initialTemp = L1_realTemp;
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_wash = W_CALENTAR;
				}
				else if ( tick_ms && L1_prev_state == 3 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;		// chequeo de Presiones Recirculado
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_wash = W_RECIRCULAR;
				}
				else if ( tick_ms && L1_prev_state == 5 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					cont_ultson = 0;
					ultson_cycle_state = ULTSON_CYCLE_START;
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_wash = W_DESGASIFICAR;
				}
				else if ( tick_ms && L1_prev_state == 6 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					cont_timeout = 0;
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_wash = W_COMPLETAR_LLENADO;
				}
				else if ( tick_ms && L1_prev_state == 4 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_wash = W_DESAGUE_PURGA_LUMEN;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case RISING:
			switch ( substate_rising )
			{
			case R_LLENAR_CUBA:
				if ( !L1_TIME_RECYCLE_RISING )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					fsm1_delay(DESINFECT, D_LLENAR_CUBA_DESINF, 2000);
					Nextion_Send_OPA_ON(REPRO1);
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 1;
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_rising = R_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						!L1_SENSOR_CALEFACTOR &&
						cont_agua == TIMEOUT_NOWATER )
				{
					cont_agua = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (ni una gota)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_rising = R_FALTA_AGUA;
					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA &&
						L1_SENSOR_CALEFACTOR &&
						cont_timeout == TIMEOUT_NOWATER2 )
				{
					cont_timeout = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay agua! (algo llena pero nunca llega al tope)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_rising = R_FALTA_AGUA;
					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");		// agrego cuando ocurrió el error en el ticket
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua ++;
					cont_timeout ++;
					fsm_on_llenar();
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua = 0;
					cont_timeout = 0;
					cont = 0;
					fsm_off_llenar();
					recirculado = 1;		// chequeo de Presiones Recirculado
					fsm1_delay(RISING, R_RECIRCULAR, 3000);
				}
				break;

			case R_FALTA_AGUA:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_agua = 0;
					cont_puls = 0;
					fsm_on_desagote();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_rising = R_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case R_VACIAR_CUBA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_agua = 0;
					cont_timeout = 0;
					// Reinicio ERROR Nextion: No hay Agua! (cualquiera sea)
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_rising = R_LLENAR_CUBA;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					fsm_off_desagote();
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					if ( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					}
				}
				break;

			case R_RECIRCULAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 2;
					cont = 0;
					cont_agua = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_rising = R_TAPA_ABIERTA;
				}
				else if ( tick_ms && cont < L1_TIME_ON && cont_agua < ( L1_TIME_RECYCLE_RISING + 10000 ) ) 		// agregado para garantizar 1 recirculado
				{
					cont++;
					cont_agua ++;
				}
				else if ( tick_ms && cont == L1_TIME_ON && cont_agua < ( L1_TIME_RECYCLE_RISING + 10000 ) )
				{
					cont = 0;
					fsm_toggle_recircular();
				}
				else if ( tick_ms && cont_agua == ( L1_TIME_RECYCLE_RISING + 10000 ) )
				{
					cont_agua = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					fsm1_delay(RISING, R_DESAGUE_PURGA_LUMEN, 3000);
				}
				break;

			case R_DESAGUE_PURGA_LUMEN:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 3;
					cont_agua = 0;
					fsm_off_desagote();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_rising = R_TAPA_ABIERTA;
				}
				else if ( tick_ms && L1_SENSOR_CALEFACTOR )
				{
					fsm_on_desagote();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && cont_agua < L1_TIME_DRAINAGE )
				{
					cont_agua ++;
					fsm_on_desagote();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && cont_agua == L1_TIME_DRAINAGE )
				{
					cont_agua = 0;
					cont_desinf = 0;
					cont_timeout = 0;
					cont_heating = 0;
					fsm_off_desagote();
					fsm1_delay(DESINFECT, D_LLENAR_CUBA_DESINF, 7000);
					Nextion_Send_OPA_ON(REPRO1);
					Ticket_EnqStepEndRtc(REPRO1, "Enjuague");
					Ticket_EnqStepStartRtc(REPRO1, "Lavado con Desinf.");
				}
				break;
			case R_TAPA_ABIERTA:
				if ( tick_ms && L1_prev_state == 1 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_rising = R_LLENAR_CUBA;
				}
				else if ( tick_ms && L1_prev_state == 2 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;		// chequeo de Presiones Recirculado
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_rising = R_RECIRCULAR;
				}
				else if ( tick_ms && L1_prev_state == 3 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_rising = R_DESAGUE_PURGA_LUMEN;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case DESINFECT:
			switch ( substate_desinf )
			{
			case D_LLENAR_CUBA_DESINF:
				if ( !L1_TIME_RECYCLE_DESINFECT )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					fsm1_delay(FINAL_RISING, FR_LLENAR_CUBA, 2000);
					L1_vuelta = 1;
					Nextion_Send_OPA_OFF(REPRO1);
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 1;
					fsm_off_llenar_desinf();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_desinf = D_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_SENSOR_NIVEL_TAPA )
				{
					fsm_on_llenar_desinf();
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA && L1_DESINF_HEATING )
				{
					L1_initialTemp = L1_realTemp;
					fsm_off_llenar_desinf();
					recirculado = 1;		// chequeo de Presiones Recirculado
					fsm1_delay(DESINFECT, D_CALENTAR, 3000);
				}
				else if ( tick_ms && L1_SENSOR_NIVEL_TAPA && !L1_DESINF_HEATING )
				{
					fsm_off_llenar_desinf();
					recirculado = 1;		// chequeo de Presiones Recirculado
					fsm1_delay(DESINFECT, D_RECIRCULAR, 3000);
				}
				break;

			case D_CALENTAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 2;
					cont = 0;
					cont_timeout = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_desinf = D_TAPA_ABIERTA;
				}
				else if ( tick_ms && !L1_DH_TIMEOUT && !L1_realTemp )
				{
					cont = 0;
					cont_timeout = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Sensor de temperatura defectuoso
					Nextion_Error(REPRO1, "Falla en sensor de temp");
					//-----<
					substate_desinf = D_FALLA_CALEF;
					Ticket_EnqEventRtc(REPRO1, "ERROR: FALLA SENS TEMP");
				}
				else if ( tick_ms && !L1_DH_TIMEOUT && L1_realTemp < ( L1_initialTemp + 3 ) &&
						cont_timeout == TIMEOUT_CALEFACTOR	)
				{
					cont = 0;
					cont_timeout = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Calefactor defectuoso
					Nextion_Error(REPRO1, "Falla en calefactor");
					//-----<
					substate_desinf = D_FALLA_CALEF;
					Ticket_EnqEventRtc(REPRO1, "ERROR: FALLA CALEF");
				}
				else if ( tick_ms && !L1_DH_TIMEOUT && L1_realTemp < L1_TEMP_DESINF_MAX &&
						cont < L1_TIME_ON )
				{
					cont++;
					( cont_timeout < TIMEOUT_CALEFACTOR ) ? cont_timeout++ : 1;
					L1_CLF11_ON
				}
				else if ( tick_ms && L1_DH_TIMEOUT && cont < L1_TIME_ON && cont_heating < HEATING_TIME )
				{
					cont++;
					cont_heating++;
					L1_CLF11_ON
				}
				else if ( tick_ms && ( ( cont_heating == HEATING_TIME ) || ( L1_realTemp >= L1_TEMP_DESINF_MAX ) ) )
				{
					cont_desinf = 0;
					cont = 0;
					cont_timeout = 0;
					cont_heating = 0;
					fsm_off_recircular();
					recirculado = 1;		// chequeo de Presiones Recirculado
					L1_CLF11_OFF
					fsm1_delay(DESINFECT, D_RECIRCULAR, 3000);
				}
				else if ( tick_ms && cont == L1_TIME_ON )
				{
					cont = 0;
					fsm_toggle_recircular();
				}
				break;

			case D_FALLA_CALEF:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_puls = 0;
					fsm_on_bidon();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_desinf = D_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case D_VACIAR_CUBA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_agua = 0;
					cont_desinf = 0;
					cont_timeout = 0;
					cont_heating = 0;
					// Reinicio ERROR Nextion: Calefactor Defectuoso
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_desinf = D_LLENAR_CUBA_DESINF;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					fsm_off_bidon();
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					if ( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					}
				}
				break;

			case D_RECIRCULAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 3;
					cont_desinf = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_desinf = D_TAPA_ABIERTA;
				}
				else if ( tick_ms && cont_desinf < ( L1_TIME_RECYCLE_DESINFECT + 10000 ) &&		// agregado para garantizar 1 recirculado
						cont < L1_TIME_ON )
				{
					cont_desinf ++;
					cont ++;
				}
				else if ( tick_ms && cont == L1_TIME_ON )
				{
					cont = 0;
					fsm_toggle_recircular();
				}
				else if ( tick_ms && cont_desinf == ( L1_TIME_RECYCLE_DESINFECT + 10000 ) )
				{
					cont_desinf = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					fsm1_delay(DESINFECT, D_BIDON_PURGA_LUMEN, 3000);
				}
				break;

			case D_BIDON_PURGA_LUMEN:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 4;
					cont_desinf = 0;
					fsm_off_bidon();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_desinf = D_TAPA_ABIERTA;
				}
				else if ( tick_ms && L1_SENSOR_CALEFACTOR && !SENSOR_NIVEL_DESINF_4 )
				{
					fsm_on_bidon();
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && !SENSOR_NIVEL_DESINF_4 && cont_desinf < L1_TIME_DRAINAGE )
				{
					cont_desinf ++;
					fsm_on_bidon();
				}
				else if ( tick_ms && ( ( !L1_SENSOR_CALEFACTOR && cont_desinf == L1_TIME_DRAINAGE ) || SENSOR_NIVEL_DESINF_4 ) )
				{
					cont_desinf = 0;
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_bidon();
					fsm1_delay(FINAL_RISING, FR_LLENAR_CUBA, 7000);
					L1_vuelta = 1;
					Ticket_EnqStepEndRtc(REPRO1, "Lavado con Desinf.");
					Ticket_EnqStepStartRtc(REPRO1, "Enjuague Final");
					Nextion_Send_OPA_OFF(REPRO1);
				}
				break;
			case D_TAPA_ABIERTA:
				if ( tick_ms && L1_prev_state == 1 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_desinf = D_LLENAR_CUBA_DESINF;
				}
				else if ( tick_ms && L1_prev_state == 2 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;		// chequeo de Presiones Recirculado
					L1_initialTemp = L1_realTemp;
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_desinf = D_CALENTAR;
				}
				else if ( tick_ms && L1_prev_state == 3 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;		// chequeo de Presiones Recirculado
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_desinf = D_RECIRCULAR;
				}
				else if ( tick_ms && L1_prev_state == 4 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_desinf = D_BIDON_PURGA_LUMEN;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case FINAL_RISING:
			switch ( substate_f_rise )
			{
			case FR_LLENAR_CUBA:
				if ( !L1_TIME_RECYCLE_F_RISING )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					cont_alc = 0;
					fsm1_delay(DRYING, DR_ALCOHOL, 2000);
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 1;
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_f_rise = FR_TAPA_ABIERTA;
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						!L1_SENSOR_NIVEL_TAPA &&
						!L1_SENSOR_CALEFACTOR &&
						cont_agua == TIMEOUT_NOWATER )
				{
					cont_agua = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (ni una gota)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_f_rise = FR_FALTA_AGUA;
					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						!L1_SENSOR_NIVEL_TAPA &&
						L1_SENSOR_CALEFACTOR &&
						cont_timeout == TIMEOUT_NOWATER2 )
				{
					cont_timeout = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (llena algo pero nunca llega al tope)
					Nextion_Error(REPRO1, "No hay agua");
					//-----<
					substate_f_rise = FR_FALTA_AGUA;
					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN AGUA");
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						!L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua++;
					cont_timeout++;
					fsm_on_llenar();
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					recirculado = 1;		// chequeo de Presiones Recirculado
					fsm1_delay(FINAL_RISING, FR_RECIRCULAR, 3000);
				}
				else if ( tick_ms && ( L1_vuelta == 3 ) )
				{
					substate_f_rise = FR_RECIRCULAR;
				}
				break;

			case FR_FALTA_AGUA:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_agua = 0;
					cont_puls = 0;
					fsm_on_desagote();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_f_rise = FR_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case FR_VACIAR_CUBA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_desinf = 0;
					cont_agua = 0;
					cont_timeout = 0;
					// Reinicio ERROR Nextion: No hay Agua! (cualquiera sea)
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_f_rise = FR_LLENAR_CUBA;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					fsm_off_desagote();
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					if ( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					}
				}
				break;

			case FR_RECIRCULAR:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 2;
					cont_agua = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_f_rise = FR_TAPA_ABIERTA;
				}
				else if ( tick_ms && ( L1_vuelta == 1 ) &&
						cont_agua < ( L1_TIME_RECYCLE_F_RISING * 1/6 ) )
				{
					cont_agua++;
					fsm_on_recircular();
				}
				else if ( tick_ms && ( L1_vuelta == 1 ) &&
						cont_agua == ( L1_TIME_RECYCLE_F_RISING * 1/6 ) )
				{
					cont_agua = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					fsm1_delay(FINAL_RISING, FR_DESAGUE_PURGA_LUMEN, 3000);
				}
				else if ( tick_ms && ( L1_vuelta == 2 ) &&
						cont_agua < ( ( L1_TIME_RECYCLE_F_RISING + 12000 ) * 5/6 ) )		// garantizo 1 recirculado
				{
					cont_agua++;
					fsm_on_recircular();
				}
				else if ( tick_ms && ( L1_vuelta == 2 ) &&
						cont_agua == ( ( L1_TIME_RECYCLE_F_RISING + 12000 ) * 5/6 ) )
				{
					cont_agua = 0;
					cont = 0;
					fsm_off_recircular();
					recirculado = 0;		// chequeo de Presiones Recirculado
					fsm1_delay(FINAL_RISING, FR_DESAGUE_PURGA_LUMEN, 3000);
				}
				else if ( tick_ms && ( L1_vuelta == 3 ) )
				{
					substate_f_rise = FR_DESAGUE_PURGA_LUMEN;
				}
				break;

			case FR_DESAGUE_PURGA_LUMEN:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 3;
					cont_agua = 0;
					fsm_off_desagote();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_f_rise = FR_TAPA_ABIERTA;
				}
				else if( tick_ms && ( L1_vuelta < 3 ) &&
						L1_SENSOR_CALEFACTOR )
				{
					fsm_on_desagote();
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						!L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
				{
					cont_agua ++;
					fsm_on_desagote();
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						!L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					cont_agua = 0;
					fsm_off_desagote();
					fsm1_delay(FINAL_RISING, FR_LLENAR_CUBA, 7000);
					L1_vuelta++;
				}
				else if ( tick_ms && ( L1_vuelta == 3 ) )
				{
					L1_vuelta = 1;
					cont_alc = 0;
					fsm1_delay(DRYING, DR_ALCOHOL, 3000);
					Ticket_EnqStepEndRtc(REPRO1, "Enjuague Final");
					Ticket_EnqStepStartRtc(REPRO1, "Secado con Alco");
				}
				break;
			case FR_TAPA_ABIERTA:
				if ( tick_ms && L1_prev_state == 1 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_f_rise = FR_LLENAR_CUBA;
				}
				else if ( tick_ms && L1_prev_state == 2 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					recirculado = 1;		// chequeo de Presiones Recirculado
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_f_rise = FR_RECIRCULAR;
				}
				else if ( tick_ms && L1_prev_state == 3 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_f_rise = FR_DESAGUE_PURGA_LUMEN;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;

		case DRYING:
			switch ( substate_dry )
			{
			case DR_ALCOHOL:
				if ( !L1_TIME_ALCOHOL )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					cont_buzz = 0;
					cont_air = 0;
					fsm1_delay(DRYING, DR_SECADO, 1000);
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 1;
					cont_alc = 0;
					L1_BPR06_OFF
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_dry = DR_TAPA_ABIERTA;
				}
				else if ( tick_ms && ( !SENSOR_NIVEL_ALC || !L1_SENSOR_OPT_ALC ) &&
						cont_alc == L1_TIME_ALCOHOL )
				{
					cont_alc = 0;
					L1_BPR06_OFF
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Falta Alcohol en bidones! (me quede sin alcohol durante)
					Nextion_Error(REPRO1, "Falta alcohol en bidón");
					//-----<
					substate_dry = DR_FALTA_ALC;
					Ticket_EnqEventRtc(REPRO1, "ERROR: SIN ALCO");
				}
				else if ( tick_ms && ( SENSOR_NIVEL_ALC || L1_SENSOR_OPT_ALC ) &&
						cont_alc < L1_TIME_ALCOHOL )
				{
					cont_alc ++;
					L1_BPR06_ON
				}
				else if ( tick_ms && ( SENSOR_NIVEL_ALC || L1_SENSOR_OPT_ALC ) &&
						cont_alc == L1_TIME_ALCOHOL )
				{
					cont_alc = 0;
					cont_air = 0;
					L1_BPR06_OFF
					fsm1_delay(DRYING, DR_SECADO, 3000);
				}
				break;

			case DR_FALTA_ALC:
				if ( tick_ms && cont_puls == 10 )
				{
					cont_puls = 0;
					fsm_on_desagote_sinbomba();
					fsm_off_buzzer();
					flag_nextion = 1;
					substate_dry = DR_VACIAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case DR_VACIAR_CUBA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_alc = 0;
					// Reinicio ERROR Nextion: Sin alcohol
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_dry = DR_ALCOHOL;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					fsm_off_desagote_sinbomba();
					// BOTON NEXTION mostrar vis Boton continuar y vis boton Finalizar Lavado
					if ( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					}
				}
				break;

			case DR_SECADO:
				if ( !L1_TIME_DRYING )	// si el tiempo seteado es 0 entonces me voy al siguiente estado
				{
					cont_buzz = 0;
					cont_air = 0;
					fsm1_delay(DRYING, DR_FIN, 3000);
					flag_nextion = 1;
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 2;
					cont_air = 0;
					fsm_off_desagote_sinbomba();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_dry = DR_TAPA_ABIERTA;
				}
				else if ( tick_ms && cont_air < L1_TIME_DRYING )
				{
					cont_air ++;
					fsm_on_desagote_sinbomba();
				}
				else if ( tick_ms && cont_air == L1_TIME_DRYING )
				{
					cont_buzz = 0;
					cont_air = 0;
					fsm_off_desagote_sinbomba();
					substate_dry = DR_FIN;
					flag_nextion = 1;
				}
				break;

			case DR_FIN:
				if ( tick_ms && L1_botonIdle )
				{
					L1_botonIdle = 0;
					state = IDLE;
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					L1_prev_state = 3;
					fsm_off_buzzer();
					// ERROR Nextion: Tapa Abierta!
					Nextion_Error(REPRO1, "Tapa abierta");
					//-----<
					substate_dry = DR_TAPA_ABIERTA;
				}
				else if ( tick_ms && cont_buzz < 3000 )
				{
					cont_buzz ++;
					fsm_on_buzzer();
				}
				else if ( tick_ms && cont_buzz == 3000 )
				{
					fsm_off_buzzer();
					// BOTON NEXTION
					if ( flag_nextion )
					{
						flag_nextion = 0;
						Nextion_ChangeState(REPRO1, Finalizado_NextionState);
						Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Ocultar_NextionButtons);

						Ticket_EnqStepEndRtc(REPRO1, "Secado con Alco");
						Ticket_EnqFinish(REPRO1, "OK", "");
						Ticket_EnqPrint(REPRO1);
					}
				}
				break;
			case DR_TAPA_ABIERTA:
				if ( tick_ms && L1_prev_state == 1 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_dry = DR_ALCOHOL;
				}
				else if ( tick_ms && L1_prev_state == 2 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Lavando_NextionState);
					substate_dry = DR_SECADO;
				}
				else if ( tick_ms && L1_prev_state == 3 && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta
					Nextion_ChangeState(REPRO1, Finalizado_NextionState);
					Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
					substate_dry = DR_FIN;
				}
				else if ( tick_ms && cont_buzz < 1000 )
					cont_buzz ++;
				else if ( tick_ms && cont_buzz == WAIT_TIME_COUNT && cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;
			}
			break;
			/*-----------------------------------------------*/
			/*												 */
			/* 				Replace Desinfectant			 */
			/*												 */
			/*-----------------------------------------------*/
		case DRAIN_START:
			switch (substate_drain)
			{
			case DRS_INICIO:
				if ( !SENSOR_NIVEL_DESINF_1 )
				{
					// Modificar texto boton de Drain Start a: DRAIN START
					Nextion_FinDrain();
					state = IDLE;
				}
				else if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Tapa Abierta!!
					substate_drain = DRS_TAPA_ABIERTA;
				}
				else if ( tick_ms && SENSOR_NIVEL_DESINF_1 )
				{
					L1_VAL08_ON2
					cont_desinf = 0;
					fsm1_delay(DRAIN_START, DRS_DESAGUE, 3000);
				}
				break;

			case DRS_TAPA_ABIERTA:
				if ( tick_ms && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta!!
					substate_drain = DRS_INICIO;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case DRS_DESAGUE:
				if ( tick_ms && SENSOR_NIVEL_DESINF_1 )
				{
					fsm_on_llenar_desinf();
					L1_BOM01_ON
				}
				else if ( tick_ms && !SENSOR_NIVEL_DESINF_1 && cont_desinf < L1_TIME_DRAINAGE )
				{
					cont_desinf ++;
				}
				else if ( tick_ms && !SENSOR_NIVEL_DESINF_1 && cont_desinf == L1_TIME_DRAINAGE )
				{
					fsm_off_llenar_desinf();
					L1_BOM01_OFF
					L1_VAL08_OFF
					// Nextion: Drain Completed!
					Nextion_FinDrain();
					fsm1_delay(IDLE, 0, 7000);
				}
				break;
			}
			break;

		case DOSING_START:
			switch (substate_dosing)
			{
			case DOS_INICIO:
				if ( tick_ms && !L1_SENSOR_CALEFACTOR )
				{
					// Modificar texto boton de Dosing Start a: DOSING START
					Nextion_FinDosing();
					state = IDLE;
				}
				else if ( tick_ms && SENSOR_NIVEL_DESINF_4 ) // FULL Bidon!
				{
					// ERROR Nextion: STOP DOSING!
					state = IDLE;
				}
				else if ( tick_ms && L1_SENSOR_CALEFACTOR &&
						  !SENSOR_NIVEL_DESINF_3 &&
						  !SENSOR_NIVEL_DESINF_4 )
				{
					L1_VAL08_ON1
					cont_desinf = 0;
					fsm1_delay(DOSING_START, DOS_BIDON_NO_HIGH, 3000);
				}
				else if ( tick_ms && L1_SENSOR_CALEFACTOR &&
						  SENSOR_NIVEL_DESINF_3 &&
						  !SENSOR_NIVEL_DESINF_4 )		// para llenar mas alla del HIGH
				{
					cont_desinf = 0;
					substate_dosing = DOS_CONFIRMACION;
				}
				break;

			case DOS_BIDON_NO_HIGH:
				if ( tick_ms && L1_SENSOR_CALEFACTOR )
				{
					L1_BOM01_ON
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && cont_desinf < L1_TIME_DRAINAGE )
				{
					cont_desinf ++;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && cont_desinf == L1_TIME_DRAINAGE )
				{
					L1_BOM01_OFF
					L1_VAL08_OFF
					// Nextion: Dosing Completed!
					Nextion_FinDosing();
					fsm1_delay(IDLE, 0, 7000);
				}
				break;

			case DOS_CONFIRMACION:
				if ( tick_ms && L1_botonIdle ) // nextion me envia esto
				{
					L1_botonIdle = 0;
					state = IDLE;
				}
				else if ( tick_ms && L1_botonAceptar ) // nextion me envia esto
				{
					L1_botonAceptar = 0;
					substate_dosing = DOS_BIDON_HIGH;
				}
				break;

			case DOS_BIDON_HIGH:
				if ( tick_ms && L1_SENSOR_CALEFACTOR && !SENSOR_NIVEL_DESINF_4 )
				{
					L1_VAL08_ON1
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && !SENSOR_NIVEL_DESINF_4 && cont_desinf < L1_TIME_DRAINAGE )
				{
					cont_desinf++;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR && !SENSOR_NIVEL_DESINF_4 && cont_desinf == L1_TIME_DRAINAGE )
				{
					L1_VAL08_OFF
					// Nextion: Dosing Completed!
					Nextion_FinDosing();
					fsm1_delay(IDLE, 0, 7000);
				}
				else if ( tick_ms && SENSOR_NIVEL_DESINF_4 )
				{
					L1_VAL08_OFF
					// Nextion: Dosing Completed!
					fsm1_delay(IDLE, 0, 7000);
				}
				break;
			}
			break;

		case CLEANING_RECYCLE_BOX:
			switch (substate_clean_box)
			{
			case CRB_INICIO:
				if ( tick_ms && !L1_SENSOR_IMAN_TAPA )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Tapa Abierta!!
					Nextion_CleanRecBox_ErrorTapa();
					substate_clean_box = CRB_TAPA_ABIERTA;
				}
				else if ( SENSOR_NIVEL_DESINF_4 ||
						  SENSOR_NIVEL_DESINF_3 ||
						  SENSOR_NIVEL_DESINF_2 ||
						  SENSOR_NIVEL_DESINF_1 )
				{
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: Drenar Desinfectante para lavar bidon!
					Nextion_CleanRecBox_ErrorNivel();
					substate_clean_box = CRB_DESINFECTANTE;
				}
				else if ( tick_ms )
				{
					cont_agua = 0;
					cont_timeout = 0;
					L1_vuelta = 1;
					// NEXTION
					Nextion_CleanRecBox_n_lavado1();
					substate_clean_box = CRB_LLENAR_CUBA;
				}
				break;

			case CRB_TAPA_ABIERTA:
				if ( tick_ms && L1_SENSOR_IMAN_TAPA )
				{
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Tapa Abierta!!
					Nextion_CleanRecBox_Reset_ErrorAgua();
					substate_clean_box = CRB_INICIO;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case CRB_DESINFECTANTE:
				if ( tick_ms && L1_botonIdle )
				{
					L1_botonIdle = 0;
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: Drenar Desinfectante para lavar bidon! (HECHO)
					state = IDLE;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case CRB_LLENAR_CUBA:
				if ( tick_ms && L1_botonStop )
				{
					fsm_off_llenar();
					L1_prev_state = 1;
					substate_clean_box = CRB_STOP;
					L1_botonStop = 0;
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						  !L1_SENSOR_NIVEL_TAPA &&
						  !L1_SENSOR_CALEFACTOR &&
						  cont_agua == TIMEOUT_NOWATER )
				{
					cont_agua = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (ni una gota)
					Nextion_CleanRecBox_ErrorAgua();
					substate_clean_box = CRB_FALTA_AGUA;
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						  !L1_SENSOR_NIVEL_TAPA &&
						  L1_SENSOR_CALEFACTOR &&
						  cont_timeout == TIMEOUT_NOWATER2 )
				{
					cont_timeout = 0;
					fsm_off_llenar();
					cont_buzz = 0;
					cont_puls = 0;
					// ERROR Nextion: No hay Agua! (llena algo pero nunca llega al tope)
					Nextion_CleanRecBox_ErrorAgua();
					substate_clean_box = CRB_FALTA_AGUA;
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						  !L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua++;
					cont_timeout++;
					fsm_on_llenar();
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) &&
						  L1_SENSOR_NIVEL_TAPA )
				{
					cont_agua = 0;
					cont_timeout = 0;
					fsm_off_llenar();
					fsm1_delay(CLEANING_RECYCLE_BOX, CRB_BIDON, 5000);
				}
				else if ( tick_ms && ( L1_vuelta == 3 ) )
				{
					substate_clean_box = CRB_BIDON;
				}
				break;

			case CRB_FALTA_AGUA:
				if ( tick_ms && L1_botonContinuar )
				{
					L1_botonContinuar = 0;
					cont_agua = 0;
					cont_timeout = 0;
					cont_puls = 0;
					fsm_off_buzzer();
					// Reinicio ERROR Nextion: No hay Agua! (cualquiera)
					Nextion_CleanRecBox_Reset_ErrorAgua();
					substate_clean_box = CRB_LLENAR_CUBA;
				}
				else if( tick_ms && (cont_buzz < WAIT_TIME_COUNT) )
					cont_buzz ++;
				else if( tick_ms && (cont_buzz == WAIT_TIME_COUNT) &&
						cont_puls < 10 )
				{
					cont_buzz = 0;
					cont_puls ++;
					fsm_toggleBuzzer();
				}
				break;

			case CRB_BIDON:
				if ( tick_ms && ( L1_vuelta < 3 ) && L1_SENSOR_CALEFACTOR )
				{
					L1_VAL08_ON1
					L1_BOM01_ON
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) && !L1_SENSOR_CALEFACTOR &&
						  cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && ( L1_vuelta < 3 ) && SENSOR_NIVEL_DESINF_1 &&
						  !L1_SENSOR_CALEFACTOR &&
						  cont_agua == L1_TIME_DRAINAGE)
				{
					L1_BOM01_OFF
					L1_VAL08_ON2
					cont_desinf = 0;
					cont_agua = 0;
					fsm1_delay(CLEANING_RECYCLE_BOX, CRB_DESAGUE, 5000);
				}
				else if ( tick_ms && ( L1_vuelta == 3 ) )
				{
					substate_clean_box = CRB_DESAGUE;
				}
				break;

			case CRB_DESAGUE:
				if ( tick_ms && ( L1_vuelta < 3 ) && SENSOR_NIVEL_DESINF_1 )
				{
					L1_BOM01_ON
					fsm_on_llenar_desinf();
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) && !SENSOR_NIVEL_DESINF_1 &&
						  cont_desinf < L1_TIME_DRAINAGE * 1.5 )
				{
					cont_desinf ++;
				}
				else if ( tick_ms && ( L1_vuelta < 3 ) && !SENSOR_NIVEL_DESINF_1 &&
						  cont_desinf == L1_TIME_DRAINAGE * 1.5 )
				{
					fsm_off_llenar_desinf();
					L1_BOM01_OFF
					L1_VAL08_OFF
					fsm1_delay(CLEANING_RECYCLE_BOX, CRB_LLENAR_CUBA, 7000);
					L1_vuelta ++;
					// NEXTION
					Nextion_CleanRecBox_n_lavado2();
				}
				else if ( tick_ms && ( L1_vuelta == 3 ) )
				{
					// Nextion: Cleaning Recycle Box Complete!
					Nextion_CleanRecBox_LavComplete();
					state = IDLE;
				}
				break;

			case CRB_STOP:
				if ( tick_ms && L1_prev_state == 1 )
				{
					cont_agua = 0;
					L1_botonIdle = 0;
					L1_BOM01_ON
					L1_VAL08_ON2
					flag_nextion=1;
					substate_clean_box = CRB_VACIAR_CUBA;
				}
				break;

			case CRB_VACIAR_CUBA:
				if ( tick_ms && L1_botonIdle )
				{
					L1_botonIdle = 0;
					cont_agua = 0;
					cont_timeout = 0;
					state = IDLE;
				}
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						  cont_agua < L1_TIME_DRAINAGE )
					cont_agua++;
				else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
						cont_agua == L1_TIME_DRAINAGE )
				{
					L1_BOM01_OFF
					L1_VAL08_OFF
					// Nextion: Vaciado Completado con Exito (STOP)
					if(flag_nextion)
					{
						Nextion_CleanRecBox_LavComplete();
						flag_nextion=0;
					}
				}
				break;
			}
			break;
			/*-----------------------------------------------*/
			/*-----------------------------------------------*/
	}
	reset();
}

/*--------------------------------------------------------------------------------------------*/
/*		 	Runcycle de STOP Manual o Automatico por fuga de presion						  */
/*--------------------------------------------------------------------------------------------*/

typedef enum {			// Stop
	STP_INICIO,
	STP_DESAGUE_PURGA_LUMEN
} FSM1_STOP_T;

static FSM1_STOP_T substate_stop = STP_INICIO;

void fsm_stop1_runCycle(void)
{
	runcycle = STOPPED;				// indico para fsm1_delay que fsm deberá deshabilitar

	switch (state)
	{
		case IDLE:
			// No hago nada acá
			break;
		case PRESET:
			switch (substate_stop)
			{
				case STP_INICIO:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						fsm_off_buzzer();
						reset_states_delay(); // funcion que limpia los states de un fsm_delay previo
						substate_stop = STP_DESAGUE_PURGA_LUMEN;
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						fsm1_enable = 1;
						fsm1_stopped = 0;
						fsm_off_desagote();
						fsm_off_buzzer();
						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						state = IDLE;
						reset_states_delay(); // funcion que limpia los states de un fsm_delay previo
						substate_stop = STP_INICIO;
					}
					break;
				case STP_DESAGUE_PURGA_LUMEN:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						fsm_on_desagote();
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua < L1_TIME_DRAINAGE )
						cont_agua++;
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua == L1_TIME_DRAINAGE )
					{
						cont_agua = 0;
						fsm1_enable = 1;
						fsm1_stopped = 0;
						fsm_off_desagote();
						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						state = IDLE;
						substate_stop = STP_INICIO;
					}
					break;
			}
			break;
		case PRUEBA_ESTANQ:
			if ( tick_ms && !ready && cont_timeout < 2000 )
			{
				cont_timeout ++;
				reset_states_delay();
				fsm_on_ValvulaAire();
			}
			else if ( tick_ms && !ready && cont_timeout == 2000 )
			{
				ready = 1;
			}
			else if ( tick_ms && ready )
			{
				ready = 0;
				reset_estanqueidad();
				fsm_off_Estanqueidad();
				fsm_off_solenoide();
				fsm_off_buzzer();
				fsm1_enable = 1;
				fsm1_stopped = 0;
				cont_timeout = 0;
				// NEXTION: Mostrar en pantalla el botón continuar!!!
				Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
				state = IDLE;
			}
			break;
		case FLUSH:
			switch (substate_stop)
			{
				case STP_INICIO:
					if ( tick_ms && !L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm_off_desagote();
						fsm_off_llenar();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						fsm_off_buzzer();
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					else if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						fsm_off_llenar();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						fsm_off_buzzer();
						fsm1_delay(FLUSH, STP_DESAGUE_PURGA_LUMEN, 7000);
					}
					break;
				case STP_DESAGUE_PURGA_LUMEN:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						fsm_on_desagote();
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua < L1_TIME_DRAINAGE )
						cont_agua++;
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua == L1_TIME_DRAINAGE )
					{
						cont_agua = 0;
						fsm_off_desagote();
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					break;
			}
			break;
		case WASHING:
			switch (substate_stop)
			{
				case STP_INICIO:
					if ( tick_ms && !L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm_off_desagote();
						fsm_off_llenar();
						Detergente_Stop();
						Detergente_Arm();		// inicializacion para Dosificar Detergente
						fsm_off_recircular();
						recirculado = 0;
						cont_ultson = 0;
						ultson_cycle_state = ULTSON_CYCLE_START;
						UltSon_Apagar();
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						L1_CLF11_OFF
						fsm_off_buzzer();
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					else if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						fsm_off_llenar();
						Detergente_Stop();
						Detergente_Arm();		// inicializacion para Dosificar Detergente
						fsm_off_recircular();
						recirculado = 0;
						cont_ultson = 0;
						ultson_cycle_state = ULTSON_CYCLE_START;
						UltSon_Apagar();
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						L1_CLF11_OFF
						fsm_off_buzzer();
						fsm1_delay(WASHING, STP_DESAGUE_PURGA_LUMEN, 7000);
					}
					break;
				case STP_DESAGUE_PURGA_LUMEN:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						fsm_on_desagote();
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua < L1_TIME_DRAINAGE )
						cont_agua++;
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua == L1_TIME_DRAINAGE )
					{
						cont_agua = 0;
						fsm_off_desagote();
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					break;
			}
			break;
		case RISING:
			switch (substate_stop)
			{
				case STP_INICIO:
					if ( tick_ms && !L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm_off_desagote();
						fsm_off_llenar();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						fsm_off_buzzer();
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					else if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						fsm_off_llenar();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						fsm_off_buzzer();
						fsm1_delay(RISING, STP_DESAGUE_PURGA_LUMEN, 7000);
					}
					break;
				case STP_DESAGUE_PURGA_LUMEN:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						fsm_on_desagote();
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua < L1_TIME_DRAINAGE )
						cont_agua++;
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua == L1_TIME_DRAINAGE )
					{
						cont_agua = 0;
						fsm_off_desagote();
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					break;
			}
			break;
		case DESINFECT:
			switch (substate_stop)
			{
				case STP_INICIO:
					if ( tick_ms && !L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm_off_bidon();
						fsm_off_llenar_desinf();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						L1_CLF11_OFF
						fsm_off_buzzer();
						fsm1_delay(IDLE, 0, 7000);
						Nextion_Send_OPA_OFF(REPRO1);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// BOTON NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					else if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						cont_desinf = 0;
						fsm_off_llenar_desinf();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						L1_CLF11_OFF
						fsm_off_buzzer();
						fsm1_delay(DESINFECT, STP_DESAGUE_PURGA_LUMEN, 7000);
					}
					break;
				case STP_DESAGUE_PURGA_LUMEN:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						fsm_on_bidon();
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_desinf < L1_TIME_DRAINAGE )
						cont_desinf++;
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							cont_desinf == L1_TIME_DRAINAGE )
					{
						cont_desinf = 0;
						fsm_off_bidon();
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm1_delay(IDLE, 0, 7000);
						Nextion_Send_OPA_OFF(REPRO1);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					break;
			}
			break;
		case FINAL_RISING:
			switch (substate_stop)
			{
				case STP_INICIO:
					if ( tick_ms && !L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm_off_desagote();
						fsm_off_llenar();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						fsm_off_buzzer();
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					else if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						cont_agua = 0;
						fsm_off_llenar();
						fsm_off_recircular();
						recirculado = 0;
						state_r = _RECIRCULAR;
						fsm_reset_lock_test();
						fsm_off_buzzer();
						fsm1_delay(FINAL_RISING, STP_DESAGUE_PURGA_LUMEN, 7000);
					}
					break;
				case STP_DESAGUE_PURGA_LUMEN:
					if ( tick_ms && L1_SENSOR_CALEFACTOR )
					{
						fsm_on_desagote();
					}
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua < L1_TIME_DRAINAGE )
						cont_agua++;
					else if ( tick_ms && !L1_SENSOR_CALEFACTOR &&
							  cont_agua == L1_TIME_DRAINAGE )
					{
						cont_agua = 0;
						fsm_off_desagote();
						runcycle = RUNNING;		// aviso a fsm1_delay que habilite luego del timeout la fsm de la reprocesadora
						fsm1_stopped = 0;
						fsm1_delay(IDLE, 0, 7000);

						Ticket_EnqFinish(REPRO1, "STOP", "");
						Ticket_EnqPrint(REPRO1);

						// NEXTION: Mostrar en pantalla el botón continuar!!!
						Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
						substate_stop = STP_INICIO;
					}
					break;
			}
			break;
		case DRYING:
			if ( tick_ms )
			{
				reset_states_delay();
				fsm_off_desagote_sinbomba();
				L1_BPR06_OFF
				fsm_off_buzzer();
				fsm1_enable = 1;
				fsm1_stopped = 0;
				// NEXTION: Mostrar en pantalla el botón continuar!!!
				Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Mostrar_NextionButtons);
				state = IDLE;
				Ticket_EnqFinish(REPRO1, "STOP", "");
				Ticket_EnqPrint(REPRO1);
			}
			break;

		case DRAIN_START:
			break;
		case DOSING_START:
			break;
		case CLEANING_RECYCLE_BOX:
			break;
	}
}

/*--------------------------------------------------------------------------------------------*/
/*		 	FUNCIONES INTERNAS										  						  */
/*--------------------------------------------------------------------------------------------*/

void fsm1_raise_ev(const char * raise_ev)
{
	if( !strcmp( raise_ev, FUGA_ESTANQUEIDAD))
		fuga = 1;
	else if( !strcmp( raise_ev, ESTANQUEIDAD_OK))
		pruebaOK = 1;
}

static void reset_states_delay()
{
	fsm1_delay_ctx.next_state    = 0;
	fsm1_delay_ctx.next_substate = 0;
}

void fsm1_delay(int next_state, int next_substate, uint32_t inter_delay_ms)
{
	fsm1_delay_ctx.active      = 1;
	fsm1_delay_ctx.start_tick  = HAL_GetTick();      // base de tiempo HAL
	fsm1_delay_ctx.delay_ticks = inter_delay_ms;

	fsm1_delay_ctx.next_state    = next_state;
	fsm1_delay_ctx.next_substate = next_substate;

	// Frenás las FSMs
	switch (runcycle)
	{
		case RUNNING:				// FSM RUNCYCLE REPROCESADORA
			fsm1_enable = 0;
			break;
		case STOPPED:				// FSM RUNCYCLE STOPPED
			fsm1_stopped = 0;
			break;
	}
}

void fsm1_delay_service(void)
{
	if (!fsm1_delay_ctx.active)
		return;

	uint32_t now     = HAL_GetTick();
	uint32_t elapsed = now - fsm1_delay_ctx.start_tick;   // maneja overflow solo

	if (elapsed >= fsm1_delay_ctx.delay_ticks)
	{
		fsm1_delay_ctx.active = 0;

		// Al terminar el delay, fuerzas el siguiente estado
		state = fsm1_delay_ctx.next_state;

		switch (state)
		{
			case IDLE:
				break;

			case PRESET:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_preset = (FSM1_PRESET_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case PRUEBA_ESTANQ:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_estanq = (FSM1_ESTANQUEIDAD_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case FLUSH:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_flush = (FSM1_FLUSH_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case WASHING:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_wash = (FSM1_WASHING_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case RISING:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_rising = (FSM1_RISING_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case DESINFECT:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_desinf = (FSM1_DESINF_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case FINAL_RISING:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_f_rise = (FSM1_F_RISING_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case DRYING:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_dry = (FSM1_F_DRYING_T)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case DRAIN_START:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_drain = (FSM1_DRAIN_START)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case DOSING_START:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_dosing = (FSM1_DOSING_START)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;

			case CLEANING_RECYCLE_BOX:
				switch (runcycle)
				{
					case RUNNING:				// FSM RUNCYCLE REPROCESADORA
						substate_clean_box = (FSM1_CLEANING_RECYCLE_BOX)fsm1_delay_ctx.next_substate;
						break;
					case STOPPED:				// FSM RUNCYCLE STOPPED
						substate_stop = (FSM1_STOP_T)fsm1_delay_ctx.next_substate;
						break;
				}
				break;
		}
		// Y re-habilitás la FSM
		switch (runcycle)
		{
			case RUNNING:				// FSM RUNCYCLE REPROCESADORA
				fsm1_enable = 1;
				break;
			case STOPPED:				// FSM RUNCYCLE STOPPED
				fsm1_stopped = 1;
				break;
		}
	}
}

/*-----------------------------------------------*/
// recircula por la bacha y lumenes
/*-----------------------------------------------*/
static void fsm_on_recircular(void)
{
	L1_BOM02_ON
	//L1_BOM04_ON   <--- quedo duplicada la accion, estan puenteados fisicamente
}

static void fsm_toggle_recircular(void)
{
	// alternar entre ON y OFF con un tick, si esta prendido apago, para hacer miniciclos de recirculacion
	if (L1_BOM02)
		L1_BOM02_OFF
	else
		L1_BOM02_ON
}

static void fsm_off_recircular(void)
{
	L1_BOM02_OFF
	//L1_BOM04_OFF   <--- quedo duplicada la accion, estan puenteados fisicamente
}

/*-----------------------------------------------*/
// llenar bacha
/*-----------------------------------------------*/
static void fsm_on_llenar(void)
{
	L1_ELV13_ON		// solenoide de entrada de agua
	L1_VAL07_ON		// valvula esférica motorizada de entrada
}

static void fsm_off_llenar(void)
{
	L1_ELV13_OFF		// solenoide de entrada de agua
	L1_VAL07_OFF		// valvula esférica motorizada de entrada
}

/*-----------------------------------------------*/
// drena la bacha y lumenes al desague
/*-----------------------------------------------*/
static void fsm_on_desagote(void)
{
	L1_BOM01_ON
	L1_VAL08_ON2
	L1_SOL09_ON
	CMP14_ON
}

static void fsm_off_desagote(void)
{
	CMP14_OFF
	L1_BOM01_OFF
	L1_VAL08_OFF
	L1_SOL09_OFF
}

/*-----------------------------------------------*/
// llenar bacha con desinf
/*-----------------------------------------------*/

static void fsm_on_llenar_desinf(void)
{
	L1_ELV12_ON		// solenoide de la bomba de desinf.
	BOM03_ON	// bomba desinfectante
}

static void fsm_off_llenar_desinf(void)
{
	L1_ELV12_OFF		// solenoide de la bomba de desinf.
	BOM03_OFF	// bomba desinfectante
}

/*-----------------------------------------------*/
// drena la bacha y lumenes al bidon de desinf
/*-----------------------------------------------*/

static void fsm_on_bidon(void)
{
	L1_BOM01_ON		// bomba centrif. desague/bidon
	L1_VAL08_ON1	// apertura hacia bidon
	L1_SOL09_ON		// solenoide compresor REPRO1
	CMP14_ON		// compresor
}

static void fsm_off_bidon(void)
{
	CMP14_OFF		// compresor
	L1_BOM01_OFF	// bomba centrif. desague/bidon
	L1_VAL08_OFF	// apertura hacia bidon
	L1_SOL09_OFF	// solenoide compresor REPRO1
}

/*-----------------------------------------------*/
// drena la bacha y lumenes al desague con alcohol
/*-----------------------------------------------*/

static void fsm_on_desagote_sinbomba(void)
{
	L1_VAL08_ON2
	L1_SOL09_ON
	CMP14_ON
}

static void fsm_off_desagote_sinbomba(void)
{
	CMP14_OFF
	L1_VAL08_OFF
	L1_SOL09_OFF
}

void fsm1_change_state(const char * str)
{
	if (!strcmp(str,"estanq"))
	{
		state = PRUEBA_ESTANQ;
		substate_estanq = PRUEBA_ESTANQUEIDAD;
	}
	else if (!strcmp(str,"off_estanq"))
	{
		state = IDLE;
		fsm_off_Estanqueidad();
		reset_estanqueidad();
	}
	else if (!strcmp(str,"flush"))
	{
		state = FLUSH;
		substate_flush = F_LLENAR_CUBA;
	}
	else if (!strcmp(str,"wash"))
	{
		state = WASHING;
		substate_flush = W_LLENAR_CUBA_DET;
	}
	else if (!strcmp(str,"rinse"))
	{
		state = RISING;
		substate_flush = R_LLENAR_CUBA;
	}
	else if (!strcmp(str,"desinfect"))
	{
		state = DESINFECT;
		substate_flush = D_LLENAR_CUBA_DESINF;
	}
	else if (!strcmp(str,"final_rise"))
	{
		state = FINAL_RISING;
		substate_flush = FR_LLENAR_CUBA;
		L1_vuelta = 1;
	}
	else if (!strcmp(str,"dry"))
	{
		state = DRYING;
		substate_flush = DR_ALCOHOL;
	}
	else if (!strcmp(str,"apagar_reles485"))
	{
		state = IDLE;

		RelayModule_Init(REPRO1);
	}
	else if (!strcmp(str,"idle"))
	{
		state = IDLE;
		fsm_off_llenar_desinf();
	}
}

/*-----------------------------------------------*/
// Devuelven el state y substate:
/*-----------------------------------------------*/
uint8_t L1_FSM_Get_State()
{
	return state;
}
uint8_t L1_FSM_Get_SubState()
{
	switch(state)
	{
	case IDLE:
		return 0;
		break;

	case PRESET:
		return substate_preset;
		break;

	case PRUEBA_ESTANQ:
		return substate_estanq;
		break;

	case FLUSH:
		return substate_flush;
		break;

	case WASHING:
		return substate_wash;
		break;

	case RISING:
		return substate_rising;
		break;

	case DESINFECT:
		return substate_desinf;
		break;

	case FINAL_RISING:
		return substate_f_rise;
		break;

	case DRYING:
		return substate_dry;
		break;

	case DRAIN_START:
		return -1;
		break;

	case DOSING_START:
		return -1;
		break;

	case CLEANING_RECYCLE_BOX:
		return -1;
		break;

	}
	return -1; // dummy
}
