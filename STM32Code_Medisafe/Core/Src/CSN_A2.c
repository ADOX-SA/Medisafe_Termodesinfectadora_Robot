//#include <Arduino.h>
#include "main.h"
#include "CSN_A2.h"
#include <string.h>
#include "softserial.h"

static SoftUartTx_t CSN_A2;

uint8_t reg_enter = 0x0A;
uint8_t reg_reset[] = {0x1B, 0x40}; 
uint8_t reg_180[] = {0x1B, 0x7B, 0x01}; // terminado en 0x01 gira 180
uint8_t reg_corte[] = {0x1D, 0x56, 0x01};	// corte parcial
// disposicion de los datos en el codigo de barras:

void Inicializar_CSN_A2(void)
{
    //CSN_A2.begin(115200);

	// PB13 como GPIO Output PushPull en CubeMX:
	// GPIOB Pin 13 -> Output Push Pull, High speed, No pull
	SoftUartTx_Begin(&CSN_A2, TX_IMPRE_GPIO_Port, TX_IMPRE_Pin, 9600);
}

void CSN_A2_write(uint8_t b)
{
    SoftUartTx_WriteByte(&CSN_A2, b);
}

void CSN_A2_write_buf(const uint8_t *buf, size_t n)
{
    SoftUartTx_WriteBuf(&CSN_A2, buf, n);
}

void CSN_A2_print(const char *s)
{
    SoftUartTx_Print(&CSN_A2, s);
}

void Reset_Printer(void)
{
	CSN_A2_write_buf(reg_reset , 2); // borra el buffer de impresion y reinicia la impresora
}

void Formatear(uint8_t Justificado, uint8_t Tamanio_Letra)
{
    // Justificado: 00 -> Izquierdo       Tamaño de Letra: 00 default
    //              01 -> Centrado                         11 titulo
    //              02 -> Derecho

    uint8_t Just[] = { 0x1B , 0x61 , Justificado} ,
            Tam[]  = { 0x1D , 0x21 , Tamanio_Letra};

    CSN_A2_write_buf(Just, 3);
    CSN_A2_write_buf(Tam, 3);
}

void Imprimir_Texto(const char *buf, int N)
{
    //CSN_A2.write(reg_180 , 3);          
    //    delay(10);
	CSN_A2_print(buf);

	for (int i = 0; i < N; i++){
		CSN_A2_write(0x0D); // CR
		CSN_A2_write(reg_enter); // 0x0A // LF
	}
}

void Imprimir_Codigo(const char *data, uint8_t disposicion)
{
    int tam_data = strlen(data);
    uint8_t reg_code39[] = {0x1D, 0x6B, 0x04};
    uint8_t reg_disp[]   = {0x1D, 0x48, disposicion}; // 00 no imprime digitos
    uint8_t reg_ancho[]  = {0x1D, 0x77, 0x02}; // 0x02: 12 digitos imprimibles (Code39)
    uint8_t reg_alto[]   = {0x1D, 0x68, 0x00}; // alto del codigo de barras:

    CSN_A2_write_buf(reg_disp, 3);
    CSN_A2_write_buf(reg_ancho, 3);
    CSN_A2_write_buf(reg_alto, 3);
   
    if (tam_data > 0 && tam_data <= 12)
    {
        for (int i = 0; i < tam_data; i++)
        {
            reg_code39[i + 3] = data[i];
        }
        reg_code39[tam_data + 3] = 0x00; // Agregar el byte final 0x00.

        CSN_A2_write_buf(reg_code39, tam_data + 4);
        
        for (int i = 0; i < 3; i++){
    		CSN_A2_write(reg_enter); // 0x0A
    	}
    }
    CSN_A2_write_buf(reg_reset , 2); // borra el buffer de impresion y reinicia la impresora
}

void Corte_Papel_Parcial(void)
{
    CSN_A2_write_buf(reg_corte, 3);
}
