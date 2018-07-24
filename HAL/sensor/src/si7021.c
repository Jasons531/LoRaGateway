/*
**************************************************************************************************************
*	@file		SI7021.c
*	@author 	Jason_531@163.com
*	@version 
*	@date    
*	@brief	关于硬件I2C出现HAL_TIMEOUT：RCC_PERIPHCLK_I2C2时钟没初始化, 添加RCC_PERIPHCLK_I2C2同时重新初始化I2C
***************************************************************************************************************
*/
#include <math.h>
#include "SI7021.h"
#include "delay.h"
#include "i2c.h"

#define SI7021_WRITE_ADDR								0x80	// SI7021的设备地址: TI提供7Bit 地址，最低位Bit 缺省为0 则1000000 =  0x80

/* Register addresses */
#define SI7021_Temperature 								0xE3	// Lux寄存器高字节的地址
#define SI7021_Humidity					        		0xE5 //   

#define SI7021_Configuration            				0x3A

#define Firmware_ID_1									0x84
#define Firmware_ID_2 									0xB8

#define SI7021_EXP										16

#define version_1_0 									0xFF
#define version_2_0 									0x20

#define POLYNOMIAL 										0x31	//SI7021使用的crc8校验多项式

uint8_t Firmware_ID[2] = {Firmware_ID_1,Firmware_ID_2};

// compound commands
uint8_t SERIAL1_READ[] = { 0xFA, 0x0F };
uint8_t SERIAL2_READ[] = { 0xFC, 0xC9 };


/*
 *	InitSi7021:		初始化InitSi7021地址
 *	返回值: 			
 */
void InitSi7021(void)
{
	MX_I2C2_Init( );
}

/*
 *	Si7021ReadTH:	读取SI7021的温湿度
 *	返回值: 			光照度，放大100倍
 */
void Si7021ReadTH(int32_t *measure)
{
	float Temperature_Data = 0;
	float Humidity_Data = 0;
	uint8_t i, FID = 0;
	
	Si7021Configtrue(SI7021_Configuration);  ///配置SI7021_Configuration，初始化
	DelayMs( 15 );
	
	Si7021WriteOneByte(SERIAL2_READ, 2); ///设置CRC字节
	
	FID = Si7021WriteOneByte(Firmware_ID, 2); ///读取版本ID
	DelayMs( 15 );
	
	if(FID == version_1_0 ||  FID == version_2_0) ///初始化完成
	{
		sensors_char.AirTH_State = true;
		for(i=0;i<10;i++)
		{
			Temperature_Data += Si7021ReadOneByte(SI7021_Temperature);
			DelayMs( 15 );	
	
			Humidity_Data += Si7021ReadOneByte(SI7021_Humidity);
			DelayMs( 15 );	
		}			
	Temperature_Data = Temperature_Data/10;
	measure[0] = ((((175.72*Temperature_Data)/ exp2(SI7021_EXP))) - 46.85)*10;	
	
	Humidity_Data = Humidity_Data/10;
	measure[1] = ((125*Humidity_Data/ exp2(SI7021_EXP))-6)*10;
	DEBUG(2,"FID = %02x Temperature_Data = %d°C Humidity_Data = %d％RH\r\n", FID,measure[0],measure[1]);
	}else
	sensors_char.AirTH_State = false;
}

static void I2C_SI7021_Error (void)
{
  /* 反初始化I2C通信总线 */
  HAL_I2C_DeInit(&hi2c2);
   
  /* 重新初始化I2C通信总线*/
  InitSi7021(  );
  DEBUG(2,"I2C TIMEOUT!!! I2C...\n");
}

//IIC写一个字节  
//reg:寄存器地址
//data:要写入的数据
//返回值: 无
//    其他,错误代码
uint8_t Si7021Configtrue(uint8_t reg)
{	
	HAL_StatusTypeDef state;
		
	state = HAL_I2C_Master_Transmit(&hi2c2,SI7021_WRITE_ADDR, &reg,1, 500);
	if(state!=HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Master_Transmit state111 = %d\r\n",state);
		I2C_SI7021_Error(  );
		return 0;
	}
	return 1;

}

//IIC写一个字节  
//reg:寄存器地址
//data:要写入的数据
//返回值: 无
//    其他,错误代码
uint8_t Si7021WriteOneByte(uint8_t *data, uint8_t number)
{
	uint8_t Firmware;

	HAL_StatusTypeDef state;
	
	state = HAL_I2C_Master_Transmit(&hi2c2,SI7021_WRITE_ADDR, data, number, 500);
	if(state!=HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Master_Transmit state = %d\r\n",state);
		I2C_SI7021_Error(  );
		return 0;
	}
	
	state = HAL_I2C_Master_Receive(&hi2c2,SI7021_WRITE_ADDR+1, &Firmware, 1, 500);
	if(state != HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Master_Receive state222 = %d\r\n",state);
		I2C_SI7021_Error(  );
		return 0;
	}
	
	return Firmware;
}

//IIC读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
uint16_t Si7021ReadOneByte(uint8_t reg)
{
	uint8_t result[3] = {0};
	uint16_t Had_Data = 0;

	HAL_StatusTypeDef state;

	state = HAL_I2C_Master_Transmit(&hi2c2,SI7021_WRITE_ADDR, &reg, 1, 2000);
	if(state!=HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Master_Transmit state = %d\r\n",state);
		I2C_SI7021_Error(  );
		return 0;
	}
	
	state = HAL_I2C_Master_Receive(&hi2c2,SI7021_WRITE_ADDR+1, result, 3, 2000);
	if(state != HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Master_Receive state333 = %d\r\n",state);
		I2C_SI7021_Error(  );
		return 0;
	}	
	DEBUG(3,"high33 = %02x low33 = %02x Check_num = %02x\r\n", result[0],result[1], result[2]);
	
	if(Si7021CheckCrc(result))
	{
		Had_Data = (result[0]<<8)|result[1];
	}else
	Had_Data = 0;
	
	return Had_Data;  
}

/*
 *	Si7021CheckCrc:	对sht2x读出的数据进行校验
 *	measure：		成功：1；失败：0
 */
static uint8_t Si7021CheckCrc(uint8_t data[])
{
   uint8_t crc = 0;
   uint8_t byteCtr;
   uint8_t bit;
   for (byteCtr = 0; byteCtr < 2; ++byteCtr)
    {
       crc ^= (data[byteCtr]);
       for (bit = 8; bit > 0; --bit)
       {
            if (crc & 0x80)
           	{
            	crc = (uint8_t)((crc<< 1) ^ (uint16_t)POLYNOMIAL);
           	}
            else
            {
             	crc = (crc<< 1);
            }
   		}
	}

	return (crc==data[2]);
}


