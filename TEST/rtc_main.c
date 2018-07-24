/*
**************************************************************************************************************
*	@file	rtc_main.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	≤‚ ‘RTC°¢SPI Dome; RTC WKUP Dome
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "debug.h"
#include "usart.h"
#include "rtc-board.h"
#include "timer.h"
#include "timer-app.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

#define 	TESTRTC							 0

#define   CLOCK_16MHZ          1

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_NVIC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */


extern bool LowPowerModeEnable;

extern TimerEvent_t CadTimer;
volatile bool CadTimerEvent = false;

bool test_rtc_state = false;


TimerEvent_t SleepTimer;

void OnSleepTimerTimerEvent( void )
{
	DEBUG(2,"%s\r\n",__func__);

	TimerStop( &SleepTimer );	
	TimerSetValue( &SleepTimer, 5000 );
    TimerStart( &SleepTimer );
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
	
	RTC_DateTypeDef sdatestructureget;
	RTC_TimeTypeDef stimestructureget;
	RTC_AlarmTypeDef sAlarmtructureget;
	GPIO_InitTypeDef GPIO_InitStruct;

	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
	
	__HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE(); ///ø™∆Ù ±÷”

  /* Initialize all configured peripherals */

  /* USER CODE BEGIN 2 */
	
	MX_USART1_UART_Init( );	
	LoRapp_SenSor_States.WKUP_State = false;
	RTC_Init();
	MX_NVIC_Init( );	

	DEBUG(2,"hello world test RTC\r\n");
	TimerHwInit(  );
	TimerInit( &ReportTimer, OnReportTimerEvent );
	TimerInit( &SleepTimer, OnSleepTimerTimerEvent );
	TimerSetValue( &ReportTimer, 1000 ); ///1s
	TimerStart( &ReportTimer );
	TimerSetValue( &SleepTimer, 3000 ); ///1s
	TimerStart( &SleepTimer );
	
	delay_init( 16 );

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  
	/* Get the RTC current Time */

#if		TESTRTC	

	if(!test_rtc_state && ReportTimerEvent)
	{
		test_rtc_state = true;
		LoRapp_SenSor_States.WKUP_State = true;
		RTC_Init();
		MX_NVIC_Init(  );	
		SetRtcAlarm(4);
	}

	
#else
	
	if(ReportTimerEvent == true)
	{
		ReportTimerEvent = false;
		HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
			
		/* Get the RTC current Date */
		HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
		
		HAL_RTC_WaitForSynchro(&RtcHandle);	
		DEBUG(2, "year - %d mouth - %d date - %d, hour : %d min : %d second : %d\r\n",sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date,
					stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);		
	}
	
#endif	
	
 }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /**Initializes the CPU, AHB and APB busses clocks 
  */
	#if CLOCK_16MHZ
	
  /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

	#else
	
	  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /**Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

	/**Configure LSE Drive Capability 
	*/
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

	
 #endif

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 1, 0);
}


/* USER CODE BEGIN 4 */

static void MX_NVIC_Init(void)
{
  /* USART1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(RTC_IRQn);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
		DEBUG(2,"Error_Handler\r\n");
  }
	
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
