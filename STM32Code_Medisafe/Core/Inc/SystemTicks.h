/*
 * SystemTicks.h
 *
 *  Created on: Oct 8, 2025
 *      Author: ADOX
 */

#ifndef INC_SYSTEMTICKS_H_
#define INC_SYSTEMTICKS_H_

#include "main.h"
#include "stdbool.h"

/**
 *
 * Variables
 * agrego + "_tic"  -> valor del contador en [ms].
 * sufijo + "_tic_en" -> bool que habilita el contador.
 *
 * Por ejemplo para un buzzer:
 * buzzer_tic -> contador descendente para el buzzer en [ms] (uint16_t)
 * buzzer_tic_en -> true: timer habilitado, false: deshabilitado (bool).
 *
 * */

extern uint16_t example_tic;
extern bool example_tic_en;

extern uint16_t rs485_readinputs_tic;
extern bool rs485_readinputs_tic_en;

extern uint16_t rs485_queue_tic;
extern bool rs485_queue_tic_en;

extern uint16_t rs485_timeout_tic;
extern bool rs485_timeout_tic_en;

extern uint16_t nextion_demo_tic;
extern bool nextion_demo_tic_en;

extern uint16_t rfid_wiegand_tic;
extern bool rfid_wiegand_en;

extern uint16_t ntc_adc_tic;
extern bool ntc_adc_tic_en;

extern uint16_t nextion_tic_1seg;
extern uint16_t nextion_tic_2seg;
extern uint16_t nextion_tic_3seg;
extern uint16_t nextion_tic_4seg;
extern uint16_t nextion_tic_5seg;

void SystemTicks_Init();
void SystemTicks_1ms();

#endif /* INC_SYSTEMTICKS_H_ */
