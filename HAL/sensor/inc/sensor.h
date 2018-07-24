/*
**************************************************************************************************************
*	@file		sensor.c
*	@author 	Jason_531@163.com
*	@version 	==STRUCT_VERSION
*	@date     	20170527
*	@warming	����ȫ��USING_RS485��򿪴�rs485����������Ӧ�Ĵ������ɼ�������WERTHER_STATION��ɼ��̶�3������չ�����󴫸���
*	@warming	����ȫ��SAVE_HISTORY_DATA��򿪴�ȡ��ʷ���ݹ���
*	@brief		�������ɼ�Э��
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

#define STRUCT_VERSION				0x03		//�������ṹ��Э��汾�ţ���Ķ����´�������������´˰汾�Ų��ڷ�������������
#define SENSOR_FIELD				  20		//����ж��ٸ�������������
		
#define SOCKETS	  					  6		//�ж��ٸ�485���������
		
typedef struct
{
	uint8_t structver;						//�ṹ��汾������İ�ṹ����Ҫͬʱ���Ľṹ�汾�Ų��ڷ�������������
	uint8_t count;							//����������������
	uint8_t socket_id;				//������������չ��ID��
	uint8_t *sockets;				//�ӿ����ӵĴ�������ʶ
	uint16_t fields[SENSOR_FIELD];			//�����������򣬸��ݣ����������գ������¶ȣ�����ʪ�ȣ�485�����������ݽӿڵĴ�������ʶ�жϣ���˳���ȡ
}sample_t;


typedef struct
{
	bool Max44009_State;  ///���մ����������־
	bool AirTH_State;   ///������ʪ�ȴ����������־
	uint8_t sensors_buf[6];  ///��չ�����ݻ�����
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

extern sample_t samples;		//ȫ�ִ������������ݽṹ�����
extern FieldInfo fieldInfo[];	//ȫ�ִ�������������Ϣ����

//#define SAMPLE_SIZE	(samples.count*2+SOCKETS+2) ///�汾+����=2�� �汾+����+sockets+samples.count*2(Byte)

extern uint8_t SAMPLE_SIZE;


int8_t QueryRs485Addr(void);

int8_t ControlExtendBox(uint8_t mask);

uint8_t PollingRs485(uint8_t socketIndex);

extern uint32_t RfSend_time;

extern volatile bool Rs485Tag_UNKNOW;


/*
 *	SamplingData:		��ȡ��������һ�β�����������ݽ�������samples���棬����ȡ��������ȫ��Ϊ�Ŵ�100�����з���˫��
 */
void SamplingData(void);

/*
 *	CloseSensor:	�رմ�����ģ��
 */
void CloseSensor(void);

#ifdef __cplusplus
}
#endif

#endif
