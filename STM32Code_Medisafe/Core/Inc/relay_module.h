/*
 * relay_module.h
 *
 *  Created on: Sep 16, 2025
 *      Author: ADOX
 */

#ifndef INC_RELAY_MODULE_H_
#define INC_RELAY_MODULE_H_

#include "global_variables.h"

#define RS485_RX_BUF_LEN 64
extern uint8_t rs485_rx_buffer[RS485_RX_BUF_LEN]; // Buffer donde se almacenan los datos.
extern uint8_t queueHead;
extern uint8_t queueTail;
void RS485_SetTxMode();
void RS485_SetRxMode();

/**
 * Variables asociadas a las entradas.
 * (rm: relay module )
 * */

extern uint8_t rm_input1[16];
extern uint8_t rm_output1[16];
extern uint8_t rm_input1_valid;

extern uint8_t rm_input2[16];
extern uint8_t rm_output2[16];

/**
 * @brief Función que procesa la respuesta rs485_rx_buffer y la guarda en rm_input1, rm_input2.
 * @param ENUM_REF_LAV_T:
 * 			REPRO1: 1
 * 			REPRO2: 2
 * @return none
 */
void RS485_SaveInputStatus(uint8_t);

/**
 * @brief Función para enviar mensajes a la placa de relés.
 * @param none
 * @return none
 */
void RS485_Queue_Loop(void);

/**
 * @brief Función para calcular crc.
 * @param *data vector con datos
 * @param length tamaño del vector
 * @return crc
 */
uint16_t calcularCRC(uint8_t *, uint16_t);

void RelayModule_WriteAllOutputs(uint8_t);


/**
 * @brief Función para configurar el estado inicial de las variables y salidas.
 * @param ENUM_REF_LAV_T:
 * 			REPRO1: 1
 * 			REPRO2: 2
 * @return none
 */
void RelayModule_Init(ENUM_REF_LAV_T);


/**
 * @brief Funcion para leer las entradas de la placa de reles.
 * @param _lav referencia a que reprocesadora se quiere actuar
 * 			REPRO1: 1
 * 			REPRO2: 2
 * @return none
 */
void RelayModule_ReadAllInputs(uint8_t);

/**
 * @brief Funcion para escribir en las salidas de la placa de reles.
 * @param ENUM_REF_LAV_T referencia a que reprocesadora se quiere actuar
 * 			REPRO1: 1
 * 			REPRO2: 2
 * @param _output Salida que se desea escribir (1:16)
 * @param _value Valor para la salida (0,1)
 * @return none
 */
void RelayModule_WriteOutput(ENUM_REF_LAV_T ,uint8_t, uint8_t);


/**
 *
 * DEFINICIONES
 *
 * */

/*------------------------------------------------------------*/
/*-----   REPROCESADORA ENDOCLEAN2000  -----------------------*/
/*------------------------------------------------------------*/

#define RM1_OUT1 rm_output1[0]
#define RM1_OUT2 rm_output1[1]
#define RM1_OUT3 rm_output1[2]
#define RM1_OUT4 rm_output1[3]
#define RM1_OUT5 rm_output1[4]
#define RM1_OUT6 rm_output1[5]
#define RM1_OUT7 rm_output1[6]
#define RM1_OUT8 rm_output1[7]
#define RM1_OUT9 rm_output1[8]
#define RM1_OUT10 rm_output1[9]
#define RM1_OUT11 rm_output1[10]
#define RM1_OUT12 rm_output1[11]
#define RM1_OUT13 rm_output1[12]
#define RM1_OUT14 rm_output1[13]
#define RM1_OUT15 rm_output1[14]
#define RM1_OUT16 rm_output1[15]

#define RM1_OUT1_ON RM1_OUT1=1;
#define RM1_OUT2_ON RM1_OUT2=1;
#define RM1_OUT3_ON RM1_OUT3=1;
#define RM1_OUT4_ON RM1_OUT4=1;
#define RM1_OUT5_ON RM1_OUT5=1;
#define RM1_OUT6_ON RM1_OUT6=1;
#define RM1_OUT7_ON RM1_OUT7=1;
#define RM1_OUT8_ON RM1_OUT8=1;
#define RM1_OUT9_ON RM1_OUT9=1;
#define RM1_OUT10_ON RM1_OUT10=1;
#define RM1_OUT11_ON RM1_OUT11=1;
#define RM1_OUT12_ON RM1_OUT12=1;
#define RM1_OUT13_ON RM1_OUT13=1;
#define RM1_OUT14_ON RM1_OUT14=1;
#define RM1_OUT15_ON RM1_OUT15=1;
#define RM1_OUT16_ON RM1_OUT16=1;

#define RM1_OUT1_OFF RM1_OUT1=0;
#define RM1_OUT2_OFF RM1_OUT2=0;
#define RM1_OUT3_OFF RM1_OUT3=0;
#define RM1_OUT4_OFF RM1_OUT4=0;
#define RM1_OUT5_OFF RM1_OUT5=0;
#define RM1_OUT6_OFF RM1_OUT6=0;
#define RM1_OUT7_OFF RM1_OUT7=0;
#define RM1_OUT8_OFF RM1_OUT8=0;
#define RM1_OUT9_OFF RM1_OUT9=0;
#define RM1_OUT10_OFF RM1_OUT10=0;
#define RM1_OUT11_OFF RM1_OUT11=0;
#define RM1_OUT12_OFF RM1_OUT12=0;
#define RM1_OUT13_OFF RM1_OUT13=0;
#define RM1_OUT14_OFF RM1_OUT14=0;
#define RM1_OUT15_OFF RM1_OUT15=0;
#define RM1_OUT16_OFF RM1_OUT16=0;

#define RM1_IN1 rm_input1[0]
#define RM1_IN2 rm_input1[1]
#define RM1_IN3 rm_input1[2]
#define RM1_IN4 rm_input1[3]
#define RM1_IN5 rm_input1[4]
#define RM1_IN6 rm_input1[5]
#define RM1_IN7 rm_input1[6]
#define RM1_IN8 rm_input1[7]
#define RM1_IN9 rm_input1[8]
#define RM1_IN10 rm_input1[9]
#define RM1_IN11 rm_input1[10]
#define RM1_IN12 rm_input1[11]
#define RM1_IN13 rm_input1[12]
#define RM1_IN14 rm_input1[13]
#define RM1_IN15 rm_input1[14]
#define RM1_IN16 rm_input1[15]

/*------------------------------------------------------------*/
/*-----   REPROCESADORA 1     --------------------------------*/
/*------------------------------------------------------------*/

//		ENTRADAS

#define L1_SENSOR_CALEFACTOR 		rm_input1[3]	   //             X4
#define L1_SENSOR_NIVEL_TAPA 		rm_input1[5]	   //             X6
#define L1_SENSOR_IMAN_TAPA 		rm_input1[13]	   //             X14
#define L1_SENSOR_OPT_DET 			rm_input1[14]	   // 			  X15
#define L1_SENSOR_OPT_ALC 			rm_input1[15]	   //             X16

//		SALIDAS

#define L1_ELV13 rm_output1[0]
#define L1_BOM02 rm_output1[1]
#define L1_BOM04 rm_output1[1]
#define L1_BOM01 rm_output1[2]
#define L1_ELV12 rm_output1[3]			// solenoide de BOMB03
#define L1_SOL09 rm_output1[5]
#define L1_BPR05 rm_output1[6]
#define L1_SOL10 rm_output1[8]
#define L1_VAL07 rm_output1[9]
#define L1_CLF11 rm_output1[10]
#define L1_BPR06 rm_output1[11]

extern uint8_t l1_val08_val;
#define L1_VAL08 l1_val08_val
//#define L1_VAL08 rm_output1[12] // 12: para el desague  y 13: para el bidon

//		ENCENDER

#define L1_ELV13_ON {if(!L1_ELV13) {L1_ELV13=1;  RelayModule_WriteOutput(REPRO1, 0, 1);}}
#define L1_BOM02_ON {if(!L1_BOM02) {L1_BOM02=1;  RelayModule_WriteOutput(REPRO1, 1, 1);}}
#define L1_BOM04_ON {if(!L1_BOM04) {L1_BOM04=1;  RelayModule_WriteOutput(REPRO1, 1, 1);}}
#define L1_BOM01_ON {if(!L1_BOM01) {L1_BOM01=1;  RelayModule_WriteOutput(REPRO1, 2, 1);}}
#define L1_ELV12_ON {if(!L1_ELV12) {L1_ELV12=1;  RelayModule_WriteOutput(REPRO1, 3, 1);}}	// rele #4 que abre el solenoide para BOMB03
#define L1_SOL09_ON {if(!L1_SOL09) {L1_SOL09=1;  RelayModule_WriteOutput(REPRO1, 5, 1);}}
#define L1_BPR05_ON {if(!L1_BPR05) {L1_BPR05=1;  RelayModule_WriteOutput(REPRO1, 6, 1);}}
#define L1_SOL10_ON {if(!L1_SOL10) {L1_SOL10=1;  RelayModule_WriteOutput(REPRO1, 8, 1);}}
#define L1_VAL07_ON {if(!L1_VAL07) {L1_VAL07=1;  RelayModule_WriteOutput(REPRO1, 9, 1);}}
#define L1_CLF11_ON {if(!L1_CLF11) {L1_CLF11=1;  RelayModule_WriteOutput(REPRO1, 10, 1);}}
#define L1_BPR06_ON {if(!L1_BPR06) {L1_BPR06=1;  RelayModule_WriteOutput(REPRO1, 11, 1);}}

#if MODEL == MODEL_SIMPLE_COMPLETA
#define L1_VAL08_ON2 {if(L1_VAL08 != 2) {L1_VAL08=2; rm_output1[12]=1; rm_output1[13]=0;}}  // Apertura al drenaje
#define L1_VAL08_ON1 {if(L1_VAL08 != 1) {L1_VAL08=1; rm_output1[12]=0; rm_output1[13]=1;}}	// apertura hacia bidon
#elif MODEL == MODEL_SIMPLE_SIN_OPA
#define L1_VAL08_ON2 {if(L1_VAL08 != 2) {L1_VAL08=2; rm_output1[12]=1; rm_output1[13]=0;}}     // VAL08 es la valula de 3 vias
#define L1_VAL08_ON1
#else
#define L1_VAL08_ON2
#define L1_VAL08_ON1
#endif


//		APAGAR
#define L1_ELV13_OFF {if(L1_ELV13) {L1_ELV13=0;  RelayModule_WriteOutput(REPRO1, 0, 0);}}
#define L1_BOM02_OFF {if(L1_BOM02) {L1_BOM02=0;  RelayModule_WriteOutput(REPRO1, 1, 0);}}
#define L1_BOM04_OFF {if(L1_BOM04) {L1_BOM04=0;  RelayModule_WriteOutput(REPRO1, 1, 0);}}
#define L1_BOM01_OFF {if(L1_BOM01) {L1_BOM01=0;  RelayModule_WriteOutput(REPRO1, 2, 0);}}
#define L1_ELV12_OFF {if(L1_ELV12) {L1_ELV12=0;  RelayModule_WriteOutput(REPRO1, 3, 0);}}	// rele #4 que abre el solenoide para BOMB03
#define L1_SOL09_OFF {if(L1_SOL09) {L1_SOL09=0;  RelayModule_WriteOutput(REPRO1, 5, 0);}}
#define L1_BPR05_OFF {if(L1_BPR05) {L1_BPR05=0;  RelayModule_WriteOutput(REPRO1, 6, 0);}}
#define L1_SOL10_OFF {if(L1_SOL10) {L1_SOL10=0;  RelayModule_WriteOutput(REPRO1, 8, 0);}}
#define L1_VAL07_OFF {if(L1_VAL07) {L1_VAL07=0;  RelayModule_WriteOutput(REPRO1, 9, 0);}}
#define L1_CLF11_OFF {if(L1_CLF11) {L1_CLF11=0;  RelayModule_WriteOutput(REPRO1, 10, 0);}}
#define L1_BPR06_OFF {if(L1_BPR06) {L1_BPR06=0;  RelayModule_WriteOutput(REPRO1, 11, 0);}}

#if MODEL == MODEL_SIMPLE_COMPLETA
#define L1_VAL08_OFF {if(L1_VAL08) {L1_VAL08=0;  rm_output1[12]=1; rm_output1[13]=1;}}     // VAL08 es la valula de 3 vias
#elif MODEL == MODEL_SIMPLE_SIN_OPA
#define L1_VAL08_OFF {if(L1_VAL08) {L1_VAL08=0;  rm_output1[12]=0; rm_output1[13]=0;}}     // VAL08 es la valula de 3 vias
#else
#define L1_VAL08_OFF
#endif


/*------------------------------------------------------------*/
/*-----   COMPARTIDO          --------------------------------*/
/*------------------------------------------------------------*/

//		ENTRADAS

#define SENSOR_NIVEL_DET  		rm_input1[6]	   //   	 	  X7		/
#define SENSOR_NIVEL_ALC  		rm_input1[7]	   // 		      X8		/
#define SENSOR_NIVEL_DESINF_1 	rm_input1[8]       // LOW lvl:    X9		/
#define SENSOR_NIVEL_DESINF_2 	rm_input1[9]       // MEDIUM lvl: X10		/
#define SENSOR_NIVEL_DESINF_3 	rm_input1[10]      // HIGH lvl:   X11		/ -------->	CABLEADOS EN EL SLAVE 1
#define SENSOR_NIVEL_DESINF_4 	rm_input1[12]      // FULL lvl:   X13		/

//		SALIDAS

#define CMP14    rm_output1[4]												//
#define BOM03    rm_output1[7]												// --------> CABLEADOS EN EL SLAVE 1

#define CMP14_ON {if(!CMP14) {CMP14=1; RelayModule_WriteOutput(REPRO1, 4, 1);}}			//
#define BOM03_ON {if(!BOM03) {BOM03=1;  RelayModule_WriteOutput(REPRO1, 7, 1);}}		// --------> CABLEADOS EN EL SLAVE 1

#define CMP14_OFF {if(CMP14) {CMP14=0; RelayModule_WriteOutput(REPRO1, 4, 0);}}			//
#define BOM03_OFF {if(BOM03) {BOM03=0;  RelayModule_WriteOutput(REPRO1, 7, 0);}}		// --------> CABLEADOS EN EL SLAVE 1

/*------------------------------------------------------------*/

#endif /* INC_RELAY_MODULE_H_ */
