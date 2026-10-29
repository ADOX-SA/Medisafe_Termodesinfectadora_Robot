#include "main.h"
#include "nextion.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#include "relay_module.h"
#include "service.h"
#include "SystemTicks.h"
#include "fsm_reprocesadora1.h"
#include "global_variables.h"
#include "user_gpio.h"
#include "i2c_pressure.h"
#include "ntc_adc.h"
#include "UltrasonicGenerator.h"
#include "ticket.h"
#include "ENDOCLEAN2000.h"
#include "Medisafe.h"
#define TX_TIMER 100

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
//extern TIM_HandleTypeDef htim3;
uint16_t L1a_i,L1b_i,L2a_i,L2b_i;

/**
 * Funciones locales de nextion.c
 */

void ConvertFloatToString(char *_str, float _float_received, uint8_t _resolution)
{
	char int_str[20];
	char decimal_str[20];
	uint16_t integer;
	uint16_t decimal;
	integer = (int)_float_received;
	decimal = ((float)(_float_received - integer)) * (10*_resolution);
	sprintf(int_str,"%d",(int)integer);
	sprintf(decimal_str,"%d",(int)decimal);

	//---> Concateno todos los Strings
	strcat(_str,int_str);
	strcat(_str,".");
	strcat(_str,decimal_str);

}

uint16_t ConvertStringToInt(const char *_str,size_t _len)
{
	uint16_t value=0;
	size_t i=0;
	for(i=0;i<_len;i++)
	{
		if(_str[i]<'0'||_str[i]>'9')
			break;
		value=(uint16_t)(value*10u+(uint16_t)(_str[i]-'0'));
	}
	return value;
}

uint8_t ExtraerString(const uint8_t * _received_data,const char * _tag, char * _target_str,size_t _target_max_len)
{
	char *result1;
	char *result2;
	size_t len;

	result1=strstr((char*)_received_data,_tag);
	if(result1)
	{
		result1 = result1 + strlen(_tag);
		result2 = strchr(result1,',');
		if(!result2)
			result2 = result1 + strlen(result1);

		len=(size_t)(result2-result1);
		if(len >= _target_max_len)
			len=_target_max_len-1;
		strncpy(_target_str,result1,len);
		_target_str[len]='\0';
		return (int)len;
	}
	return -1;
}

void Nextion_commands(const uint8_t *);

static void Medsaf_nextion_iniciar_programa(void)
{
	char medsaf_buffer[50];

	Medsaf_fsm_init();
	enqueue_nextion_command("page Main");
	if (!medsaf_fsm_enable)
		return;

	enqueue_nextion_command("data.L1_mensaje.txt=\"\"");
	Nextion_ChangeState(REPRO1, Lavando_NextionState);
	Nextion_SetButton_vis(REPRO1, Boton1_NextionButtons, Ocultar_NextionButtons);
	Nextion_SetButton_vis(REPRO1, Boton2_NextionButtons, Mostrar_NextionButtons);
	snprintf(medsaf_buffer, sizeof(medsaf_buffer),
			 "data.L1_on_modo.txt=\"P%d %s\"",
			 (int)medsaf_programa,
			 (medsaf_modo == MEDSAF_MODO_ROBOT) ? "Robot" : "Estandar");
	enqueue_nextion_command(medsaf_buffer);
}

static uint8_t Medsaf_nextion_parse_start(const uint8_t *medsaf_received_data)
{
	const char *medsaf_comando = (const char *)medsaf_received_data;

	if (strcmp(medsaf_comando, "start:p1_std") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P1;
		medsaf_modo = MEDSAF_MODO_ESTANDAR;
	}
	else if (strcmp(medsaf_comando, "start:p2_std") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P2;
		medsaf_modo = MEDSAF_MODO_ESTANDAR;
	}
	else if (strcmp(medsaf_comando, "start:p3_std") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P3;
		medsaf_modo = MEDSAF_MODO_ESTANDAR;
	}
	else if (strcmp(medsaf_comando, "start:p4_std") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P4;
		medsaf_modo = MEDSAF_MODO_ESTANDAR;
	}
	else if (strcmp(medsaf_comando, "start:p5_std") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P5;
		medsaf_modo = MEDSAF_MODO_ESTANDAR;
	}
	else if (strcmp(medsaf_comando, "start:p6_std") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P6;
		medsaf_modo = MEDSAF_MODO_ESTANDAR;
	}
	else if (strcmp(medsaf_comando, "start:p1_rbt") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P1;
		medsaf_modo = MEDSAF_MODO_ROBOT;
	}
	else if (strcmp(medsaf_comando, "start:p2_rbt") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P2;
		medsaf_modo = MEDSAF_MODO_ROBOT;
	}
	else if (strcmp(medsaf_comando, "start:p3_rbt") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P3;
		medsaf_modo = MEDSAF_MODO_ROBOT;
	}
	else if (strcmp(medsaf_comando, "start:p4_rbt") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P4;
		medsaf_modo = MEDSAF_MODO_ROBOT;
	}
	else if (strcmp(medsaf_comando, "start:p5_rbt") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P5;
		medsaf_modo = MEDSAF_MODO_ROBOT;
	}
	else if (strcmp(medsaf_comando, "start:p6_rbt") == 0)
	{
		medsaf_programa = MEDSAF_PROGRAMA_P6;
		medsaf_modo = MEDSAF_MODO_ROBOT;
	}
	else
	{
		return 0;
	}

	return 1;
}

/**
 * Variables utilizadas:
 */
uint8_t nextion_rx_buffer[NEXTION_RX_BUF_LEN] = {0}; // Buffer donde se almacenan los datos.
uint8_t nextion_rx_available = 0; //Si hay datos disponibles en el buffer: nextion_rx_buffer.
bool nextion_rx_receiving = false;      // Flag para indicar si estamos recibiendo una cadena
uint8_t nextion_rx_index = 0;        // Índice para almacenar caracteres en el buffer
uint8_t nextion_rx_byte; //Aca se guarda el byte recibido por serie.

char nextion_queue[NEXTION_QUEUE_SIZE][NEXTION_COMMAND_LENGTH];
uint8_t nextion_queue_head = 0;
uint8_t nextion_queue_tail = 0;
bool nextion_busy = false;
uint32_t last_nextion_tick = 0;

static uint8_t nextion_tx_buf[NEXTION_TX_BUF_LEN];
static volatile uint8_t nextion_tx_busy = 0;

bool accionado = 0;
extern bool flagAccionar_Air, flagAccionarMez, flagAccionarAg, flagAccionarPeris, flagAccionarAlc;

extern int16_t prev_air, prev_det, prev_agua, prev_pedal, prev_lvl, prev_caudal, prev_sole_air, prev_sole_mez, prev_sole_ag, prev_bomb_peris, prev_bomb_alc;
extern int8_t peris_prev;

static volatile uint8_t rtc_req_pending = 0;
ENUM_REF_LAV_T rtc_lav;

void Nextion_RxCallback(){
	if (nextion_rx_byte == '<') {
		// Comienza una nueva cadena
		nextion_rx_receiving = true;
		nextion_rx_index  = 0;  // Reinicia el índice
		memset(nextion_rx_buffer, 0, NEXTION_RX_BUF_LEN);  // Limpia el buffer
	} else if (nextion_rx_byte == '>' && nextion_rx_receiving ) {
		// Termina la cadena
		nextion_rx_receiving = false;
		nextion_rx_buffer[nextion_rx_index ] = '\0';
		nextion_rx_available =1;
	} else if (nextion_rx_receiving ) {
		// Agrega el carácter al buffer si está dentro de la cadena
		if (nextion_rx_index  < NEXTION_RX_BUF_LEN - 1) {  // Evita desbordamiento
			nextion_rx_buffer[nextion_rx_index++] = nextion_rx_byte;
		}
	}
	HAL_UART_Receive_IT(NEXTION_PORT, &nextion_rx_byte, 1);
}


void Nextion_ReceiveData()
{
	HAL_UART_Receive_IT(NEXTION_PORT, &nextion_rx_byte, 1);
}

// ---> cola: NO avances tail aquí; se avanza en el callback
static inline size_t safe_cmd_len(const char* s) {
	// por si acaso, limita a tu tamaño de slot
	size_t n = strnlen(s, NEXTION_COMMAND_LENGTH);
	return n;
}

uint8_t nextion_queue_size(void) {
	return (nextion_queue_head + NEXTION_QUEUE_SIZE - nextion_queue_tail) % NEXTION_QUEUE_SIZE;
}
uint8_t nextion_queue_free(void) {
	return (NEXTION_QUEUE_SIZE - 1) - nextion_queue_size(); // 1 hueco reservado
}


HAL_StatusTypeDef Nextion_TransmitData_IT(const char* buf, size_t len)
{
	if (nextion_tx_busy) return HAL_BUSY;

	if (len + 3 > NEXTION_TX_BUF_LEN)
		len = NEXTION_TX_BUF_LEN - 3; // clamp

	memcpy(nextion_tx_buf, buf, len);
	nextion_tx_buf[len]   = 0xFF;
	nextion_tx_buf[len+1] = 0xFF;
	nextion_tx_buf[len+2] = 0xFF;

	nextion_tx_busy = 1;

	return HAL_UART_Transmit_IT(NEXTION_PORT, nextion_tx_buf, len + 3);
}

void Nextion_Init()
{
	HAL_Delay(250);
	enqueue_nextion_command(MODEL_NAME);
	enqueue_nextion_command("page Login");
	enqueue_nextion_command("data.L1_status.txt=\"Preparado\"");
}

void Enviar_232_Nextion(const char *cmd)
{
	size_t len = safe_cmd_len(cmd);
	(void)Nextion_TransmitData_IT(cmd, len);
}

void nextion_tx_cplt_isr(void)
{
	nextion_queue_tail = (nextion_queue_tail + 1) % NEXTION_QUEUE_SIZE;
	nextion_tx_busy = 0;
}

void Nextion_CleanRxBuffer()
{
	memset(nextion_rx_buffer, 0, sizeof(nextion_rx_buffer));
	nextion_rx_available = 0;
}

bool enqueue_nextion_command(const char* cmd)
{
	uint8_t next_head = (nextion_queue_head + 1) % NEXTION_QUEUE_SIZE;
	bool ok = false;

	if (next_head != nextion_queue_tail)
	{
		strncpy(nextion_queue[nextion_queue_head], cmd, NEXTION_COMMAND_LENGTH - 1);
		nextion_queue[nextion_queue_head][NEXTION_COMMAND_LENGTH - 1] = '\0';
		nextion_queue_head = next_head;
		ok = true;
	}

	return ok;
}


void Nextion_Loop()
{
	if (nextion_rx_available)
	{
		Nextion_commands(nextion_rx_buffer);

		//---> Filtro comandos de la pagina de servicio.
		Service_Diagram(nextion_rx_buffer);
		Service_Buttons(nextion_rx_buffer);

		//----> Se limpia el buffer:
		Nextion_CleanRxBuffer();
	}

	if (nextion_queue_tail != nextion_queue_head &&
			!nextion_tx_busy &&
			( HAL_GetTick() - last_nextion_tick) >= TX_TIMER )
	{
		const char* cmd = nextion_queue[nextion_queue_tail];

		if (Nextion_TransmitData_IT(cmd, safe_cmd_len(cmd)) == HAL_OK)
		{
			// NO mover tail aquí. Se mueve en el callback al finalizar el TX.
			last_nextion_tick = HAL_GetTick();
		}
		// si devolvió BUSY/ERROR, probaremos en el siguiente tick sin perder el elemento
	}
}

void Nextion_send_opendoor(uint16_t _lav)
{
	if(_lav==1)
	{
		enqueue_nextion_command("data.L1_puerta.txt=\"open\"");
	}
}

void Nextion_send_closedoor(uint16_t _lav)
{
	if(_lav==1)
	{
		enqueue_nextion_command("data.L1_puerta.txt=\"close\"");
	}
}

void Nextion_commands(const uint8_t *_received_data)
{
	/************************************************************************/
	/*					REPROCESADORA 1										*/
	/************************************************************************/

	if((strstr((char *)_received_data,(char *)"L1a") != NULL) || ((strstr((char *)_received_data,(char *)"WHE1") != NULL))
			|| ((strstr((char *)_received_data,(char *)"L2a") != NULL)) || ((strstr((char *)_received_data,(char *)"WHE2") != NULL)))
	{
		/*
		 char *strstr(const char *string1, const char *string2);

		 La función strstr() devuelve un puntero al principio de la
		 primera aparición de string2 en string1. Si string2 no aparece en string1,
		 la función strstr() devuelve NULL. Si string2 apunta a una cadena con longitud cero, la función strstr() devuelve string1.

		 */
		char str_aux[16];
		uint8_t length;

		//----------------------------
		//---> TIEMPOS LAVADORA 1
		//----------------------------
		if(strstr((char *)_received_data,(char *)"L1a") != NULL)
		{

			length = ExtraerString(_received_data,"L1a:",str_aux,sizeof(str_aux)); // Prueba Estanqueidad
			L1_estanq_en = ConvertStringToInt(str_aux,length);

			length = ExtraerString(_received_data,"L1b:",str_aux,sizeof(str_aux)); // Prueba de Bloqueo
			L1_lock_test = ConvertStringToInt(str_aux,length); // sin uso, queda preparada para estanqueidad o chequeo de presiones

			length = ExtraerString(_received_data,"L1c:",str_aux,sizeof(str_aux)); // Enjuagado Inicial
			L1_standard_times[0] = ConvertStringToInt(str_aux,length) * 1000;

			length = ExtraerString(_received_data,"L1d:",str_aux,sizeof(str_aux)); // Lavado
			L1_standard_times[1] = ConvertStringToInt(str_aux,length)* 1000;

			//L1e es la dilucion
			if(strstr((char *)_received_data,(char *)"L1e:1:200") != NULL)
			{
				L1_dilucion=28000;
			}
			else if(strstr((char *)_received_data,(char *)"L1e:1:500") != NULL)
			{
				L1_dilucion=(int)(28000 * (2/5));
			}
			else if(strstr((char *)_received_data,(char *)"L1e:1:1000") != NULL)
			{
				L1_dilucion=(int)(28000/5);
			}

			length = ExtraerString(_received_data,"L1f:",str_aux,sizeof(str_aux)); // Enjuague intermedio
			L1_standard_times[2] = ConvertStringToInt(str_aux,length)* 1000;

			length = ExtraerString(_received_data,"L1g:",str_aux,sizeof(str_aux)); // Desinfeccion
			L1_standard_times[3] = ConvertStringToInt(str_aux,length)* 1000;

			length = ExtraerString(_received_data,"L1h:",str_aux,sizeof(str_aux)); // Enjuagado final
			L1_standard_times[4] = ConvertStringToInt(str_aux,length)* 1000;

			length = ExtraerString(_received_data,"L1i:",str_aux,sizeof(str_aux)); // Secado
			L1_standard_times[6] = ConvertStringToInt(str_aux,length)* 1000;

			length = ExtraerString(_received_data,"L1j:",str_aux,sizeof(str_aux)); // Alcohol
			L1_standard_times[5] = ConvertStringToInt(str_aux,length)* 1000;

		}
		//----------------------------
		//---> CALEFACTOR 1
		//----------------------------
		if(strstr((char *)_received_data,(char *)"WHE1") != NULL)
		{

			length = ExtraerString(_received_data,"WHE1:",str_aux,sizeof(str_aux));
			L1_WASHING_HEATING = ConvertStringToInt(str_aux,length);

			length = ExtraerString(_received_data,"WHT1:",str_aux,sizeof(str_aux));
			L1_MaxTempW = ConvertStringToInt(str_aux,length);

			length = ExtraerString(_received_data,"DHE1:",str_aux,sizeof(str_aux));
			L1_DESINF_HEATING = ConvertStringToInt(str_aux,length);

			length = ExtraerString(_received_data,"DHT1:",str_aux,sizeof(str_aux));
			L1_MaxTempD = ConvertStringToInt(str_aux,length);

		}
		//----------------------------
		//---> CONFIG. SENSOR DE TEMP INICIAL
		//----------------------------
		if(strstr((char *)_received_data,(char *)"STT:EN") != NULL)
		{
			// Apago logica de calentado por tiempo
			L1_WH_TIMEOUT = 0;
			L1_DH_TIMEOUT = 0;
		}
		else if(strstr((char *)_received_data,(char *)"STT:DIS") != NULL)
		{
			// Arranco logica de calentado por tiempo: 18 min 40 seg -> 40º (medido con recirculacion continua)
			L1_WH_TIMEOUT = 1;
			L1_DH_TIMEOUT = 1;
		}
		//----------------------------
		//---> ¿Quedó opa?
		//----------------------------
		if(strstr((char *)_received_data,(char *)"OPA1:si") != NULL)
		{
			L1_opa_bacha = true;
		}
		else if(strstr((char *)_received_data,(char *)"OPA1:no") != NULL)
		{
			L1_opa_bacha = false;
		}
	}
	else if (strstr((char *)_received_data, "rtc:") != NULL)
	{
		char *p = (char *)_received_data + 4;

		if (p[4]=='-' && p[7]=='-' &&
				p[10]=='T' &&
				p[13]==':' && p[16]==':')
		{
			// FECHA → YYYY.MM.DD
			g_fecha[0]=p[0]; g_fecha[1]=p[1];
			g_fecha[2]=p[2]; g_fecha[3]=p[3];
			g_fecha[4]='.';
			g_fecha[5]=p[5]; g_fecha[6]=p[6];
			g_fecha[7]='.';
			g_fecha[8]=p[8]; g_fecha[9]=p[9];
			g_fecha[10]='\0';

			// HORA → HH:MM:SS
			g_hora[0]=p[11]; g_hora[1]=p[12];
			g_hora[2]=':';
			g_hora[3]=p[14]; g_hora[4]=p[15];
			g_hora[5]=':';
			g_hora[6]=p[17]; g_hora[7]=p[18];
			g_hora[8]='\0';
		}
		else
		{
			// Fallback robusto
			strcpy(g_fecha, "----.--.--");
			strcpy(g_hora,  "--:--:--");
		}

		// Liberar request SIEMPRE
		rtc_req_pending = 0;

		// Permitir que la cola avance SIEMPRE
		if (rtc_lav == REPRO1) L1_rtc_ready = 1;
	}
	/*	else if((strcmp((char *)_received_data,"L1_b_puerta"))==0)
	{
		//----> CODIGO PARA ABRIR PUERTA
		L1_Open_Door();
	}*/
	else if(strncmp((char *)_received_data, "start:", 6)==0)
	{
		if (Medsaf_nextion_parse_start(_received_data))
			Medsaf_nextion_iniciar_programa();
		else
			enqueue_nextion_command("data.L1_mensaje.txt=\"Programa invalido\"");
	}
	else if((strcmp((char *)_received_data,"L1_b_comenzar"))==0)
	{
		Medsaf_nextion_iniciar_programa();
	}
	else if((strcmp((char *)_received_data,"L1_b_pausar"))==0)
	{
		//----> PAUSAR: 1- pausar fsm. 2- apagar todos los relés.
		Medsaf_fsm_pause();
		if (medsaf_fsm_paused)
			Nextion_ChangeState(REPRO1, Pausado_NextionState);
	}
	else if(((strcmp((char *)_received_data,"L1_b_continuar"))==0)||
			((strcmp((char *)_received_data,"L1_b_reanudar"))==0))
	{
		//---> Si esta la fsm pausada, la reactivo:
		Medsaf_fsm_continue();
		if (medsaf_fsm_enable)
		{
			enqueue_nextion_command("data.L1_mensaje.txt=\"\"");
			Nextion_ChangeState(REPRO1, Lavando_NextionState);
		}
	}
	else if((strcmp((char *)_received_data,"L1_b_error_continuar"))==0)
	{
		L1_botonContinuar=1;
	}
	else if((strcmp((char *)_received_data,"L1_b_idle"))==0)
	{
		L1_botonIdle=1;
		enqueue_nextion_command("page EC2000_Sel");
	}
	else if((strcmp((char *)_received_data,"L1_b_finalizar"))==0)
	{
		Medsaf_fsm_finish();
	}
	else if((strcmp((char *)_received_data,"L1_sensor_temp_enable"))==0)
	{
		// Apago logica de calentado por tiempo
		L1_WH_TIMEOUT = 0;
		L1_DH_TIMEOUT = 0;
	}
	else if((strcmp((char *)_received_data,"L1_sensor_temp_disable"))==0)
	{
		// Arranco logica de calentado por tiempo: 18 min 40 seg -> 40º (medido con recirculacion continua)
		L1_WH_TIMEOUT = 1;
		L1_DH_TIMEOUT = 1;
	}
	else if((strcmp((char *)_received_data,"ignorar_sensores"))==0)
	{
		L1_ignora_sensor = true;
	}
	//------------------------------------------
	//-------------> Reemplazar desinfectante:
	//------------------------------------------
	else if((strcmp((char *)_received_data,"Drain_start"))==0)
	{
		L1_botonDRS=1;
	}
	else if((strcmp((char *)_received_data,"Drain_stop"))==0)
	{
		EC2000_mantenimiento_stop();
	}
	else if((strcmp((char *)_received_data,"Dosing_start"))==0)
	{
		L1_botonDOS=1;
	}
	else if((strcmp((char *)_received_data,"Dosing_stop"))==0)
	{
		EC2000_mantenimiento_stop();
	}
	else if((strcmp((char *)_received_data,"Dosing_aceptar"))==0)
	{
		L1_botonAceptar = 1;
	}
	else if((strcmp((char *)_received_data,"CleanRecBox_start"))==0)
	{
		L1_botonCRB=1;
	}
	else if((strcmp((char *)_received_data,"CleanRecBox_pause"))==0)
	{
		EC2000_fsm_pause();
	}
	else if((strcmp((char *)_received_data,"CleanRecBox_continue"))==0)
	{
		EC2000_fsm_continue();
	}
	else if((strcmp((char *)_received_data,"CleanRecBox_stop"))==0)
	{
		L1_botonStop=1;
	}
	else if((strcmp((char *)_received_data,"CleanRecBox_idle"))==0)
	{
		L1_botonIdle=1;
	}
	else if((strcmp((char *)_received_data,"CleanRecBox_ErrorContinue"))==0)
	{
		L1_botonContinuar=1; //para los errores!!!
	}
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	//------> GENERADOR ULTRASONIDO
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	else if((strcmp((char *)_received_data,"UltSon_Encender"))==0)
	{
		UltSon_Encender();
	}
	else if((strcmp((char *)_received_data,"UltSon_Apagar"))==0)
	{
		UltSon_Apagar();
	}
	else if((strcmp((char *)_received_data,"UltSon_W_ENABLE"))==0)
	{
		ultson_enable = ULTSON_ENABLE_VALUE;
	}
	else if((strcmp((char *)_received_data,"UltSon_W_DISABLE"))==0)
	{
		ultson_enable = ULTSON_DISABLE_VALUE;
	}
	else if(strstr((char *)_received_data,(char *)"UltSon_time") != NULL)
	{
		char str_aux[16];
		uint8_t length;

		length = ExtraerString(_received_data,"UltSon_time:",str_aux,sizeof(str_aux));
		ultson_time_min = ConvertStringToInt(str_aux,length);
	}
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	//------> IMPRESORA
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	else if(strstr((char *)_received_data,(char *)"test_impresora") != NULL)
	{
		//Funciones para imprimir una prueba
		Ticket_EnqEventCached(REPRO1, "PRUEBA IMPRESORA TTL");
		Ticket_EnqFinish(REPRO1, "OK", "PRUEBA");
		Ticket_EnqPrint(REPRO1);
	}
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	//------> ENDOCLEAN2000
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	else if((strcmp((char *)_received_data,"EC2000_lavado_init"))==0)
	{
		EC2000_fsm_init();
		EC2000_LAVADO_INIT;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		enqueue_nextion_command("page EC2000_Main");
		enqueue_nextion_command("data.L1_on_modo.txt=\"Lavado\"");
	}
	else if((strcmp((char *)_received_data,"EC2000_detergente_init"))==0)
	{
		EC2000_fsm_init();
		EC2000_DETERGENTE_INIT;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		enqueue_nextion_command("page EC2000_Main");
		enqueue_nextion_command("data.L1_on_modo.txt=\"Detergente\"");
	}
	else if((strcmp((char *)_received_data,"EC2000_aire_init"))==0)
	{
		EC2000_fsm_init();
		EC2000_AIRE_INIT;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		enqueue_nextion_command("page EC2000_Main");
		enqueue_nextion_command("data.L1_on_modo.txt=\"Aire\"");
	}
	else if((strcmp((char *)_received_data,"EC2000_desinfectante_init"))==0)
	{
		EC2000_fsm_init();
		EC2000_DESINFECTANTE_INIT;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		enqueue_nextion_command("page EC2000_Main");
		enqueue_nextion_command("data.L1_on_modo.txt=\"Desinfeccion\"");
	}
	else if((strcmp((char *)_received_data,"EC2000_alcohol_init"))==0)
	{
		EC2000_fsm_init();
		EC2000_ALCOHOL_INIT;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		enqueue_nextion_command("page EC2000_Main");
		enqueue_nextion_command("data.L1_on_modo.txt=\"Alcohol\"");
	}
	else if((strcmp((char *)_received_data,"EC2000_completo_init"))==0)
	{
		EC2000_fsm_init();
		EC2000_COMPLETO_INIT;
		Nextion_ChangeState(REPRO1, Lavando_NextionState);
		enqueue_nextion_command("page EC2000_Main");
		enqueue_nextion_command("data.L1_on_modo.txt=\"Lavado completo\"");
	}
	else if((strcmp((char *)_received_data,"EC2000_pistola_aire_init"))==0)
	{
		EC2000_fsm_pistola_aire_start();
	}
	else if((strcmp((char *)_received_data,"EC2000_pistola_aire_stop"))==0)
	{
		EC2000_fsm_pistola_aire_stop();
	}
	else if((strcmp((char *)_received_data,"EC2000_estanq_init"))==0)
	{
		EC2000_fsm_estanq_start();
		enqueue_nextion_command("page EC2000_Main");
	}
	else if((strcmp((char *)_received_data,"EC2000_drenaje_manual_stop"))==0)
	{
		EC2000_fsm_drenaje_manual_stop();
	}
	else if((strcmp((char *)_received_data,"EC2000_drenaje_manual_init"))==0)
	{
		EC2000_fsm_drenaje_manual_start();
	}
}

void Nextion_Error(uint16_t _lav,const char * _msj)
{

	char buffer[100]="";
	if(_lav==1)
	{
		strcpy(buffer, "data.L1_mensaje.txt=\"");
		strcat(buffer, _msj);
		strcat(buffer, "\"");
		enqueue_nextion_command(buffer);

		enqueue_nextion_command("data.L1_status.txt=\"Error\"");
	}

	Nextion_SetButton_vis(_lav, Boton1_NextionButtons, Ocultar_NextionButtons);
}

void Nextion_Error_MostrarBotones(uint16_t _lav)
{
	if(_lav == 1)
	{
		enqueue_nextion_command("data.L1_error_vis.txt=\"si\"");
	}
}

void Nextion_Error_OcultarBotones(uint16_t _lav)
{
	if(_lav == 1)
	{
		enqueue_nextion_command("data.L1_error_vis.txt=\"no\"");
	}
}

void Nextion_RecursoOcupado(uint16_t _lav, const char * _msj)
{
	char buffer[100]="";

	if(_lav == 1)
	{
		strcpy(buffer, "data.L1_mensaje.txt=\"");
		strcat(buffer, _msj);
		strcat(buffer, "\"");
		enqueue_nextion_command(buffer);

		enqueue_nextion_command("data.L1_status.txt=\"Ocupado\"");
	}
}

void Nextion_Reset_RecursoOcupado(uint16_t _lav)
{
	if(_lav == 1)
	{
		enqueue_nextion_command("data.L1_status.txt=\"Lavando\"");
	}
}

//-----> DRAIN
void Nextion_Drain_ErrorTapa()
{
	enqueue_nextion_command("data_remdes.status.txt=\"Error\"");
	enqueue_nextion_command("data_remdes.error_msj.txt=\"Tapa Abierta\"");
}

void Nextion_Drain_Reset_ErrorTapa()
{
	enqueue_nextion_command("data_remdes.status.txt=\"Espera\"");
}

//-----> CleanRecBox
void Nextion_CleanRecBox_ErrorTapa()
{
	enqueue_nextion_command("page RClean_ErrTapa");
}

void Nextion_CleanRecBox_ErrorNivel()
{
	enqueue_nextion_command("page RClean_ErrBid");
}

void Nextion_CleanRecBox_ErrorAgua()
{
	enqueue_nextion_command("page RClean_ErrAgua");
}

void Nextion_CleanRecBox_Reset_ErrorAgua()
{
	enqueue_nextion_command("page RDesin_Clean");
}

void Nextion_CleanRecBox_LavComplete()
{
	enqueue_nextion_command("page RClean_Fin");
}

void Nextion_CleanRecBox_n_lavado1()
{
	enqueue_nextion_command("n_lavado.val=1");
}
void Nextion_CleanRecBox_n_lavado2()
{
	enqueue_nextion_command("n_lavado.val=2");
}

/**
 *
 * Funcion para mostrar/ocultar botones.
 *
 * */

void Nextion_SetButton_vis(uint8_t _lav,uint8_t _button,uint8_t _state)
{
	if(_lav == 1)
	{
		if(_button==Boton1_NextionButtons)
		{
			if(_state==0)
				enqueue_nextion_command("vis Lav1_p1,0"); //Ocultar
			if(_state==1)
				enqueue_nextion_command("vis Lav1_p1,1"); //Mostrar

		}else if(_button==Boton2_NextionButtons)
		{
			if(_state==0)
				enqueue_nextion_command("vis Lav1_p0,0"); //Ocultar
			if(_state==1)
				enqueue_nextion_command("vis Lav1_p0,1"); //Mostrar
		}
	}
}

void Nextion_ChangeState(uint8_t _lav,uint8_t _state)
{
	char buffer[100]="";

	if(_lav == 1)
	{
		strcpy(buffer, "data.L1_status.txt=\"");
	}

	switch (_state) {
	case Lavando_NextionState:
		strcat(buffer, "Lavando\"");
		break;
	case Preparado_NextionState:
		strcat(buffer, "Preparado\"");
		break;
	case Pausado_NextionState:
		strcat(buffer, "Pausado\"");
		break;
	case Error_NextionState:
		strcat(buffer, "Error\"");
		break;
	case Finalizado_NextionState:
		strcat(buffer, "Finalizado\"");
		break;
	case Ocupado_NextionState:
		strcat(buffer, "Ocupado\"");
		break;
	default:
		break;
	}
	enqueue_nextion_command(buffer);
}


void Nextion_FinDrain()
{
	enqueue_nextion_command("data_remdes.status.txt=\"Espera\"");
}

void Nextion_FinDosing()
{
	enqueue_nextion_command("data_remdes.status.txt=\"Espera\"");
}

void Nextion_Send_NivelBidon()
{
	if(!SENSOR_NIVEL_DESINF_1 && !SENSOR_NIVEL_DESINF_2 && !SENSOR_NIVEL_DESINF_3 && !SENSOR_NIVEL_DESINF_4)
	{
		nivel_bidon=EMPTY;
	}
	else if(SENSOR_NIVEL_DESINF_1 && !SENSOR_NIVEL_DESINF_2 && !SENSOR_NIVEL_DESINF_3 && !SENSOR_NIVEL_DESINF_4)
	{
		nivel_bidon=LOW;
	}
	else if(SENSOR_NIVEL_DESINF_1 && SENSOR_NIVEL_DESINF_2 && !SENSOR_NIVEL_DESINF_3 && !SENSOR_NIVEL_DESINF_4)
	{
		nivel_bidon=MEDIUM;
	}
	else if(SENSOR_NIVEL_DESINF_1 && SENSOR_NIVEL_DESINF_2 && SENSOR_NIVEL_DESINF_3 && !SENSOR_NIVEL_DESINF_4)
	{
		nivel_bidon=HIGH;
	}
	else if(SENSOR_NIVEL_DESINF_1 && SENSOR_NIVEL_DESINF_2 && SENSOR_NIVEL_DESINF_3 && SENSOR_NIVEL_DESINF_4)
	{
		nivel_bidon=FULL;
	}
	else
	{
		nivel_bidon=NIVEL_ERROR;
	}

	switch (nivel_bidon)
	{
	case EMPTY:
		enqueue_nextion_command("data.nivel_bidon.txt=\"empty\"");
		break;
	case LOW:
		enqueue_nextion_command("data.nivel_bidon.txt=\"low\"");
		break;
	case MEDIUM:
		enqueue_nextion_command("data.nivel_bidon.txt=\"medium\"");
		break;
	case HIGH:
		enqueue_nextion_command("data.nivel_bidon.txt=\"high\"");
		break;
	case FULL:
		enqueue_nextion_command("data.nivel_bidon.txt=\"full\"");
		break;
	case NIVEL_ERROR:
		enqueue_nextion_command("data.nivel_bidon.txt=\"error\"");
		break;
	default:
		break;
	}
}


void Nextion_send_on_modo(uint8_t _lav)
{
	//---> Variables
	char buffer[50]="";
	char lavado_modo_str[50]="";
	uint8_t state=0;

	//---> Filtro que lavadora es:
	switch (_lav) {
	case REPRO1:
		state = L1_FSM_Get_State();
		strcpy(buffer, "data.L1_on_modo.txt=\"");
		break;
	case REPRO2:
		break;
	default:
		break;
	}

	switch(state)
	{
	case 0:	// IDLE
		strcpy(lavado_modo_str, "Preparando para iniciar");
		break;

	case 1: // PRESET
		strcpy(lavado_modo_str, "Iniciando lavado...");
		break;

	case 2: // ESTANQUEIDAD
		strcpy(lavado_modo_str, "Testeando estanqueidad...");
		break;

	case 3: // FLUSH
		strcpy(lavado_modo_str, "Lavado inicial en curso...");
		break;

	case 4: // WASHING
		strcpy(lavado_modo_str, "Lavando con detergente...");
		break;

	case 5: // RISING
		strcpy(lavado_modo_str, "Enjuagando...");
		break;

	case 6: // DESINFECT
		strcpy(lavado_modo_str, "Lavando con desinfectante...");
		break;

	case 7: // FINAL RISING
		strcpy(lavado_modo_str, "Enjuagado final...");
		break;

	case 8: // DRYING
		strcpy(lavado_modo_str, "Alcohol y secado en curso...");
		break;
	default:

		break;
	}

	strcat(buffer, lavado_modo_str);
	strcat(buffer, "\"");
	enqueue_nextion_command(buffer);
}

void Nextion_send_on_estanq()
{
	/* PRESION ESTANQUEIDAD */

	char buffer[50]="";
	char float_str[10];

	sprintf(float_str,"%d",(int)estanq_presionKPa);
	strcpy(buffer,"data.L1_on_presion3.txt=\"");
	strcat(buffer,float_str);
	strcat(buffer,"\"");
	enqueue_nextion_command(buffer);
}

void Nextion_send_on_temp(uint8_t _lav)
{
	char buffer[50]="";
	char float_str[10];

	/* TEMPERATURA */
	switch (_lav) {
	case REPRO1:
		sprintf(float_str,"%d",(int)L1_realTemp);
		strcpy(buffer,"data.L1_on_temp.txt=\"");
		strcat(buffer,float_str);
		strcat(buffer,"\"");
		enqueue_nextion_command(buffer);

		break;
	case REPRO2:
		sprintf(float_str,"%d",(int)L2_realTemp);
		strcpy(buffer,"data.L2_on_temp.txt=\"");
		strcat(buffer,float_str);
		strcat(buffer,"\"");
		enqueue_nextion_command(buffer);

		break;
	default:
		break;
	}
}

void Service_SendPres()
{
	char buffer[150]="";
	char float_str[20];

	/*-----------------------*/
	sprintf(float_str,"%d",(int)L1_pressure_1);
	strcpy(buffer,"SrvButtons.num5.val=");
	strcat(buffer,float_str);
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
	/*-----------------------*/
	sprintf(float_str,"%d",(int)L1_pressure_2);
	strcpy(buffer,"SrvButtons.num6.val=");
	strcat(buffer,float_str);
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
	/*-----------------------*/
}

void Service_SendTemp()
{
	char buffer[150]="";
	char float_str[20];

	/*-----------------------*/
	sprintf(float_str,"%d",(int)L1_ntc_adc_value);
	strcpy(buffer,"SrvButtons.num1.val=");
	strcat(buffer,float_str);
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
	/*-----------------------*/
	sprintf(float_str,"%d",(int)L1_realTemp);
	strcpy(buffer,"SrvButtons.num2.val=");
	strcat(buffer,float_str);
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
	/*------------------------*/
	//--------------> PARA MOSTRAR CON DECIMALES...
	memset(float_str, 0, sizeof(float_str));
	ConvertFloatToString(float_str, L1_realTemp, 1);
	strcpy(buffer,"SrvButtons.str1.txt=\"");
	strcat(buffer,float_str);
	strcat(buffer,"\"");
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
	/*------------------------*/
}

void Service_SendTemp_ADC()
{
	char buffer[150]="";
	char float_str[20];

	/*-----------------------*/
	sprintf(float_str,"%d",(int)L1_ntc_adc_value);
	strcpy(buffer,"data.aux_str1.txt=\"");
	strcat(buffer,float_str);
	strcat(buffer,"\"");
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
	/*-----------------------*/
	sprintf(float_str,"%d",(int)L2_ntc_adc_value);
	strcpy(buffer,"data.aux_str2.txt=\"");
	strcat(buffer,float_str);
	strcat(buffer,"\"");
	enqueue_nextion_command(buffer);
	memset(buffer, 0, sizeof(buffer));
}


void Nextion_send_on_presion(uint8_t _lav)
{
	char buffer[50]="";
	char float_str[10];

	switch (_lav) {
	case REPRO1:
		/* PRESION 1*/
		sprintf(float_str,"%d",(int)L1_pressure_1);
		strcpy(buffer,"data.L1_on_presion1.txt=\"");
		strcat(buffer,float_str);
		strcat(buffer,"\"");
		enqueue_nextion_command(buffer);

		memset(buffer, 0, sizeof(buffer));
		memset(float_str, 0, sizeof(float_str));

		/* PRESION 2*/
		sprintf(float_str,"%d",(int)L1_pressure_2);
		strcpy(buffer,"data.L1_on_presion2.txt=\"");
		strcat(buffer,float_str);
		strcat(buffer,"\"");
		enqueue_nextion_command(buffer);
		break;
	case REPRO2:
		break;
	default:
		break;
	}
}

void Nextion_RTC_Request(ENUM_REF_LAV_T repro)
{
	if (rtc_req_pending) return;
	rtc_req_pending = 1;
	rtc_lav = repro;
	enqueue_nextion_command("send_rtc_stm32.en=1");
}

void Nextion_Send_OPA_ON(ENUM_REF_LAV_T _lav)
{
/*	if(_lav == 1)
	{
		L1_opa_bacha = 1;
		enqueue_nextion_command("L1_OPA_ON.en=1");
	}
*/
	L1_opa_bacha = 1;
	char buffer[150];
	strcpy(buffer,"");
	strcat(buffer,"data.L1_opa.txt=\"si\"\xFF\xFF\xFF");
	//strcat(buffer,"wepo data.L1_opa.txt,300\xFF\xFF\xFF");
	strcat(buffer,"wepo \"si\",300\xFF\xFF\xFF");
		enqueue_nextion_command(buffer);
}


void Nextion_Send_OPA_OFF(ENUM_REF_LAV_T _lav)
{
	/*
	if(_lav == 1)
	{
		L1_opa_bacha = false;
		enqueue_nextion_command("L1_OPA_OFF.en=1");
	}
	*/
	L1_opa_bacha = false;
	char buffer[150];
	strcpy(buffer,"");
	strcat(buffer,"data.L1_opa.txt=\"no\"\xFF\xFF\xFF");
	//strcat(buffer,"wepo data.L1_opa.txt,300\xFF\xFF\xFF");
	strcat(buffer,"wepo \"no\",300\xFF\xFF\xFF");
	enqueue_nextion_command(buffer);
}

void Nextion_Logger(ENUM_REF_LAV_T _lav, const char * _codigo, const char * _descripcion)
{
	/* Como usarlo */
	//Nextion_Logger(REPRO1,"E31","Falla sensor");
	//Nextion_Logger(REPRO2,"E36","Falla optico");
	/*----------------*/

	char buffer[256];
	strcpy(buffer,"");

	strcpy(buffer,"DataLogger.data.txt=\"");

	if(_lav == 1)
	{
		strcat(buffer,"Lavadora 1");
	}

	strcat(buffer,"^");
	strcat(buffer,_codigo);
	strcat(buffer,"^");
	strcat(buffer,_descripcion);
	strcat(buffer,"\"");
	strcat(buffer,"\xFF\xFF\xFF");
	strcat(buffer,"Logger.en=1");

	enqueue_nextion_command(buffer);
}



void Service_SendSensores()
{
	char buffer[256];
	strcpy(buffer,"");

	//------------------------------------------------

	if(L1_SENSOR_CALEFACTOR)
		strcat(buffer,"Srv.L1_X4.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L1_X4.val=0\xFF\xFF\xFF");

	if(L1_SENSOR_NIVEL_TAPA)
		strcat(buffer,"Srv.L1_X6.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L1_X6.val=0\xFF\xFF\xFF");

	if(L1_SENSOR_IMAN_TAPA)
		strcat(buffer,"Srv.L1_X14.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L1_X14.val=0\xFF\xFF\xFF");

	//------------------------------------------------
	if(SENSOR_NIVEL_DET)
		strcat(buffer,"Srv.L2_X7.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L2_X7.val=0\xFF\xFF\xFF");

	if(SENSOR_NIVEL_ALC)
		strcat(buffer,"Srv.L2_X8.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L2_X8.val=0\xFF\xFF\xFF");

	if(SENSOR_NIVEL_DESINF_1)
		strcat(buffer,"Srv.L2_X9.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L2_X9.val=0\xFF\xFF\xFF");

	if(SENSOR_NIVEL_DESINF_2)
		strcat(buffer,"Srv.L2_X10.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L2_X10.val=0\xFF\xFF\xFF");

	if(SENSOR_NIVEL_DESINF_3)
		strcat(buffer,"Srv.L2_X11.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L2_X11.val=0\xFF\xFF\xFF");

	if(SENSOR_NIVEL_DESINF_4)
		strcat(buffer,"Srv.L2_X13.val=1\xFF\xFF\xFF");
	else
		strcat(buffer,"Srv.L2_X13.val=0");
	//------------------------------------------------
	enqueue_nextion_command(buffer);
	/*
	enqueue_nextion_command(
			"vis t6,0\xFF\xFF\xFF"
			"vis b5,0\xFF\xFF\xFF"
			"vis b4,0\xFF\xFF\xFF"
			"vis b0,0\xFF\xFF\xFF"
			"vis b2,0\xFF\xFF\xFF"
			"vis b1,0\xFF\xFF\xFF"
			"vis b3,0\xFF\xFF\xFF"
			"vis b6,1\xFF\xFF\xFF"
			"t1.txt=\"20 g cada 10 L\"\xFF\xFF\xFF"
			"t7.txt=\"Desinfectante Solido\"\xFF\xFF\xFF"
			"wepo t7.txt,64"
	);
	 */
}


void Service_SendMonitorGPIO()
{
	uint16_t inputs_value = 0;

	for (uint8_t i = 0; i < 16; i++)
	{
		inputs_value += rm_input1[i] << i;
	}

	uint16_t outputs_value = 0;

	for (uint8_t i = 0; i < 16; i++)
	{
		outputs_value += rm_output1[i] << i;
	}

	//--------------------------------------------------------
	char buffer[100]="";
	strcpy(buffer,"");
	char float_str[10];

	sprintf(float_str,"%d",(int)inputs_value);
	strcpy(buffer,"data.L1_rm_inputs.val=");
	strcat(buffer,float_str);
	strcat(buffer,"\xFF\xFF\xFF");

	memset(float_str, 0, sizeof(float_str));

	sprintf(float_str,"%d",(int)outputs_value);
	strcat(buffer,"data.L1_rm_outputs.val=");
	strcat(buffer,float_str);
	strcat(buffer,"\xFF\xFF\xFF");


	//------------> Envio si el ultrasonido esta habilitado o no.
	if(ultson_enable == ULTSON_ENABLE_VALUE)
		strcat(buffer,"data.L1_ultson_en.txt=\"si\"");
	else if (ultson_enable == ULTSON_DISABLE_VALUE)
		strcat(buffer,"data.L1_ultson_en.txt=\"no\"");
	else
		strcat(buffer,"data.L1_ultson_en.txt=\"error\"");
	strcat(buffer,"\xFF\xFF\xFF");

	enqueue_nextion_command(buffer);
}



//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-----> Para probar botones, carteles, errores, etc.
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void Nextion_Demo()
{
	Medsaf_sendstate_runcycle();

	//---> Timer 1 segundo
	if(!nextion_tic_1seg)
	{
		nextion_tic_1seg=1000;

		/*-----------------------*/
	}
	//---> Timer 2 segundo
	if(!nextion_tic_2seg)
	{
		nextion_tic_2seg=2000;
		/*-----------------------*/
		Nextion_send_on_temp(REPRO1);
		Nextion_send_on_temp(REPRO2);
		Nextion_send_on_presion(REPRO1);
		Service_SendMonitorGPIO();
	}
	//---> Timer 3 segundo
	if(!nextion_tic_3seg)
	{
		nextion_tic_3seg=3000;
		/*-----------------------*/
		Nextion_send_on_estanq(REPRO1);
		EC2000_Nextion_Send_NivelBidon();
		Medsaf_Nextion_Send_NivelBacha();
	}
	//---> Timer 4 segundo
	if(!nextion_tic_4seg)
	{
		nextion_tic_4seg=4000;
		/*-----------------------*/
		Service_SendTemp_ADC();
	}
	//---> Timer 5 segundo
	if(!nextion_tic_5seg)
	{
		nextion_tic_5seg=5000;
	}

	if(!nextion_demo_tic)
	{
		nextion_demo_tic=3000;

	}
}
