/*
 * rs485_pressure_sensor.c
 *
 *  Created on: Dec 17, 2025
 *      Author: ADOX
 */

/**
 * Tramas para cambiar el address de los sensores:
 * 0x03: 01 06 00 00 00 03 C9 CB
 * 0x04: 01 06 00 00 00 04 88 09
 * 0x05: 01 06 00 00 00 05 49 C9
 * 0x06: 01 06 00 00 00 06 09 C8
 *
 * */

/**
 * Tramas para cambiar el baudrate a 19200 de los sensores:
 * 0x03: 03 06 00 01 00 04 D8 2B
 * 0x04: 04 06 00 01 00 04 D9 9C
 * 0x05:
 * 0x06:
 *
 * */

/**
 * Relay module: trama para encender TODAS LAS ENTRADAS:
 * 01 0F 00 00 00 10 02 FF FF E3 90
 * 02 0F 00 00 00 10 02 FF FF F7 60
 *
 * Relay module: trama para apagar TODAS LAS ENTRADAS:
 * 01 0F 00 00 00 10 02 00 00 E2 20
 * 02 0F 00 00 00 10 02 00 00 F6 D0
 *
 *	Relay module: trama para leer entradas:
 *	01 02 00 00 00 10 79 C6
 *	02 02 00 00 00 10 79 F5
 *
 * */

#include <string.h>
#include "main.h"
#include "rs485_pressure_sensor.h"
#include "relay_module.h"
#include "global_variables.h"
#include "stdbool.h"
#include "SystemTicks.h"
#include "UltrasonicGenerator.h"

//---> Puerto RS485
extern UART_HandleTypeDef huart2;

typedef enum {
	IDLE_RS485,
	L1_RM_WRITE_OUTPUT,
	L1_RM_READ_INPUTS,
	L1_READ_PRESSURE_1,
	L1_READ_PRESSURE_2,
	L1_ULTRASONIC_GENERATOR
} RS485_DEVICE_T;

//---> Tiempo que espero que llegue la respuesta de los dispositivos:
#define RS485_CALLBACK_TIMEOUT 50

//-----> Variables
//bool rs485_rm_outputs_en; //---> Para escribir salidas en Relay Module.

static RS485_DEVICE_T rs485_state;
bool rs485_read_inputs_fsm = true; //por si estoy escribiendo entradas.

uint8_t rs485_rx_available = 0;

void RS485_PressureSensor_Request(uint8_t);

/**
 * Direcciones del puerto RS485
 *
 * 0x01: Placa de relé de la lavadora 1
 * 0x03: Lavadora 1 -> Sensor de presión 1
 * 0x04: Lavadora 1 -> Sensor de presión 2
 *
 * */

void RS485_Init()
{
	//rs485_rm_outputs_en = true;
	rs485_rx_available = 0;
	rs485_read_inputs_fsm = true;
	rs485_state = 0;
}

void RS485_Loop()
{
	if(!rs485_queue_tic)
	{
		rs485_queue_tic=50;

		if(rs485_read_inputs_fsm)
		{
			memset(rs485_rx_buffer, 0, sizeof(rs485_rx_buffer));//Limpio el buffer donde voy a leer
			switch (rs485_state)
			{
			case IDLE_RS485:
				HAL_Delay(1);
				break;
			case L1_RM_WRITE_OUTPUT:
				RelayModule_WriteAllOutputs(1);
				break;
			case L1_RM_READ_INPUTS:
				RelayModule_ReadAllInputs(1);
				break;
			case L1_READ_PRESSURE_1:
				RS485_PressureSensor_Request(3);
				break;
			case L1_READ_PRESSURE_2:
				RS485_PressureSensor_Request(4);
				break;
			case L1_ULTRASONIC_GENERATOR:
				UltSon_SendData();
				//---> Vuelvo al primer estado nuevamente:
				rs485_state=0;
				break;
			default:
				break;

			}
			rs485_state++; // Se incrementa el estado
			rs485_read_inputs_fsm = false; //lo desactivo y lo reactico con el Callback
			rs485_timeout_tic = RS485_CALLBACK_TIMEOUT; //Activo el timeout
		}
		else
		{
			if(rs485_rx_available)
			{
				rs485_rx_available=0; //---> Flag de recepcion desactivado.
				//--->
				rs485_read_inputs_fsm=true;//Dejo preparada la fsm para despues
				//---<
				//---> Variables:
				uint16_t pressure_aux;
				uint8_t rs485_address = rs485_rx_buffer[0];

				switch (rs485_address)
				{
				case 1: //---> Módulo relé 1
					RS485_SaveInputStatus(1);
					break;
				case ULTSON_RS485_DIR: //---> Generador de ultrasonido.
					HAL_Delay(1);
					break;
				case 3: //---> Sensor presión 0x03 (L1 -> sensor 1)
					pressure_aux = ((uint16_t)rs485_rx_buffer[3] << 8) | rs485_rx_buffer[4];
					L1_pressure_1=(float) pressure_aux/10;
					break;
				case 4: //---> Sensor presión 0x04 (L1 -> sensor 2)
					pressure_aux = ((uint16_t)rs485_rx_buffer[3] << 8) | rs485_rx_buffer[4];
					L1_pressure_2=(float) pressure_aux/10;
					break;
				default:
					break;
				}
			}else if(!rs485_timeout_tic)
			{
				HAL_UART_AbortReceive(&huart2);
				//Si llego aca es porque el sensor/rm no respondio a tiempo...
				//---> Paso a escribir las salidas
				rs485_read_inputs_fsm=true;//Dejo preparada la fsm para despues
				//---<
			}
		}
	}
}


void RS485_PressureSensor_Request(uint8_t _sensor_address)
{
	/**
	 * Las tramas deben ser las siguientes:
	 * read_pressure_01[8] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0xCB}; -> 01 03 00 04 00 01 C5 CB
	 * read_pressure_02[8] = {0x02, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0xF8}; -> 02 03 00 04 00 01 C5 F8
	 * read_pressure_03[8] = {0x03, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC4, 0x29}; -> 03 03 00 04 00 01 C4 29
	 * read_pressure_04[8] = {0x04, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0x9E}; -> 04 03 00 04 00 01 C5 9E
	 * read_pressure_05[8] = {0x05, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC4, 0x4F}; -> 05 03 00 04 00 01 C4 4F
	 * read_pressure_06[8] = {0x06, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC4, 0x7C}; -> 06 03 00 04 00 01 C4 7C
	 *
	 * */

	uint8_t read_pressure[8] = {0x00, 0x03, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00};

	switch (_sensor_address)
	{
	case 3:
		read_pressure[0] = 0x03;
		break;
	case 4:
		read_pressure[0] = 0x04;
		break;
	case 5:
		read_pressure[0] = 0x05;
		break;
	case 6:
		read_pressure[0] = 0x06;
		break;
	default:
		break;
	}

	//-----> Calculo el crc y lo pongo en los ultimos 2 bytes:
	uint16_t crc = calcularCRC(read_pressure, 6);
	read_pressure[6] = crc & 0xFF;         // CRC Low
	read_pressure[7] = (crc >> 8) & 0xFF;  // CRC High

	//-----> Envío la trama al puerto RS485
	RS485_SetTxMode();
	HAL_UART_Transmit(&huart2, read_pressure,sizeof(read_pressure), HAL_MAX_DELAY);
	RS485_SetRxMode();
	/* Necesario limpiar los flags para que funcione bien:*/
	//__HAL_UART_CLEAR_OREFLAG(&huart2);
	//__HAL_UART_CLEAR_FEFLAG(&huart2);
	//__HAL_UART_CLEAR_NEFLAG(&huart2);
	__HAL_UART_CLEAR_PEFLAG(&huart2);
	/**/
	HAL_UART_Receive_IT(&huart2, rs485_rx_buffer, 7);
}
