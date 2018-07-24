/*
**************************************************************************************************************
*	@file	max44009.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	
***************************************************************************************************************
*/
#include <stdint.h>
#include <math.h>
#include "max44009.h"
#include "delay.h"
#include "i2c.h"
#include "sensor.h"

#define MAX44009_WRITE_ADDR		0x96	// MAX44009的设备地址

uint8_t LUX_ADDR = 0x03;

/*
 *	InitMax44009:		初始化max44009地址
 *	返回值: 			
 */
void InitMax44009(void)
{
	MX_I2C2_Init( );
}

/*
 *	Max44009ReadLux:	读取Max44009的光照度数据
 *	返回值: 			光照度，放大100倍
 */
int32_t Max44009ReadLux(void)
{
	uint8_t data[2],i,count=0;
	uint32_t mantissa, exp;
	uint32_t lux = 0;
	HAL_StatusTypeDef state;

	for(i=0;i<10;i++)
	{		
		for(uint8_t j = 0; j < 2; j++)
		{
			state = HAL_I2C_Master_Transmit(&hi2c2,MAX44009_WRITE_ADDR, &LUX_ADDR+j, 1, 2000);
			if(state!=HAL_OK)
			{
				DEBUG(3,"HAL_I2C_Master_Transmit state = %d\r\n",state);
				return 0;
			}
			
			state = HAL_I2C_Master_Receive(&hi2c2,MAX44009_WRITE_ADDR+1, &data[j], 1, 2000);
			if(state != HAL_OK)
			{
				DEBUG(3,"HAL_I2C_Master_Receive state = %d\r\n",state);
				
				if(state == HAL_TIMEOUT)
				{
					I2C_Max44009_Error(  );
				}
				
				sensors_char.Max44009_State = false;
				
				return 0;
			}else
				sensors_char.Max44009_State = true;
			HAL_Delay( 50 );
		}

		mantissa = ((data[0]&0xF)<<4) | (data[1]&0xF);
		exp = (data[0]&0xF0)>>4;
		DEBUG(4,"max44009 raw data;%x %x %x %x\r\n",data[0],data[1],exp,mantissa);
		if (exp != 0xf)									//不对exp进行有效性判断了，因为测出的数据有mantissa为0 exp不为0的情况
		{
			lux += (int)(exp2(exp))*mantissa*0.0455;	
			count++;
		}

	}
	DEBUG(3,"lux = %d\r\n", lux/count);

	return count>0?lux/count:0;				//如果读出有效数据则返回有效读取次数的平均值，否则返回0
}

static void I2C_Max44009_Error (void)
{
  /* 反初始化I2C通信总线 */
  HAL_I2C_DeInit(&hi2c2);
   
  /* 重新初始化I2C通信总线*/
  InitMax44009(  );
  DEBUG(3,"Max44009 I2C TIMEOUT!!! I2C...\n");
}
