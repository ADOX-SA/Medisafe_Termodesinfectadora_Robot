/*
 * watchdog.c
 *
 *  Created on: Jan 7, 2025
 *      Author: ADOX
 */

#include "watchdog.h"
#include "main.h"

extern IWDG_HandleTypeDef hiwdg;
//extern WWDG_HandleTypeDef hwwdg;

uint8_t wwdg_counter=0;

void watchdog_refresh(){
	//---> Refrescar IWDG.
	HAL_IWDG_Refresh(&hiwdg);

	//---> Refrescar WWDG.
	// En caso de usarlo activarlo en .ioc y descomentar estas lineas.
	//uint8_t wwdg_value_now = (WWDG->CR & 0x7F);  // Lee el valor actual del contador
	//if (wwdg_value_now > WWDG_MIN && wwdg_value_now < WWDG_MAX) {
	//    HAL_WWDG_Refresh(&hwwdg); // Refresca el contador con un valor permitido
	//}

}

//WWDG interrupt
//void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg) {
//	wwdg_counter++;
//}

