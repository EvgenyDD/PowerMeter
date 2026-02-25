/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define D_DC_Pin GPIO_PIN_13
#define D_DC_GPIO_Port GPIOC
#define RST_EM_Pin GPIO_PIN_14
#define RST_EM_GPIO_Port GPIOC
#define IRQ_ADC_Pin GPIO_PIN_15
#define IRQ_ADC_GPIO_Port GPIOC
#define D_CS_Pin GPIO_PIN_1
#define D_CS_GPIO_Port GPIOF
#define D_CLK_Pin GPIO_PIN_0
#define D_CLK_GPIO_Port GPIOA
#define D_RST_Pin GPIO_PIN_3
#define D_RST_GPIO_Port GPIOA
#define D_DAT_Pin GPIO_PIN_4
#define D_DAT_GPIO_Port GPIOA
#define CS_ADC_Pin GPIO_PIN_5
#define CS_ADC_GPIO_Port GPIOA
#define CS_EM_Pin GPIO_PIN_7
#define CS_EM_GPIO_Port GPIOA
#define IRQ_EM_Pin GPIO_PIN_0
#define IRQ_EM_GPIO_Port GPIOB
#define ZC_Pin GPIO_PIN_1
#define ZC_GPIO_Port GPIOB
#define CF_Pin GPIO_PIN_2
#define CF_GPIO_Port GPIOB
#define ENC_A_Pin GPIO_PIN_10
#define ENC_A_GPIO_Port GPIOB
#define ENC_BTN_Pin GPIO_PIN_11
#define ENC_BTN_GPIO_Port GPIOB
#define ENC_C_Pin GPIO_PIN_12
#define ENC_C_GPIO_Port GPIOB
#define ENC_D_Pin GPIO_PIN_13
#define ENC_D_GPIO_Port GPIOB
#define ENC_B_Pin GPIO_PIN_14
#define ENC_B_GPIO_Port GPIOB
#define D_BL_Pin GPIO_PIN_9
#define D_BL_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
