/*
**************************************************************************************************************
*	@file	timeout-app.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	timer处理函数
***************************************************************************************************************
*/

#include "timer-app.h"
#include "rtimer.h"
#include "rtimer-arch.h"
#include "stm32l0xx_hal.h"
#include  <stdint.h>
#include  <stdbool.h>
#include  <string.h>

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */


#define SECOND    1000 /* 1kHz */

/**
 * @addtogroup  Timer for delay and trace timeout.
 * @{
 */

/**
  * @brief  Delay a few of millisecond.
  * @note  This function ONLY delay a few of millisecond since that waiting and loop 
  *            would consumes CPU.
  * @note  Be careful of overflow that capability of "uint16_t" is 65535, so the MAX count
  *            of rtimer equal=(65535 / RTIMER_ARCH_SECOND) seconds.
  * @param  wMs: count of millisecond.
  * @retval  None
  */
void TimerDelayMs(uint16_t wMs)
{
    uint16_t    wCount;
    rtimer_clock_t    tStart;

    /* Conver millisecond to Hz of rtimer */
    wCount = (uint32_t)wMs * SECOND / 1000;

    tStart = RTIMER_NOW();

    while (RTIMER_NOW() - tStart < wCount) ;	
    return;
}

/**
  * @brief  Initialize the timer that trace Tx or Rx of radio.
  * @param  Callback: point to callback function by this pointer, this callback would
  *              been invoked when the timer expired.
  * @retval  None
  */
void SX1278InitTimer(struct rtimer *t, void (*Callback)(void))
{
    /* Save this callback function */
    t->func = (rtimer_callback_t)Callback;

    return;
}

/**
  * @brief  Set as well as start the timer that trace Tx or Rx of radio.
  * @note  Be careful of overflow that capability of "uint16_t" is 65535, so the MAX count
  *            of rtimer equal=(65535 * 10000 / RTIMER_ARCH_SECOND).  
  * @param  wMs: count of milliseconds of timeout for this timer.
  * @retval  None
  */
void SX1278StartTimer(struct rtimer *t, uint16_t wMs)
{
    uint16_t    wCount;

    /* Conver millisecond to Hz of rtimer */
    wCount = (uint32_t)wMs * SECOND / 1000;

    /* Set rtimer as well as start it! */
    rtimer_set(t, RTIMER_NOW() + wCount, 0, t->func, NULL);

    return;
}

/**
  * @brief  Stop the timer that trace Tx or Rx of radio.
  * @param  None.
  * @retval  None
  */
void SX1278StopTimer(void)
{
    rtimer_reset();

    return;
}

/*--------------------------------------------------------------------------
                                                     0ooo
                                          ooo0      (   )
                                          (   )      ) /
                                           \ (      (_/
                                            \_)
----------------------------------------------------------------------------*/