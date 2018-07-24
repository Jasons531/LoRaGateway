/*
**************************************************************************************************************
*	@file	sht2x.c
*	@author 
*	@version 
*	@date    
*	@brief	用软件模拟I2C读取SHT2X的温度和湿度数据，因为STM32的I2C有点BUG，不大好用
***************************************************************************************************************
*/

#include "sht2x.h"
#include "delay.h"
#include "i2c.h"

/*********************************************************************
 * 与SHT2x通讯的接口
 */

#define SHT2x_WRITE_ADDR	0x80	// SHT2x的设备地址
#define SHT2x_NO_HOLD_T		0xF3	// NO HOLD模式，测量温度
#define SHT2x_NO_HOLD_H		0xF5	// NO HOLD模式，测量湿度
#define POLYNOMIAL 			0x31	//sht2x使用的crc8校验多项式

/*
 *	SHT2xCheckCrc:	对sht2x读出的数据进行校验
 *	measure：		成功：1；失败：0
 */
static uint8_t SHT2xCheckCrc(uint8_t data[])
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

/*
 *	Sht2xReadTH:	读取STH2x的温度和湿度数据
 *	measure：		存储温度和湿度数据，放大了10倍
 */
void Sht2xReadTH(int32_t *measure)
{
	uint8_t result[3],i,num;
	uint16_t temperature, humidity;	 

	// 读取温度
	measure[0] = 0;	//清空数据
	num = 0;		//读取次数清零
	for(i=0;i<10;i++)
	{
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR);
		I2cCheckAck();
		I2cWritex(SHT2x_NO_HOLD_T);	// 发次一次测量
		I2cCheckAck();
		delay_us(20);	// 手册说需要20us延时
		I2cStop();
		delay_ms(90);		// 最大延时85ms，这里增加延时时间，后面就不判断，直接读取
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR+1);
		I2cCheckAck();
		result[0] = I2cReadx();		//高位数据
		I2cAck();
		result[1] = I2cReadx(); 	//低位数据
		I2cAck();
		result[2] = I2cReadx();		//crc校验数据
			
		I2cNack();					// 最后一个字节无需应答	 
		I2cStop();
		DEBUG(4,"%d,sht2x raw data,%x,%x,%x\r\n",RTC_CNT,result[0],result[1],result[2]);	
		if (SHT2xCheckCrc(result))	//crc校验通过
		{
			temperature = ((result[0]<<8)|result[1])&(~0x3);
			measure[0] += (-46.85 + 175.72*temperature/65536)*10;	// 放大10倍，即保留了小数点后一位
			num++;												   	//读取成功次数+1
		}
	}
	if (num>0)
		measure[0] = measure[0]/num;		  						//读取到有效数据则除以有效读出的次数
	
		
	// 读取湿度
	measure[1] = 0;	//清空数据
	num = 0;
	for(i=0;i<10;i++)
	{
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR);
		I2cCheckAck();
		I2cWritex(SHT2x_NO_HOLD_H);	// 发次一次测量
		I2cCheckAck();
		delay_us(20);	// 手册说需要20us延时
		I2cStop();
		delay_ms(35);	// 最大延时29ms，这里增加延时时间，后面就不判断，直接读取
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR+1);
		I2cCheckAck();
		result[0] = I2cReadx();		//高位数据
		I2cAck();
		result[1] = I2cReadx(); 	//低位数据
		I2cAck();
		result[2] = I2cReadx();		//crc校验数据
				
		I2cNack();	// 最后一个字节无需应答	 
		I2cStop();
		DEBUG(4,"%d,sht2x raw data,%x,%x,%x\r\n",RTC_CNT,result[0],result[1],result[2]);	
		if (SHT2xCheckCrc(result))	  //crc校验通过
		{
			humidity = ((result[0]<<8)|result[1])&(~0x3);
			measure[1] += (-6 + 125.0*humidity/65536)*10;	// 放大10倍，即保留了小数点后一位
			num++;											//读取成功次数+1
		}
	}
	if (num>0)
		measure[1] = measure[1]/num;		  				//读取到有效数据则除以有效读出的次数
}
