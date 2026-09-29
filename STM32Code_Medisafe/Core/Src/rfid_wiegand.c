/*
 * rfid_wiegand.c
 *
 *  Created on: Jan 6, 2026
 *      Author: ADOX
 */

#include "rfid_wiegand.h"
#include "SystemTicks.h"

#include "main.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#define MAX_BITS 64
uint16_t pulse_counter;

uint8_t tarjeta[MAX_BITS];
uint8_t indice_tarjeta;

bool rfid_wiegand_available;
uint32_t rfid_wiegand_id;


void Rfid_Wiegand_Init()
{
	pulse_counter=0;
	rfid_wiegand_id = 0;
	indice_tarjeta=0;
	for (int i= 0; i < MAX_BITS; i++)
	{
		tarjeta[i]=0;
	}
}

void Rfid_Wiegand_Loop()
{

	if(!rfid_wiegand_tic)
	{
		rfid_wiegand_tic=500;

		if (pulse_counter == 32)
		{
			// Cierro la cadena como string C
			if (indice_tarjeta < MAX_BITS)
				tarjeta[indice_tarjeta] = '\0';
			else
				tarjeta[MAX_BITS-1] = '\0';

			rfid_wiegand_id=0;

			for (int i = 0; i < 32; i++) {
				rfid_wiegand_id <<= 1;              // corre 1 bit a la izquierda
				rfid_wiegand_id |= (tarjeta[i] & 1);   // agrega el bit 0 o 1
			}

			//----> EN rfid_wiegand_id tengo la lectura ESPEJADA del rfid como hace el MRF522
			//--- Ahora lo invierto:

			uint8_t b0 = (rfid_wiegand_id >> 24) & 0xFF;
			uint8_t b1 = (rfid_wiegand_id >> 16) & 0xFF;
			uint8_t b2 = (rfid_wiegand_id >>  8) & 0xFF;
			uint8_t b3 =  rfid_wiegand_id        & 0xFF;


			rfid_wiegand_id = ((uint32_t)b3 << 24) |
					((uint32_t)b2 << 16) |
					((uint32_t)b1 <<  8) |
					((uint32_t)b0);

			//---> Listo en rfid_wiegand_id tengo el rfid tipo MRF522

			//----> Habilito el flag para la lectura.
			rfid_wiegand_available=true;

			// Reset para próxima tarjeta
			pulse_counter   = 0;
			indice_tarjeta  = 0;
			for (int i = 0; i < MAX_BITS; i++)
				tarjeta[i] = 0;
		}
	}
}

void Rfid_Wiegand_Callback(uint16_t _gpio_pin)
{

	switch (_gpio_pin)
	{
	case RFID_D0_Pin:   // pulso en D0 -> bit 0
		if (indice_tarjeta < MAX_BITS - 1) {
			tarjeta[indice_tarjeta] = 0;
			indice_tarjeta++;
			pulse_counter++;
		}
		break;

	case RFID_D1_Pin:   // pulso en D1 -> bit 1
		if (indice_tarjeta < MAX_BITS - 1) {
			tarjeta[indice_tarjeta] = 1;
			indice_tarjeta++;
			pulse_counter++;
		}
		break;
	}
}

