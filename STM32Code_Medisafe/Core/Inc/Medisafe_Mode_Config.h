/*
 * Medisafe_Mode_Config.h
 *
 * Parametros de los programas de lavado Medisafe y limites de seguridad.
 */

#ifndef INC_MEDISAFE_MODE_CONFIG_H_
#define INC_MEDISAFE_MODE_CONFIG_H_

#include <stdint.h>

/* Parametros temporales del ciclo P1 usado actualmente para pruebas. */
#define MEDSAF_TIME_PRE_WASH 30000
#define MEDSAF_TEMP_PRE_WASH 35.0f
#define MEDSAF_TIME_TAPA_SPRAY 10000
#define MEDSAF_TIME_LUMEN_FLUSH 5000
#define MEDSAF_TIME_DRENAJE_EXTRA 30000
#define MEDSAF_TIME_SENDSTATE 1000
#define MEDSAF_TIME_BUZZER_ERROR 1000
#define MEDSAF_TIME_BUZZER_FIN 2000
#define MEDSAF_TIME_DETERGENTE 10000
#define MEDSAF_TEMP_WASH 40.0f
#define MEDSAF_TIME_DEGAS 30000
#define MEDSAF_TIME_WASH 30000
#define MEDSAF_TIME_ULTRASONIDO_ON 10000
#define MEDSAF_TIME_ULTRASONIDO_OFF 5000
#define MEDSAF_TIME_WASH_ULTRASONIDO 10000
#define MEDSAF_TIME_WASH_LUMEN 5000
#define MEDSAF_TEMP_RINSE 60.0f
#define MEDSAF_TIME_RINSE 30000
#define MEDSAF_TIME_FINAL_RINSE 30000
#define MEDSAF_TEMP_DISINFECTION_STD 90.0f
#define MEDSAF_TEMP_DISINFECTION_RBT 91.0f
#define MEDSAF_TIME_DISINFECTION_STD 30000
#define MEDSAF_TIME_DISINFECTION_RBT 30000
#define MEDSAF_HISTERESIS_DISINFECTION 1.0f
#define MEDSAF_TIME_COOLING 30000
#define MEDSAF_TEMP_COOLING 60

/* Timeouts y limites de seguridad del proceso. */
#define MEDSAF_TIMEOUT_LLENADO 300000
#define MEDSAF_TIMEOUT_DRENAJE 300000
#define MEDSAF_TIMEOUT_CALEFACCION 600000
#define MEDSAF_TIMEOUT_DISINFECTION_HEATING 1800000
#define MEDSAF_TIMEOUT_COOLING 600000
#define MEDSAF_MARGEN_SOBRETEMPERATURA 5
#define MEDSAF_TIME_MUESTRA_CALEFACTOR 1000
#define MEDSAF_MUESTRAS_PROMEDIO_CALEFACTOR 5
#define MEDSAF_DIFERENCIA_MAX_TERMOCUPLAS 5.0f
#define MEDSAF_TEMP_CAMBIO_CONTROL_CALEFACTOR 70.0f
#define MEDSAF_TIMEOUT_CALEFACTOR_BAJA_TEMP 240000
#define MEDSAF_STEP_CALEFACTOR_BAJA_TEMP 1.0f
#define MEDSAF_TIMEOUT_CALEFACTOR_ALTA_TEMP 360000
#define MEDSAF_STEP_CALEFACTOR_ALTA_TEMP 0.5f

typedef struct
{
	uint32_t timeout;
	float step_temperatura;
} MEDSAF_CALEFACTOR_CONTROL_T;

/* Variante seleccionada para los parametros del programa P1...P6. */
typedef enum
{
	MEDSAF_MODO_ESTANDAR = 0,
	MEDSAF_MODO_ROBOT
} MEDSAF_MODO_T;

/* Programa seleccionado desde la pantalla Nextion. */
typedef enum
{
	MEDSAF_PROGRAMA_P1 = 1,
	MEDSAF_PROGRAMA_P2,
	MEDSAF_PROGRAMA_P3,
	MEDSAF_PROGRAMA_P4,
	MEDSAF_PROGRAMA_P5,
	MEDSAF_PROGRAMA_P6
} MEDSAF_PROGRAMA_T;

/* Parametros nominales de un programa. Los tiempos se expresan en SystemTicks. */
typedef struct
{
	uint32_t pre_wash_tic;
	uint16_t wash_chemical_ml;
	float wash_temperature_c;
	uint32_t degas_tic;
	uint32_t wash_tic;
	float rinse_temperature_c;
	uint32_t rinse_tic;
	float disinfection_temperature_c;
	uint32_t disinfection_tic;
} MEDSAF_MODE_CONFIG_T;

/* Tiempos compartidos, errores y limites de seguridad de la maquina. */
typedef struct
{
	uint32_t fill_timeout_tic;
	uint32_t drain_timeout_tic;
	uint32_t drain_extra_tic;
	uint32_t heating_timeout_tic;
	uint32_t disinfection_heating_timeout_tic;
	uint32_t cooling_timeout_tic;
	uint32_t heater_sample_tic;
	uint8_t heater_average_samples;
	float thermocouple_max_difference_c;
	float overtemperature_margin_c;
	float heater_control_change_temperature_c;
	uint32_t heater_low_temperature_timeout_tic;
	float heater_low_temperature_step_c;
	uint32_t heater_high_temperature_timeout_tic;
	float heater_high_temperature_step_c;
} MEDSAF_TIMEOUT_CONFIG_T;

extern const MEDSAF_MODE_CONFIG_T medsaf_p1_std;
extern const MEDSAF_MODE_CONFIG_T medsaf_p2_std;
extern const MEDSAF_MODE_CONFIG_T medsaf_p3_std;
extern const MEDSAF_MODE_CONFIG_T medsaf_p4_std;
extern const MEDSAF_MODE_CONFIG_T medsaf_p5_std;
extern const MEDSAF_MODE_CONFIG_T medsaf_p6_std;

extern const MEDSAF_MODE_CONFIG_T medsaf_p1_rbt;
extern const MEDSAF_MODE_CONFIG_T medsaf_p2_rbt;
extern const MEDSAF_MODE_CONFIG_T medsaf_p3_rbt;
extern const MEDSAF_MODE_CONFIG_T medsaf_p4_rbt;
extern const MEDSAF_MODE_CONFIG_T medsaf_p5_rbt;
extern const MEDSAF_MODE_CONFIG_T medsaf_p6_rbt;

extern const MEDSAF_TIMEOUT_CONFIG_T medsaf_timeout_config;

const MEDSAF_MODE_CONFIG_T *Medsaf_get_mode_config(MEDSAF_PROGRAMA_T medsaf_programa_seleccionado,
		MEDSAF_MODO_T medsaf_modo_seleccionado);

#endif /* INC_MEDISAFE_MODE_CONFIG_H_ */
