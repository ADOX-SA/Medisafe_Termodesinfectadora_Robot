/*
 * rfid_wiegand.h
 *
 *  Created on: Jan 6, 2026
 *      Author: ADOX
 */

#ifndef INC_RFID_WIEGAND_H_
#define INC_RFID_WIEGAND_H_

#include "main.h"
#include "stdbool.h"


/**
 * ¿Como usarlo?
 *
 * 		if(rfid_wiegand_available)
		{
			rfid_wiegand_available=false; //---> Deshabilito el flag.
			rfid_wiegand_id --> En esta variable está el id de la tarjeta leída.
		}
 *
 * */


/**
 * Funciones de Init, Loop y Callback.
 * */
void Rfid_Wiegand_Init();
void Rfid_Wiegand_Loop();
void Rfid_Wiegand_Callback(uint16_t);

/**
 * Variable que se utiliza para saber si se leyó una nueva tarjeta.
 * Cuando rfid_wiegand_available=1, es porque una nueva tarjeta fué leída.
 * */
extern bool rfid_wiegand_available;

/**
 * rfid_wiegand_id es la variable que guarda el id de la tarjeta leída.
 * Por ejemplo: 93-95-32-10
 * */
extern uint32_t rfid_wiegand_id;

#endif /* INC_RFID_WIEGAND_H_ */
