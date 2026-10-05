/*
 * Medisafe_Gpio_Config.h
 *
 *  Created on: 5 oct 2026
 *      Author: ADOX
 */

#ifndef INC_MEDISAFE_GPIO_CONFIG_H_
#define INC_MEDISAFE_GPIO_CONFIG_H_

//------------------------------------------------------
//------------------------------------------------------
//-----> FUNCIONES FSM
//------------------------------------------------------
//------------------------------------------------------

void MEDSAF_fsm_off_sistema(void);
void MEDSAF_fsm_on_calefactor(void);
void MEDSAF_fsm_off_calefactor(void);
void MEDSAF_fsm_on_ultrasonido(void);
void MEDSAF_fsm_off_ultrasonido(void);
void MEDSAF_fsm_on_puerta_sole(void);
void MEDSAF_fsm_off_puerta_sole(void);
void MEDSAF_fsm_on_detergente(void);
void MEDSAF_fsm_off_detergente(void);
void MEDSAF_fsm_on_cold_water(void);
void MEDSAF_fsm_on_hot_water(void);
void MEDSAF_fsm_off_llenar_bacha(void);
void MEDSAF_fsm_on_lavado_bomba(void);
void MEDSAF_fsm_off_lavado_bomba(void);
void MEDSAF_fsm_on_drenar_bacha(void);
void MEDSAF_fsm_off_drenar_bacha(void);

//------------------------------------------------------
//------------------------------------------------------
//-----> ENTRADAS
//------------------------------------------------------
//------------------------------------------------------

#define MEDSAF_NIVEL_BACHA_BAJO RM1_IN1
#define MEDSAF_NIVEL_BACHA_MEDIO RM1_IN2
#define MEDSAF_NIVEL_BACHA_ALTO RM1_IN3
#define MEDSAF_NIVEL_BACHA_MAXIMO RM1_IN4

//------------------------------------------------------
//------------------------------------------------------
//-----> SALIDAS
//------------------------------------------------------
//------------------------------------------------------

#define MEDSAF_1 RM1_OUT1
#define MEDSAF_2 RM1_OUT2
#define MEDSAF_3 RM1_OUT3
#define MEDSAF_BOM_LUBRI RM1_OUT4 //BOMBA DE LUBRICANTE
#define MEDSAF_SOLIN_TAPA RM1_OUT5 //SOLENOIDE DE ENTRADA DE AGUA A LA TAPA
#define MEDSAF_6 RM1_OUT6 //--------------> VACIO O NO IDENTIFICADO TODAVIA
#define MEDSAF_PUERTA RM1_OUT7 //PUERTA
#define MEDSAF_SOLIN_HOT RM1_OUT8 //SOLENOIDE ENTRADA AGUA CALIENTE
#define MEDSAF_BOM_DREN RM1_OUT9 //BOMBA PARA DRENAJE
#define MEDSAF_BOM_TRIF RM1_OUT10 //CONTACTOR BOMBA TRIFASICA
#define MEDSAF_SOLIN_COLD RM1_OUT11 //SOLENOIDE ENTRADA DE AGUA FRIA
#define MEDSAF_BOM_DET RM1_OUT12 //BOMBA DETERGENTE
#define MEDSAF_SOL_DREN RM1_OUT13 //SOLENOIDE DRENAJE
#define MEDSAF_CALEFACTOR RM1_OUT14 //CALEFACTOR TRIFASICO
#define MEDSAF_ULTS_2 RM1_OUT15 //ULTRASONICO LADO IZQUIERDO
#define MEDSAF_ULTS_1 RM1_OUT16 //ULTRASONIDO LADO DERECHO

#define MEDSAF_1_ON RM1_OUT1=1;
#define MEDSAF_2_ON RM1_OUT2=1;
#define MEDSAF_3_ON RM1_OUT3=1;
#define MEDSAF_BOM_LUBRI_ON RM1_OUT4=1;
#define MEDSAF_SOLIN_TAPA_ON RM1_OUT5=1;
#define MEDSAF_6_ON RM1_OUT6=1;
#define MEDSAF_PUERTA_ON RM1_OUT7=1;
#define MEDSAF_SOLIN_HOT_ON RM1_OUT8=1;
#define MEDSAF_BOM_DREN_ON RM1_OUT9=1;
#define MEDSAF_BOM_TRIF_ON RM1_OUT10=1;
#define MEDSAF_SOLIN_COLD_ON RM1_OUT11=1;
#define MEDSAF_BOM_DET_ON RM1_OUT12=1;
#define MEDSAF_SOL_DREN_ON RM1_OUT13=1;
#define MEDSAF_CALEFACTOR_ON RM1_OUT14=1;
#define MEDSAF_ULTS_2_ON RM1_OUT15=1;
#define MEDSAF_ULTS_1_ON RM1_OUT16=1;

#define MEDSAF_1_OFF RM1_OUT1=0;
#define MEDSAF_2_OFF RM1_OUT2=0;
#define MEDSAF_3_OFF RM1_OUT3=0;
#define MEDSAF_BOM_LUBRI_OFF RM1_OUT4=0;
#define MEDSAF_SOLIN_TAPA_OFF RM1_OUT5=0;
#define MEDSAF_6_OFF RM1_OUT6=0;
#define MEDSAF_PUERTA_OFF RM1_OUT7=0;
#define MEDSAF_SOLIN_HOT_OFF RM1_OUT8=0;
#define MEDSAF_BOM_DREN_OFF RM1_OUT9=0;
#define MEDSAF_BOM_TRIF_OFF RM1_OUT10=0;
#define MEDSAF_SOLIN_COLD_OFF RM1_OUT11=0;
#define MEDSAF_BOM_DET_OFF RM1_OUT12=0;
#define MEDSAF_SOL_DREN_OFF RM1_OUT13=0;
#define MEDSAF_CALEFACTOR_OFF RM1_OUT14=0;
#define MEDSAF_ULTS_2_OFF RM1_OUT15=0;
#define MEDSAF_ULTS_1_OFF RM1_OUT16=0;


#endif /* INC_MEDISAFE_GPIO_CONFIG_H_ */
