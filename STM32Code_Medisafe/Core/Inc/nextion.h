#ifndef INC_NEXTION_H_
#define INC_NEXTION_H_

#include "stm32f1xx_hal.h"
#include "stdbool.h"
#include "global_variables.h"


#define NEXTION_RX_BUF_LEN 256 //Tamaño del buffer para recibir los datos de Nextion
#define NEXTION_TX_BUF_LEN 128

#define NEXTION_QUEUE_SIZE 30
#define NEXTION_COMMAND_LENGTH 256


void nextion_tx_cplt_isr(void);
uint8_t nextion_queue_size(void);
uint8_t nextion_queue_free(void);

typedef enum {
	Lavando_NextionState,
	Preparado_NextionState,
	Pausado_NextionState,
	Error_NextionState,
	Finalizado_NextionState,
	Ocupado_NextionState
} NEXTION_STATES;

typedef enum {
	Boton1_NextionButtons, //---> Boton de arriba (Continuar, Pausar...)
	Boton2_NextionButtons //---> Boton de abajo (Finalizar, Comenzar...)
} NEXTION_MAIN_BUTTON;

typedef enum {
	Ocultar_NextionButtons,
	Mostrar_NextionButtons
} NEXTION_MAIN_BUTTON_VIS;


/**
 * Variables utilizadas
 * */
extern uint8_t nextion_rx_buffer[NEXTION_RX_BUF_LEN]; // Buffer donde se almacenan los datos.
extern uint8_t nextion_rx_available; //Si hay datos disponibles en el buffer: nextion_rx_buffer.

void Nextion_Init();
void Nextion_ChangeState(uint8_t,uint8_t);

void Nextion_RTC_Request(ENUM_REF_LAV_T);

void Nextion_Send_NivelBidon();

void Nextion_Send_OPA_ON(ENUM_REF_LAV_T);
void Nextion_Send_OPA_OFF(ENUM_REF_LAV_T);

/**
 *
 * Estas funciones sirven para mandar los datos:
 * Modo (Standard,Fast), Substate actual, temperatura y presion.
 * El primer parametro es la lavadora (1 o 2)
 *
 * */


/**
 * @brief Funcion mostrar/ocultar botones de la pantalla principal.
 * @details El botón superior es el 1 y el inferior el 2.
 * @param _lav : lavadora a colocar el error (1 o 2)
 * @param _button : número de boton que de desea modificar.
 * @param _state : ---> 0: oculto ---> 1: visible
 * @return none
 */
void Nextion_SetButton_vis(uint8_t,uint8_t,uint8_t);

/**
 *
 * Estas funciones sirven para manejar errores de "Remplazar Desinfectante":
 * Se usan siempre con la lavadora 1, por lo que no reciben parámetros.
 *
 * */
void Nextion_FinDrain();
void Nextion_FinDosing();
void Nextion_Drain_ErrorTapa();
void Nextion_Drain_Reset_ErrorTapa();
void Nextion_CleanRecBox_ErrorTapa();
void Nextion_CleanRecBox_ErrorNivel();
void Nextion_CleanRecBox_ErrorAgua();
void Nextion_CleanRecBox_Reset_ErrorAgua();
void Nextion_CleanRecBox_LavComplete();
void Nextion_CleanRecBox_n_lavado1();
void Nextion_CleanRecBox_n_lavado2();

/**
 * @brief Funcion para probar la pantalla Nextion y funciones.
 * @param none
 * @return none
 */
void Nextion_Demo();

/**
 * @brief Funcion para enviar "Esperando... Recurso compartido" a la pantalla Nextion.
 * @param _lav : lavadora a colocar el error (1 o 2)
 * @return none
 */
void Nextion_RecursoOcupado(uint16_t ,const char *);

/**
 * @brief Funcion para LIMPIAR el mensaje de recurso compartido ocupado la pantalla Nextion.
 * @param _lav : lavadora a colocar el error (1 o 2)
 * @return none
 */
void Nextion_Reset_RecursoOcupado(uint16_t);

/**
 * @brief Funcion para enviar "PUERTA ABIERTA" a la pantalla Nextion.
 * @param _lav : lavadora a colocar el error (1 o 2)
 * @return none
 */
void Nextion_send_opendoor(uint16_t);

/**
 * @brief Funcion para enviar "PUERTA CERRADA" a la pantalla Nextion.
 * @param _lav : lavadora a colocar el error (1 o 2)
 * @return none
 */
void Nextion_send_closedoor(uint16_t);

/**
 * @brief Funcion para enviar un error a la pantalla Nextion.
 * @param _lav : lavadora a colocar el error (1 o 2)
 * @param _msj : mensaje de error
 * @return none
 */
void Nextion_Error(uint16_t ,const char *);

/**
 * @brief Funcion para mostrar los botones luego de un error
 * @param _lav : lavadora donde se mostraran los botones (1 o 2)
 * @return none
 */
void Nextion_Error_MostrarBotones(uint16_t);

/**
 * @brief Funcion para ocultar los botones luego de un error
 * @param _lav : lavadora donde se mostraran los botones (1 o 2)
 * @return none
 */
void Nextion_Error_OcultarBotones(uint16_t);

/**
 * @brief Funcion para procesar la cadena en el callback de la interrupcion.
 * @param none
 * @return none
 */
void Nextion_RxCallback();


/**
 * @brief Funcion para activar la recepcion de datos con interrupcion.
 * @param none
 * @return none
 */
void Nextion_ReceiveData();


/**
 * @brief Funcion para transmitir datos con interrupcion.
 * @param _buffer : datos a enviar
 * @param _len : tamaño del vector de los datos a enviar.
 * @return none
 */
void Nextion_TransmitData(const char *,uint16_t);

/**
 * @brief Funcion para transmitir datos que hace de intermediario.
 * @param macro : es un #define que contiene el comando a enviar
 * @return none
 */

HAL_StatusTypeDef Nextion_TransmitData_IT(const char* buf, size_t len);

void Enviar_232_Nextion(const char * macro);

/**
 * @brief Funcion para limpiar el buffer de Nextion y nextion_rx_available.
 * @param none
 * @return none
 */
void Nextion_CleanRxBuffer();

/**
 * @brief Lee los datos recibidos si "nextion_rx_available" está en "1".
 * @param none
 * @return none
 */
void Nextion_Loop();
bool enqueue_nextion_command(const char*);

#endif /* INC_NEXTION_H_ */
