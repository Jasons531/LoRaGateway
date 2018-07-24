/*
**************************************************************************************************************
*	@file	max44009.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#ifndef __MAX44009_H
#define __MAX44009_H

#include <stdint.h>
#include "debug.h"
#include "sensor.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
 *	InitMax44009:		初始化max44009地址
 *	返回值: 			
 */
void InitMax44009(void);

/*
 *	Max44009ReadLux:	读取Max44009的光照度数据
 *	返回值: 			    光照度，放大100倍
 */
int32_t Max44009ReadLux(void);
		
static void I2C_Max44009_Error (void);


#ifdef __cplusplus
}
#endif

#endif
