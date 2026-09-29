/*
 * i2c_pressure.c
 *
 *  Created on: Dec 16, 2024
 *      Author: momo
 */

#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#include "global_variables.h"
#include "fsm_reprocesadora1.h"
#include "i2c_pressure.h"
#include "user_gpio.h"

extern I2C_HandleTypeDef hi2c1;
extern bool botonComenzar;

// Dirección del esclavo (ajustada a 7 bits)
#define SLAVE_ADDRESS 0x6D
#define I2C_TIMEOUT 100 // Tiempo de espera para operaciones I2C

uint32_t cont = 0;
uint32_t presion = 0;
float temp = 0;
char comando[50];

bool iniciar = 0;
uint8_t etapa = 0; // 0: esperar 20kPa, 1: esperar 30s sin bajar de 15kPa, 2: fin
uint16_t estabilidad_counter = 0;
uint16_t timeout_presion_counter = 0;

// Función para escribir un byte en un registro del esclavo
HAL_StatusTypeDef WriteOneByte(uint8_t reg_addr, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c1, SLAVE_ADDRESS << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, &data, 1, I2C_TIMEOUT);
}

// Función para leer un byte desde un registro del esclavo
HAL_StatusTypeDef ReadOneByte(uint8_t reg_addr, uint8_t *data)
{
    return HAL_I2C_Mem_Read(&hi2c1, SLAVE_ADDRESS << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_TIMEOUT);
}

// Función para leer la presión en Pascales
int32_t ReadPressurePa(void)
{
    uint8_t pressure_H = 0, pressure_M = 0, pressure_L = 0;
    uint32_t pressure_adc = 0;
    int32_t pressure = 0;

    // Leer los registros de presión (0x06, 0x07, 0x08)
    if (ReadOneByte(0x06, &pressure_H) != HAL_OK) return -1; // Error en la lectura
    if (ReadOneByte(0x07, &pressure_M) != HAL_OK) return -1; // Error en la lectura
    if (ReadOneByte(0x08, &pressure_L) != HAL_OK) return -1; // Error en la lectura

    // Combinar los bytes en un entero de 24 bits
    pressure_adc = (pressure_H << 16) | (pressure_M << 8) | pressure_L;

    // Calcular la presión en Pascales
    float K = 128;  // 0 - 40 kPa

    if (pressure_adc > 8388608)
        pressure = (int32_t)((pressure_adc - 16777216) / K);
    else
        pressure = (int32_t)(pressure_adc / K);

    return pressure; // en Pa
}

// Función para leer la temperatura en grados Celsius
float ReadTemperature(void)
{
    uint8_t temperature_H = 0, temperature_L = 0;
    uint16_t temperature_adc = 0;
    float temperature = 0;

    // Leer los registros de temperatura (0x09, 0x0A)
    if (ReadOneByte(0x09, &temperature_H) != HAL_OK) return -1; // Error en la lectura
    if (ReadOneByte(0x0A, &temperature_L) != HAL_OK) return -1; // Error en la lectura

    // Combinar los bytes en un entero de 16 bits
    temperature_adc = (temperature_H << 8) | temperature_L;

    // Calcular la temperatura en grados Celsius
    if (temperature_adc > 65536)
        temperature = (temperature_adc - 65536) / 256.0f;
    else
        temperature = temperature_adc / 256.0f;

    return temperature;
}

void sensadoPresion(void)
{
	if (!i2c_tic)
	{
		i2c_tic = 200;

		// Iniciar conversión
		if (WriteOneByte(0x30, 0x0A) != HAL_OK)
			return;

		uint32_t timeout = HAL_GetTick();
		uint8_t status = 0;

		do {
			if (ReadOneByte(0x30, &status) != HAL_OK)
				return;
			if ((HAL_GetTick() - timeout) > 300)
				return;
		} while (status & 0x08); // Bit 3: conversión en curso

		presion = ReadPressurePa();
		estanq_presionKPa = presion / 1000.0f;
		temp = ReadTemperature();
/*
		int angulo = (presionkPa * 255) / 40;
		sprintf(comando, "mano.val=%d", angulo);
		enqueue_nextion_command(comando);
		memset(comando, 0, sizeof(comando));*/
	}
}


void fsm_on_Estanqueidad(void)
{
	if(!iniciar)
	{
		fsm_on_BombaValvula();
		fsm_on_solenoide();

		iniciar = 1;
	}

	if (!pressure_tic)
	{
		pressure_tic = 100;

		switch (etapa)
		{
			case 0: // Esperar que suba a 20 kPa
				if (estanq_presionKPa >= 20.0f)
				{
					fsm_off_Estanqueidad();
					estabilidad_counter = 0;
					etapa = 1;
				}
				else
				{
					timeout_presion_counter++;

					if (timeout_presion_counter >= TIMEOUT_ESTANQUE)
					{
						fsm1_raise_ev("Fuga"); // Falla
						reset_estanqueidad();
					}
				}
			break;

			case 1: // Esperar 30 s sin bajar de 15 kPa
				if (estanq_presionKPa < 17.0f)
				{
					fsm1_raise_ev("Fuga"); // Falla
					reset_estanqueidad();
					break;
				}

				estabilidad_counter++;
				if (estabilidad_counter >= 300) // 30 s * 10 Hz
				{
					fsm1_raise_ev("Estanq_OK"); // Prueba superada
					reset_estanqueidad();
				}
			break;
		}
	}
}

void reset_estanqueidad(void)
{
	etapa = 0;
	estabilidad_counter = 0;
	iniciar = 0;
	timeout_presion_counter = 0;
}
