/*
 * ntc_adc.c
 *
 *  Created on: Jan 28, 2026
 *      Author: ADOX
 */
#include "ntc_adc.h"

#include "main.h"
#include "nextion.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#include "relay_module.h"
#include "service.h"
#include "SystemTicks.h"
#include "fsm_reprocesadora1.h"
#include "global_variables.h"
#include "user_gpio.h"
#include "i2c_pressure.h"

/*
 * TABLA DE VALORES (Calibrado con termocupla larga)
 * TEMP - ADC
 * 10.5 - 3800
 * 12.3 - 3700
 * 20.5 - 3150
 * 24.9 - 2950
 * 28.5 - 2780
 * 31   - 2650
 * 35   - 2430
 * 40   - 2200
 * 42.8 - 2060
 * 46   - 1940
 * 56   - 1560
 * 61.5 - 1390
 * 76.5 - 930
 *
 * */

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

#define DATA_LENGTH 13 //---> Cantidad de puntos de nuestra curva.
#define SAMPLES_QUANTITY 10    //---> Cantidad de muestras para promediar

//---> Lavadora 1
uint16_t L1_ntc_adc_value; // Variable para uso externo.
float L1_ntc_temperature;  // Variable para uso externo.
uint16_t L1_adc_sample[SAMPLES_QUANTITY]={0};

//---> Lavadora 2
uint16_t L2_ntc_adc_value; // Variable para uso externo.
float L2_ntc_temperature;  // Variable para uso externo.
uint16_t L2_adc_sample[SAMPLES_QUANTITY]={0};

float table_temps[DATA_LENGTH]={ 10.5, 12.3, 20.5, 24.9, 28.5, 31, 35, 40, 42.8, 46, 56, 61.5, 76.5 };
uint16_t table_adc[DATA_LENGTH]= { 3800, 3700, 3150, 2950, 2780, 2650, 2430, 2200, 2060, 1940, 1560, 1390, 930 };

static float NTC_ADC_CalculateTemp(uint16_t _adc_received)
{
	if (_adc_received >= table_adc[0])
		return table_temps[0];

	if (_adc_received <= table_adc[DATA_LENGTH-1])
		return table_temps[DATA_LENGTH-1];

	for (uint8_t i = 0; i < (DATA_LENGTH - 1); i++)
	{
		uint16_t a0 = table_adc[i];
		uint16_t a1 = table_adc[i + 1];

		if ((_adc_received <= a0) && (_adc_received >= a1))
		{
			float t0 = table_temps[i];
			float t1 = table_temps[i + 1];

			float frac = ((float)_adc_received - (float)a0) / ((float)a1 - (float)a0);
			return t0 + frac * (t1 - t0);
		}
	}
	// Por seguridad
	return table_temps[DATA_LENGTH-1];
}


void NTC_ADC_Loop()
{
	if(!ntc_adc_tic)
	{
		//----> Medicion cada "x" milisegundos
		ntc_adc_tic=500;
		//-----<

		static uint8_t sample_index=0; // Uso el mismo indice para los 2.
		uint16_t L1_adc_new_sample, L2_adc_new_sample;
		uint32_t L1_acumulador = 0, L2_acumulador = 0;

		//------> LECTURA ADC 1
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 10);
		L1_adc_new_sample = (uint16_t)HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		//------<

		//------> LECTURA ADC 2
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc2, 10);
		L2_adc_new_sample = (uint16_t)HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc2);
		//------<

		L1_adc_sample[sample_index] = L1_adc_new_sample;
		L2_adc_sample[sample_index] = L2_adc_new_sample;
		sample_index++;

		if(sample_index >= SAMPLES_QUANTITY)
			sample_index = 0;

		for(uint8_t i = 0; i < SAMPLES_QUANTITY; i++)
		{
			L1_acumulador += L1_adc_sample[i];
			L2_acumulador += L2_adc_sample[i];
		}

		L1_ntc_adc_value = (uint16_t)(L1_acumulador / SAMPLES_QUANTITY);
		L2_ntc_adc_value = (uint16_t)(L2_acumulador / SAMPLES_QUANTITY);

		//---------------------------------------------
		//------> CALCULO DE TEMPERATURA
		//---------------------------------------------

		//buscar valor ntc_adc_value en el vector table_adc[]
		//siempre agarrar el valor mas grande
		uint8_t table_index=0;
		while((table_index < (DATA_LENGTH-1)) && (L1_ntc_adc_value < table_adc[table_index]))
		{
			table_index++;
		}
		L1_ntc_temperature = table_temps[table_index];

		table_index=0;
		while((table_index < (DATA_LENGTH-1)) && (L2_ntc_adc_value < table_adc[table_index]))
		{
			table_index++;
		}
		L2_ntc_temperature = table_temps[table_index];

		//------> Copio a los valores que usa la maquina de estado.
		//L1_realTempW=L1_ntc_temperature;
		//L1_realTempD=L1_ntc_temperature;
		//L2_realTempW=L2_ntc_temperature;
		//L2_realTempD=L2_ntc_temperature;

		L1_realTemp=NTC_ADC_CalculateTemp(L1_ntc_adc_value);
		L2_realTemp=NTC_ADC_CalculateTemp(L2_ntc_adc_value);
	}
}
