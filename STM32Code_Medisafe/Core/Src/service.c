/*
 * service.c
 *
 *  Created on: Oct 8, 2025
 *      Author: ADOX
 */

#include "main.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#include "nextion.h"
#include "relay_module.h"
#include "fsm_reprocesadora1.h"
#include "service.h"
#include "global_variables.h"


void Service_Diagram(const uint8_t *_received_data)
{

	if((strcmp((char *)_received_data,"STOP"))==0)
	{
	}
	else if((strcmp((char *)_received_data,"RM1_OUT1_ON"))==0)
	{
		RM1_OUT1_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT2_ON"))==0)
	{
		RM1_OUT2_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT3_ON"))==0)
	{
		RM1_OUT3_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT4_ON"))==0)
	{
		RM1_OUT4_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT5_ON"))==0)
	{
		RM1_OUT5_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT6_ON"))==0)
	{
		RM1_OUT6_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT7_ON"))==0)
	{
		RM1_OUT7_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT8_ON"))==0)
	{
		RM1_OUT8_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT9_ON"))==0)
	{
		RM1_OUT9_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT10_ON"))==0)
	{
		RM1_OUT10_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT11_ON"))==0)
	{
		RM1_OUT11_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT12_ON"))==0)
	{
		RM1_OUT12_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT13_ON"))==0)
	{
		RM1_OUT13_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT14_ON"))==0)
	{
		RM1_OUT14_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT15_ON"))==0)
	{
		RM1_OUT15_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT16_ON"))==0)
	{
		RM1_OUT16_ON
	}
	else if((strcmp((char *)_received_data,"RM1_OUT1_OFF"))==0)
	{
		RM1_OUT1_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT2_OFF"))==0)
	{
		RM1_OUT2_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT3_OFF"))==0)
	{
		RM1_OUT3_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT4_OFF"))==0)
	{
		RM1_OUT4_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT5_OFF"))==0)
	{
		RM1_OUT5_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT6_OFF"))==0)
	{
		RM1_OUT6_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT7_OFF"))==0)
	{
		RM1_OUT7_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT8_OFF"))==0)
	{
		RM1_OUT8_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT9_OFF"))==0)
	{
		RM1_OUT9_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT10_OFF"))==0)
	{
		RM1_OUT10_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT11_OFF"))==0)
	{
		RM1_OUT11_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT12_OFF"))==0)
	{
		RM1_OUT12_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT13_OFF"))==0)
	{
		RM1_OUT13_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT14_OFF"))==0)
	{
		RM1_OUT14_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT15_OFF"))==0)
	{
		RM1_OUT15_OFF
	}
	else if((strcmp((char *)_received_data,"RM1_OUT16_OFF"))==0)
	{
		RM1_OUT16_OFF
	}
	//------------------------------------------------------------
	//------------------------------------------------------------
	//------------------------------------------------------------
	//-----> COMANDOS DE LA LAVADORA
	//------------------------------------------------------------
	else if((strcmp((char *)_received_data,"L1_BOM01:ON"))==0)
	{
		L1_BOM01_ON
	}
	else if((strcmp((char *)_received_data,"L1_BOM01:OFF"))==0)
	{
		L1_BOM01_OFF
	}
	else if((strcmp((char *)_received_data,"L1_BOM02:ON"))==0)
	{
		L1_BOM02_ON
	}
	else if((strcmp((char *)_received_data,"L1_BOM02:OFF"))==0)
	{
		L1_BOM02_OFF
	}
	else if((strcmp((char *)_received_data,"BOM03:ON"))==0) 			// COMPARTIDO ENTRE LAVADORAS
	{
		BOM03_ON
	}
	else if((strcmp((char *)_received_data,"BOM03:OFF"))==0)			// COMPARTIDO ENTRE LAVADORAS
	{
		BOM03_OFF
	}
	else if((strcmp((char *)_received_data,"L1_BOM04:ON"))==0)
	{
		L1_BOM04_ON
	}
	else if((strcmp((char *)_received_data,"L1_BOM04:OFF"))==0)
	{
		L1_BOM04_OFF
	}
	else if((strcmp((char *)_received_data,"L1_BPR05:ON"))==0)
	{
		L1_BPR05_ON
	}
	else if((strcmp((char *)_received_data,"L1_BPR05:OFF"))==0)
	{
		L1_BPR05_OFF
	}
	else if((strcmp((char *)_received_data,"L1_BPR06:ON"))==0)
	{
		L1_BPR06_ON
	}
	else if((strcmp((char *)_received_data,"L1_BPR06:OFF"))==0)
	{
		L1_BPR06_OFF
	}
	else if((strcmp((char *)_received_data,"L1_VAL07:ON"))==0)
	{
		L1_VAL07_ON
	}
	else if((strcmp((char *)_received_data,"L1_VAL07:OFF"))==0)
	{
		L1_VAL07_OFF
	}
	else if((strcmp((char *)_received_data,"L1_VAL08:ON1"))==0)
	{
		L1_VAL08_ON1
	}
	else if((strcmp((char *)_received_data,"L1_VAL08:ON2"))==0)
	{
		L1_VAL08_ON2
	}
	else if((strcmp((char *)_received_data,"L1_VAL08:OFF"))==0)
	{
		L1_VAL08_OFF
	}
	else if((strcmp((char *)_received_data,"L1_SOL09:ON"))==0)
	{
		L1_SOL09_ON
	}
	else if((strcmp((char *)_received_data,"L1_SOL09:OFF"))==0)
	{
		L1_SOL09_OFF
	}
	else if((strcmp((char *)_received_data,"L1_SOL10:ON"))==0)
	{
		L1_SOL10_ON
	}
	else if((strcmp((char *)_received_data,"L1_SOL10:OFF"))==0)
	{
		L1_SOL10_OFF
	}
	else if((strcmp((char *)_received_data,"L1_CLF11:ON"))==0)
	{
		L1_CLF11_ON
	}
	else if((strcmp((char *)_received_data,"L1_CLF11:OFF"))==0)
	{
		L1_CLF11_OFF
	}
	else if((strcmp((char *)_received_data,"L1_ELV12:ON"))==0)
	{
		L1_ELV12_ON
	}
	else if((strcmp((char *)_received_data,"L1_ELV12:OFF"))==0)
	{
		L1_ELV12_OFF
	}
	else if((strcmp((char *)_received_data,"L1_ELV13:ON"))==0)
	{
		L1_ELV13_ON
	}
	else if((strcmp((char *)_received_data,"L1_ELV13:OFF"))==0)
	{
		L1_ELV13_OFF
	}
	else if((strcmp((char *)_received_data,"CMP14:ON"))==0)			// COMPARTIDO ENTRE LAVADORAS
	{
		CMP14_ON
	}
	else if((strcmp((char *)_received_data,"CMP14:OFF"))==0)		// COMPARTIDO ENTRE LAVADORAS
	{
		CMP14_OFF
	}
}


void Service_Buttons(const uint8_t *_received_data)
{

	if((strcmp((char *)_received_data,"SRV_WASH"))==0)
	{
		fsm1_change_state("flush");
	}
	else if((strcmp((char *)_received_data,"SRV_CLEAN"))==0)
	{
		fsm1_change_state("wash");
	}

	else if((strcmp((char *)_received_data,"SRV_RINSE"))==0)
	{
		fsm1_change_state("rinse");
	}

	else if((strcmp((char *)_received_data,"SRV_DISINF"))==0)
	{
		fsm1_change_state("desinfect");
	}

	else if((strcmp((char *)_received_data,"SRV_FINWASH"))==0)
	{
		fsm1_change_state("final_rise");
	}

	else if((strcmp((char *)_received_data,"SRV_ALCH"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}

	else if((strcmp((char *)_received_data,"SRV_DRY"))==0)
	{
		fsm1_change_state("dry");
	}

	else if((strcmp((char *)_received_data,"PRG_STAND"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}

	else if((strcmp((char *)_received_data,"PRG_DRY"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}

	else if((strcmp((char *)_received_data,"PRG_PIPEDIS"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}

	else if((strcmp((char *)_received_data,"prueba_1"))==0)
	{
		fsm1_change_state("apagar_reles485");
	}

	else if((strcmp((char *)_received_data,"prueba_2"))==0)
	{
		L1_WASHING_HEATING = 1;		// habilito calefactor en WASHING
		L1_DESINF_HEATING = 1;		// habilito calefactor en DESINFECT
	}

	else if((strcmp((char *)_received_data,"prueba_3"))==0)	   // ON Estanqueidad
	{
		fsm1_change_state("estanq");
	}

	else if((strcmp((char *)_received_data,"prueba_4"))==0)    // OFF Estanqueidad
	{
		fsm1_change_state("off_estanq");
	}

	else if((strcmp((char *)_received_data,"prueba_5"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}

	else if((strcmp((char *)_received_data,"prueba_6"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
	else if((strcmp((char *)_received_data,"<L1_recircular>"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
	else if((strcmp((char *)_received_data,"<L1_stop>"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
	else if((strcmp((char *)_received_data,"<L1_drenaje>"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
	else if((strcmp((char *)_received_data,"<L1_opa_bidon>"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
	else if((strcmp((char *)_received_data,"<L1_llenar_cuba>"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
	else if((strcmp((char *)_received_data,"<L1_llenar_opa>"))==0)
	{
		HAL_Delay(1);
		//---> Agregar código.
	}
}
//----------------------------------------------------------------

void Service_SendStateSubstate(uint8_t _lav)
{

	char str_state[100]="";
	char str_substate[100]="";

	//strcpy(str_state, "");
	//strcpy(str_substate, "");

	//runcycle = RUNNING;				// indico para fsm1_delay que fsm deberá deshabilitar

	uint8_t state=0;
	uint8_t substate = 0;
	uint8_t substate_preset=0,substate_dry=0,substate_f_rise=0,substate_desinf=0,
			substate_rising=0,substate_wash=0,substate_flush=0,substate_estanq=0;

	switch (_lav)
	{
	case REPRO1:
		state = L1_FSM_Get_State();
		substate = L1_FSM_Get_SubState();
		break;
	case REPRO2:

		break;
	default:
		break;
	}

	substate_preset=substate;
	substate_dry=substate;
	substate_f_rise=substate;
	substate_desinf=substate;
	substate_rising=substate;
	substate_wash=substate;
	substate_flush=substate;
	substate_estanq=substate;

	switch(state)
	{
	case 0: // IDLE
		strcpy(str_state, "IDLE");
		strcpy(str_substate, "IDLE");
		break;
		/*-----------------------------------------------*/
		/*												 */
		/* 				Modo Standard					 */
		/*												 */
		/*-----------------------------------------------*/
	case 1: // PRESET
		switch(substate_preset)
		{
		case 0: // INICIO
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "INICIO");
			break;

		case 1: // DESAGUE PURGA
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "DESAGUE_PURGA_LUMEN");
			break;

		case 2: // TAPA ABIERTA
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "TAPA_ABIERTA");
			break;

		case 3: // FALTA DESINF
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "FALTA_DESINF");
			break;

		case 4: // FALTA DETERG
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "FALTA_DETERG");
			break;

		case 5: // FALTA ALC
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "FALTA_ALC");
			break;

		case 6: // PRUEBA BLOQUEO
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "PRUEB_BLOCK");
			break;

		case 7: // ENDOSC OBSTR O DESCONEC
			strcpy(str_state, "PRESET");
			strcpy(str_substate, "ENDOSC. OBS o DESC");
			break;
		}
		break;

		case 2: // ESTANQUEIDAD
			switch ( substate_estanq )
			{
			case 0: // PRUEBA_ESTANQUEIDAD
				strcpy(str_state, "PRUEBA_ESTANQ");
				strcpy(str_substate, "PRUEBA_ESTANQUEIDAD");
				break;

			case 1: // FUGA AIRE
				strcpy(str_state, "PRUEBA_ESTANQ");
				strcpy(str_substate, "FUGA_AIRE");
				break;

			case 2: // EXITO
				strcpy(str_state, "PRUEBA_ESTANQ");
				strcpy(str_substate, "EXITO");
				break;
			case 3: // TAPA ABIERTA
				strcpy(str_state, "PRUEBA_ESTANQ");
				strcpy(str_substate, "EST_TAPA_ABIERTA");
				break;
			}
			break;

			case 3: // FLUSH
				switch ( substate_flush )
				{
				case 0: // LLENAR CUBA
					strcpy(str_state, "FLUSH");
					strcpy(str_substate, "F_LLENAR_CUBA");
					break;

				case 1: // FALTA AGUA
					strcpy(str_state, "FLUSH");
					strcpy(str_substate, "F_FALTA_AGUA");
					break;

				case 2: // VACIAR CUBA
					strcpy(str_state, "FLUSH");
					strcpy(str_substate, "F_VACIAR_CUBA");
					break;

				case 3: // RECIRCULAR
					strcpy(str_state, "FLUSH");
					strcpy(str_substate, "F_RECIRCULAR");
					break;

				case 4: // DESAGUE PURGA
					strcpy(str_state, "FLUSH");
					strcpy(str_substate, "F_DESAGÜE_PURGA_LUMEN");
					break;
				case 5: // TAPA ABIERTA
					strcpy(str_state, "FLUSH");
					strcpy(str_substate, "F_TAPA_ABIERTA");
					break;
				}
				break;

				case 4: // WASHING
					switch ( substate_wash )
					{
					case 0:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_LLENAR_CUBA_DET");
						break;

					case 1:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_FALTA_AGUA");
						break;

					case 2:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_FALTA_DET");
						break;

					case 3:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_CALENTAR");
						break;

					case 4:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_FALLA_CALEF");
						break;

					case 5:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_VACIAR_CUBA");
						break;

					case 6:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_DESGASIFICAR");
						break;

					case 7:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_COMPLETAR_LLENADO");
						break;

					case 8:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_RECIRCULAR");
						break;

					case 9:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_DESAGUE_PURGA_LUMEN");
						break;

					case 10:
						strcpy(str_state, "WASHING");
						strcpy(str_substate, "W_TAPA_ABIERTA");
						break;
					}
					break;

					case 5: // RISING
						switch ( substate_rising )
						{
						case 0:
							strcpy(str_state, "RISING");
							strcpy(str_substate, "R_LLENAR_CUBA");
							break;

						case 1:
							strcpy(str_state, "RISING");
							strcpy(str_substate, "R_FALTA_AGUA");
							break;

						case 2:
							strcpy(str_state, "RISING");
							strcpy(str_substate, "R_VACIAR_CUBA");
							break;

						case 3:
							strcpy(str_state, "RISING");
							strcpy(str_substate, "R_RECIRCULAR");
							break;

						case 4:
							strcpy(str_state, "RISING");
							strcpy(str_substate, "R_DESAGUE_PURGA_LUMEN");
							break;
						case 5:
							strcpy(str_state, "RISING");
							strcpy(str_substate, "R_TAPA_ABIERTA");
							break;
						}
						break;

						case 6: // DESINFECT
							switch ( substate_desinf )
							{
							case 0:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_LLENAR_CUBA_DESINF");
								break;

							case 1:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_CALENTAR");
								break;

							case 2:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_FALLA_CALEF");
								break;

							case 3:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_VACIAR_CUBA");
								break;

							case 4:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_RECIRCULAR");
								break;

							case 5:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_BIDON_PURGA_LUMEN");
								break;
							case 6:
								strcpy(str_state, "DESINFECT");
								strcpy(str_substate, "D_TAPA_ABIERTA");
								break;
							}
							break;

							case 7: // FINAL RISING
								switch ( substate_f_rise )
								{
								case 0:
									strcpy(str_state, "FINAL_RISING");
									strcpy(str_substate, "FR_LLENAR_CUBA");
									break;

								case 1:
									strcpy(str_state, "FINAL_RISING");
									strcpy(str_substate, "FR_FALTA_AGUA");
									break;

								case 2:
									strcpy(str_state, "FINAL_RISING");
									strcpy(str_substate, "FR_VACIAR_CUBA");
									break;

								case 3:
									strcpy(str_state, "FINAL_RISING");
									strcpy(str_substate, "FR_RECIRCULAR");
									break;

								case 4:
									strcpy(str_state, "FINAL_RISING");
									strcpy(str_substate, "FR_DESAGUE_PURGA_LUMEN");
									break;
								case 5:
									strcpy(str_state, "FINAL_RISING");
									strcpy(str_substate, "FR_TAPA_ABIERTA");
									break;
								}
								break;

								case 8: // SECADO + ALCOHOL
									switch ( substate_dry )
									{
									case 0:
										strcpy(str_state, "DRYING");
										strcpy(str_substate, "DR_ALCOHOL");
										break;

									case 1:
										strcpy(str_state, "DRYING");
										strcpy(str_substate, "DR_FALTA_ALC");
										break;

									case 2:
										strcpy(str_state, "DRYING");
										strcpy(str_substate, "DR_VACIAR_CUBA");
										break;

									case 3:
										strcpy(str_state, "DRYING");
										strcpy(str_substate, "DR_SECADO");
										break;

									case 4:
										strcpy(str_state, "DRYING");
										strcpy(str_substate, "DR_FIN");
										break;

									case 5:
										strcpy(str_state, "DRYING");
										strcpy(str_substate, "DR_TAPA_ABIERTA");
										break;
									}
									break;

									default:
										strcpy(str_state, "NAN-d");
										strcpy(str_substate, "NAN-d");
										break;
	}

	char buffer[150]="";
	switch (_lav)
	{
	case REPRO1:
		strcpy(buffer, "SrvButtons.L1_state.txt=\"");
		break;
	case REPRO2:
		break;
	default:
		break;
	}
	strcat(buffer, str_state);
	strcat(buffer, "\"");
	enqueue_nextion_command(buffer);

	memset(buffer, 0, sizeof(buffer));//Limpio el buffer donde voy a leer

	switch (_lav)
	{
	case REPRO1:
		strcpy(buffer, "SrvButtons.L1_substate.txt=\"");
		break;
	case REPRO2:
		break;
	default:
		break;
	}

	strcat(buffer, str_substate);
	strcat(buffer, "\"");
	enqueue_nextion_command(buffer);
}

