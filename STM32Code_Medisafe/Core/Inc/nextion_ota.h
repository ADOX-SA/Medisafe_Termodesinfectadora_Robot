#ifndef INC_NEXTION_OTA_H_
#define INC_NEXTION_OTA_H_

#include "main.h"
#include <stdbool.h>

/* Protocolo Node -> STM32 por USART1; la HMI se programa por USART3. */
void nextion_ota_init(void);
void nextion_ota_rx_callback(UART_HandleTypeDef *huart);
void nextion_ota_restart_rx(void);
void nextion_ota_runcycle(void);
bool nextion_ota_is_active(void);

#endif /* INC_NEXTION_OTA_H_ */
