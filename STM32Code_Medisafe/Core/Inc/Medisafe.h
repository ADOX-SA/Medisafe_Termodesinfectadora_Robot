/*
 * Medisafe.h
 *
 * Maquina de estados supervisora de Medisafe.
 */

#ifndef INC_MEDISAFE_H_
#define INC_MEDISAFE_H_

#include <stdint.h>

/* Estados principales de la FSM Medisafe. */
typedef enum
{
	MEDSAF_INIT = 0,
	MEDSAF_PRE_WASH,
	MEDSAF_WASH,
	MEDSAF_RINSE,
	MEDSAF_FINAL_RINSE,
	MEDSAF_COOLING,
	MEDSAF_END
} MEDSAF_FSM_STATES_T;

/*
 * Nivel calculado de la bacha.
 * Los sensores deben activarse en orden: bajo, medio, alto y maximo.
 */
typedef enum
{
	MEDSAF_BACHA_EMPTY = 0,
	MEDSAF_BACHA_LOW,
	MEDSAF_BACHA_MEDIUM,
	MEDSAF_BACHA_HIGH,
	MEDSAF_BACHA_FULL,
	MEDSAF_BACHA_NIVEL_ERROR
} MEDSAF_NIVEL_BACHA_T;

extern uint8_t medsaf_fsm_enable;
extern uint8_t medsaf_fsm_paused;
extern uint8_t medsaf_error_puerta;
extern MEDSAF_NIVEL_BACHA_T medsaf_nivel_bacha;

void Medsaf_fsm_init(void);
void Medsaf_fsm_runcycle(void);
void Medsaf_actualizar_nivel_bacha(void);
MEDSAF_NIVEL_BACHA_T Medsaf_get_nivel_bacha(void);
MEDSAF_FSM_STATES_T Medsaf_fsm_get_state(void);

#endif /* INC_MEDISAFE_H_ */
