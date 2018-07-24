/*
**************************************************************************************************************
*	@file	bq24195.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	用于配置bq24195电源管理芯片
***************************************************************************************************************
*/

#include "i2c.h"
#include "bq24195.h"
#include "user_bq24195.h"
#include "delay.h"
#include "debug.h"
#include "power.h"
#include "user-app.h"

#define BATTERY_DEVICE		1	//电池设备
#define NON_BATTERY_DEVICE	0	//非电池设备

#define SET_BQ24195  		1

uint8_t device_type = NON_BATTERY_DEVICE;//设备类型

void InitPower(void)
{
	InitBq24195();				//电源管理初始化，初始化i2c以及相应中断脚
	
	Bq24195EnableCharge( );	
	
	//161208烧录工程强制设定设备类型为电池设备，量产的第一批watcher和gprs v2、lora节点均为带电池且直接连电源的设备。
	device_type = BATTERY_DEVICE;

#if SET_BQ24195  ///不软件配置充电：软件配置存在问题
	
	device_type = BATTERY_DEVICE;

	SetHighImpedanceMode(DISABLE_CONTROL);		//清除高阻态状态	
	SetMiniSysVoltage(SYSTEM_VOLTAGE_3000MV		//设置最小系统电压3.7v
		+SYSTEM_VOLTAGE_400MV
		+SYSTEM_VOLTAGE_200MV
		+SYSTEM_VOLTAGE_100MV);
	SetWdgTimer(WATCHER_DOG_TIMER_DISABLE);		//禁用芯片读写看门狗
	SetInputVoltageLimit(INPUT_VOLTAGE_3880MV
		+INPUT_VOLTAGE_640MV
		+INPUT_VOLTAGE_320MV
		+INPUT_VOLTAGE_160MV
		+INPUT_VOLTAGE_80MV);//限制输入电压
	SetInputCurrentLimit(INPUT_CURRENT_100MA);	//设置输入电流限制2A
	SetEnableChargeTimer(DISABLE_CONTROL);		//禁用充电超时时间		

#endif
	if (device_type==BATTERY_DEVICE)			//如果是带电池设备
	{
		//SetChargeType(CHARGER_BATTERY);	//加入有问题
		//充电限制电压=4.016v
		//SetChargeVoltageLimit(CHARGE_VOLTAGE_3504MV+CHARGE_VOLTAGE_512MV);
		//重新充电阈值为低300mv（即3.716v）
		SetBatteryRechargeThreshold(BTATTERY_RECHARGE_300MV);
		DEBUG(3,"BATTERY_DEVICE \r\n");
		if (CheckBattery()<=3)					//电池电量低
		{
			DEBUG(2,"battery extremely low %dmV;enter standby\r\n",LoRapp_Handle.Battery*6+3600);
			IntoLowPower( ); 					//电池电量特别低，直接重置进入休眠
		}
	}
	else
	{
		SetChargeType(CHARGER_DISABLE);			//不是电池设备则禁用充电
	}
}
  
/*
*PB8  PB9分开初始化，否则会出现中断、GPIO初始化覆盖
*/
void InitBq24195(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟

	GPIO_Initure.Pin=GPIO_PIN_9; //PB9 
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_NOPULL;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_LOW;     //高速
	
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);  
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);	//PB9置1

	GPIO_Initure.Pin=GPIO_PIN_8;  
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      			//上升沿触发：开启中断影响充电
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
		//中断线4-PC15
	HAL_NVIC_SetPriority(EXTI4_15_IRQn,7,0);       //抢占优先级为0，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);             //使能中断线

}

void EnableCharge(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
	HAL_Delay(100);  						
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
}
