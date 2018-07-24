/*
**************************************************************************************************************
*	@file		HDC1080.c
*	@author 	Jason_531@163.com
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#ifndef __HDC1080_H
#define __HDC1080_H

#include <stdint.h>
#include "debug.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
 *	InitHdc1080:		��ʼ��InitHdc1080��ַ
 *	����ֵ: 			
 */
void InitHdc1080(void);

static void I2C_Hdc1080_Error(void);

/*
 *	Hdc1080ReadTH:	��ȡHdc1080�Ŀ�����ʪ��
 *	����ֵ: 			���նȣ��Ŵ�100��
 */
void Hdc1080ReadTH(int32_t *measure);
		
uint8_t Hdc1080_WriteOneByte(uint8_t reg, uint8_t data[2]);
		
uint16_t Hdc1080ReadOneByte(uint8_t reg);

#ifdef __cplusplus
}
#endif

#endif
