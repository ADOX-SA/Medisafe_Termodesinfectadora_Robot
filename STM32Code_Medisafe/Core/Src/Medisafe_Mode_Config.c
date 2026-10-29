/*
 * Medisafe_Mode_Config.c
 *
 * Tabla de programas tomada del manual Medisafe SI PCF.
 */

#include "Medisafe_Mode_Config.h"

#define MEDSAF_MINUTES_TO_TIC(minutes) ((uint32_t)(minutes) * 60000)

const MEDSAF_MODE_CONFIG_T medsaf_p1_std =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(5), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 0.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p2_std =
{
	0, 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3), MEDSAF_MINUTES_TO_TIC(10),
	60.0f, MEDSAF_MINUTES_TO_TIC(2), 0.0f, 0
};

const MEDSAF_MODE_CONFIG_T medsaf_p3_std =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(20), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 0.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p4_std =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(5), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 90.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p5_std =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(10), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 90.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p6_std =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(20), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 90.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p1_rbt =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(5), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 0.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p2_rbt =
{
	0, 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3), MEDSAF_MINUTES_TO_TIC(10),
	60.0f, MEDSAF_MINUTES_TO_TIC(2), 0.0f, 0
};

const MEDSAF_MODE_CONFIG_T medsaf_p3_rbt =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(20), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 0.0f,
	MEDSAF_MINUTES_TO_TIC(1)
};

const MEDSAF_MODE_CONFIG_T medsaf_p4_rbt =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(5), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 91.0f,
	MEDSAF_MINUTES_TO_TIC(3)
};

const MEDSAF_MODE_CONFIG_T medsaf_p5_rbt =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(10), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 91.0f,
	MEDSAF_MINUTES_TO_TIC(3)
};

const MEDSAF_MODE_CONFIG_T medsaf_p6_rbt =
{
	MEDSAF_MINUTES_TO_TIC(3), 360, 40.0f, MEDSAF_MINUTES_TO_TIC(3),
	MEDSAF_MINUTES_TO_TIC(20), 60.0f, MEDSAF_MINUTES_TO_TIC(2), 91.0f,
	MEDSAF_MINUTES_TO_TIC(3)
};

const MEDSAF_TIMEOUT_CONFIG_T medsaf_timeout_config =
{
	300000,  /* fill_timeout_tic */
	300000,  /* drain_timeout_tic */
	30000,   /* drain_extra_tic */
	600000,  /* heating_timeout_tic */
	1800000, /* disinfection_heating_timeout_tic */
	600000,  /* cooling_timeout_tic */
	1000,    /* heater_sample_tic */
	5,       /* heater_average_samples */
	5.0f,    /* thermocouple_max_difference_c */
	5.0f,    /* overtemperature_margin_c */
	70.0f,   /* heater_control_change_temperature_c */
	240000,  /* heater_low_temperature_timeout_tic */
	1.0f,    /* heater_low_temperature_step_c */
	360000,  /* heater_high_temperature_timeout_tic */
	0.5f     /* heater_high_temperature_step_c */
};

const MEDSAF_MODE_CONFIG_T *Medsaf_get_mode_config(MEDSAF_PROGRAMA_T medsaf_programa_seleccionado,
		MEDSAF_MODO_T medsaf_modo_seleccionado)
{
	static const MEDSAF_MODE_CONFIG_T *const medsaf_std_modes[] =
	{
		&medsaf_p1_std, &medsaf_p2_std, &medsaf_p3_std,
		&medsaf_p4_std, &medsaf_p5_std, &medsaf_p6_std
	};
	static const MEDSAF_MODE_CONFIG_T *const medsaf_rbt_modes[] =
	{
		&medsaf_p1_rbt, &medsaf_p2_rbt, &medsaf_p3_rbt,
		&medsaf_p4_rbt, &medsaf_p5_rbt, &medsaf_p6_rbt
	};
	uint8_t medsaf_mode_index;

	if ((medsaf_programa_seleccionado < MEDSAF_PROGRAMA_P1) ||
		(medsaf_programa_seleccionado > MEDSAF_PROGRAMA_P6))
	{
		return 0;
	}

	medsaf_mode_index = (uint8_t)medsaf_programa_seleccionado - 1;
	if (medsaf_modo_seleccionado == MEDSAF_MODO_ROBOT)
	{
		return medsaf_rbt_modes[medsaf_mode_index];
	}

	return medsaf_std_modes[medsaf_mode_index];
}
