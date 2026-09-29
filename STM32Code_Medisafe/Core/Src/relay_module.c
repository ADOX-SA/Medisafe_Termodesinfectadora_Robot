/*
 * relay_module.c
 *
 *  Created on: Sep 16, 2025
 *      Author: ADOX
 */

#include <string.h>
#include "main.h"
#include "stdbool.h"
#include "relay_module.h"
#include "rs485_pressure_sensor.h"
#include "SystemTicks.h"

//---> Puerto RS485
extern UART_HandleTypeDef huart2;

/**
 * Variables utilizadas para la lectura de entradas
 * */
#define RS485_RX_BUF_LEN 64
uint8_t rs485_rx_buffer[RS485_RX_BUF_LEN] = {0}; // Buffer donde se almacenan los datos.
//uint8_t rs485_rx_available = 0; //Si hay datos disponibles en el buffer: rs485_rx_buffer.
bool rs485_rx_receiving = false;      // Flag para indicar si estamos recibiendo una cadena
uint8_t rs485_rx_index = 0;        // Índice para almacenar caracteres en el buffer
uint8_t rs485_rx_byte; //Aca se guarda el byte recibido por serie.

/**
 * RS485 QUEUE
 * Variables para encolar mensajes.
 * */
#define RS485_QUEUE_SIZE 16 //Tamaño de la cola de mensajes
#define RS485_SEND_PERIOD 100  // Tiempo entre mensajes [ms]

typedef struct {
	uint8_t data[8];
	uint8_t len;
} RS485_Message;

RS485_Message rs485Queue[RS485_QUEUE_SIZE];
uint8_t queueHead = 0; //volatile?
uint8_t queueTail = 0;
uint32_t lastSendTick = 0;

/**
 * Variables para los estados de las entradas y salidas
 * */
uint8_t rm_input1[16];	// LAVADORA 1
uint8_t rm_output1[16];
uint8_t rm_input1_valid;
uint8_t l1_val08_val;
uint8_t l2_val08_val;

uint8_t rm_input2[16];  // LAVADORA 2
uint8_t rm_output2[16];

// Funciones para habilitar/deshabilitar transmisión RS485
void RS485_SetTxMode()
{
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_SET); // Activar transmisión
}
void RS485_SetRxMode()
{
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET); // Activar recepción
}

void RelayModule_Init(ENUM_REF_LAV_T ID)
{
	switch (ID)
	{
	case GENERAL:	// dummy

		break;

	case REPRO1:
		rm_input1_valid = 0;
		for (uint8_t i = 0;  i < 16; ++ i)
		{
			rm_input1[i]=0;
			rm_output1[i]=0;
		}
		//CMP14_OFF
		//L1_VAL08 = 1;
		//L1_VAL08_OFF //Para bloquear la valvula de 3 vias.

		//RelayModule_WriteAllOutputs(1);
		break;

	case REPRO2:
		for (uint8_t i = 0;  i < 16; ++ i)
		{
			rm_input2[i]=0;
			rm_output2[i]=0;
		}
		CMP14_OFF
		//L2_VAL08 = 1;
		//L2_VAL08_OFF //Para bloquear la valvula de 3 vias.

		//RelayModule_WriteAllOutputs(2);
		break;
	}
}

void RelayModule_ReadAllInputs(uint8_t _lav){

	const uint8_t read_input_module_1[8] = {0x01,0x02,0x00,0x00,0x00,0x10,0x79,0xC6};
	const uint8_t read_input_module_2[8] = {0x02,0x02,0x00,0x00,0x00,0x10,0x79,0xF5};

	uint8_t data_to_send[8] = {0};

	if (_lav == 1)
	{
		memcpy(data_to_send, read_input_module_1, sizeof(read_input_module_1));
	}else if(_lav == 2)
	{
		memcpy(data_to_send, read_input_module_2, sizeof(read_input_module_2));
	}
	else
	{
		return;
	}

	//-----> Envío la trama al puerto RS485
	RS485_SetTxMode();
	HAL_UART_Transmit(&huart2, data_to_send,sizeof(data_to_send), HAL_MAX_DELAY);
	RS485_SetRxMode();
	/**/
	//__HAL_UART_CLEAR_OREFLAG(&huart2);
	//__HAL_UART_CLEAR_FEFLAG(&huart2);
	//__HAL_UART_CLEAR_NEFLAG(&huart2);
	__HAL_UART_CLEAR_PEFLAG(&huart2);
	/**/
	HAL_UART_Receive_IT(&huart2, rs485_rx_buffer, 7);

}


uint16_t calcularCRC(uint8_t *data, uint16_t length)
{
	uint16_t crc = 0xFFFF;

	for (uint16_t pos = 0; pos < length; pos++) {
		crc ^= (uint16_t)data[pos];  // XOR byte con parte baja del CRC

		for (uint8_t i = 0; i < 8; i++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0xA001;  // Polinomio inverso (MODBUS)
			} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}



void RelayModule_WriteOutput(ENUM_REF_LAV_T ID, uint8_t _output, uint8_t _value)
{

	/**
	 *
	 * Logica RS485 para escribir una salida
	 *
	 * */
	/*
	uint8_t data_to_send[8];

	switch(ID)
	{
	case GENERAL:	// dummy
		data_to_send[0] = 0x01;       // Slave 1
		break;

	case REPRO1:
		data_to_send[0] = 0x01;       // Slave 1
		break;

	case REPRO2:
		data_to_send[0] = 0x02;       // Slave 2
		break;
	}

	data_to_send[1] = 0x05;       // Función Write Single Coil
	data_to_send[2] = 0x00;       // Dirección alta
	data_to_send[3] = _output;     // Dirección baja (0–16)
	data_to_send[4] = _value ? 0xFF : 0x00;  // Valor alto
	data_to_send[5] = 0x00;       // Valor bajo

	uint16_t crc = calcularCRC(data_to_send, 6);
	data_to_send[6] = crc & 0xFF;         // CRC Low
	data_to_send[7] = (crc >> 8) & 0xFF;  // CRC High

	// Con cola de mensajes:
	uint8_t nextHead = (queueHead + 1) % RS485_QUEUE_SIZE;
	if (nextHead != queueTail) {  // cola no llena
		memcpy(rs485Queue[queueHead].data, data_to_send, 8);
		rs485Queue[queueHead].len = 8;
		queueHead = nextHead;
	}*/
}


void RelayModule_WriteAllOutputs(uint8_t _addr)
{
	// Cabecera
	uint8_t trama[11];
	trama[0] = _addr;   // 0x01 / 0x02 / etc.
	trama[1] = 0x0F;         // Write Multiple Coils
	trama[2] = 0x00;         // Start addr Hi (0x0000)
	trama[3] = 0x00;         // Start addr Lo
	trama[4] = 0x00;         // Quantity Hi
	trama[5] = 0x10;         // Quantity Lo = 16
	trama[6] = 0x02;         // Byte count = 2

	// Empaquetar 16 salidas (LSB-first)
	uint8_t b0 = 0, b1 = 0;
	for (uint8_t i = 0; i < 8; i++) {
		if(_addr==1)
		{
			if (rm_output1[i])     b0 |= (uint8_t)(1u << i);     // coils 0..7
			if (rm_output1[i + 8]) b1 |= (uint8_t)(1u << i);     // coils 8..15
		}
		else if(_addr==2)
		{
			if (rm_output2[i])     b0 |= (uint8_t)(1u << i);     // coils 0..7
			if (rm_output2[i + 8]) b1 |= (uint8_t)(1u << i);     // coils 8..15
		}
	}

	trama[7] = b0;
	trama[8] = b1;

	// CRC sobre 9 bytes: trama[0]..trama[8]
	uint16_t crc = calcularCRC(trama, 9);
	trama[9]  = (uint8_t)(crc & 0xFF);        // CRC Low
	trama[10] = (uint8_t)((crc >> 8) & 0xFF); // CRC High

	RS485_SetTxMode();
	HAL_UART_Transmit(&huart2, trama,sizeof(trama), HAL_MAX_DELAY);
	RS485_SetRxMode();

}

void RS485_Queue_Loop(void)
{

	if (queueTail != queueHead) { // hay mensajes en cola
		if(!rs485_queue_tic){ //Si el tick es = 0 y esta habilitado:
			rs485_queue_tic = RS485_SEND_PERIOD;
			RS485_SetTxMode();
			HAL_UART_Transmit(&huart2, rs485Queue[queueTail].data,
					rs485Queue[queueTail].len, HAL_MAX_DELAY);
			RS485_SetRxMode();

			queueTail = (queueTail + 1) % RS485_QUEUE_SIZE;
		}
	} else
	{
		//---> No hay mensajes en la cola:
		//rs485_rm_outputs_en = false; //Leo entradas RS485.
		//rs485_read_inputs_fsm = true;
	}
}

void RS485_SaveInputStatus(uint8_t _lav)
{
	// Selección del vector destino según esclavo
	uint8_t *dst = NULL;
	if (_lav == 1) {
		dst = rm_input1;
	}
	else if (_lav == 2)
	{
		dst = rm_input2;
	}
	else {
		return;
	}

	//---> Chequeo errores de trama
	if (rs485_rx_buffer[0] != _lav) return;     // No es el address esperado
	if (rs485_rx_buffer[1] != 0x02) return;     // No es función 0x02


	if (rs485_rx_buffer[2] != 0x02) return;

	uint8_t idx = 0;
	for (uint8_t b = 0; b < 2; b++)
	{
		uint8_t data = rs485_rx_buffer[3 + b];
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			dst[idx++] = (data >> bit) & 0x01;
		}
	}

	if (_lav == REPRO1)
		rm_input1_valid = 1;
}



