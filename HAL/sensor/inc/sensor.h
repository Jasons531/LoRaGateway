/*
**************************************************************************************************************
*	@file		sensor.c
*	@author 	Jason_531@163.com
*	@version 	==STRUCT_VERSION
*	@date     	20170527
*	@warming	定义全局USING_RS485宏打开打开rs485传感器和相应的传感器采集，定义WERTHER_STATION宏采集固定3个非扩展盒气象传感器
*	@warming	定义全局SAVE_HISTORY_DATA宏打开存取历史数据功能
*	@brief		传感器采集协议
***************************************************************************************************************
*/
#ifndef __SENSOR_H
#define __SENSOR_H

#include <stdint.h>	
#include <stdbool.h>	
#include "rs485.h"
#include "power.h"
#include "max44009.h"
#include "SI7021.h"
#include "hdc1080.h"


#ifdef __cplusplus
	extern "C" {
#endif	 

#define STRUCT_VERSION				0x03		//传感器结构体协议版本号，如改动以下传感器定义需更新此版本号并在服务器端做适配
#define SENSOR_FIELD				  20		//最多有多少个传感器数据域
		
#define SOCKETS	  					  6		//有多少个485传感器插槽
		
typedef struct
{
	uint8_t structver;						//结构体版本，如果改版结构体需要同时更改结构版本号并在服务器端做适配
	uint8_t count;							//传感器数据域数量
	uint8_t socket_id;				//传感器接入扩展盒ID号
	uint8_t *sockets;				//接口所接的传感器标识
	uint16_t fields[SENSOR_FIELD];			//传感器数据域，根据：电量，光照，空气温度，空气湿度，485传感器（根据接口的传感器标识判断）的顺序存取
}sample_t;


typedef struct
{
	bool Max44009_State;  ///光照传感器接入标志
	bool AirTH_State;   ///空气温湿度传感器接入标志
	uint8_t sensors_buf[6];  ///扩展口数据缓存区
}sensor_char;

extern sensor_char sensors_char;

typedef enum
{
	U16=0,
	S16=1,
	U16D1=2,
	S16D1=3,
	U16D2=4,
	S16D2=5,
	U32D2=0xff
}DataType;

typedef struct
{
	char* fieldName;
	DataType type;
}FieldInfo;

extern sample_t samples;		//全局传感器采样数据结构体变量
extern FieldInfo fieldInfo[];	//全局传感器数据域信息数组

//#define SAMPLE_SIZE	(samples.count*2+SOCKETS+2) ///版本+数量=2； 版本+数量+sockets+samples.count*2(Byte)

extern uint8_t SAMPLE_SIZE;


int8_t QueryRs485Addr(void);

int8_t ControlExtendBox(uint8_t mask);

uint8_t PollingRs485(uint8_t socketIndex);

extern uint32_t RfSend_time;

extern volatile bool Rs485Tag_UNKNOW;


/*
 *	SamplingData:		获取传感器的一次采样结果，数据将保存在samples里面，，获取到的数据全部为放大100倍的有符号双字
 */
void SamplingData(void);

/*
 *	CloseSensor:	关闭传感器模块
 */
void CloseSensor(void);

#ifdef __cplusplus
}
#endif

#endif
