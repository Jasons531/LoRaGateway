/*
*Function:
*Programed by:Jason_531@163.com
*Complete date:
*Modified by:
*Modified date:
*Remarks:
*/

#include "stm32l0xx_hal.h"
#include "contiki.h"
#include "sys/clock.h"
#include "sys/cc.h"
#include "sys/etimer.h"
#include "debug-uart.h"

#define RELOAD_VALUE 100000-1    /* 1 ms with a 24 MHz clock */

static volatile clock_time_t current_clock = 0;
static volatile unsigned long current_seconds = 0;
static unsigned int second_countdown = CLOCK_SECOND;
volatile uint32_t TickCounter = 0;

uint32_t TimerOverTime = 0; ///timer2³¬Ê±»úÖÆ

uint32_t Send_time = 0;

void SysTick_Handler(void)
{
  current_clock++;
  if(etimer_pending()) {
    etimer_request_poll();
  }
  if (--second_countdown == 0) {
    current_seconds++;
    second_countdown = CLOCK_SECOND;
  }
  
  TickCounter++;
  TimerOverTime ++;
  Send_time ++;
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}


void
clock_init()
{  
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/CLOCK_SECOND);
    
    /**Configure the Systick 
    */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
      /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

clock_time_t
clock_time(void)
{
  return current_clock;
}

#if 0
/* The inner loop takes 4 cycles. The outer 5+SPIN_COUNT*4. */

#define SPIN_TIME 2 /* us */
#define SPIN_COUNT (((MCK*SPIN_TIME/1000000)-5)/4)

#ifndef __MAKING_DEPS__

void
clock_delay(unsigned int t)
{
#ifdef __THUMBEL__ 
  asm volatile("1: mov r1,%2\n2:\tsub r1,#1\n\tbne 2b\n\tsub %0,#1\n\tbne 1b\n":"=l"(t):"0"(t),"l"(SPIN_COUNT));
#else
#error Must be compiled in thumb mode
#endif
}
#endif
#endif /* __MAKING_DEPS__ */

unsigned long
clock_seconds(void)
{
  return current_seconds;
}
