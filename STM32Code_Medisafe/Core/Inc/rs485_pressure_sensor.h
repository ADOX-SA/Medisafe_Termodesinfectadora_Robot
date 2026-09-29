/*
 * rs485_pressure_sensor.h
 *
 *  Created on: Dec 17, 2025
 *      Author: ADOX
 */

#ifndef INC_RS485_PRESSURE_SENSOR_H_
#define INC_RS485_PRESSURE_SENSOR_H_

#include "stdbool.h"

extern bool rs485_read_inputs_fsm;
extern uint8_t rs485_rx_available;

void RS485_Init();
void RS485_Loop();

#endif /* INC_RS485_PRESSURE_SENSOR_H_ */
