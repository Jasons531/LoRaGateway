/**
 * \file
 *         Real-timer header file for STM8L151C8.
 * \author
 *         JiangJun <jiangjunjie_2005@126.com>
 */

#ifndef __RTIMER_ARCH_H__
#define __RTIMER_ARCH_H__

#include "contiki-conf.h"
#include "rtimer.h"

#define RTIMER_ARCH_SECOND    1000 /* 1kHz */


/**
 * \brief    Initialize the real timer
 * \return  none
 */
void rtimer_arch_init(void);

/**
 * \brief    Get the current clock time
 * \return  The current time
 *
 *            This function returns what the real-time module thinks
 *            is the current time. The current time is used to set
 *            the timeouts for real-time tasks.
 *
 * \hideinitializer
 */
rtimer_clock_t rtimer_arch_now(void);

/**
 * \brief      Set an interrupt to timeout event.
 * \param    rtimer_clock_t t    the quantity of timeout, unit is ms.
 *
 *              This function schedules a real-time task at a specified
 *              time in the future.
 *
 */
void rtimer_arch_schedule(rtimer_clock_t t);

/**
 * \brief    Disable interrupt of rtimer timeout.
 *
 *            This function disable interrupt of real timer for removing timer.
 */
void rtimer_arch_disable_irq(void);

/**
 * \brief    Enable interrupt of rtimer timeout.
 *
 *            This function enable interrupt of real timer for restarting timer.
 */
extern void rtimer_arch_enable_irq(void);

/**
 * \brief      turn on the real timer.
 * \param    none
 *
 *              This function would turn on the real timer.
 */
void rtimer_arch_TurnOn(void);

/**
 * \brief      turn off the real timer.
 * \param    none
 *
 *              This function would turn off the real timer for saved energy.
 */
void rtimer_arch_TurnOff(void);


#endif

/*--------------------------------------------------------------------------------------------------------
                   									     0ooo
                   								ooo0     (   )
                								(   )     ) /
                								 \ (     (_/
                								  \_)
----------------------------------------------------------------------------------------------------------*/
