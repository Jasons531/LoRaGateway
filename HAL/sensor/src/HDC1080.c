/*
**************************************************************************************************************
*	@file		HDC1080.c
*	@author 	Jason_531@163.com
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#include <math.h>
#include "hdc1080.h"
#include "delay.h"
#include "i2c.h"
#include "sensor.h"

#define Hdc1080_WRITE_ADDR							0x80	// Hdc1080备地址: TI提供7Bit 地址，最低位Bit 缺省为0 则1000000 =  0x80

/* Register addresses */
#define Temperature 								0x00	// Lux寄存器高字节的地址
#define Humidity					              	0x01
#define Configuration                  				0x02
#define HDC_Manufacturer_ID							0xFE
#define HDC_Device_ID 								0xFF
#define HDC1080_EXP									16

#define Manufacturer_ID_value 						0x5449
#define Device_ID_value 							0x1050

uint8_t TESTMID[2] = {0xB0, 0x00}; //0x9000

/********************************************************************************
	*函数名：空气温湿度初始化

	注意事项：
	HAL_I2C_Mem_Write、HAL_I2C_Mem_Read：只作为多字节读写使用，否则容易导致HAL_TIMME_OUT	
	
********************************************************************************/
/*
 *	InitHdc1080:		初始化InitHdc1080地址
 *	返回值: 			
 */
void InitHdc1080(void)
{
	MX_I2C2_Init( );
}

static void I2C_Hdc1080_Error (void)
{
  /* 反初始化I2C通信总线 */
  HAL_I2C_DeInit(&hi2c2);
   
  /* 重新初始化I2C通信总线*/
  InitHdc1080(  );
  DEBUG(3,"I2C TIMEOUT!!! I2C...\n");
}

/*
 *	Hdc1080ReadTH:	读取Hdc1080ReadTH温湿度传感器
 *	返回值: 				温湿度，放大100倍
 */
void Hdc1080ReadTH(int32_t *measure)
{
	uint8_t i,num;
	float Temperature_Data, Humidity_Data;
	
	// 读取温度
	measure[0] = 0;	//清空数据
	num = 0;		//读取次数清零
	
	Hdc1080_WriteOneByte(Configuration, TESTMID); ///配置InitHdc1080，初始化
	DelayMs( 15 );
	
	uint16_t MID = Hdc1080ReadOneByte(HDC_Manufacturer_ID);
	DelayMs( 1 );
	
	uint16_t DID = Hdc1080ReadOneByte(HDC_Device_ID);
	
	if(MID == Manufacturer_ID_value &&  DID == Device_ID_value) ///初始化完成
	{
		sensors_char.AirTH_State = true;

		for(i=0;i<10;i++)
		{
			Temperature_Data = Hdc1080ReadOneByte(Temperature);
			measure[0] += (((Temperature_Data/ exp2(HDC1080_EXP)) * 165) - 40)*10;	// 放大10倍，即保留了小数点后一位
			num++;			
		}
		if (num>0)
		measure[0] = measure[0]/num;		  						//读取到有效数据则除以有效读出的次数
			
		DelayMs( 15 );
		
		// 读取湿度
		measure[1] = 0;	//清空数据
		num = 0;
		for(i=0;i<10;i++)
		{		
			Humidity_Data = Hdc1080ReadOneByte(Humidity);
			measure[1] += (Humidity_Data/ exp2(HDC1080_EXP))*1000;	// 放大10倍，即保留了小数点后一位
			num++;
		}
		if (num>0)
		measure[1] = measure[1]/num;		  						//读取到有效数据则除以有效读出的次数
	}else
	{
		DEBUG(4,"Temperature_Data = %d°C Humidity_Data = %d％RH\r\n", measure[0],measure[1]);
		sensors_char.AirTH_State = false;
	}
		
	DEBUG(3,"Temperature_Data = %d°C Humidity_Data = %d％RH\r\n", measure[0],measure[1]);
}

/*Hdc1080WriteOneByte ： 写入一个字节
 *reg:					 寄存器地址
 *data:					 要写入的指针
 *返回值:                0：失败  1：成功
*/  
uint8_t Hdc1080_WriteOneByte(uint8_t reg, uint8_t data[2])
{
	HAL_StatusTypeDef state;
	
	uint8_t temp[3] = {reg, data[0], data[1]};
	
	state = HAL_I2C_Master_Transmit(&hi2c2,Hdc1080_WRITE_ADDR, temp,3, 2000);
	if(state!=HAL_OK)
	{
		DEBUG(4,"HAL_I2C_Master_Transmit state = %d\r\n",state);
		I2C_Hdc1080_Error(  );
		return 0;
	}
	DEBUG(3,"hdc = %02x hacth = %02x\r\n", (data[0]>>8)&0xff, data[1]&0xff);
	return 1;
}



/*Hdc1080ReadOneByte ：  读入一个字节
 *reg:					 寄存器地址
 *返回值:                0：失败  非0：成功
*/  
uint16_t Hdc1080ReadOneByte(uint8_t reg)
{
	uint8_t data[2];
	uint16_t Had_Data = 0;
	HAL_StatusTypeDef state;
	
	state = HAL_I2C_Master_Transmit(&hi2c2,Hdc1080_WRITE_ADDR, &reg,1, 20);
	if(state != HAL_OK)
	{
		DEBUG(4,"HAL_I2C_Master_Transmit state = %d\r\n",state);
		I2C_Hdc1080_Error(  );
		return 0;
	}
	HAL_Delay(15);
		
	state = HAL_I2C_Master_Receive(&hi2c2,Hdc1080_WRITE_ADDR+1, data, 2, 20);
		
	if(state != HAL_OK)
	{
		DEBUG(3,"HAL_I2C_Master_Receive state = %d\r\n",state);
		I2C_Hdc1080_Error(  );
		return 0;
	}
	DEBUG(4,"high33 = %02x low33 = %02x\r\n", data[0],data[1]);
	Had_Data = (data[0]<<8)|data[1];
	
	return Had_Data;  
}


