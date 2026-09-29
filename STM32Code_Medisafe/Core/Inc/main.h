/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

// Elegí el puerto serial:
/**
 * Elegir puerto para pantalla Nextion.
 * Si se comenta la linea: #define USE_HUART1
 * Se usa el puerto &huart3
 * */
//#define NEXTION_USE_HUART1    					// comentar o descomentar para elegir puerto

#ifdef NEXTION_USE_HUART1
    #define NEXTION_PORT &huart1			// TTL Nextion
#else
    #define NEXTION_PORT &huart3			// RS232
#endif

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADC2_Pin GPIO_PIN_1
#define ADC2_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_2
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_3
#define RS485_RX_GPIO_Port GPIOA
#define RS485_DIR_Pin GPIO_PIN_4
#define RS485_DIR_GPIO_Port GPIOA
#define ADC1_Pin GPIO_PIN_5
#define ADC1_GPIO_Port GPIOA
#define RFID_D1_Pin GPIO_PIN_7
#define RFID_D1_GPIO_Port GPIOA
#define RFID_D1_EXTI_IRQn EXTI9_5_IRQn
#define R_IN6_Pin GPIO_PIN_0
#define R_IN6_GPIO_Port GPIOB
#define R_IN5_Pin GPIO_PIN_1
#define R_IN5_GPIO_Port GPIOB
#define ADC1B2_Pin GPIO_PIN_2
#define ADC1B2_GPIO_Port GPIOB
#define RS232_TX_Pin GPIO_PIN_10
#define RS232_TX_GPIO_Port GPIOB
#define RS232_RX_Pin GPIO_PIN_11
#define RS232_RX_GPIO_Port GPIOB
#define CS1_Pin GPIO_PIN_12
#define CS1_GPIO_Port GPIOB
#define CS2_Pin GPIO_PIN_15
#define CS2_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOA
#define RFID_D0_Pin GPIO_PIN_11
#define RFID_D0_GPIO_Port GPIOA
#define RFID_D0_EXTI_IRQn EXTI15_10_IRQn
#define TX_IMPRE_Pin GPIO_PIN_12
#define TX_IMPRE_GPIO_Port GPIOA
#define R_IN4_Pin GPIO_PIN_15
#define R_IN4_GPIO_Port GPIOA
#define R_IN3_Pin GPIO_PIN_3
#define R_IN3_GPIO_Port GPIOB
#define R_IN2_Pin GPIO_PIN_4
#define R_IN2_GPIO_Port GPIOB
#define R_IN1_Pin GPIO_PIN_5
#define R_IN1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
