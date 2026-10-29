/*
 * ticket.c
 *
 *  Created on: Feb 18, 2026
 *      Author: momo
 */

#include "ticket.h"
#include "CSN_A2.h"
#include "stm32f1xx_hal.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define TICKET_BUF_SZ        1152
#define SEP_LINE             "----------------------\r\n"
#define PRINT_COOLDOWN_MS    5000u
#define TQ_LEN               20u   // cantidad de acciones en cola por lavadora (ajustable)

typedef enum {
    ACT_NONE = 0,

    ACT_HDR_RTC,

    ACT_STEP_START_RTC,
    ACT_STEP_START_CACHED,

    ACT_STEP_END_RTC,
    ACT_STEP_END_CACHED,

    ACT_EVENT_RTC,
    ACT_EVENT_CACHED,
	ACT_STAGE_DURATION,

	ACT_LOCK_TEST_RESULT_RTC,

    ACT_ESTANQ_RESULT_RTC,

    ACT_FINISH,
    ACT_PRINT
} TicketActType;

/*
 * CAMBIO CLAVE:
 * Antes TicketAction tenía TODOS los payloads siempre, y cada acción "pesaba" como un header.
 * Ahora usamos union: cada acción ocupa solo el payload más grande (header), no la suma de todos.
 */
typedef struct {
    TicketActType type;

    union {
        struct { // ACT_HDR_RTC
            char modelo[10], serie[10], personal[24], endo[10], modo[24], dil[8];
        } hdr;

        struct { // STEP_START / STEP_END
            char label[16];
        } step;

        struct { // EVENT / ESTANQ RESULT
            char msg[50];
        } event;

		struct { // ACT_STAGE_DURATION
			char label[24];
			uint32_t duration_ms;
		} stage;

        struct { // FINISH
            char resultado[16];
            char fallas[30];
        } finish;
    } u;

} TicketAction;

typedef struct {
    TicketAction q[TQ_LEN];
    uint8_t head, tail, count;
} TicketQueue;

typedef struct {
    // ticket activo
    char     buf[TICKET_BUF_SZ];
    uint16_t len;
    uint8_t  trunc;

    // cola
    TicketQueue tq;

    // cache per-lavadora de fecha/hora
    char last_fecha[11]; // "YYYY.MM.DD"
    char last_hora[9];   // "HH:MM:SS"
    uint8_t last_valid;

    // duración real
    uint32_t step_start_ms;

    // snapshot para imprimir
    char     print_buf[TICKET_BUF_SZ];
    uint16_t print_len;
    uint8_t  print_pending;
} TicketCtx;

static TicketCtx tk_L1;

// arbiter impresora
static uint32_t cooldown_until_ms = 0;
static uint8_t printer_busy = 0;

// -------------------- helpers strings/buffer --------------------
static void tk_reset_active(TicketCtx *t)
{
    t->len = 0;
    t->trunc = 0;
    t->buf[0] = '\0';
    t->step_start_ms = 0;
}

static void tk_reset_queue(TicketCtx *t)
{
    t->tq.head = t->tq.tail = t->tq.count = 0;
}

static void tk_init(TicketCtx *t)
{
    memset(t, 0, sizeof(*t));
    tk_reset_active(t);
    tk_reset_queue(t);
    t->last_valid = 0;
}

static void tk_append(TicketCtx *t, const char *s)
{
    if (t->trunc) return;
    size_t n = strlen(s);
    if ((size_t)t->len + n >= sizeof(t->buf)) {
        size_t room = (sizeof(t->buf) - 1) - t->len;
        if (room) {
            memcpy(&t->buf[t->len], s, room);
            t->len += (uint16_t)room;
            t->buf[t->len] = '\0';
        }
        t->trunc = 1;
        return;
    }
    memcpy(&t->buf[t->len], s, n);
    t->len += (uint16_t)n;
    t->buf[t->len] = '\0';
}

static void tk_printf(TicketCtx *t, const char *fmt, ...)
{
    if (t->trunc) return;

    int avail = (int)(sizeof(t->buf) - t->len);
    if (avail <= 1) { t->trunc = 1; return; }

    va_list ap;
    va_start(ap, fmt);
    int wrote = vsnprintf(&t->buf[t->len], avail, fmt, ap);
    va_end(ap);

    if (wrote <= 0) return;

    if (wrote >= avail) {
        t->len = (uint16_t)(sizeof(t->buf) - 1);
        t->buf[t->len] = '\0';
        t->trunc = 1;
    } else {
        t->len += (uint16_t)wrote;
    }
}

static void fmt_mmss(char *out, size_t outsz, uint32_t ms)
{
    uint32_t sec = ms / 1000u;
    uint32_t mm  = sec / 60u;
    uint32_t ss  = sec % 60u;
    snprintf(out, outsz, "%02lu:%02lu", (unsigned long)mm, (unsigned long)ss);
}

// -------------------- helpers L1/L2 --------------------
static TicketCtx* get_ctx(ENUM_REF_LAV_T lav)
{
    if (lav == REPRO1) return &tk_L1;
    return NULL;
}

static volatile bool* get_rtc_ready_ptr(ENUM_REF_LAV_T lav)
{
    if (lav == REPRO1) return &L1_rtc_ready;
    return NULL;
}

// -------------------- FIFO queue --------------------
static uint8_t tq_push(TicketQueue *tq, const TicketAction *a)
{
    if (tq->count >= TQ_LEN) return 0;
    tq->q[tq->tail] = *a;
    tq->tail = (uint8_t)((tq->tail + 1u) % TQ_LEN);
    tq->count++;
    return 1;
}

static uint8_t tq_peek(TicketQueue *tq, TicketAction **a)
{
    if (tq->count == 0) return 0;
    *a = &tq->q[tq->head];
    return 1;
}

static void tq_pop(TicketQueue *tq)
{
    if (tq->count == 0) return;
    tq->head = (uint8_t)((tq->head + 1u) % TQ_LEN);
    tq->count--;
}

static uint8_t act_needs_rtc(TicketActType t)
{
    return (t == ACT_HDR_RTC ||
            t == ACT_STEP_START_RTC ||
            t == ACT_STEP_END_RTC ||
            t == ACT_EVENT_RTC ||
            t == ACT_ESTANQ_RESULT_RTC);
}

// -------------------- API --------------------
void Ticket_InitAll(void)
{
    tk_init(&tk_L1);
}

uint8_t Ticket_EnqHeader(ENUM_REF_LAV_T lav,
                         const char *modelo,
                         const char *serie,
                         const char *nro_personal,
                         const char *nro_endoscopio,
                         const char *modo)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = ACT_HDR_RTC;

    strncpy(a.u.hdr.modelo,  modelo,        sizeof(a.u.hdr.modelo)-1);
    strncpy(a.u.hdr.serie,   serie,         sizeof(a.u.hdr.serie)-1);
    strncpy(a.u.hdr.personal,nro_personal,  sizeof(a.u.hdr.personal)-1);
    strncpy(a.u.hdr.endo,    nro_endoscopio,sizeof(a.u.hdr.endo)-1);
    strncpy(a.u.hdr.modo,    modo,          sizeof(a.u.hdr.modo)-1);

    switch (lav)
    {
        case REPRO1:
            if (L1_dilucion == 28000) {
                strncpy(a.u.hdr.dil, "1:200", sizeof(a.u.hdr.dil)-1);
            } else if (L1_dilucion == ((28000 * 2)/5)) {
                strncpy(a.u.hdr.dil, "1:500", sizeof(a.u.hdr.dil)-1);
            } else if (L1_dilucion == ((28000 * 1)/5)) {
                strncpy(a.u.hdr.dil, "1:1000", sizeof(a.u.hdr.dil)-1);
            }
            break;

        case REPRO2:
            break;

        default:
            break;
    }

    return tq_push(&t->tq, &a);
}

static uint8_t enq_label(ENUM_REF_LAV_T lav, TicketActType type, const char *label)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = type;
    strncpy(a.u.step.label, label, sizeof(a.u.step.label)-1);

    return tq_push(&t->tq, &a);
}

static uint8_t enq_msg(ENUM_REF_LAV_T lav, TicketActType type, const char *msg)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = type;
    strncpy(a.u.event.msg, msg, sizeof(a.u.event.msg)-1);

    return tq_push(&t->tq, &a);
}

uint8_t Ticket_EnqStepStartRtc(ENUM_REF_LAV_T lav, const char *label)    { return enq_label(lav, ACT_STEP_START_RTC, label); }
uint8_t Ticket_EnqStepStartCached(ENUM_REF_LAV_T lav, const char *label) { return enq_label(lav, ACT_STEP_START_CACHED, label); }

uint8_t Ticket_EnqStepEndRtc(ENUM_REF_LAV_T lav, const char *label)      { return enq_label(lav, ACT_STEP_END_RTC, label); }
uint8_t Ticket_EnqStepEndCached(ENUM_REF_LAV_T lav, const char *label)   { return enq_label(lav, ACT_STEP_END_CACHED, label); }

uint8_t Ticket_EnqEventRtc(ENUM_REF_LAV_T lav, const char *msg)          { return enq_msg(lav, ACT_EVENT_RTC, msg); }
uint8_t Ticket_EnqEventCached(ENUM_REF_LAV_T lav, const char *msg)       { return enq_msg(lav, ACT_EVENT_CACHED, msg); }

uint8_t Ticket_EnqStageDuration(ENUM_REF_LAV_T lav, const char *label, uint32_t duration_ms)
{
	TicketCtx *t = get_ctx(lav);
	TicketAction a;

	if (!t) return 0;

	memset(&a, 0, sizeof(a));
	a.type = ACT_STAGE_DURATION;
	strncpy(a.u.stage.label, label, sizeof(a.u.stage.label) - 1);
	a.u.stage.duration_ms = duration_ms;
	return tq_push(&t->tq, &a);
}

uint8_t Ticket_EnqFinish(ENUM_REF_LAV_T lav, const char *resultado, const char *fallas)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = ACT_FINISH;

    strncpy(a.u.finish.resultado, resultado, sizeof(a.u.finish.resultado)-1);
    strncpy(a.u.finish.fallas,    fallas,    sizeof(a.u.finish.fallas)-1);

    return tq_push(&t->tq, &a);
}

uint8_t Ticket_EnqPrint(ENUM_REF_LAV_T lav)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = ACT_PRINT;

    return tq_push(&t->tq, &a);
}

bool Ticket_NeedsRtcNow(ENUM_REF_LAV_T lav)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction *a;
    if (!tq_peek(&t->tq, &a)) return 0;

    if (!act_needs_rtc(a->type)) return 0;

    volatile bool *rr = get_rtc_ready_ptr(lav);
    if (!rr) return 0;

    return (*rr == 0);
}

// -------------------- ejecución de acciones --------------------
static void cache_from_globals(TicketCtx *t)
{
    strncpy(t->last_fecha, g_fecha, sizeof(t->last_fecha)-1);
    t->last_fecha[sizeof(t->last_fecha)-1] = 0;

    strncpy(t->last_hora, g_hora, sizeof(t->last_hora)-1);
    t->last_hora[sizeof(t->last_hora)-1] = 0;

    t->last_valid = 1;
}

static const char* hora_cached_or_placeholder(TicketCtx *t)
{
    return t->last_valid ? t->last_hora : "--:--:--";
}

static const char* fecha_cached_or_placeholder(TicketCtx *t)
{
    return t->last_valid ? t->last_fecha : "----.--.--";
}

uint8_t Ticket_EnqLockTestResultadoRtc(ENUM_REF_LAV_T lav, const char *resultado)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = ACT_LOCK_TEST_RESULT_RTC;

    // Reutilizo event.msg para guardar el texto ("OK" / "FALLO")
    strncpy(a.u.event.msg, resultado, sizeof(a.u.event.msg)-1);

    return tq_push(&t->tq, &a);
}

uint8_t Ticket_EnqEstanqueidadResultadoRtc(ENUM_REF_LAV_T lav, const char *resultado)
{
    TicketCtx *t = get_ctx(lav);
    if (!t) return 0;

    TicketAction a;
    memset(&a, 0, sizeof(a));
    a.type = ACT_ESTANQ_RESULT_RTC;

    // Reutilizo event.msg para guardar el texto ("OK" / "FALLO")
    strncpy(a.u.event.msg, resultado, sizeof(a.u.event.msg)-1);

    return tq_push(&t->tq, &a);
}

static void exec_action(ENUM_REF_LAV_T lav, TicketCtx *t, const TicketAction *a)
{
    switch (a->type)
    {
        case ACT_HDR_RTC:
            tk_reset_active(t);

            tk_printf(t, "HOSPITAL\r\n");
            tk_printf(t, "FECHA: %s\r\n", fecha_cached_or_placeholder(t));
            tk_printf(t, "MODO: %s\r\n", a->u.hdr.modo);
            tk_append(t, SEP_LINE);
            break;

        case ACT_STEP_START_RTC:
        case ACT_STEP_START_CACHED:
            tk_printf(t, "Inicio: %s\r\n", hora_cached_or_placeholder(t));
            t->step_start_ms = HAL_GetTick();
            break;

        case ACT_STEP_END_RTC:
        case ACT_STEP_END_CACHED: {
            uint32_t dur_ms = HAL_GetTick() - t->step_start_ms;
            char mmss[16]; fmt_mmss(mmss, sizeof(mmss), dur_ms);
            tk_printf(t, "Duracion: %s\r\n", mmss);
            tk_printf(t, "Finalizacion: %s\r\n", hora_cached_or_placeholder(t));
			tk_append(t, SEP_LINE);
            break;
        }

        case ACT_EVENT_RTC:
        case ACT_EVENT_CACHED:
            tk_printf(t, "*** %s (%s) ***\r\n", a->u.event.msg, hora_cached_or_placeholder(t));
            break;

		case ACT_STAGE_DURATION: {
			uint32_t total_seconds = a->u.stage.duration_ms / 1000;
			uint32_t minutes = total_seconds / 60;
			uint32_t seconds = total_seconds % 60;
			tk_printf(t, "%s: %lu min %02lu s\r\n",
					a->u.stage.label,
					(unsigned long)minutes,
					(unsigned long)seconds);
			break;
		}

        case ACT_LOCK_TEST_RESULT_RTC:
        	tk_printf(t, "P. Bloqueo (resultado): %s (%s)\r\n", a->u.event.msg, hora_cached_or_placeholder(t));
            break;

        case ACT_ESTANQ_RESULT_RTC:
            tk_printf(t, "Estanqueidad (resultado): %s (%s)\r\n", a->u.event.msg, hora_cached_or_placeholder(t));
            tk_append(t, SEP_LINE);
            break;

        case ACT_FINISH:
            tk_append(t, SEP_LINE);
            tk_printf(t, "Resultado: %s\r\n", a->u.finish.resultado);
            tk_printf(t, "Fallas: %s\r\n", a->u.finish.fallas);
            break;

        case ACT_PRINT: {
            uint16_t copy_len = t->len;
            if (copy_len >= TICKET_BUF_SZ) copy_len = (uint16_t)(TICKET_BUF_SZ - 1);

            memcpy(t->print_buf, t->buf, copy_len);
            t->print_buf[copy_len] = '\0';
            t->print_len = copy_len;
            t->print_pending = 1;

            tk_reset_active(t);
            tk_reset_queue(t);
            break;
        }

        default:
            break;
    }
}

static void Ticket_LoopOne(ENUM_REF_LAV_T lav, TicketCtx *t)
{
    TicketAction *a;
    if (!tq_peek(&t->tq, &a)) return;

    volatile bool *rtc_ready = get_rtc_ready_ptr(lav);
    if (!rtc_ready) return;

    // Si la acción requiere RTC, esperar a que llegue el rtc_ready
    if (act_needs_rtc(a->type)) {
        if (!(*rtc_ready)) return;

        // g_fecha/g_hora tienen datos del RTC recién recibido.
        cache_from_globals(t);
    }

    exec_action(lav, t, a);

    // Consumir RTC solo si se usó
    if (act_needs_rtc(a->type)) {
        *rtc_ready = 0;
    }

    tq_pop(&t->tq);
}

void Ticket_LoopAll(void)
{
    Ticket_LoopOne(REPRO1, &tk_L1);
}

// -------------------- impresión serializada --------------------
static void Ticket_PrintSnapshot(const TicketCtx *t)
{
    Reset_Printer();
    Formatear(0, 0x00);
    CSN_A2_print(t->print_buf);

    for (int i = 0; i < 5; i++) {
        CSN_A2_write(0x0D);
        CSN_A2_write(0x0A);
    }
    Corte_Papel_Parcial();
}

void Ticket_ServicePrintAll(void)
{
    uint32_t now = HAL_GetTick();

    if ((int32_t)(now - cooldown_until_ms) < 0) return;
    if (printer_busy) return;

    TicketCtx *to_print = NULL;

    if (tk_L1.print_pending) to_print = &tk_L1;
    else return;

    printer_busy = 1;

    Ticket_PrintSnapshot(to_print);
    to_print->print_pending = 0;

    printer_busy = 0;
    cooldown_until_ms = HAL_GetTick() + PRINT_COOLDOWN_MS;
}
