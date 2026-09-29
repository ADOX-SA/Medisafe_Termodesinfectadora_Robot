/*
 * SystemTicks.c
 *
 *  Created on: Oct 8, 2025
 *      Author: ADOX
 */

#include "main.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include <string.h>

#include "SystemTicks.h"
#include "ENDOCLEAN2000.h"

uint16_t example_tic;
bool example_tic_en;

uint16_t rs485_readinputs_tic;
bool rs485_readinputs_tic_en;

uint16_t rs485_queue_tic;
bool rs485_queue_tic_en;

uint16_t rs485_timeout_tic;
bool rs485_timeout_tic_en;

uint16_t nextion_demo_tic;
bool nextion_demo_tic_en;

uint16_t rfid_wiegand_tic;
bool rfid_wiegand_en;

uint16_t ntc_adc_tic;
bool ntc_adc_tic_en;

uint16_t nextion_tic_1seg;
uint16_t nextion_tic_2seg;
uint16_t nextion_tic_3seg;
uint16_t nextion_tic_4seg;
uint16_t nextion_tic_5seg;



void SystemTicks_Init(){

	example_tic=2500;
	example_tic_en=true;

	rs485_readinputs_tic=1000;
	rs485_readinputs_tic_en=true;

	rs485_queue_tic=100;
	rs485_queue_tic_en=true;

	rs485_timeout_tic=1000;
	rs485_timeout_tic_en=true;

	nextion_demo_tic=1000;
	nextion_demo_tic_en=true;

	rfid_wiegand_tic=5000;
	rfid_wiegand_en=true;

	ntc_adc_tic = 1000;
	ntc_adc_tic_en = true;

	nextion_tic_1seg=1000;
	nextion_tic_2seg=1000;
	nextion_tic_3seg=1000;
	nextion_tic_4seg=1000;
	nextion_tic_5seg=1000;
}

void SystemTicks_1ms(){

	if (EC2000_lavado_tic > 0)
		EC2000_lavado_tic--;

	if (EC2000_llenado_tic > 0)
		EC2000_llenado_tic--;

	if (EC2000_buzzer_tic > 0)
		EC2000_buzzer_tic--;

	if (EC2000_aire_tic > 0)
		EC2000_aire_tic--;

	if (EC2000_pistola_aire_tic > 0)
		EC2000_pistola_aire_tic--;

	if (EC2000_drenaje_manual_tic > 0)
		EC2000_drenaje_manual_tic--;

	if (EC2000_estanq_tic > 0)
		EC2000_estanq_tic--;

	if (EC2000_puerta_pedal_tic > 0)
		EC2000_puerta_pedal_tic--;

	if (EC2000_detergente_tic > 0)
		EC2000_detergente_tic--;

	if (EC2000_desinfectante_tic > 0)
		EC2000_desinfectante_tic--;

	if (EC2000_alcohol_tic > 0)
		EC2000_alcohol_tic--;

	if (EC2000_mantenimiento_tic > 0)
		EC2000_mantenimiento_tic--;

	if (EC2000_mantenimiento_timeout_tic > 0)
		EC2000_mantenimiento_timeout_tic--;

	if (EC2000_vaciado_bacha_tic > 0)
		EC2000_vaciado_bacha_tic--;

	if (EC2000_sendstate_tic > 0)
		EC2000_sendstate_tic--;

	if (example_tic > 0)
		example_tic--;

	if ((rs485_readinputs_tic > 0) && (rs485_readinputs_tic_en))
		rs485_readinputs_tic--;

	if ((rs485_queue_tic > 0) && (rs485_queue_tic_en))
		rs485_queue_tic--;

	if ((rs485_timeout_tic > 0) && (rs485_timeout_tic_en))
		rs485_timeout_tic--;

	if ((nextion_demo_tic > 0) && (nextion_demo_tic_en))
		nextion_demo_tic--;

	if ((rfid_wiegand_tic > 0) && (rfid_wiegand_en))
		rfid_wiegand_tic--;

	if ((ntc_adc_tic > 0) && (ntc_adc_tic_en))
		ntc_adc_tic--;

	/* TICS PARA NEXTION */

	if(nextion_tic_1seg > 0)
		nextion_tic_1seg--;

	if(nextion_tic_2seg > 0)
		nextion_tic_2seg--;

	if(nextion_tic_3seg > 0)
		nextion_tic_3seg--;

	if(nextion_tic_4seg > 0)
		nextion_tic_4seg--;

	if(nextion_tic_5seg > 0)
		nextion_tic_5seg--;
}
