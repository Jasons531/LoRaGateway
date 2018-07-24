/*
**************************************************************************************************************
*	@file	timer-board.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	Description: MCU timer
***************************************************************************************************************
*/


#ifndef __TIMER_BOARD_H__
#define __TIMER_BOARD_H__

#include <stdint.h>
#include "board.h"

/*!
 * \brief Timer time variable definition
 */
#ifndef TimerTime_t
typedef uint32_t TimerTime_t;
#endif

#define TIMx                         TIM2
#define TIMx_CLK_ENABLE              __TIM2_CLK_ENABLE
#define TIMx_CLK_DISABLE             __TIM2_CLK_DISABLE

/* Definition for TIMx's NVIC */
#define TIMx_IRQn                    TIM2_IRQn
#define TIMx_IRQHandler             TIM2_IRQHandler


extern TIM_HandleTypeDef htim2;
extern volatile uint64_t TimerTickCounter;
extern volatile uint64_t TimeoutCntValue;


/*!
 * \brief Initializes the timer
 *
 * \remark The timer is based on TIM2 with a 10uS time basis
 */
void TimerHwInit( void );

/*!
 * \brief DeInitializes the timer
 */
void TimerHwDeInit( void );

/*!
 * \brief Return the minimum timeout the Timer is able to handle
 *
 * \retval minimum value for a timeout
 */
uint32_t TimerHwGetMinimumTimeout( void );

/*!
 * \brief Start the Standard Timer counter
 *
 * \param [IN] rtcCounter Timer duration
 */
void TimerHwStart( uint32_t rtcCounter );

/*!
 * \brief Perfoms a standard blocking delay in the code execution
 *
 * \param [IN] delay Delay value in ms
 */
void TimerHwDelayMs( uint32_t delay );

/*!
 * \brief Stop the the Standard Timer counter
 */
void TimerHwStop( void ); 

TimerTime_t TimerHwGetTime( void );

/*!
 * \brief Return the value on the timer counter
 */
uint64_t TimerHwGetTimerValue( void );

/*!
 * \brief Return the value on the timer Tick counter
 */
uint64_t TimerHwGetElapsedTime( void );

/*!
 * \brief Set the ARM core in Wait For Interrupt mode (only working if Debug mode is not used)
 */

void TimerHwEnterLowPowerStopMode( void );

/*!
 * \brief increment Tick counter
 */
void TimerIncrementTickCounter( void );

#endif // __TIMER_BOARD_H__
