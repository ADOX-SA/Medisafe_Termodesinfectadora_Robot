/* ds18b20.c
 *
 *  The MIT License.
 *  Created on: 20.09.2018
 *  Author: Mateusz Salamon
 *  Modified: multi-bus (2x 1-Wire) integration + scheduler per bus
 */
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

#include "ds18b20.h"
#include "global_variables.h"

/* ===== Scheduler interno ===== */
typedef enum { DS18_ST_IDLE=0, DS18_ST_START, DS18_ST_WAIT, DS18_ST_READ } ds18_st_t;

#define DS18_CONV_MS_12BIT  750u

/* ===== Estado por bus ===== */
static Ds18b20Sensor_t ds18b20[DS18_BUS_COUNT][_DS18B20_MAX_SENSORS];
static OneWire_t       OneWire[DS18_BUS_COUNT];
static uint8_t         TempSensorCount[DS18_BUS_COUNT];

/* ===== Scheduler por bus ===== */
static volatile ds18_st_t  ds18_st[DS18_BUS_COUNT];
static volatile uint32_t   ds18_period_cnt[DS18_BUS_COUNT];
static volatile uint32_t   ds18_wait_cnt[DS18_BUS_COUNT];
static uint32_t            ds18_period_cfg[DS18_BUS_COUNT];

/* ===== Helpers internos bus-aware ===== */

static uint8_t DS18B20_ReadBus(ds18_bus_t bus, uint8_t number, float *destination)
{
    if (number >= TempSensorCount[bus]) return 0;

    uint16_t temperature;
    uint8_t resolution;
    float result;
    uint8_t data[DS18B20_DATA_LEN];

#ifdef _DS18B20_USE_CRC
    uint8_t crc;
#endif

    if (!DS18B20_Is((uint8_t*)&ds18b20[bus][number].Address))
        return 0;

    // si el bus está ocupado (conversión no finalizada) retorna 0
    if (!OneWire_ReadBit(&OneWire[bus]))
        return 0;

    OneWire_Reset(&OneWire[bus]);
    OneWire_SelectWithPointer(&OneWire[bus], (uint8_t*)&ds18b20[bus][number].Address);
    OneWire_WriteByte(&OneWire[bus], ONEWIRE_CMD_RSCRATCHPAD);

    for (uint8_t i = 0; i < DS18B20_DATA_LEN; i++)
        data[i] = OneWire_ReadByte(&OneWire[bus]);

#ifdef _DS18B20_USE_CRC
    crc = OneWire_CRC8(data, 8);
    if (crc != data[8])
        return 0;
#endif

    temperature = (uint16_t)(data[0] | ((uint16_t)data[1] << 8));

    OneWire_Reset(&OneWire[bus]);

    resolution = (uint8_t)(((data[4] & 0x60) >> 5) + 9);

    switch (resolution)
    {
        case DS18B20_Resolution_9bits:  result = (float)temperature * DS18B20_STEP_9BIT;  break;
        case DS18B20_Resolution_10bits: result = (float)temperature * DS18B20_STEP_10BIT; break;
        case DS18B20_Resolution_11bits: result = (float)temperature * DS18B20_STEP_11BIT; break;
        case DS18B20_Resolution_12bits: result = (float)temperature * DS18B20_STEP_12BIT; break;
        default: result = 0xFF; break;
    }

    *destination = result;
    return 1;
}

static uint8_t DS18B20_SetResolutionBus(ds18_bus_t bus, uint8_t number, DS18B20_Resolution_t resolution)
{
    if (number >= TempSensorCount[bus]) return 0;

    uint8_t th, tl, conf;

    if (!DS18B20_Is((uint8_t*)&ds18b20[bus][number].Address))
        return 0;

    OneWire_Reset(&OneWire[bus]);
    OneWire_SelectWithPointer(&OneWire[bus], (uint8_t*)&ds18b20[bus][number].Address);
    OneWire_WriteByte(&OneWire[bus], ONEWIRE_CMD_RSCRATCHPAD);

    OneWire_ReadByte(&OneWire[bus]);
    OneWire_ReadByte(&OneWire[bus]);

    th   = OneWire_ReadByte(&OneWire[bus]);
    tl   = OneWire_ReadByte(&OneWire[bus]);
    conf = OneWire_ReadByte(&OneWire[bus]);

    if (resolution == DS18B20_Resolution_9bits)
    {
        conf &= ~(1 << DS18B20_RESOLUTION_R1);
        conf &= ~(1 << DS18B20_RESOLUTION_R0);
    }
    else if (resolution == DS18B20_Resolution_10bits)
    {
        conf &= ~(1 << DS18B20_RESOLUTION_R1);
        conf |=  (1 << DS18B20_RESOLUTION_R0);
    }
    else if (resolution == DS18B20_Resolution_11bits)
    {
        conf |=  (1 << DS18B20_RESOLUTION_R1);
        conf &= ~(1 << DS18B20_RESOLUTION_R0);
    }
    else // 12 bits
    {
        conf |= (1 << DS18B20_RESOLUTION_R1);
        conf |= (1 << DS18B20_RESOLUTION_R0);
    }

    OneWire_Reset(&OneWire[bus]);
    OneWire_SelectWithPointer(&OneWire[bus], (uint8_t*)&ds18b20[bus][number].Address);
    OneWire_WriteByte(&OneWire[bus], ONEWIRE_CMD_WSCRATCHPAD);

    OneWire_WriteByte(&OneWire[bus], th);
    OneWire_WriteByte(&OneWire[bus], tl);
    OneWire_WriteByte(&OneWire[bus], conf);

    OneWire_Reset(&OneWire[bus]);
    OneWire_SelectWithPointer(&OneWire[bus], (uint8_t*)&ds18b20[bus][number].Address);
    OneWire_WriteByte(&OneWire[bus], ONEWIRE_CMD_CPYSCRATCHPAD);

    return 1;
}

static uint8_t DS18B20_AllDoneBus(ds18_bus_t bus)
{
    return OneWire_ReadBit(&OneWire[bus]);
}

/* ===== Public multi-bus ===== */

void DS18B20_InitBus(ds18_bus_t bus, DS18B20_Resolution_t resolution, GPIO_TypeDef *port, uint16_t pin)
{
    uint8_t next = 0, i = 0;

    if (bus >= DS18_BUS_COUNT) return;

    TempSensorCount[bus] = 0;

    OneWire_Init(&OneWire[bus], port, pin);

    next = OneWire_First(&OneWire[bus]);
    while (next)
    {
        TempSensorCount[bus]++;
        OneWire_GetFullROM(&OneWire[bus], (uint8_t*)&ds18b20[bus][i++].Address);

        next = OneWire_Next(&OneWire[bus]);

        if (TempSensorCount[bus] >= _DS18B20_MAX_SENSORS)
            break;
    }

    // Configurar resolución a los sensores detectados (si hay)
    for (uint8_t j = 0; j < TempSensorCount[bus]; j++)
    {
        (void)DS18B20_SetResolutionBus(bus, j, resolution);
    }

    // Disparar primera conversión (opcional)
    DS18B20_StartAllBus(bus);
}

uint8_t DS18B20_QuantityBus(ds18_bus_t bus)
{
    if (bus >= DS18_BUS_COUNT) return 0;
    return TempSensorCount[bus];
}

uint8_t DS18B20_GetTemperatureBus(ds18_bus_t bus, uint8_t number, float *destination)
{
    if (bus >= DS18_BUS_COUNT) return 0;
    if (number >= TempSensorCount[bus]) return 0;

    if (!ds18b20[bus][number].ValidDataFlag)
        return 0;

    *destination = ds18b20[bus][number].Temperature;
    return 1;
}

void DS18B20_StartAllBus(ds18_bus_t bus)
{
    if (bus >= DS18_BUS_COUNT) return;

    OneWire_Reset(&OneWire[bus]);
    OneWire_WriteByte(&OneWire[bus], ONEWIRE_CMD_SKIPROM);
    OneWire_WriteByte(&OneWire[bus], DS18B20_CMD_CONVERTTEMP);
}

void DS18B20_ReadAllForceBus(ds18_bus_t bus)
{
    if (bus >= DS18_BUS_COUNT) return;

    for (uint8_t i = 0; i < TempSensorCount[bus]; i++)
    {
        ds18b20[bus][i].ValidDataFlag = 0;

        if (DS18B20_Is((uint8_t*)&ds18b20[bus][i].Address))
        {
            ds18b20[bus][i].ValidDataFlag =
                DS18B20_ReadBus(bus, i, &ds18b20[bus][i].Temperature);
        }
    }
}

/* ===== Scheduler multi-bus ===== */

void DS18B20_SchedulerInitBus(ds18_bus_t bus, uint32_t period_ms)
{
    if (bus >= DS18_BUS_COUNT) return;

    ds18_period_cfg[bus] = (period_ms == 0) ? 1000u : period_ms;
    ds18_period_cnt[bus] = ds18_period_cfg[bus];
    ds18_wait_cnt[bus]   = 0;
    ds18_st[bus]         = DS18_ST_IDLE;
}

// OJO: esta función va en HAL_IncTick(), NO debe tocar el bus
void DS18B20_Tick1msBus(ds18_bus_t bus)
{
    if (bus >= DS18_BUS_COUNT) return;

    if (ds18_period_cnt[bus]) ds18_period_cnt[bus]--;
    if (ds18_wait_cnt[bus])   ds18_wait_cnt[bus]--;

    switch (ds18_st[bus])
    {
        case DS18_ST_IDLE:
            if (ds18_period_cnt[bus] == 0)
            {
                ds18_period_cnt[bus] = ds18_period_cfg[bus];
                ds18_st[bus] = DS18_ST_START;
            }
            break;

        case DS18_ST_WAIT:
            if (ds18_wait_cnt[bus] == 0)
                ds18_st[bus] = DS18_ST_READ;
            break;

        default:
            break; // START y READ se atienden en ServiceBus()
    }
}

// Esta función va en while(1). Acá sí se toca el 1-Wire
void DS18B20_ServiceBus(ds18_bus_t bus)
{
    if (bus >= DS18_BUS_COUNT) return;

    switch (ds18_st[bus])
    {
        case DS18_ST_START:
            DS18B20_StartAllBus(bus);
            ds18_wait_cnt[bus] = DS18_CONV_MS_12BIT;
            ds18_st[bus] = DS18_ST_WAIT;
            break;

        case DS18_ST_READ:
        {
            DS18B20_ReadAllForceBus(bus);

            // 1 sensor por bus -> index 0
            float t;
            if (DS18B20_GetTemperatureBus(bus, 0, &t))
            {
                if (bus == DS18_BUS_LAV1)
                    L1_realTemp = t;
            }

            ds18_st[bus] = DS18_ST_IDLE;
            break;
        }

        default:
            break;
    }
}

void DS18B20_Tick1msAll(void)
{
    DS18B20_Tick1msBus(DS18_BUS_LAV1);
}

void DS18B20_ServiceAll(void)
{
    DS18B20_ServiceBus(DS18_BUS_LAV1);
}

/* ===== ORIGINAL API (compat: BUS 0) ===== */

uint8_t DS18B20_Is(uint8_t *ROM)
{
    return (*ROM == DS18B20_FAMILY_CODE) ? 1u : 0u;
}

void DS18B20_Init(DS18B20_Resolution_t resolution, GPIO_TypeDef *port, uint16_t pin)
{
    DS18B20_InitBus(DS18_BUS_LAV1, resolution, port, pin);
}

uint8_t DS18B20_Quantity(void)
{
    return DS18B20_QuantityBus(DS18_BUS_LAV1);
}

uint8_t DS18B20_Start(uint8_t number)
{
    // si lo usás, opera sobre BUS 0
    if (number >= TempSensorCount[DS18_BUS_LAV1]) return 0;

    if (!DS18B20_Is((uint8_t*)&ds18b20[DS18_BUS_LAV1][number].Address))
        return 0;

    OneWire_Reset(&OneWire[DS18_BUS_LAV1]);
    OneWire_SelectWithPointer(&OneWire[DS18_BUS_LAV1], (uint8_t*)ds18b20[DS18_BUS_LAV1][number].Address);
    OneWire_WriteByte(&OneWire[DS18_BUS_LAV1], DS18B20_CMD_CONVERTTEMP);

    return 1;
}

void DS18B20_StartAll(void)
{
    DS18B20_StartAllBus(DS18_BUS_LAV1);
}

uint8_t DS18B20_Read(uint8_t number, float *destination)
{
    return DS18B20_ReadBus(DS18_BUS_LAV1, number, destination);
}

uint8_t DS18B20_GetResolution(uint8_t number)
{
    // versión simple: lee scratchpad config (BUS 0)
    if (number >= TempSensorCount[DS18_BUS_LAV1]) return 0;

    uint8_t conf;

    if (!DS18B20_Is((uint8_t*)&ds18b20[DS18_BUS_LAV1][number].Address))
        return 0;

    OneWire_Reset(&OneWire[DS18_BUS_LAV1]);
    OneWire_SelectWithPointer(&OneWire[DS18_BUS_LAV1], (uint8_t*)&ds18b20[DS18_BUS_LAV1][number].Address);
    OneWire_WriteByte(&OneWire[DS18_BUS_LAV1], ONEWIRE_CMD_RSCRATCHPAD);

    OneWire_ReadByte(&OneWire[DS18_BUS_LAV1]);
    OneWire_ReadByte(&OneWire[DS18_BUS_LAV1]);
    OneWire_ReadByte(&OneWire[DS18_BUS_LAV1]);
    OneWire_ReadByte(&OneWire[DS18_BUS_LAV1]);

    conf = OneWire_ReadByte(&OneWire[DS18_BUS_LAV1]);
    conf &= 0x60;
    conf >>= 5;
    conf += 9;

    return conf;
}

uint8_t DS18B20_SetResolution(uint8_t number, DS18B20_Resolution_t resolution)
{
    return DS18B20_SetResolutionBus(DS18_BUS_LAV1, number, resolution);
}

uint8_t DS18B20_AllDone(void)
{
    return DS18B20_AllDoneBus(DS18_BUS_LAV1);
}

void DS18B20_ReadAll(void)
{
    // mantiene la semántica original (BUS 0)
    if (DS18B20_AllDone())
    {
        for (uint8_t i = 0; i < TempSensorCount[DS18_BUS_LAV1]; i++)
        {
            ds18b20[DS18_BUS_LAV1][i].ValidDataFlag = 0;

            if (DS18B20_Is((uint8_t*)&ds18b20[DS18_BUS_LAV1][i].Address))
            {
                ds18b20[DS18_BUS_LAV1][i].ValidDataFlag =
                    DS18B20_ReadBus(DS18_BUS_LAV1, i, &ds18b20[DS18_BUS_LAV1][i].Temperature);
            }
        }
    }
}

void DS18B20_ReadAllForce(void)
{
    DS18B20_ReadAllForceBus(DS18_BUS_LAV1);
}

void DS18B20_GetROM(uint8_t number, uint8_t *ROM)
{
    if (number >= TempSensorCount[DS18_BUS_LAV1])
        number = TempSensorCount[DS18_BUS_LAV1];

    for (uint8_t i = 0; i < 8; i++)
        ROM[i] = ds18b20[DS18_BUS_LAV1][number].Address[i];
}

void DS18B20_WriteROM(uint8_t number, uint8_t *ROM)
{
    if (number >= TempSensorCount[DS18_BUS_LAV1]) return;

    for (uint8_t i = 0; i < 8; i++)
        ds18b20[DS18_BUS_LAV1][number].Address[i] = ROM[i];
}

uint8_t DS18B20_GetTemperature(uint8_t number, float *destination)
{
    return DS18B20_GetTemperatureBus(DS18_BUS_LAV1, number, destination);
}

/* ===== Scheduler compat BUS 0 ===== */

void DS18B20_SchedulerInit(uint32_t period_ms)
{
    DS18B20_SchedulerInitBus(DS18_BUS_LAV1, period_ms);
}

void DS18B20_Tick1ms(void)
{
    DS18B20_Tick1msBus(DS18_BUS_LAV1);
}

void DS18B20_Service(void)
{
    DS18B20_ServiceBus(DS18_BUS_LAV1);
}
