/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_tim.h"
#include "utilities.h"
#include "timerserver.h"
#include "iwdg.h"
#include "delay.h"
#include "gpio.h"
#include "gpio-board.h"
#include "spi.h"
#include "i2c.h"
#include "adc.h"
#include "rtc-board.h"
#include "usart.h"
#include "radio.h"
#include "gps.h"
#include "sensor.h"
#include "user-app.h"
#include "stmflash.h"
#include "user_bq24195.h"
#include "bq24195.h"
#include "power.h"
#include "debug.h"
#include "sx1276/sx1276.h"
#include "rtc-board.h"
#include "timer-board.h"
#include "sx1276-board.h"
#include "LoRaMac-api-v3.h"
#include "LoRa-cad.h"

#if defined( USE_USB_CDC )
#include "uart-usb-board.h"
#endif

/*!
 * Define indicating if an external IO expander is to be used
 */
#define BOARD_IOE_EXT

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif


/*!
 * Unique Devices IDs register set ( STM32F1xxx )
 */
#define         ID1                                 ( 0x1FFFF7E8 )
#define         ID2                                 ( 0x1FFFF7EC )
#define         ID3                                 ( 0x1FFFF7F0 )

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   ( ( *( uint32_t* )ID1 ) ^ \
                                                      ( *( uint32_t* )ID2 ) ^ \
                                                      ( *( uint32_t* )ID3 ) )


/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 PA_2

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PB_0

#define RADIO_DIO_0                                 PB_5//PB_1
#define RADIO_DIO_1                                 PA_0//PB_10
#define RADIO_DIO_2                                 PB_11
#define RADIO_DIO_3                                 PB_7
#define RADIO_DIO_4                                 PB_5
#define RADIO_DIO_5                                 PB_4


enum BoardPowerSource
{
    USB_POWER = 0,
    BATTERY_POWER
};

/*!
 * System Clock Configuration
 */
void SystemClockConfig( void );

void SystemClockReConfig( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief Sleeep mode.
 */
void BoardSleep( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level ( 0: very low, 254: fully charged )
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

#endif // __BOARD_H__
