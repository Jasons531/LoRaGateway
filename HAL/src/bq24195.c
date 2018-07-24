/*
**************************************************************************************************************
*	@file	bq24195.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	��������bq24195��Դ����оƬ
***************************************************************************************************************
*/

#include "i2c.h"
#include "bq24195.h"
#include "user_bq24195.h"
#include "delay.h"
#include "debug.h"
#include "power.h"
#include "user-app.h"

#define BATTERY_DEVICE		1	//����豸
#define NON_BATTERY_DEVICE	0	//�ǵ���豸

#define SET_BQ24195  		1

uint8_t device_type = NON_BATTERY_DEVICE;//�豸����

void InitPower(void)
{
	InitBq24195();				//��Դ�����ʼ������ʼ��i2c�Լ���Ӧ�жϽ�
	
	Bq24195EnableCharge( );	
	
	//161208��¼����ǿ���趨�豸����Ϊ����豸�������ĵ�һ��watcher��gprs v2��lora�ڵ��Ϊ�������ֱ������Դ���豸��
	device_type = BATTERY_DEVICE;

#if SET_BQ24195  ///��������ó�磺������ô�������
	
	device_type = BATTERY_DEVICE;

	SetHighImpedanceMode(DISABLE_CONTROL);		//�������̬״̬	
	SetMiniSysVoltage(SYSTEM_VOLTAGE_3000MV		//������Сϵͳ��ѹ3.7v
		+SYSTEM_VOLTAGE_400MV
		+SYSTEM_VOLTAGE_200MV
		+SYSTEM_VOLTAGE_100MV);
	SetWdgTimer(WATCHER_DOG_TIMER_DISABLE);		//����оƬ��д���Ź�
	SetInputVoltageLimit(INPUT_VOLTAGE_3880MV
		+INPUT_VOLTAGE_640MV
		+INPUT_VOLTAGE_320MV
		+INPUT_VOLTAGE_160MV
		+INPUT_VOLTAGE_80MV);//���������ѹ
	SetInputCurrentLimit(INPUT_CURRENT_100MA);	//���������������2A
	SetEnableChargeTimer(DISABLE_CONTROL);		//���ó�糬ʱʱ��		

#endif
	if (device_type==BATTERY_DEVICE)			//����Ǵ�����豸
	{
		//SetChargeType(CHARGER_BATTERY);	//����������
		//������Ƶ�ѹ=4.016v
		//SetChargeVoltageLimit(CHARGE_VOLTAGE_3504MV+CHARGE_VOLTAGE_512MV);
		//���³����ֵΪ��300mv����3.716v��
		SetBatteryRechargeThreshold(BTATTERY_RECHARGE_300MV);
		DEBUG(3,"BATTERY_DEVICE \r\n");
		if (CheckBattery()<=3)					//��ص�����
		{
			DEBUG(2,"battery extremely low %dmV;enter standby\r\n",LoRapp_Handle.Battery*6+3600);
			IntoLowPower( ); 					//��ص����ر�ͣ�ֱ�����ý�������
		}
	}
	else
	{
		SetChargeType(CHARGER_DISABLE);			//���ǵ���豸����ó��
	}
}
  
/*
*PB8  PB9�ֿ���ʼ�������������жϡ�GPIO��ʼ������
*/
void InitBq24195(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();           //����GPIOBʱ��

	GPIO_Initure.Pin=GPIO_PIN_9; //PB9 
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
	GPIO_Initure.Pull=GPIO_NOPULL;          //����
	GPIO_Initure.Speed=GPIO_SPEED_LOW;     //����
	
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);  
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);	//PB9��1

	GPIO_Initure.Pin=GPIO_PIN_8;  
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      			//�����ش����������ж�Ӱ����
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
		//�ж���4-PC15
	HAL_NVIC_SetPriority(EXTI4_15_IRQn,7,0);       //��ռ���ȼ�Ϊ0�������ȼ�Ϊ0
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);             //ʹ���ж���

}

void EnableCharge(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
	HAL_Delay(100);  						
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
}
