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
#include "sensor.h"

#include "stm32l0xx_hal.h"
#include "rs485.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <math.h>

sample_t samples = {0, 0, 0, NULL,0};			//ȫ�ִ������������ݽṹ�����
uint8_t rs485Buf[32] = {0};		//485���ͽ��ջ�����

sensor_char sensors_char = {false, false, {0}};

uint8_t SAMPLE_SIZE = 0;

typedef struct rs485SensorType
{
	uint8_t addr;		//��������ַ
	uint16_t regAddr;	//�Ĵ�����ַ
	uint16_t fieldCount;//����������
	uint16_t delay;		//�����ȴ���ʱ
	uint8_t fieldType;	//���������ͣ����ڱ���������Ϳɸ���̨�������ݶԽ�
}Rs485Device;

typedef enum rs485Stat
{
	NONE,				//δ�����κ��豸
	SINGLE,				//���ӵ���485�豸
	EXTEND,				//������չ���豸
	UNKNOW=0xff			//δ֪����״̬����������ʱΪ��״̬
}Rs485Stat;
	
struct 
{
	Rs485Stat status;	//�豸����״̬��
	uint8_t sockets[SOCKETS];//�ӿڵ����ӱ�ǣ���¼�豸��ַ��
	uint32_t TALE;		//��β���������������ڴ�ȡ����ʱ��ֹԽλ
}rs485Tag={UNKNOW};		//485�豸����״̬��������ʼ״̬UNKNOW

enum FIELDS_TYPE				//�������������ʶ����Ҫ�����������������һһ��Ӧ
{
	SENSOR_BATTERY			=0,
	ILLUMINATION			=1,
	AIR_TEMPERATURE			=2,
	AIR_HUMIDITY			=3,
	SOIL_TEMP_HUM			=4,
	SOIL_TEMPERATURE		=5,
	SOIL_HUMIDITY			=6,
	PHOTO_RADIO				=7,
	LEAF_TEMPERATURE		=8,
	LEAF_HUMIDITY			=9,
	RAIN_FALL				=10,
	WIND_SPEED				=11,
	WIND_DIRECTION			=12,
	SOIL_EC					=13,
	SOIL_SALINITY			=14,
	CO2						=15,
	ATMOSPHERIC				=16,
	SOIL_PH					=17
};

FieldInfo fieldInfo[]=
{
	{"Battery",U16},
	{"Illumination",U32D2},
	{"AirTemperature",S16D1},
	{"AirHumidity",U16D1},
	{"SoilTemperature",S16D1},
	{"SoilHumidity",U16D1},
	{"Photosynthetic",U16},
	{"LeafTemperature",S16D1},
	{"LeafHumidity",U16D2},
	{"RainFall",U16D1},
	{"WindSpeed",U16D1},
	{"WindDirection",U16D1},
	{"SoilEleCond",U16D2},
	{"SoilSalinity",U16D1},
	{"Co2",U16},
	{"Atmospheric",U16D1},
	{"SoilPH",U16D2}
};

enum SENSOR_TYPE		//�豸���ͣ�ÿ���豸��Ҫ��Ӧһ��Ψһ�ҹ̶��ĵ�ַ
{
	SWR_100W	= 0x02,
	ST_PH		= 0x05,
	ST_GH		= 0x06,
	ST_YMW		= 0x07,
	ST_YMS		= 0x08,
	ST_YL		= 0x09,
	ST_FS		= 0x0A,
	ST_FX		= 0x0B,
	ST_TW		= 0x0C,
	ST_EC		= 0x0D,
	ST_CO2		= 0x0E,
	ST_AP		= 0x0F
};

#define DEVICE_COUNTS	12		//�豸���������������豸��Ҫ�޸Ĵ�ֵ�����ڱ�����ѯ����
Rs485Device rs485Devices[] = 
{
/*	��ַ��		�Ĵ�����ַ���������������ȴ��ӳ٣�	����������*/					//���һ������������������������������������ʹӶ���ı�ʶ����˳��
	{SWR_100W,	0x0000,		0x0002,		0,			SOIL_TEMP_HUM},				//�������SWR_100W������ʪ�ȴ�������ʵ��ʹ�õ�������ʪ�ȴ�����
	{ST_PH,		0x0000,		0x0001,		0,			SOIL_PH},						//�������ST_PH����PHֵ������
	{ST_GH,		0x0000,		0x0001,		0,			PHOTO_RADIO},					//�������ST_GH�����Ч�ȴ�����
	{ST_YMW,	0x0000,		0x0001,		0,			LEAF_TEMPERATURE},				//�������ST_YMWҶ���¶ȴ�����
	{ST_YMS,	0x0001,		0x0001,		0,			LEAF_HUMIDITY},					//�������ST_YMSҶ��ʪ�ȴ�����
	{ST_YL,		0x0000,		0x0001,		0,			RAIN_FALL},						//�������ST_YL�������������ô�������Ҫ��ʱ�����޷�ͬ��ͨ485һ����ѯ
	{ST_FS,		0x0000,		0x0001,		100,		WIND_SPEED},					//�������ST_FS���ٴ�����
	{ST_FX,		0x0000,		0x0001,		0,			WIND_DIRECTION},				//�������ST_FX���򴫸���
	{ST_TW,		0x0000,		0x0001,		0,			SOIL_TEMPERATURE},				//�������ST_TW�����¶ȴ�������ʹ�ú�������ʪ��ͬ�����������ʶ�����������¶�
	{ST_EC,		0x0000,		0x0001,		0,			SOIL_EC},						//�������ST_EC����ecֵ������
	{ST_CO2,	0x0000,		0x0001,		5000,		CO2},							//�������ST_CO2������̼������
	{ST_AP,		0x0000,		0x0001,		0,			ATMOSPHERIC}					//�������ST_AP����ѹ������
};


/*
 *	QueryRs485Data:	��ѯ485����������
 *	device��		�豸״̬�ṹ��
 *	socketIndex��	������ݵĽӿ�
 *	����ֵ��		���յ������ݳ���
 */
uint8_t QueryRs485Data(Rs485Device device,uint8_t socketIndex)
{
	uint8_t i;
	Rs485Command command;
	
	HAL_Delay(device.delay);							//ĳЩ�豸��Ҫ������ʱ			
	rs485Buf[0] = device.addr;						//�豸��ַ
	rs485Buf[1] = 0x03;								//��ѯ����
	rs485Buf[2] = device.regAddr>>8;				//��ʼ�Ĵ�����ַ��
	rs485Buf[3] = device.regAddr&0xff;				//��ʼ�Ĵ�����ַ��
	rs485Buf[4] = device.fieldCount>>8;				//�Ĵ���������
	rs485Buf[5] = device.fieldCount&0xff;			//�Ĵ���������
	command.data = rs485Buf;						//��������
	command.data_len = 8;							//�����8������2byte crc��
	command.recive_len = 3+(device.fieldCount<<1)+2;//(3Byte ͷ��)+(2*�Ĵ�������)+(2Byte CRC)
	command.recive_timeout = 40+device.delay;		//��ʱʱ��
	DEBUG(3,"recive_timeout = %d\r\n",40+device.delay);
	
	if (ExcuteRs485Command(command)==EXCUTE_OK)
	{
		sensors_char.sensors_buf[socketIndex] = socketIndex|(device.fieldType<<3); ///��¼ÿ����չ��ID��ѯ���
		for (i=0;i<device.fieldCount;i++)
		{
			samples.fields[samples.count++] = (rs485Buf[3+2*i]<<8)+rs485Buf[4+2*i]; ///��ѯ����485���ݴ��뻺����
			DEBUG(3,"samples.count = %d, %d\r\n",rs485Buf[3+2*i]<<8,rs485Buf[4+2*i]);
		}
		return device.fieldCount;
	}
	else
	{
		DEBUG(3,"socketIndex11 = %d\r\n",socketIndex);
		sensors_char.sensors_buf[socketIndex] = 0; ///�����쳣
	    DEBUG(3,"socketIndex22 = %d\r\n",socketIndex);
		return 0;
	}
}

/*
 *	QueryRs485DataByAddr:		���ݵ�ַ��ѯ485����������
 *	addr��			�豸��ַ
 *	socketIndex��	������ݵĽӿ�
 *	����ֵ��		���յ������ݳ���
 */
uint8_t QueryRs485DataByAddr(uint8_t addr,uint8_t socketIndex)
{
	uint8_t i;
	Rs485Device device;

	for (i=0;i<DEVICE_COUNTS;i++)
	{
		if (rs485Devices[i].addr == addr)
		{
			device = rs485Devices[i];
			break;
		}
	}
	if(device.addr != addr)
	{
		return 0;
	}
	return QueryRs485Data(device,socketIndex);
}

#ifdef USING_RS485		//��ͨ485��չ���豸����Ҫ�õ���ѯ��ַ�Ϳ�����չ�й���
/*
 *	PollingRs485:	��ѯĳһ���ӿڵ�485�豸����ѯ��������������ýӿ����ӵ��豸��ַ
 *	����ֵ��		���յ������ݳ���
 */
uint8_t PollingRs485(uint8_t socketIndex)
{
	uint8_t i;
		
	for (i=0;i<DEVICE_COUNTS;i++)					//��ѯ���д������Ƿ��л�Ӧ
	{
		DEBUG(3,"DEVICE_COUNTS\r\n");
		if (QueryRs485Data(rs485Devices[i],socketIndex)>0)
		{
			rs485Tag.sockets[socketIndex] = rs485Buf[0];		//����ô�������ַ
			sensors_char.sensors_buf[socketIndex] = socketIndex|(rs485Devices[i].fieldType<<3); ///��3λΪ��socket_id -- socketIndex

			return SUCCESS;
		}else if (QueryRs485Data(rs485Devices[i],socketIndex)==0)

		DEBUG(3,"samples.sockets\r\n");
	}
	
	DEBUG(2,"polling socket%i fail\r\n",socketIndex);
	return ERROR;
}

/*
 *	QueryRs485Addr:		��ѯ485��ַ,��ѯ���ĵ�ַ�����rs485Buf[3]
 *	���أ�				SUCCESS|ERROR
 */
int8_t QueryRs485Addr(void)
{
	Rs485Command excuteAddr;
	rs485Buf[0] = 0xfe;
	rs485Buf[1] = 0x03;
	rs485Buf[2] = 0x04;
	rs485Buf[3] = 0x00;
	rs485Buf[4] = 0x00;
	rs485Buf[5] = 0x00;
	rs485Buf[6] = 0x00;
	excuteAddr.data = rs485Buf;
	excuteAddr.data_len = 9;
	excuteAddr.recive_len = 9;
	excuteAddr.recive_timeout = 400;
	if (ExcuteRs485Command(excuteAddr)==EXCUTE_OK)
	{
		DEBUG(2,"query addr success:0x%02x\r\n",rs485Buf[3]);
		return SUCCESS;
	}

	DEBUG(2,"query addr fail:0x%02x\r\n",rs485Buf[3]);
	return ERROR;
}

/*
 *	ControlExtendBox:	������չ�п��ؽӿ�
 *	mask:				�˿����룬λ0~4��ʶ�˿�1~5���������λΪ1��ʾ����5~7λ����
 */
int8_t ControlExtendBox(uint8_t mask)
{
	Rs485Command controlPort;
	rs485Buf[0] = 0x00;
	rs485Buf[1] = 0x05;
	rs485Buf[2] = 0x00;
	rs485Buf[3] = 0x01;
	rs485Buf[4] = 0x00;
	rs485Buf[5] = mask;
	rs485Buf[6] = 0x00;
	controlPort.data = rs485Buf;
	controlPort.data_len = 9;
	controlPort.recive_len = 9;
	controlPort.recive_timeout = 40;
	
	if (ExcuteRs485Command(controlPort)==EXCUTE_OK)
	{
		DEBUG(3,"control success:%d\r\n",rs485Buf[5]);
		return SUCCESS;
	}else
	{
		DEBUG(2,"control fail:%d\r\n",rs485Buf[5]);
	}
	return ERROR;
}
#elif WEATHER_STATION	//����վ�豸��Ҫ�õ�����������������ݹ���
/*
 *	ClearRainFallData:	������������������
 */
int8_t ClearRainFallData(void)
{
	Rs485Command controlPort;
	rs485Buf[0] = ST_YL;		//������������ַ
	rs485Buf[1] = 0x02;			//д�����
	rs485Buf[2] = 0x04;			//���ݳ���4
	rs485Buf[3] = 0x00;			//�Ĵ�����λ
	rs485Buf[4] = 0x00;			//�Ĵ�����λ
	rs485Buf[5] = 0x00;			//�Ĵ������ȸ�
	rs485Buf[6] = 0x01;			//�Ĵ������ȵ�
	controlPort.data = rs485Buf;
	controlPort.data_len = 9;
	controlPort.recive_len = 9;
	controlPort.recive_timeout = 40;
	if (ExcuteRs485Command(controlPort)==EXCUTE_OK)
	{
		DEBUG(3,"clear success\r\n");
		return SUCCESS;
	}
	DEBUG(2,"clear fail\r\n");
	return ERROR;
}
#endif

#define MAX44009_STATE_ID 		0x10  

uint8_t AirTH_INT_ID = 0x20;

#define test_ok					0

uint32_t sensor_time = 0;

volatile bool Rs485Tag_UNKNOW = false;

/*
 *	SamplingData:	��ȡ��������һ�β�����������ݽ�������samples���棬��ȡ��������ȫ��Ϊ�Ŵ�100�����з���˫��
 */
void SamplingData(void)
{
	int32_t AirTH[2] = {0},lux=0;  
	uint8_t i = 0,temp=0;
  static float luxdata = 0;

	InitUsart5(  );

	sensors_char.Max44009_State = false;
	sensors_char.AirTH_State = false;

	#if 1
	samples.count = 0;
	
	if (CheckBattery()<=3)					//��ص�����
	{
		DEBUG(2,"battery extremely low %dmV;enter standby\r\n",LoRapp_Handle.Battery*6+3600);
		IntoLowPower( ); 					//��ص����ر�ͣ�ֱ�����ý�������
	}
    
    DEBUG(2,"bettry:%d;\r\n",LoRapp_Handle.Battery);
		samples.fields[samples.count++] = ((LoRapp_Handle.Rechargeing) <<8) | LoRapp_Handle.Battery;	//���״̬+����
		lux = Max44009ReadLux(); 							//��ȡ����

    luxdata = lux;
    
		if(sensors_char.Max44009_State)
		{
			luxdata *= 100; ///�Ŵ�100��
			samples.fields[samples.count++] = ((uint16_t)luxdata>>16);			//���ո�λ
			samples.fields[samples.count++] = ((uint16_t)luxdata&0xffff);		//���յ�λ
			DEBUG(2,"lux:%d;\r\n",(uint16_t)(luxdata)/100);
		}
    
	Hdc1080ReadTH(AirTH);																	//��ȡ������ʪ��
	if(AirTH[0]<-400||AirTH[0]>1250)				//�����¶��쳣������
		AirTH[0] = 0;
	if(AirTH[1]<0)									//����ʪ�ȳ������ޣ�����
		AirTH[1] = 0;
	else if (AirTH[1]>1000)							//����ʪ�ȳ������ޣ���100
		AirTH[1] = 1000;
	DEBUG(2,"tem:%d hum:%d;\r\n",AirTH[0], AirTH[1]); 
    
	if(sensors_char.AirTH_State)
	{
		if(AirTH[0] < 0)
		{
			AirTH[0] = ~(AirTH[0] - 1); ///����ת����
			AirTH_INT_ID = 0x40;
		}else
		AirTH_INT_ID = 0x20;

		samples.fields[samples.count++] = AirTH[0];		//�����¶�
		samples.fields[samples.count++] = AirTH[1];		//����ʪ��
	}
	
	DEBUG(3,"samples.Max44009_State=%d  sensors_char.Hdc1080_State = %d\r\n",sensors_char.Max44009_State,sensors_char.AirTH_State);
	
	/***********************�ж���Щ����������*************************/
	if(sensors_char.Max44009_State && !sensors_char.AirTH_State)
	samples.structver = STRUCT_VERSION|MAX44009_STATE_ID;
	else if(sensors_char.AirTH_State && !sensors_char.Max44009_State)
	samples.structver = STRUCT_VERSION|AirTH_INT_ID;
	else if(sensors_char.Max44009_State == true && sensors_char.AirTH_State == true)
	{
		samples.structver = STRUCT_VERSION|MAX44009_STATE_ID|AirTH_INT_ID;
	}else
		samples.structver = STRUCT_VERSION;	//���ݸ�ʽ�汾��
	
	DEBUG(3,"structver = %02x samples.count = %d\r\n",samples.structver, samples.count);
	#endif
    
	Enble485Power( );	
	
	switch(rs485Tag.status)
	{
		case EXTEND:					//��ѯ������չ����չ485������
			Rs485Tag_UNKNOW = false;
			DEBUG(2,"extern\r\n");
			HAL_Delay(1000);				//�򿪵�Դ֮����Ҫһ��ʱ��ȴ��豸�ȶ�
		
			for (i=1;i<=5;i++)
			{
				DEBUG(2,"socket%d:%d\r\n",i,rs485Tag.sockets[i]);
				if (rs485Tag.sockets[i]) ///ȡ���洫������ַ
				{
					HAL_Delay(20);					//ÿ֡modbus������Ҫ���3.5���ַ����ϣ���ʱ�ٷ��Ϳ�������
					if(i==5)
						temp=7;
					else
						temp=i;
					if (SUCCESS==ControlExtendBox(1<<(temp-1)))//ֻ�д򿪿��ƽӿڳɹ�֮��Ų�ѯ������
					{
						HAL_Delay(1000);				//����չ��֮����Ҫһ��ʱ��ȴ��������ȶ�
						
						QueryRs485DataByAddr(rs485Tag.sockets[i], i); ///5s?
											
						uint32_t ExtendBox_Over_time = HAL_GetTick( ); ///��ֹ485�ڲ�ѯ״̬ʱ�ϵ��쳣

						while((SUCCESS!=ControlExtendBox((1<<(temp-1) & 0x00))) && (HAL_GetTick( ) - ExtendBox_Over_time < 5000));
					}
					else
					{
						for(;i<=5;i++)
						{
							sensors_char.sensors_buf[i] = 0;		//��չ��û����Ӧ���ÿմ��������ӱ�־λ
						}
						break;
					}
				}
				else
				{
					sensors_char.sensors_buf[i] = 0;	 //��չ��û����Ӧ���ÿմ��������ӱ�־λ   ��û��������ʱ��SOCKETS = 1 samples.sockets����ֻ��һ����ֵ
                }
			}
	
			break;
		case SINGLE:					//��ѯ������չ�е����ӿڵ�485������
			Rs485Tag_UNKNOW = false;
			DEBUG(2,"single\r\n");
			HAL_Delay(1000);				//�򿪵�Դ֮����Ҫһ��ʱ��ȴ��豸�ȶ�
			uint8_t data = QueryRs485DataByAddr(rs485Tag.sockets[0],0);
			break;
		case NONE:						//�����ӣ�������
			Rs485Tag_UNKNOW = false;
			DEBUG(2,"none\r\n");
			break;
		case UNKNOW:								//δ֪�豸����״̬����ѯ�豸����״̬
		default:									//����δ֪״̬����ͬ����
		{
			Rs485Tag_UNKNOW = true;
		
			DEBUG(2,"UNKNOW\r\n");
			HAL_Delay(1000);							//�򿪵�Դ֮����Ҫһ��ʱ��ȴ��豸�ȶ�
			if (SUCCESS==QueryRs485Addr())			//��ѯ���豸
			{
				if (rs485Buf[3]==0)					//��ѯ����ַΪ0Ϊ��չ��
				{
					uint8_t tag = rs485Buf[4];		//��ȡ�Ѿ����ӵ��豸��ʶ������ڷ������ݵ���λ
					DEBUG(2,"tag:%d\r\n",tag);
					if (tag)
                    {
						rs485Tag.status = EXTEND;	//��չ���������豸����Ǵ�����״̬Ϊ������չ��״̬
                    }					
                    else
						rs485Tag.status = NONE;		//��չ����û�������豸�����Ϊ���豸����״̬
					
//					system_time = HAL_GetTick( );
					for (i=1;i<=5;i++)
					{
						//�˴�
						if(i==5)
							temp=7;
						else
							temp=i;
						if ((tag>>(temp-1))&1)
						{
							HAL_Delay(20);					//ÿ֡modbus������Ҫ���3.5���ַ����ϣ���ʱ�ٷ��Ϳ�������
							if (SUCCESS==ControlExtendBox(1<<(temp-1)))//ֻ�д򿪿��ƽӿڳɹ�֮��Ų�ѯ������
							{
								HAL_Delay(1000);				//����չ��֮����Ҫһ��ʱ��ȴ��������ȶ�
								PollingRs485(i);			//��ѯ��ѯ�ýӿڵ��豸
								
								uint32_t ExtendBox_Over_time = HAL_GetTick( ); ///��ֹ485�ڲ�ѯ״̬ʱ�ϵ��쳣

								while((SUCCESS!=ControlExtendBox((1<<(temp-1) & 0x00))) && (HAL_GetTick( ) - ExtendBox_Over_time < 5000));
							}
							else
							{
								for(;i<=5;i++)
								{
									sensors_char.sensors_buf[i] = 0;		//��չ��û����Ӧ���ÿմ��������ӱ�־λ
								}
								break;						//��չ��û����Ӧ��������ѯ
							}
						}
						else
						{
							sensors_char.sensors_buf[i] = 0;		//��չ��û����Ӧ���ÿմ��������ӱ�־λ
						}
						DEBUG(2,"socket%d:%d\r\n",i,rs485Tag.sockets[i]);
					}
				}
				else								//��ѯ�����ǵ�����ͨ������
				{
					DEBUG(2,"SINGLES\r\n");
					rs485Tag.status = SINGLE;
					rs485Tag.sockets[0] = rs485Buf[3];				//�㲥��ѯ�����豸��ַ����ڵ�4λ��
					QueryRs485DataByAddr(rs485Tag.sockets[0],0);	//��ѯ��������������
				}
			}
			else
			{
				if (PollingRs485(0)==SUCCESS)
				{
					rs485Tag.status = SINGLE;						//��ѯ�Ƿ���ֱ�������������ϵ�485�豸�����ݴ���ڽӿ�0
                    DEBUG(2,"get 485 SINGLE\r\n");
                }						
				else
				{
					rs485Tag.status = NONE;							//��ѯ����Ӧ�����Ϊ�޴�����
                    DEBUG(2,"get 485 NONE\r\n");
                }
			}
			
			break;
		}
	}
	
	/**************************���������Ż���ȥ��BUF��Ϊ������******************************/
	Disable485Power( );
	
	uint8_t buf[2]; ///��������
    samples.socket_id = 0;
	for(uint8_t i = 1; i < 6; i++)
	{
		if(sensors_char.sensors_buf[i] != 0)  ///��ÿ����չ�ڲ�ѯ��������ж��Ƿ��޴��������룬�������ݹ���
		{
			samples.sockets[samples.socket_id] = sensors_char.sensors_buf[i];
			buf[0] = (samples.sockets[samples.socket_id]&0xf8)>>3;
			buf[1] = samples.sockets[samples.socket_id]&0x07;
			DEBUG(2,"samples.sockets[i = %d] = %02x, %d, %d\r\n",samples.socket_id, samples.sockets[samples.socket_id],buf[0],buf[1]);
			samples.socket_id ++;
		}
	}
	if(samples.socket_id>0)
	{
		samples.sockets[samples.socket_id] = '_';
		DEBUG(3, "socket_id_22 = %d\r\n",samples.socket_id);
	}
	else if(samples.socket_id == 0)
	{
		samples.sockets[0] = '_';
		DEBUG(3, "socket_id_33 = %d\r\n",samples.socket_id);
	}
	/**************************����Ϊ��ȡ������չ��ID��******************************/

	SAMPLE_SIZE = samples.count*2+strlen((char *)samples.sockets)+2; ///���㵱ǰ���ݳ���
	DEBUG(2,"sockets = %d count = %d SAMPLE_SIZE = %d\r\n", strlen((char *)samples.sockets), samples.count*2, SAMPLE_SIZE);			

	for (uint8_t i=0;i<=samples.socket_id;i++) 
	{
		DEBUG(3,"socket%d=%02x,",i,samples.sockets[i]);
	}
	for (uint8_t i=0;i<samples.count;i++)
	{
		DEBUG(3,"data%d=%d,",i,samples.fields[i]);
	}		
}
