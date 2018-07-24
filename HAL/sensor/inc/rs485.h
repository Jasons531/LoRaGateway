/*
**************************************************************************************************************
*	@file	sensor.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	读取传感器数据
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
	uint8_t* data;				//待发送以及返回数据指针，数据返回时将覆盖待发送数据
	uint8_t data_len;			//数据长度
	uint8_t recive_len;			//期望接收数据长度
	uint16_t recive_timeout;	//接收超时时间
}Rs485Command;

#define RECIVE_TIMEOUT	0
#define EXCUTE_OK		1
#define CRC_ERROR		2
#define TURN_OFF_RS485() 	// 关断RS485总线与电源，进入低功耗模式

void InitUsart5(void);

/*
 *	InitRs:			初始化RS传感器
 *	参数：			无
 *	返回值：		无	
 */
void InitRs(void);

void InitPowerPin(void);

/*
 *	Enble485Power:	使能485电源引脚
 *	参数：			无
 *	返回值：		无	
 */
void Enble485Power(void);

/*
 *	Disable485Power:	使能485电源引脚
 *	参数：			无
 *	返回值：		无	
 */
void Disable485Power(void);

/*
 *	ExcuteRs485Command:		查询485传感器数据
 *	command：				Rs485_Command结构体，返回接收到的数据将覆盖发送的data数据域
 *	返回值：				EXCUTE_OK|CRC_ERROR|RECIVE_TIMEOUT
 */
int8_t ExcuteRs485Command(Rs485Command command);

#ifdef __cplusplus
}
#endif

#endif
