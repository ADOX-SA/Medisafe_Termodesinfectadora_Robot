/*
 * ticket.h
 *
 *  Created on: Feb 18, 2026
 *      Author: momo
 */

#ifndef INC_TICKET_H_
#define INC_TICKET_H_

#pragma once
#include <stdint.h>
#include "global_variables.h"   // ENUM_REF_LAV_T, g_fecha, g_hora, L1_rtc_ready, L2_rtc_ready, L1_estanq_en, L2_estanq_en

// Inicialización (una vez)
void Ticket_InitAll(void);

// -------- Encolado de acciones (FIFO) --------
// Header requiere RTC (usa g_fecha/g_hora cuando llegue Lx_rtc_ready)
uint8_t Ticket_EnqHeader(ENUM_REF_LAV_T lav,
                         const char *modelo,
                         const char *serie,
                         const char *nro_personal,
                         const char *nro_endoscopio,
                         const char *modo);

// StepStart:
// - Rtc: espera RTC para usar hora actualizada
// - Cached: usa última hora cacheada (no requiere RTC)
uint8_t Ticket_EnqStepStartRtc(ENUM_REF_LAV_T lav, const char *label);
uint8_t Ticket_EnqStepStartCached(ENUM_REF_LAV_T lav, const char *label);

// StepEnd:
// - Rtc: espera RTC para "Tiempo Final" actualizado
// - Cached: usa última hora cacheada para "Tiempo Final"
uint8_t Ticket_EnqStepEndRtc(ENUM_REF_LAV_T lav, const char *label);
uint8_t Ticket_EnqStepEndCached(ENUM_REF_LAV_T lav, const char *label);

// Evento (línea destacada):
uint8_t Ticket_EnqEventRtc(ENUM_REF_LAV_T lav, const char *msg);
uint8_t Ticket_EnqEventCached(ENUM_REF_LAV_T lav, const char *msg);

// Duracion real de una instancia del ciclo.
uint8_t Ticket_EnqStageDuration(ENUM_REF_LAV_T lav, const char *label, uint32_t duration_ms);

// Finalización (no requiere RTC)
uint8_t Ticket_EnqFinish(ENUM_REF_LAV_T lav, const char *resultado, const char *fallas);

// Solicitar impresión (snapshot + queda pendiente de imprimir)
uint8_t Ticket_EnqPrint(ENUM_REF_LAV_T lav);

uint8_t Ticket_EnqLockTestResultadoRtc(ENUM_REF_LAV_T lav, const char *resultado);
uint8_t Ticket_EnqEstanqueidadResultadoRtc(ENUM_REF_LAV_T lav, const char *resultado);

// -------- Service / Loop --------
// Devuelve 1 si la próxima acción de esa lavadora requiere RTC y todavía no llegó.
// En el while(1) usás esto para decidir si llamar Nextion_RTC_Request(lav)
bool Ticket_NeedsRtcNow(ENUM_REF_LAV_T lav);

// Procesa acciones en cola cuando corresponda (llamar en while(1))
void Ticket_LoopAll(void);

// Imprime snapshots pendientes (serializado + cooldown)
void Ticket_ServicePrintAll(void);

#endif /* INC_TICKET_H_ */
