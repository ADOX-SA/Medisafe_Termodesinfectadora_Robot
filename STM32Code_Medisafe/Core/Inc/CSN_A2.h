#include "stm32f1xx_hal.h"
#include <stddef.h>
#include <stdint.h>

void Inicializar_CSN_A2(void);
void CSN_A2_write(uint8_t b);
void CSN_A2_write_buf(const uint8_t *buf, size_t n);
void CSN_A2_print(const char *s);

void Reset_Printer(void);
void Formatear(uint8_t Justificado, uint8_t Tamanio_Letra);

void Imprimir_Texto(const char *buf, int N);
void Imprimir_Codigo(const char *data, uint8_t disposicion);

void Corte_Papel_Parcial(void);
