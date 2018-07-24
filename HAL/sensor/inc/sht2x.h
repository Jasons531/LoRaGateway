/*
**************************************************************************************************************
*	@file	sht2x.h
*	@author 
*	@version 
*	@date    
*	@brief	�����ģ��I2C��ȡSHT2X���¶Ⱥ�ʪ�����ݣ���ΪSTM32��I2C�е�BUG���������
***************************************************************************************************************
*/
#ifndef __SHT2X_H
#define __SHT2X_H

#include "stm32l0xx_hal.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*
 *	Sht2xReadTH:	��ȡSTH2x���¶Ⱥ�ʪ������
 *	measure��		�洢�¶Ⱥ�ʪ�����ݣ��Ŵ���10��
 */
void Sht2xReadTH(int32_t *measure);
	

#ifdef __cplusplus
}
#endif

#endif
