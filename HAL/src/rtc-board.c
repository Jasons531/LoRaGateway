/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdbool.h>
#include "utilities.h"
#include "board.h"
#include "rtc-board.h"

RTC_HandleTypeDef RtcHandle;

/*!
 * RTC timer context
 */
typedef struct RtcCalendar_s
{
    uint16_t CalendarCentury;     //! Keep track of century value
    RTC_DateTypeDef CalendarDate; //! Reference time in calendar format
    RTC_TimeTypeDef CalendarTime; //! Reference date in calendar format
} RtcCalendar_t;


/***********************************以下为RTC底层代码部分***************************************/


/* RTC init function */
void RtcInit(void)
{
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

    /**Initialize RTC Only 
    */
  RtcHandle.Instance = RTC;
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = 127;
  RtcHandle.Init.SynchPrediv = 255;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    Error_Handler( );
  }

    /**Initialize RTC and set the Time and Date 
    */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;
  sTime.TimeFormat = RTC_HOURFORMAT12_AM;    
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler( );
  }

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&RtcHandle, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler( );
  }
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(RTC_IRQn);

  }
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

/*******************************以上为RTC底层代码部分****************************************/

/*SetRtcAlarm：设置RTC闹钟休眠唤醒
*
*/
void SetRtcAlarm(uint16_t time)
{
	RTC_AlarmTypeDef RTC_AlarmStructure;
	RTC_TimeTypeDef  RTC_TimeStruct;
	RTC_DateTypeDef  RTC_DateStruct;
 
    HAL_RTC_WaitForSynchro(&RtcHandle);
    
    HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
    HAL_RTCEx_DeactivateWakeUpTimer( &RtcHandle );
    
    HAL_NVIC_DisableIRQ(RTC_IRQn);

	HAL_RTC_GetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RtcHandle, &RTC_DateStruct, RTC_FORMAT_BIN);
	DEBUG(2,"currenttime hour : %d min : %d second : %d\r\n",RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);

	RTC_AlarmStructure.AlarmTime.Seconds = (RTC_TimeStruct.Seconds+time)%60;  
	RTC_AlarmStructure.AlarmTime.Minutes = (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)%60;
	RTC_AlarmStructure.AlarmTime.Hours = (RTC_TimeStruct.Hours + (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)/60)%24;
	RTC_AlarmStructure.AlarmTime.SubSeconds = 0;
	
    DEBUG(2,"wkuptime    hour : %d min : %d second : %d\r\n",RTC_AlarmStructure.AlarmTime.Hours,RTC_AlarmStructure.AlarmTime.Minutes,RTC_AlarmStructure.AlarmTime.Seconds);

    RTC_AlarmStructure.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
    RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;//RTC_ALARMMASK_NONE;为精准匹配
    RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	RTC_AlarmStructure.Alarm = RTC_ALARM_A;
    	
	if( HAL_RTC_SetAlarm_IT( &RtcHandle, &RTC_AlarmStructure, RTC_FORMAT_BIN ) != HAL_OK )
	{
        assert_param( FAIL );
	}
    
    HAL_NVIC_EnableIRQ(RTC_IRQn);
}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
