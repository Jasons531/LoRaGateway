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
 *	InitMax44009:		��ʼ��max44009��ַ
 *	����ֵ: 			
 */
void InitMax44009(void);

/*
 *	Max44009ReadLux:	��ȡMax44009�Ĺ��ն�����
 *	����ֵ: 			    ���նȣ��Ŵ�100��
 */
int32_t Max44009ReadLux(void);
		
static void I2C_Max44009_Error (void);


#ifdef __cplusplus
}
#endif

#endif
