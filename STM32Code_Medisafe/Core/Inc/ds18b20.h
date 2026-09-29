/* ds18b20.h
 *
 *  The MIT License.
 *  Created on: 20.09.2018
 *  Author: Mateusz Salamon
 *  Modified: multi-bus (2x 1-Wire) integration
 */
#ifndef _DS18B20_H
#define _DS18B20_H

#include "stm32f1xx_hal.h"
#include "onewire.h"

extern TIM_HandleTypeDef htim1;

/* ===== CONFIG ===== */

// Max sensores POR BUS
#define _DS18B20_MAX_SENSORS 4

// Timer usado para delays us (1us por tick)
#define _DS18B20_TIMER htim1

// Cantidad de buses (PA11 y PA12)
#define DS18_BUS_COUNT 2

// #define _DS18B20_USE_CRC

/* ===== TYPES ===== */

typedef struct
{
    uint8_t Address[8];
    float   Temperature;
    uint8_t ValidDataFlag;
} Ds18b20Sensor_t;

#define DS18B20_FAMILY_CODE 0x28

#define DS18B20_CMD_ALARMSEARCH 0xEC
#define DS18B20_CMD_CONVERTTEMP 0x44

#define DS18B20_STEP_12BIT 0.0625f
#define DS18B20_STEP_11BIT 0.125f
#define DS18B20_STEP_10BIT 0.25f
#define DS18B20_STEP_9BIT  0.5f

#define DS18B20_RESOLUTION_R1 6
#define DS18B20_RESOLUTION_R0 5

#ifdef _DS18B20_USE_CRC
#define DS18B20_DATA_LEN 9
#else
#define DS18B20_DATA_LEN 5
#endif

typedef enum {
    DS18B20_Resolution_9bits  = 9,
    DS18B20_Resolution_10bits = 10,
    DS18B20_Resolution_11bits = 11,
    DS18B20_Resolution_12bits = 12
} DS18B20_Resolution_t;

typedef enum {
    DS18_BUS_LAV1 = 0,   // PA11 (sensor 1)
    DS18_BUS_LAV2 = 1    // PA12 (sensor 2)
} ds18_bus_t;

/* ===== ORIGINAL API (compat: opera sobre BUS 0) ===== */
void    DS18B20_Init(DS18B20_Resolution_t resolution, GPIO_TypeDef *port, uint16_t pin);
uint8_t DS18B20_GetResolution(uint8_t number);
uint8_t DS18B20_SetResolution(uint8_t number, DS18B20_Resolution_t resolution);
uint8_t DS18B20_Start(uint8_t number);
void    DS18B20_StartAll(void);
uint8_t DS18B20_Read(uint8_t number, float *destination);
void    DS18B20_ReadAll(void);
void    DS18B20_ReadAllForce(void);
uint8_t DS18B20_Is(uint8_t *ROM);
uint8_t DS18B20_AllDone(void);
void    DS18B20_GetROM(uint8_t number, uint8_t *ROM);
void    DS18B20_WriteROM(uint8_t number, uint8_t *ROM);
uint8_t DS18B20_Quantity(void);
uint8_t DS18B20_GetTemperature(uint8_t number, float *destination);

/* ===== MULTI-BUS API ===== */
void    DS18B20_InitBus(ds18_bus_t bus, DS18B20_Resolution_t resolution, GPIO_TypeDef *port, uint16_t pin);
uint8_t DS18B20_QuantityBus(ds18_bus_t bus);
uint8_t DS18B20_GetTemperatureBus(ds18_bus_t bus, uint8_t number, float *destination);
void    DS18B20_StartAllBus(ds18_bus_t bus);
void    DS18B20_ReadAllForceBus(ds18_bus_t bus);

/* ===== Scheduler (tick 1ms + service) ===== */
void DS18B20_SchedulerInit(uint32_t period_ms);      // compat: BUS 0
void DS18B20_Tick1ms(void);                          // compat: BUS 0
void DS18B20_Service(void);                          // compat: BUS 0

void DS18B20_SchedulerInitBus(ds18_bus_t bus, uint32_t period_ms);
void DS18B20_Tick1msBus(ds18_bus_t bus);
void DS18B20_ServiceBus(ds18_bus_t bus);

void DS18B20_Tick1msAll(void);
void DS18B20_ServiceAll(void);

#endif
