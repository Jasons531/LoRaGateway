/**
* \file
*			Real-timer specific implementation for STM8L151C8.
* \author
*			JiangJun <jiangjunjie_2005@126.com>
*/

#include "sys/energest.h"
#include "sys/rtimer.h"
#include "rtimer-arch.h"
#include "stm32l0xx_hal.h"
#include "timer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

TIM_HandleTypeDef htim2;

#define RTIMER_ARCH_SECOND        10000 /* 10kHz(100us)*/

/*---------------------------------------------------------------------------*/
/**
 * Handle interrupt of real time timer.
 *
 * This function was invoked when timer of real time is expired.
 *
 */
void
rtimer_irq_handler(void)
{
	/* Clear interrupt pending bit */
	__HAL_TIM_CLEAR_IT(&htim2, TIM_IT_CC1);
	
    rtimer_arch_disable_irq();
    rtimer_run_next();
    return;	
}


/*---------------------------------------------------------------------------*/
/**
 * Initialize timer for real time timer.
 *
 * This function would initialize TIM1 for real time timer.
 *
 */
void
rtimer_arch_init(void)
{
   /* TIM1 configuration: (1+prescaler)/16mhz * (1+period) = 6.5535sÒ»´ÎÖÐ¶Ï
   */
	
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
	
  uint16_t psc=(uint16_t)(SystemCoreClock  / RTIMER_ARCH_SECOND) -1;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = psc;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

  HAL_TIM_OC_Init(&htim2);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0xFFFF;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1); 

  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 4, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
  
  return;
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
rtimer_clock_t
rtimer_arch_now(void)
{	
	rtimer_clock_t    tT1, tT2;
	
	do
    {
          /* Avoid race condition on reading counter of TIM1 */
        tT1 = __HAL_TIM_GetCounter(&htim2);
        tT2 = __HAL_TIM_GetCounter(&htim2);
     } while (tT1 != tT2);
  
    return tT1;
}

/**
 * \brief      Set an interrupt to timeout event.
 * \param    rtimer_clock_t t    the quantity of timeout, unit is ms.
 *
 *              This function schedules a real-time task at a specified
 *              time in the future.
 */
void
rtimer_arch_schedule(rtimer_clock_t t)
{
    /* Sets the TIM1 Capture Compare1 Register value */
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, (uint16_t)t);

    /* MUST clear the remained flag of TIM1 compare */
	 __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_CC1);

    /* Enable interrupt of Capture compare 1 */
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);
    return;
}

/**
 * \brief    Disable interrupt of rtimer timeout.
 *
 *            This function disable interrupt of real timer for removing timer.
 */
void
rtimer_arch_disable_irq(void)
{  	
	__HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);

    return;
}

/**
 * \brief    Enable interrupt of rtimer timeout.
 *
 *            This function enable interrupt of real timer for restarting timer.
 */
void
rtimer_arch_enable_irq(void)
{  
	/* MUST clear the remained flag of TIM1 compare */	
	 __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_CC1);

	/* Enable interrupt of Capture compare 1 */
	 __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);

    return;
}

/**
 * \brief      turn off the real timer.
 * \param    none
 *
 *              This function would turn off the real timer for saved energy.
 */
void
rtimer_arch_TurnOff(void)
{	
	 /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

    return;
}

/**
 * \brief      turn on the real timer.
 * \param    none
 *
 *              This function would turn on the real timer.
 */
void
rtimer_arch_TurnOn(void)
{
    /* Enable TIM2 CLK */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Enable TIM2 */
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    return;
}


/*--------------------------------------------------------------------------------------------------------
                   									     0ooo
                   								ooo0     (   )
                								(   )     ) /
                								 \ (     (_/
                								  \_)
----------------------------------------------------------------------------------------------------------*/

