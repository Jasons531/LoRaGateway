/*
**************************************************************************************************************
*	@file	timer-board.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	Description: MCU RTC timer and low power modes management
***************************************************************************************************************
*/


#include <math.h>
#include "timer-board.h"


TIM_HandleTypeDef htim2;

/*!
 * Hardware Time base in us
 */
#define HW_TIMER_TIME_BASE                              1 //ms


/*!
 * Hardware Timer tick counter
 */
volatile uint64_t TimerTickCounter = 1;

/*!
 * Saved value of the Tick counter at the start of the next event
 */
static uint64_t TimerTickCounterContext = 0;

/*!
 * Value trigging the IRQ
 */
volatile uint64_t TimeoutCntValue = 0;


uint32_t uwPrescalerValue = 0;

void TimerHwInit( void )
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
   
  if(HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
	/* Starting Error */
	 Error_Handler( );
  }
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{

  if(htim_base->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

  }
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
} 

void TimerHwDeInit( void )
{
    /* Deinitialize the timer */
	HAL_TIM_Base_DeInit(&htim2);
}

uint32_t TimerHwGetMinimumTimeout( void )
{
    return( ceil( HW_TIMER_TIME_BASE ) );
}

void TimerHwStart( uint32_t val )
{
    
    TimerTickCounterContext = TimerHwGetTimerValue( );

    if( val <= HW_TIMER_TIME_BASE + 1 )
    {
        TimeoutCntValue = TimerTickCounterContext + 1;
    }
    else
    {
        TimeoutCntValue = TimerTickCounterContext + ( ( val - 1 ) / HW_TIMER_TIME_BASE );
     
    }
}

void TimerHwStop( void )
{
//    TIM_ITConfig( TIM2, TIM_IT_CC1, DISABLE );
//    TIM_Cmd( TIM2, DISABLE );
    HAL_TIM_Base_Stop_IT(&htim2);
}

extern uint32_t TimerOverTime;

void TimerHwDelayMs( uint32_t delay )
{
    uint64_t timeout = 0;
    uint32_t systick = 0;

    timeout = TimerHwGetTimerValue( );
    systick = TimerOverTime;
 
    ///系统时钟超时10ms：防止Timer2异常,采用10ms原因是TimerHwDelayMs最大延时使用6ms
    while( ((( TimerHwGetTimerValue( ) - timeout  ) * HW_TIMER_TIME_BASE ) < delay) && ((TimerOverTime - systick)<10) )
    {
        __NOP( ); 
//         DEBUG(2,"TimerHwGetTimerValue( )  = %lld delayValue = %lld\r\n",( ( TimerHwGetTimerValue( ) - timeout  ) * HW_TIMER_TIME_BASE ),delayValue);
//       DEBUG(2,"systick  = %d Tick-systick = %d\r\n",systick,Tick-systick);
	}
	TimerOverTime = 0;   
}

uint64_t TimerHwGetElapsedTime( void )
{
     return( ( ( TimerHwGetTimerValue( ) - TimerTickCounterContext ) + 1 )  * HW_TIMER_TIME_BASE );
}

TimerTime_t TimerHwGetTime( void )
{
    return TimerHwGetTimerValue( ) * HW_TIMER_TIME_BASE;
}

uint64_t TimerHwGetTimerValue( void )
{
    uint64_t val = 0;

    __disable_irq( );

    val = TimerTickCounter;

    __enable_irq( );

    return( val );
}

void TimerIncrementTickCounter( void )
{
    __disable_irq( );

    TimerTickCounter++;

    __enable_irq( );
}

void TimerHwEnterLowPowerStopMode( void )
{
//#ifndef USE_DEBUGGER
//    __WFI( );
//#endif
}
