/*
 * Medisafe_Gpio_Config.c
 *
 *  Created on: 5 oct 2026
 *      Author: ADOX
 */
#include "main.h"
#include "relay_module.h"
#include "global_variables.h"
#include "nextion.h"
#include "ticket.h"
#include "user_gpio.h"
#include "fsm_reprocesadora1.h"
#include "Medisafe_Gpio_Config.h"
#include <stdio.h>




//----> Funciones basicas para fsm. (NO MODIFICAR)

void MEDSAF_fsm_off_sistema()
{
 MEDSAF_1_OFF
 MEDSAF_2_OFF
 MEDSAF_3_OFF
 MEDSAF_BOM_LUBRI_OFF
 MEDSAF_SOLIN_TAPA_OFF
 MEDSAF_6_OFF
 MEDSAF_PUERTA_OFF
 MEDSAF_SOLIN_HOT_OFF
 MEDSAF_BOM_DREN_OFF
 MEDSAF_BOM_TRIF_OFF
 MEDSAF_SOLIN_COLD_OFF
 MEDSAF_BOM_DET_OFF
 MEDSAF_SOL_DREN_OFF
 MEDSAF_CALEFACTOR_OFF
 MEDSAF_ULTS_2_OFF
 MEDSAF_ULTS_1_OFF
}



void MEDSAF_fsm_on_calefactor(void)
{
	MEDSAF_CALEFACTOR_ON
}
void MEDSAF_fsm_off_calefactor(void)
{
	MEDSAF_CALEFACTOR_OFF
}

void MEDSAF_fsm_on_ultrasonido(void)
{
	MEDSAF_ULTS_1_ON
	MEDSAF_ULTS_2_ON
}

void MEDSAF_fsm_off_ultrasonido(void)
{
	MEDSAF_ULTS_1_OFF
	MEDSAF_ULTS_2_OFF
}
void MEDSAF_fsm_on_puerta_sole(void)
{
	MEDSAF_PUERTA_ON
}
void MEDSAF_fsm_off_puerta_sole(void)
{
	MEDSAF_PUERTA_OFF
}
void MEDSAF_fsm_on_detergente(void)
{
	MEDSAF_BOM_DET_ON
}
void MEDSAF_fsm_off_detergente(void)
{
	MEDSAF_BOM_DET_OFF
}

void MEDSAF_fsm_on_cold_water(void)
{
	MEDSAF_SOLIN_COLD_ON
}

void MEDSAF_fsm_on_hot_water(void)
{
	MEDSAF_SOLIN_HOT_ON
}

void MEDSAF_fsm_off_llenar_bacha(void)
{
	MEDSAF_SOLIN_COLD_OFF
	MEDSAF_SOLIN_HOT_OFF
	MEDSAF_SOLIN_TAPA_OFF
}

void MEDSAF_fsm_on_lavado_bomba(void)
{
	MEDSAF_BOM_TRIF_ON
}

void MEDSAF_fsm_off_lavado_bomba(void)
{
	MEDSAF_BOM_TRIF_OFF
}

void MEDSAF_fsm_on_drenar_bacha(void)
{
	MEDSAF_SOL_DREN_ON
	MEDSAF_BOM_DREN_ON
}

void MEDSAF_fsm_off_drenar_bacha(void)
{
	MEDSAF_SOL_DREN_OFF
	MEDSAF_BOM_DREN_OFF
}

