/*
**************************************************************************************************************
*	@file		SI7021.c
*	@author 	Jason_531@163.com
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#ifndef __SI7021_H
#define __SI7021_H

#include <stdint.h>
#include "debug.h"
#include "sensor.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
 *	InitSi7021:		初始化SI7021地址
 *	返回值: 			
 */
void InitSi7021(void);

/*
 *	Si7021:	读取SI7021的光照度数据
 *	返回值: 			光照度，放大100倍
 */
void Si7021ReadTH(int32_t *measure);
		
static void I2C_SI7021_Error (void);		
		
uint8_t Si7021Configtrue(uint8_t reg);		
		
uint8_t Si7021WriteOneByte(uint8_t *data, uint8_t number);
		
uint16_t Si7021ReadOneByte(uint8_t reg);
		
static uint8_t Si7021CheckCrc(uint8_t data[]);

#ifdef __cplusplus
}
#endif

#endif
