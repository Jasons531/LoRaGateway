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
#include "sensor.h"

#include "stm32l0xx_hal.h"
#include "rs485.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include <math.h>

sample_t samples = {0, 0, 0, NULL,0};			//全局传感器采样数据结构体变量
uint8_t rs485Buf[32] = {0};		//485发送接收缓冲区

sensor_char sensors_char = {false, false, {0}};

uint8_t SAMPLE_SIZE = 0;

typedef struct rs485SensorType
{
	uint8_t addr;		//传感器地址
	uint16_t regAddr;	//寄存器地址
	uint16_t fieldCount;//数据域数量
	uint16_t delay;		//采样等待延时
	uint8_t fieldType;	//数据域类型，用于标记数据类型可跟后台进行数据对接
}Rs485Device;

typedef enum rs485Stat
{
	NONE,				//未连接任何设备
	SINGLE,				//连接单个485设备
	EXTEND,				//连接扩展盒设备
	UNKNOW=0xff			//未知连接状态，初次启动时为此状态
}Rs485Stat;
	
struct 
{
	Rs485Stat status;	//设备连接状态，
	uint8_t sockets[SOCKETS];//接口的连接标记，记录设备地址。
	uint32_t TALE;		//结尾无意义数据域，用于存取数据时防止越位
}rs485Tag={UNKNOW};		//485设备连接状态变量，初始状态UNKNOW

enum FIELDS_TYPE				//传感器数据域标识，需要和下面的数据域名称一一对应
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

enum SENSOR_TYPE		//设备类型，每种设备需要对应一个唯一且固定的地址
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

#define DEVICE_COUNTS	12		//设备类型数量，增加设备需要修改此值，用于遍历查询数组
Rs485Device rs485Devices[] = 
{
/*	地址，		寄存器地址，数据域数量，等待延迟，	数据域类型*/					//如果一个传感器包含多个数据域，则后面的数据域类型从定义的标识往后顺延
	{SWR_100W,	0x0000,		0x0002,		0,			SOIL_TEMP_HUM},				//雷神电子SWR_100W土壤温湿度传感器，实际使用的土壤温湿度传感器
	{ST_PH,		0x0000,		0x0001,		0,			SOIL_PH},						//雷神电子ST_PH土壤PH值传感器
	{ST_GH,		0x0000,		0x0001,		0,			PHOTO_RADIO},					//雷神电子ST_GH光和有效度传感器
	{ST_YMW,	0x0000,		0x0001,		0,			LEAF_TEMPERATURE},				//雷神电子ST_YMW叶面温度传感器
	{ST_YMS,	0x0001,		0x0001,		0,			LEAF_HUMIDITY},					//雷神电子ST_YMS叶面湿度传感器
	{ST_YL,		0x0000,		0x0001,		0,			RAIN_FALL},						//雷神电子ST_YL雨量传感器，该传感器需要长时供电无法同普通485一样查询
	{ST_FS,		0x0000,		0x0001,		100,		WIND_SPEED},					//雷神电子ST_FS风速传感器
	{ST_FX,		0x0000,		0x0001,		0,			WIND_DIRECTION},				//雷神电子ST_FX风向传感器
	{ST_TW,		0x0000,		0x0001,		0,			SOIL_TEMPERATURE},				//雷神电子ST_TW土壤温度传感器，使用和土壤温湿度同样的数据域标识（都是土壤温度
	{ST_EC,		0x0000,		0x0001,		0,			SOIL_EC},						//雷神电子ST_EC土壤ec值传感器
	{ST_CO2,	0x0000,		0x0001,		5000,		CO2},							//雷神电子ST_CO2二氧化碳传感器
	{ST_AP,		0x0000,		0x0001,		0,			ATMOSPHERIC}					//雷神电子ST_AP大气压传感器
};


/*
 *	QueryRs485Data:	查询485传感器数据
 *	device：		设备状态结构体
 *	socketIndex：	存放数据的接口
 *	返回值：		接收到的数据长度
 */
uint8_t QueryRs485Data(Rs485Device device,uint8_t socketIndex)
{
	uint8_t i;
	Rs485Command command;
	
	HAL_Delay(device.delay);							//某些设备需要额外延时			
	rs485Buf[0] = device.addr;						//设备地址
	rs485Buf[1] = 0x03;								//查询操作
	rs485Buf[2] = device.regAddr>>8;				//起始寄存器地址高
	rs485Buf[3] = device.regAddr&0xff;				//起始寄存器地址低
	rs485Buf[4] = device.fieldCount>>8;				//寄存器数量高
	rs485Buf[5] = device.fieldCount&0xff;			//寄存器数量低
	command.data = rs485Buf;						//命令数据
	command.data_len = 8;							//命令长度8（加上2byte crc）
	command.recive_len = 3+(device.fieldCount<<1)+2;//(3Byte 头部)+(2*寄存器个数)+(2Byte CRC)
	command.recive_timeout = 40+device.delay;		//超时时间
	DEBUG(3,"recive_timeout = %d\r\n",40+device.delay);
	
	if (ExcuteRs485Command(command)==EXCUTE_OK)
	{
		sensors_char.sensors_buf[socketIndex] = socketIndex|(device.fieldType<<3); ///记录每个扩展口ID查询结果
		for (i=0;i<device.fieldCount;i++)
		{
			samples.fields[samples.count++] = (rs485Buf[3+2*i]<<8)+rs485Buf[4+2*i]; ///查询到的485数据存入缓存区
			DEBUG(3,"samples.count = %d, %d\r\n",rs485Buf[3+2*i]<<8,rs485Buf[4+2*i]);
		}
		return device.fieldCount;
	}
	else
	{
		DEBUG(3,"socketIndex11 = %d\r\n",socketIndex);
		sensors_char.sensors_buf[socketIndex] = 0; ///导致异常
	    DEBUG(3,"socketIndex22 = %d\r\n",socketIndex);
		return 0;
	}
}

/*
 *	QueryRs485DataByAddr:		根据地址查询485传感器数据
 *	addr：			设备地址
 *	socketIndex：	存放数据的接口
 *	返回值：		接收到的数据长度
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

#ifdef USING_RS485		//普通485扩展盒设备，需要用到查询地址和控制扩展盒功能
/*
 *	PollingRs485:	轮询某一个接口的485设备，查询到则跳出并保存该接口连接的设备地址
 *	返回值：		接收到的数据长度
 */
uint8_t PollingRs485(uint8_t socketIndex)
{
	uint8_t i;
		
	for (i=0;i<DEVICE_COUNTS;i++)					//轮询所有传感器是否有回应
	{
		DEBUG(3,"DEVICE_COUNTS\r\n");
		if (QueryRs485Data(rs485Devices[i],socketIndex)>0)
		{
			rs485Tag.sockets[socketIndex] = rs485Buf[0];		//保存该传感器地址
			sensors_char.sensors_buf[socketIndex] = socketIndex|(rs485Devices[i].fieldType<<3); ///低3位为：socket_id -- socketIndex

			return SUCCESS;
		}else if (QueryRs485Data(rs485Devices[i],socketIndex)==0)

		DEBUG(3,"samples.sockets\r\n");
	}
	
	DEBUG(2,"polling socket%i fail\r\n",socketIndex);
	return ERROR;
}

/*
 *	QueryRs485Addr:		查询485地址,查询到的地址存放在rs485Buf[3]
 *	返回：				SUCCESS|ERROR
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
 *	ControlExtendBox:	控制扩展盒开关接口
 *	mask:				端口掩码，位0~4标识端口1~5开关情况，位为1表示开，5~7位保留
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
#elif WEATHER_STATION	//气象站设备需要用到清空雨量传感器数据功能
/*
 *	ClearRainFallData:	清零雨量传感器数据
 */
int8_t ClearRainFallData(void)
{
	Rs485Command controlPort;
	rs485Buf[0] = ST_YL;		//雨量传感器地址
	rs485Buf[1] = 0x02;			//写入操作
	rs485Buf[2] = 0x04;			//数据长度4
	rs485Buf[3] = 0x00;			//寄存器高位
	rs485Buf[4] = 0x00;			//寄存器低位
	rs485Buf[5] = 0x00;			//寄存器长度高
	rs485Buf[6] = 0x01;			//寄存器长度低
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
 *	SamplingData:	获取传感器的一次采样结果，数据将保存在samples里面，获取到的数据全部为放大100倍的有符号双字
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
	
	if (CheckBattery()<=3)					//电池电量低
	{
		DEBUG(2,"battery extremely low %dmV;enter standby\r\n",LoRapp_Handle.Battery*6+3600);
		IntoLowPower( ); 					//电池电量特别低，直接重置进入休眠
	}
    
    DEBUG(2,"bettry:%d;\r\n",LoRapp_Handle.Battery);
		samples.fields[samples.count++] = ((LoRapp_Handle.Rechargeing) <<8) | LoRapp_Handle.Battery;	//充电状态+电量
		lux = Max44009ReadLux(); 							//读取光照

    luxdata = lux;
    
		if(sensors_char.Max44009_State)
		{
			luxdata *= 100; ///放大100倍
			samples.fields[samples.count++] = ((uint16_t)luxdata>>16);			//光照高位
			samples.fields[samples.count++] = ((uint16_t)luxdata&0xffff);		//光照低位
			DEBUG(2,"lux:%d;\r\n",(uint16_t)(luxdata)/100);
		}
    
	Hdc1080ReadTH(AirTH);																	//读取空气温湿度
	if(AirTH[0]<-400||AirTH[0]>1250)				//空气温度异常，置零
		AirTH[0] = 0;
	if(AirTH[1]<0)									//空气湿度超出下限，置零
		AirTH[1] = 0;
	else if (AirTH[1]>1000)							//空气湿度超出上限，置100
		AirTH[1] = 1000;
	DEBUG(2,"tem:%d hum:%d;\r\n",AirTH[0], AirTH[1]); 
    
	if(sensors_char.AirTH_State)
	{
		if(AirTH[0] < 0)
		{
			AirTH[0] = ~(AirTH[0] - 1); ///负数转正数
			AirTH_INT_ID = 0x40;
		}else
		AirTH_INT_ID = 0x20;

		samples.fields[samples.count++] = AirTH[0];		//空气温度
		samples.fields[samples.count++] = AirTH[1];		//空气湿度
	}
	
	DEBUG(3,"samples.Max44009_State=%d  sensors_char.Hdc1080_State = %d\r\n",sensors_char.Max44009_State,sensors_char.AirTH_State);
	
	/***********************判断哪些传感器接入*************************/
	if(sensors_char.Max44009_State && !sensors_char.AirTH_State)
	samples.structver = STRUCT_VERSION|MAX44009_STATE_ID;
	else if(sensors_char.AirTH_State && !sensors_char.Max44009_State)
	samples.structver = STRUCT_VERSION|AirTH_INT_ID;
	else if(sensors_char.Max44009_State == true && sensors_char.AirTH_State == true)
	{
		samples.structver = STRUCT_VERSION|MAX44009_STATE_ID|AirTH_INT_ID;
	}else
		samples.structver = STRUCT_VERSION;	//数据格式版本号
	
	DEBUG(3,"structver = %02x samples.count = %d\r\n",samples.structver, samples.count);
	#endif
    
	Enble485Power( );	
	
	switch(rs485Tag.status)
	{
		case EXTEND:					//查询控制扩展函扩展485传感器
			Rs485Tag_UNKNOW = false;
			DEBUG(2,"extern\r\n");
			HAL_Delay(1000);				//打开电源之后需要一段时间等待设备稳定
		
			for (i=1;i<=5;i++)
			{
				DEBUG(2,"socket%d:%d\r\n",i,rs485Tag.sockets[i]);
				if (rs485Tag.sockets[i]) ///取保存传感器地址
				{
					HAL_Delay(20);					//每帧modbus数据需要间隔3.5个字符以上，延时再发送控制命令
					if(i==5)
						temp=7;
					else
						temp=i;
					if (SUCCESS==ControlExtendBox(1<<(temp-1)))//只有打开控制接口成功之后才查询传感器
					{
						HAL_Delay(1000);				//打开扩展盒之后需要一段时间等待传感器稳定
						
						QueryRs485DataByAddr(rs485Tag.sockets[i], i); ///5s?
											
						uint32_t ExtendBox_Over_time = HAL_GetTick( ); ///防止485在查询状态时断电异常

						while((SUCCESS!=ControlExtendBox((1<<(temp-1) & 0x00))) && (HAL_GetTick( ) - ExtendBox_Over_time < 5000));
					}
					else
					{
						for(;i<=5;i++)
						{
							sensors_char.sensors_buf[i] = 0;		//扩展盒没有相应，置空传感器连接标志位
						}
						break;
					}
				}
				else
				{
					sensors_char.sensors_buf[i] = 0;	 //扩展盒没有相应，置空传感器连接标志位   当没接入数据时：SOCKETS = 1 samples.sockets数组只有一个数值
                }
			}
	
			break;
		case SINGLE:					//查询不带扩展盒单独接口的485传感器
			Rs485Tag_UNKNOW = false;
			DEBUG(2,"single\r\n");
			HAL_Delay(1000);				//打开电源之后需要一段时间等待设备稳定
			uint8_t data = QueryRs485DataByAddr(rs485Tag.sockets[0],0);
			break;
		case NONE:						//无连接，不动作
			Rs485Tag_UNKNOW = false;
			DEBUG(2,"none\r\n");
			break;
		case UNKNOW:								//未知设备连接状态，查询设备连接状态
		default:									//其它未知状态，等同处理
		{
			Rs485Tag_UNKNOW = true;
		
			DEBUG(2,"UNKNOW\r\n");
			HAL_Delay(1000);							//打开电源之后需要一段时间等待设备稳定
			if (SUCCESS==QueryRs485Addr())			//查询到设备
			{
				if (rs485Buf[3]==0)					//查询到地址为0为扩展盒
				{
					uint8_t tag = rs485Buf[4];		//获取已经连接的设备标识，存放在返回数据第五位
					DEBUG(2,"tag:%d\r\n",tag);
					if (tag)
                    {
						rs485Tag.status = EXTEND;	//扩展盒有连接设备，标记传感器状态为连接扩展盒状态
                    }					
                    else
						rs485Tag.status = NONE;		//扩展盒上没有连接设备，标记为无设备连接状态
					
//					system_time = HAL_GetTick( );
					for (i=1;i<=5;i++)
					{
						//此处
						if(i==5)
							temp=7;
						else
							temp=i;
						if ((tag>>(temp-1))&1)
						{
							HAL_Delay(20);					//每帧modbus数据需要间隔3.5个字符以上，延时再发送控制命令
							if (SUCCESS==ControlExtendBox(1<<(temp-1)))//只有打开控制接口成功之后才查询传感器
							{
								HAL_Delay(1000);				//打开扩展盒之后需要一段时间等待传感器稳定
								PollingRs485(i);			//轮询查询该接口的设备
								
								uint32_t ExtendBox_Over_time = HAL_GetTick( ); ///防止485在查询状态时断电异常

								while((SUCCESS!=ControlExtendBox((1<<(temp-1) & 0x00))) && (HAL_GetTick( ) - ExtendBox_Over_time < 5000));
							}
							else
							{
								for(;i<=5;i++)
								{
									sensors_char.sensors_buf[i] = 0;		//扩展盒没有相应，置空传感器连接标志位
								}
								break;						//扩展盒没有相应，跳出轮询
							}
						}
						else
						{
							sensors_char.sensors_buf[i] = 0;		//扩展盒没有相应，置空传感器连接标志位
						}
						DEBUG(2,"socket%d:%d\r\n",i,rs485Tag.sockets[i]);
					}
				}
				else								//查询到的是单个普通传感器
				{
					DEBUG(2,"SINGLES\r\n");
					rs485Tag.status = SINGLE;
					rs485Tag.sockets[0] = rs485Buf[3];				//广播查询到的设备地址存放在第4位中
					QueryRs485DataByAddr(rs485Tag.sockets[0],0);	//查询单个传感器数据
				}
			}
			else
			{
				if (PollingRs485(0)==SUCCESS)
				{
					rs485Tag.status = SINGLE;						//轮询是否有直接连接在主机上的485设备，数据存放在接口0
                    DEBUG(2,"get 485 SINGLE\r\n");
                }						
				else
				{
					rs485Tag.status = NONE;							//轮询无相应，标记为无传感器
                    DEBUG(2,"get 485 NONE\r\n");
                }
			}
			
			break;
		}
	}
	
	/**************************发送数据优化，去除BUF内为空数据******************************/
	Disable485Power( );
	
	uint8_t buf[2]; ///测试数据
    samples.socket_id = 0;
	for(uint8_t i = 1; i < 6; i++)
	{
		if(sensors_char.sensors_buf[i] != 0)  ///将每个扩展口查询结果进行判断是否无传感器接入，进行数据过滤
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
	/**************************以上为获取接入扩展盒ID号******************************/

	SAMPLE_SIZE = samples.count*2+strlen((char *)samples.sockets)+2; ///计算当前数据长度
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
