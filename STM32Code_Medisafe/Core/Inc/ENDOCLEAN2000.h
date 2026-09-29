#ifndef INC_ENDOCLEAN2000_H_
#define INC_ENDOCLEAN2000_H_

#include <stdint.h>

/* Tiempo usado por un proceso individual cuando su tiempo configurado es cero, en ms. */
#define EC2000_TIME_PROCESO_INDIVIDUAL 10000

/* Tiempo de funcionamiento del compresor para la pistola de aire, en ms. */
#define EC2000_TIME_PISTOLA_AIRE 25000

/* Tiempo maximo del drenaje manual, en ms. */
#define EC2000_TIME_DRENAJE_MANUAL 60000

/* Tiempo de prueba con la bacha llena y la bomba de estanqueidad encendida, en ms. */
#define EC2000_TIME_ESTANQUEIDAD 25000

/* Tiempo de activacion del solenoide de puerta al presionar el pedal, en ms. */
#define EC2000_TIME_PUERTA_PEDAL 2000

/* Tiempo maximo desde el inicio del llenado hasta detectar nivel bajo. */
#define TIMEOUT_LLENADO_1 300000

/* Tiempo maximo desde nivel bajo hasta detectar nivel medio. */
#define TIMEOUT_LLENADO_2 300000

/* Tiempo entre cambios de estado del buzzer durante un error. */
#define TIME_BUZZER_ERROR 1000

/* Duracion de cada pulso y pausa del buzzer al finalizar, en ms. */
#define TIME_BUZZER_FIN 2000

/* Periodo de envio del estado y subestado a la pantalla Nextion, en ms. */
#define TIME_SENDSTATE 1000

/* Retardos de las rutinas de mantenimiento del desinfectante, en ms. */
#define EC2000_TIME_MANTENIMIENTO_INICIO 3000
#define EC2000_TIME_LAVADO_BIDON 5000
#define EC2000_TIME_ENTRE_LAVADOS_BIDON 7000

/* Tiempo adicional de drenaje luego de detectar la bacha vacia, en ms. */
#define EC2000_TIME_POST_DRENAJE_BACHA 20000

/* Drenaje preventivo al iniciar desinfeccion con la bacha ya vacia, en ms. */
#define EC2000_TIME_DRENAJE_INICIAL_DESINFECTANTE 5000


extern uint8_t EC2000_lavado_init;
extern uint8_t EC2000_detergente_init;
extern uint8_t EC2000_aire_init;
extern uint8_t EC2000_desinfectante_init;
extern uint8_t EC2000_completo_init;
extern uint8_t EC2000_alcohol_init;
extern uint8_t EC2000_fsm_enable;
extern uint8_t EC2000_fsm_paused;

#define EC2000_LAVADO_INIT { \
	EC2000_lavado_init = 1; \
	EC2000_detergente_init = 0; \
	EC2000_aire_init = 0; \
	EC2000_desinfectante_init = 0; \
	EC2000_completo_init = 0; \
	EC2000_alcohol_init = 0; \
}

#define EC2000_DETERGENTE_INIT { \
	EC2000_lavado_init = 0; \
	EC2000_detergente_init = 1; \
	EC2000_aire_init = 0; \
	EC2000_desinfectante_init = 0; \
	EC2000_completo_init = 0; \
	EC2000_alcohol_init = 0; \
}

#define EC2000_AIRE_INIT { \
	EC2000_lavado_init = 0; \
	EC2000_detergente_init = 0; \
	EC2000_aire_init = 1; \
	EC2000_desinfectante_init = 0; \
	EC2000_completo_init = 0; \
	EC2000_alcohol_init = 0; \
}

#define EC2000_DESINFECTANTE_INIT { \
	EC2000_lavado_init = 0; \
	EC2000_detergente_init = 0; \
	EC2000_aire_init = 0; \
	EC2000_desinfectante_init = 1; \
	EC2000_completo_init = 0; \
	EC2000_alcohol_init = 0; \
}

#define EC2000_COMPLETO_INIT { \
	EC2000_lavado_init = 0; \
	EC2000_detergente_init = 0; \
	EC2000_aire_init = 0; \
	EC2000_desinfectante_init = 0; \
	EC2000_completo_init = 1; \
	EC2000_alcohol_init = 0; \
}

#define EC2000_ALCOHOL_INIT { \
	EC2000_lavado_init = 0; \
	EC2000_detergente_init = 0; \
	EC2000_aire_init = 0; \
	EC2000_desinfectante_init = 0; \
	EC2000_completo_init = 0; \
	EC2000_alcohol_init = 1; \
}

void EC2000_fsm_init(void);
void EC2000_fsm_runcycle(void);
void EC2000_sendstate_runcycle(void);
void EC2000_Nextion_Send_NivelBidon(void);
void EC2000_Nextion_Send_NivelBacha(void);
void EC2000_fsm_pause(void);
void EC2000_fsm_continue(void);
void EC2000_fsm_finish(void);
void EC2000_fsm_pistola_aire_start(void);
void EC2000_fsm_pistola_aire_stop(void);
void EC2000_fsm_drenaje_manual_start(void);
void EC2000_fsm_drenaje_manual_stop(void);
void EC2000_fsm_estanq_start(void);
void EC2000_mantenimiento_stop(void);

void EC2000_fsm_lavado_init(void);
void EC2000_fsm_lavado_start(void);
void EC2000_fsm_lavado_stop(void);
void EC2000_fsm_lavado_runcycle(void);

void EC2000_fsm_detergente_init(void);
void EC2000_fsm_detergente_runcycle(void);

void EC2000_fsm_aire_init(void);
void EC2000_fsm_aire_runcycle(void);

void EC2000_fsm_desinfectante_init(void);
void EC2000_fsm_desinfectante_runcycle(void);

void EC2000_fsm_alcohol_init(void);
void EC2000_fsm_alcohol_runcycle(void);

void EC2000_fsm_completo_init(void);
void EC2000_fsm_completo_runcycle(void);

/* Timer descendente de la FSM, expresado en milisegundos. */
extern uint32_t EC2000_lavado_tic;
extern uint32_t EC2000_llenado_tic;
extern uint32_t EC2000_buzzer_tic;
extern uint32_t EC2000_aire_tic;
extern uint32_t EC2000_pistola_aire_tic;
extern uint32_t EC2000_drenaje_manual_tic;
extern uint32_t EC2000_estanq_tic;
extern uint32_t EC2000_puerta_pedal_tic;
extern uint32_t EC2000_detergente_tic;
extern uint32_t EC2000_desinfectante_tic;
extern uint32_t EC2000_alcohol_tic;
extern uint32_t EC2000_mantenimiento_tic;
extern uint32_t EC2000_mantenimiento_timeout_tic;
extern uint32_t EC2000_vaciado_bacha_tic;
extern uint32_t EC2000_sendstate_tic;

#endif /* INC_ENDOCLEAN2000_H_ */
