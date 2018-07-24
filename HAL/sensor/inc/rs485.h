/*
**************************************************************************************************************
*	@file	sensor.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	��ȡ����������
***************************************************************************************************************
*/
#ifndef __RS485_H
#define __RS485_H

#include <stdint.h>
#include "gpio.h"

#ifdef __cplusplus
	extern "C" {
#endif
 
typedef struct rs485Command
{
	uint8_t* data;				//�������Լ���������ָ�룬���ݷ���ʱ�����Ǵ���������
	uint8_t data_len;			//���ݳ���
	uint8_t recive_len;			//�����������ݳ���
	uint16_t recive_timeout;	//���ճ�ʱʱ��
}Rs485Command;

#define RECIVE_TIMEOUT	0
#define EXCUTE_OK		1
#define CRC_ERROR		2
#define TURN_OFF_RS485() 	// �ض�RS485�������Դ������͹���ģʽ

void InitUsart5(void);

/*
 *	InitRs:			��ʼ��RS������
 *	������			��
 *	����ֵ��		��	
 */
void InitRs(void);

void InitPowerPin(void);

/*
 *	Enble485Power:	ʹ��485��Դ����
 *	������			��
 *	����ֵ��		��	
 */
void Enble485Power(void);

/*
 *	Disable485Power:	ʹ��485��Դ����
 *	������			��
 *	����ֵ��		��	
 */
void Disable485Power(void);

/*
 *	ExcuteRs485Command:		��ѯ485����������
 *	command��				Rs485_Command�ṹ�壬���ؽ��յ������ݽ����Ƿ��͵�data������
 *	����ֵ��				EXCUTE_OK|CRC_ERROR|RECIVE_TIMEOUT
 */
int8_t ExcuteRs485Command(Rs485Command command);

#ifdef __cplusplus
}
#endif

#endif
