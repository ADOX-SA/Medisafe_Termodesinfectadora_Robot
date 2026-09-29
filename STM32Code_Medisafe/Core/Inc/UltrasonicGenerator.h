/*
 * UltrasonicGenerator.h
 *
 *  Created on: 23 jul 2026
 *      Author: ADOX
 */

#ifndef INC_ULTRASONICGENERATOR_H_
#define INC_ULTRASONICGENERATOR_H_

#define ULTSON_RS485_DIR 0x07

#define ULTSON_STATE_OFF 0
#define ULTSON_STATE_ON 1
#define ULTSON_ENABLE_VALUE 1
#define ULTSON_DISABLE_VALUE 0

extern uint8_t ultson_state; //---> ENCENDIDO / APAGADO
extern uint8_t ultson_enable; //---> Utilizada en el ciclo de lavado de la fsm.
extern uint8_t ultson_time_min;
extern uint8_t ultson_time_sec;
extern uint8_t ultson_power;
extern uint8_t ultson_current;
extern uint8_t ultson_frecuency;

void UltSon_Init();

void UltSon_SendData();

void UltSon_LeerEstado();
void UltSon_LeerTiempo();
void UltSon_LeerPotencia();
void UltSon_LeerCorrienteMedida();
void UltSon_LeerFrecuencia();
void UltSon_Encender();
void UltSon_Apagar();
void UltSon_ConfigTiempo(uint8_t,uint8_t); // minutos - segundos
void UltSon_ConfigPotencia(uint8_t); // valor entre 1-20

#endif /* INC_ULTRASONICGENERATOR_H_ */
