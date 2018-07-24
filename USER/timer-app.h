/*
**************************************************************************************************************
*	@file	timer-app.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	timer´¦Àí¿â
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_APP_H
#define __TIMER_APP_H

#include <stdint.h>
#include "rtimer.h"

void TimerDelayMs(uint16_t wMs);

void SX1278InitTimer(struct rtimer *t, void (*Callback)(void));

void SX1278StartTimer(struct rtimer *t, uint16_t wMs);

void SX1278StopTimer(void);



#endif /* __USER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
