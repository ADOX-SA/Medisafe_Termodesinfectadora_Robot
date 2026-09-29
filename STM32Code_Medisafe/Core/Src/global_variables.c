/*
 * global_variables.c
 *
 *  Created on: Nov 26, 2025
 *      Author: momo
 */

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include "global_variables.h"

bool test = false;				// usado en user_gpio_loop para ver presion de estanqueidad en test

int fsm1_state = 0,
	fsm1_substate = 0;

bool fsm1_enable = 1, fsm1_stopped = 0;

ENUM_NIVEL_BIDON nivel_bidon;
ENUM_NIVEL_BACHA nivel_bacha;

/************************************************************************/
/*					REPROCESADORA 1										*/
/************************************************************************/

/*------------------------------*/
/*		  MODO STANDARD			*/
/*------------------------------*/
int L1_standard_times[] = {			// por defecto tiene esos valores, se pueden sobreescribir por la nextion
		120000,	// FLUSH
		300000,	// WASH
		60000,  // RISING
		300000, // DESINFECT
		120000, // FINAL RISING
		20000,  // ALCOHOL
		60000,  // DRYING

		5000,	// TIME_ON
		20000	// TIME_DRAINAGE
};

bool L1_estanq_en; // Habilitar estanqueidad - true = estanqueidad habilitada / false = deshabilitada

bool  L1_enable_wh = false,
	  L1_enable_dh = false;

bool L1_ignora_sensor = false; // sensor CALEFACTOR a ignorar si se cuelga el desagote automatico por corte luz
bool L1_opa_bacha = false; // ¿Quedó OPA? - true = enviar al bidon / false = enviar al drenaje

int L1_dilucion=28000; // 1 = 1:200, 2 = 1:500, 3 = 1:1000

float L1_initialTemp = 0,
	  L1_realTemp = 0,
	  L1_MaxTempW = 40,
	  L1_MaxTempD = 40;

int L1_vuelta = 0;
int L1_prev_state = 0;

float L1_pressure_1;
float L1_pressure_2;

bool L1_WH_TIMEOUT = 0;
bool L1_DH_TIMEOUT = 0;

bool L1_rtc_ready = 0;

bool L1_lock_test = 1, L1_blocked = 0, L1_not_blocked = 0;

/*------------------------------*/

/************************************************************************/
/*					COMPARTIDO											*/
/************************************************************************/

float estanq_presionKPa = 0;

// Ticket
char g_fecha[11]; // "YYYY.MM.DD"
char g_hora[9];   // "HH:MM:SS"

