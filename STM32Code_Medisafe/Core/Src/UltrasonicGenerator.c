/*
 * UltrasonicGenerator.c
 *
 *  Created on: 23 jul 2026
 *      Author: ADOX
 */

/**
 * TRAMAS PARA GENERADOR DE ULTRASONIDO
 * Parámetros de comunicación: 19200 baud rate, N,8,1
 * ID= 0x02
 *
 * - Encender ultrasonido: 02 06 00 00 00 01 48 39
 * - Apagar ultrasonido:   02 06 00 00 00 00 89 F9
 * - Configurar Tiempo de funcionamiento: 02 06 00 02 xx yy zz zz (xx minutos - yy segundos - zz zz checksum)
 * - Configurar potencia: 02 06 00 04 xx xx yy yy (xx xx valor entre 1 y 20 - yy yy checksum)
 *
 *
 *
 * */

#include "main.h"
#include "UltrasonicGenerator.h"

#include <string.h>
#include "main.h"
#include "rs485_pressure_sensor.h"
#include "relay_module.h"
#include "global_variables.h"
#include "stdbool.h"
#include "SystemTicks.h"

//---> Puerto RS485
extern UART_HandleTypeDef huart2;

bool ultson_send_enable=false;
uint8_t ultson_buffer_rs485[8];

uint8_t ultson_state; //---> ENCENDIDO = 1 / APAGADO = 0
uint8_t ultson_enable; //---> Utilizada en el ciclo de lavado de la fsm.
uint8_t ultson_time_min;
uint8_t ultson_time_sec;
uint8_t ultson_power;
uint8_t ultson_current;
uint8_t ultson_frecuency;

void UltSon_Init()
{
	ultson_state = ULTSON_STATE_OFF;
	//ultson_enable = ULTSON_DISABLE_VALUE;
	ultson_enable = ULTSON_ENABLE_VALUE; //Arranca encendido
}

void UltSon_SendData()
{
	if (ultson_send_enable)
	{
		//-----> Envío la trama al puerto RS485
		RS485_SetTxMode();
		HAL_UART_Transmit(&huart2, ultson_buffer_rs485,sizeof(ultson_buffer_rs485), HAL_MAX_DELAY);
		RS485_SetRxMode();
		/* Necesario limpiar los flags para que funcione bien:*/
		//__HAL_UART_CLEAR_OREFLAG(&huart2);
		//__HAL_UART_CLEAR_FEFLAG(&huart2);
		//__HAL_UART_CLEAR_NEFLAG(&huart2);
		__HAL_UART_CLEAR_PEFLAG(&huart2);
		/**/
		HAL_UART_Receive_IT(&huart2, rs485_rx_buffer, 8);

		//---> Desactivo el envío de comandos.
		ultson_send_enable = false;
	}
}

void UltSon_Encender()
{
	//uint8_t data_to_send[] = {0x02, 0x06, 0x00, 0x00, 0x00, 0x01, 0x48, 0x39};
	uint8_t data_to_send[] = {ULTSON_RS485_DIR, 0x06, 0x00, 0x00, 0x00, 0x01, 0x48, 0x6C}; // PARA ID=0x07
	memcpy(ultson_buffer_rs485, data_to_send, sizeof(data_to_send));

	//---> Habilito el envío de comandos.
	ultson_send_enable = true;
}


void UltSon_Apagar()
{
	//uint8_t data_to_send[] = {0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x89, 0xF9};
	uint8_t data_to_send[] = {ULTSON_RS485_DIR, 0x06, 0x00, 0x00, 0x00, 0x00, 0x89, 0xAC};// PARA ID=0x07
	memcpy(ultson_buffer_rs485, data_to_send, sizeof(data_to_send));

	//---> Habilito el envío de comandos.
	ultson_send_enable = true;
}


void UltSon_ConfigTiempo(uint8_t _min,uint8_t _seg) // minutos - segundos
{
	uint8_t data_to_send[8];
	data_to_send[0] = ULTSON_RS485_DIR;      // Dirección del esclavo
	data_to_send[1] = 0x06;      // Write Single Register
	data_to_send[2] = 0x00;      // Dirección registro High
	data_to_send[3] = 0x02;      // Dirección registro Low
	data_to_send[4] = _min;   // XX: minutos
	data_to_send[5] = _seg;  // YY: segundos

	// CRC sobre los primeros 6 bytes
	uint16_t crc = calcularCRC(data_to_send, 6);

	data_to_send[6] = (uint8_t)(crc & 0xFF);         // CRC Low
	data_to_send[7] = (uint8_t)((crc >> 8) & 0xFF);  // CRC High

	//---> Lo copio al buffer que se enviara.
	memcpy(ultson_buffer_rs485, data_to_send, sizeof(data_to_send));

	//---> Habilito el envío de comandos.
	ultson_send_enable = true;

}

void UltSon_ConfigPotencia(uint8_t _potencia) // valor entre 1-20
{
	uint8_t data_to_send[8];
	data_to_send[0] = ULTSON_RS485_DIR;      // Dirección del esclavo
	data_to_send[1] = 0x06;      // Write Single Register
	data_to_send[2] = 0x00;      // Dirección registro High
	data_to_send[3] = 0x04;      // Dirección registro Low
	data_to_send[4] = 0x00;       // Potencia High
	data_to_send[5] = _potencia; // Potencia Low

	// CRC sobre los primeros 6 bytes
	uint16_t crc = calcularCRC(data_to_send, 6);

	data_to_send[6] = (uint8_t)(crc & 0xFF);         // CRC Low
	data_to_send[7] = (uint8_t)((crc >> 8) & 0xFF);  // CRC High

	//---> Lo copio al buffer que se enviara.
	memcpy(ultson_buffer_rs485, data_to_send, sizeof(data_to_send));

	//---> Habilito el envío de comandos.
	ultson_send_enable = true;
}


/**/
void UltSon_LeerEstado();
void UltSon_LeerTiempo();
void UltSon_LeerPotencia();
void UltSon_LeerCorrienteMedida();
void UltSon_LeerFrecuencia();
