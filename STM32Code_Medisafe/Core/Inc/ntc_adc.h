/*
 * ntc_adc.h
 *
 *  Created on: Jan 28, 2026
 *      Author: ADOX
 */

#ifndef INC_NTC_ADC_H_
#define INC_NTC_ADC_H_

#include "stdint.h"

//---> Función para leer ADC y calcular temperatura.
void NTC_ADC_Loop();

//---> Lavadora 1
extern uint16_t L1_ntc_adc_value;
extern float L1_ntc_temperature;

//---> Lavadora 2
extern uint16_t L2_ntc_adc_value;
extern float L2_ntc_temperature;

#endif /* INC_NTC_ADC_H_ */
