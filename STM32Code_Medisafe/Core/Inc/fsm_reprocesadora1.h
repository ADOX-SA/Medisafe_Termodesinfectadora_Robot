/*
 * fsm_reprocesadora1.h
 *
 *  Created on: Nov 25, 2025
 *      Author: momo
 */

#ifndef INC_FSM_REPROCESADORA1_H_
#define INC_FSM_REPROCESADORA1_H_

#include "stm32f1xx_hal.h"
#include <stdbool.h>

extern bool L1_botonIdle, L1_botonSTD_Mode_1, L1_botonContinuar, L1_botonVolver,
			L1_botonDRS, L1_botonDOS, L1_botonCRB, L1_botonAceptar, L1_botonStop;

void fsm_reprocesadora1_init(void);
void fsm_reprocesadora1_runCycle(void);
void fsm_stop1_runCycle(void);
void fsm1_recirculado(void);

void fsm_reprocesadora1_tick(void);
void fsm_detergente1_tick(void);

void fsm1_raise_ev(const char *);
void fsm1_delay(int , int , uint32_t);
void fsm1_delay_service(void);

void fsm1_change_state(const char *);

//----> Funciones para preguntar el numero de estado:
uint8_t L1_FSM_Get_State();
uint8_t L1_FSM_Get_SubState();
//uint8_t L2_FSM_Get_State();
//uint8_t L2_FSM_Get_SubState();

#endif /* INC_FSM_REPROCESADORA1_H_ */
