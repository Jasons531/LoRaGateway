/*
**************************************************************************************************************
*	@file	sht2x.h
*	@author 
*	@version 
*	@date    
*	@brief	用软件模拟I2C读取SHT2X的温度和湿度数据，因为STM32的I2C有点BUG，不大好用
***************************************************************************************************************
*/
#ifndef __SHT2X_H
#define __SHT2X_H

#include "stm32l0xx_hal.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
 *	Sht2xReadTH:	读取STH2x的温度和湿度数据
 *	measure：		存储温度和湿度数据，放大了10倍
 */
void Sht2xReadTH(int32_t *measure);
	

#ifdef __cplusplus
}
#endif

#endif
