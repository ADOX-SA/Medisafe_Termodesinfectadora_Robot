/*
 * Medisafe.h
 *
 * Maquina de estados supervisora de Medisafe.
 */

#ifndef INC_MEDISAFE_H_
#define INC_MEDISAFE_H_

#include <stdint.h>

/* Estados principales de la FSM. Se agregaran nuevos procesos sobre esta base. */
typedef enum
{
	Medsaf_FSM_IDLE = 0
} Medsaf_FSM_STATES_T;

/*
 * Nivel calculado de la bacha.
 * Los sensores deben activarse en orden: bajo, medio, alto y maximo.
 */
typedef enum
{
	Medsaf_BACHA_EMPTY = 0,
	Medsaf_BACHA_LOW,
	Medsaf_BACHA_MEDIUM,
	Medsaf_BACHA_HIGH,
	Medsaf_BACHA_FULL,
	Medsaf_BACHA_NIVEL_ERROR
} Medsaf_NIVEL_BACHA_T;

extern uint8_t Medsaf_fsm_enable;
extern uint8_t Medsaf_fsm_paused;
extern Medsaf_NIVEL_BACHA_T Medsaf_nivel_bacha;

void Medsaf_fsm_init(void);
void Medsaf_actualizar_nivel_bacha(void);
Medsaf_NIVEL_BACHA_T Medsaf_get_nivel_bacha(void);

#endif /* INC_MEDISAFE_H_ */
