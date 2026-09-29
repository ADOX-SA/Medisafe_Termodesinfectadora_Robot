/*
 * global_variables.h
 *
 *  Created on: Nov 25, 2025
 *      Author: momo
 */

#ifndef INC_GLOBAL_VARIABLES_H_
#define INC_GLOBAL_VARIABLES_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>

//-----------------------------------------------------------
//------> Seleccion de lavadora.
//-----------------------------------------------------------
#define MODEL_SIMPLE_COMPLETA   1 // USA KLD 3 VIAS
#define MODEL_SIMPLE_SIN_OPA    2 // USA KLD 2 VIAS (CONECTADA EN EL PIN 13)
#define MODEL_ENDOCLEAN2000    3 // USA KLD 3 VIAS


#define MODEL MODEL_ENDOCLEAN2000

#if MODEL == MODEL_SIMPLE_COMPLETA
#define MODEL_NAME "data.model.txt=\"Simple completa\""
#elif MODEL == MODEL_SIMPLE_SIN_OPA
#define MODEL_NAME "data.model.txt=\"Simple sin desinf.\""
#elif MODEL == MODEL_ENDOCLEAN2000
#define MODEL_NAME "data.model.txt=\"ENDOCLEAN2000\""
#else

#endif


//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------

extern int fsm1_state, fsm1_substate;
extern bool fsm1_enable, fsm1_stopped;

typedef enum {
	EMPTY,
	LOW,
	MEDIUM,
	HIGH,
	FULL,
	NIVEL_ERROR
} ENUM_NIVEL_BIDON;

extern ENUM_NIVEL_BIDON nivel_bidon;

typedef enum {
	BACHA_EMPTY,
	BACHA_LOW,
	BACHA_MEDIUM,
	BACHA_HIGH,
	BACHA_FULL,
	BACHA_NIVEL_ERROR
} ENUM_NIVEL_BACHA;

extern ENUM_NIVEL_BACHA nivel_bacha;

/************************************************************************/
/*					REPROCESADORA 1										*/
/************************************************************************/

/*------------------------------*/
/*		  MODO STANDARD			*/
/*------------------------------*/

extern int L1_standard_times[9];
extern bool L1_enable_wh, L1_enable_dh;

#define L1_TIME_RECYCLE_FLUSH 		L1_standard_times[0]
#define L1_TIME_RECYCLE_WASH		L1_standard_times[1]
#define L1_TIME_RECYCLE_RISING 		L1_standard_times[2]
#define L1_TIME_RECYCLE_DESINFECT 	L1_standard_times[3]
#define L1_TIME_RECYCLE_F_RISING	L1_standard_times[4]
#define L1_TIME_ALCOHOL				L1_standard_times[5]
#define L1_TIME_DRYING				L1_standard_times[6]

#define L1_TIME_ON					L1_standard_times[7]
#define L1_TIME_DRAINAGE			L1_standard_times[8]

#define L1_WASHING_HEATING 			L1_enable_wh				// ON u OFF
#define L1_DESINF_HEATING 			L1_enable_dh				// ON u OFF

extern float L1_initialTemp;
extern float L1_realTemp;
extern float L1_MaxTempW;
extern float L1_MaxTempD;

extern bool L1_estanq_en;
extern bool L1_ignora_sensor;
extern bool L1_opa_bacha;

extern int L1_dilucion;

#define L1_TEMP_WASHING_MAX			L1_MaxTempW
#define L1_TEMP_DESINF_MAX 			L1_MaxTempD

extern int L1_vuelta;		// utilizado en Final Rising
extern int L1_prev_state;	// utilizado para ERRORES Tapa Abierta!

extern float L1_pressure_1;
extern float L1_pressure_2;

extern bool L1_WH_TIMEOUT;	// flag que deshabilita sensor de temp en WASH
extern bool L1_DH_TIMEOUT;  // flag que deshabilita sensor de temp en DESINFECT

extern bool L1_rtc_ready;

// Sensores de Presion: Prueba de Bloqueo inicial
extern bool L1_lock_test, L1_blocked, L1_not_blocked;

#define L1_TIME_DET L1_dilucion				// -> 60 ml, 0.5 % 1/200 de dilucion teorica

/*------------------------------*/

/************************************************************************/
/*					COMPARTIDO											*/
/************************************************************************/

typedef enum {
	GENERAL,
	REPRO1,
	REPRO2
}ENUM_REF_LAV_T;

#define TIMEOUT_ESTANQUE 100 // 10 s * 10 Hz = 100
extern float estanq_presionKPa;

#define TIMEOUT_NOWATER 120000		// error: falta agua pero hay agua en calefactor pero no llega al nivel!
#define TIMEOUT_NOWATER2 300000		// error: no hay agua

#define TIMEOUT_DET 10000

#define HEATING_TIME 1120000		// 18 min 40 seg -> ms
#define TIMEOUT_CALEFACTOR 600000	// error falla calefactor en 10 min no sube 3º

// Ticket

extern char g_fecha[11]; // "YYYY.MM.DD"
extern char g_hora[9];   // "HH:MM:SS"

// Sensores de Presion: Prueba de Bloqueo inicial

#define TIME_CHECK 8000
#define TIME_LOCK_TEST 15000
#define TIME_PRESS 3000

#define kPa_AIR_MIN 10
#define kPa_AIR_MAX 150

#define kPa_AG_MIN 80
#define kPa_AG_MAX 300

#endif /* INC_GLOBAL_VARIABLES_H_ */
