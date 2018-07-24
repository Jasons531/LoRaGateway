/*
**************************************************************************************************************
*	@file	gpio.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#ifndef __GPIO_H__
#define __GPIO_H__

#include "board.h"

#define  LORA_IO				GPIOB
#define  LORA_REST_PIN		    GPIO_PIN_0
#define  LORA_POWER_ON		    GPIO_PIN_12
#define  LORA_DIO0				GPIO_PIN_1
#define  LORA_DIO1              GPIO_PIN_2
#define  LORA_DIO2              GPIO_PIN_10


#define  LORA_LED			    GPIOA
#define  LORA_LED_PIN			GPIO_PIN_15


#define  GPS_IO					GPIOB
#define  GPS_Power_ON           GPIO_PIN_7


#define  UART_485_IO			GPIOB
#define  UART_485_DE			GPIO_PIN_5

#define  POWER_IO				GPIOA
#define  POWER_12V_ON           GPIO_PIN_8

#define  USART1_IO				GPIOA
#define  USART1_TX				GPIO_PIN_9
#define  USART1_RX				GPIO_PIN_10

#define  USART2_IO				GPIOA
#define  USART2_TX				GPIO_PIN_2
#define  USART2_RX				GPIO_PIN_3

#define  USART5_IO				GPIOB
#define  USART5_TX				GPIO_PIN_3
#define  USART5_RX				GPIO_PIN_4

#define  I2C2_IO				GPIOB
#define  I2C2_SCL			    GPIO_PIN_13
#define  I2C2_SDA				GPIO_PIN_14

#define  GPS_IO					GPIOB
#define  GPS_IO_PIN				GPIO_PIN_7


/*!
 * \brief GPIO初始化
 * \param SX1276 RESET引脚初始化---PB13
 * \retval None
 */
void SX1276GPIO_Init(void);

void SenSor_Close(void);

/**
  * @brief GPIO初始化
  * @param  LoRa电源控制  HOST_2_Lora_DFU_EN引脚初始化---PB12 
  * @retval None
  */
void LoRaPower_Init(void);

/*!
 * \brief GPIO IRQ Initialization
 *
 */
void SX1276EXTI_Init(void);


/*!
 * \brief Writes the given value to the GPIO output
 *
 * \param [IN] obj   Pointer to the GPIO object
 * \param [IN] value New GPIO output value
 */
void GpioWrite( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState );

/*!
 * \brief Toggle the value to the GPIO output
 *
 * \param [IN] obj   Pointer to the GPIO object
 */
void GpioToggle( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin );

/*!
 * \brief Reads the current GPIO input value
 *
 * \param [IN] obj Pointer to the GPIO object
 * \retval value   Current GPIO input value
 */
uint32_t GpioRead( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin );

#endif // __GPIO_H__
