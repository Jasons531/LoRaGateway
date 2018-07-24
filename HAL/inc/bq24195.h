/*
**************************************************************************************************************
*	@file	bq24195.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	配置文件
***************************************************************************************************************
*/
#ifndef __BQ24195_H
#define __BQ24195_H	 

#include "stm32l0xx_hal.h"

#ifdef __cplusplus
	extern "C" {
#endif

//0x??为mask位，0x??<<#为数据位（函数参数）

#define BQ24195_WRITE_ADDR	 		0xD6 
#define BQ24195_READ_ADDR	 		0xD7  
#define REG00						0x00 
#define REG01						0x01 
#define REG02						0x02 
#define REG03						0x03 
#define REG04						0x04 
#define REG05						0x05 
#define REG06						0x06 
#define REG07						0x07 
#define REG08						0x08 
#define REG09						0x09 
#define REG0A						0x0A

#define ENABLE_CONTROL				0x01
#define DISABLE_CONTROL				0x00

#define INPUT_SOURCE_CONTROL		REG00  
#define ENABLE_HIGH_IMPEDANCE_MODE	(0x80)
#define INPUT_VOLTAGE_LIMIT			(0x78)
#define INPUT_VOLTAGE_3880MV		(0x0<<6)
#define INPUT_VOLTAGE_640MV			(0x1<<6)
#define INPUT_VOLTAGE_320MV			(0x1<<5)
#define INPUT_VOLTAGE_160MV			(0x1<<4)
#define INPUT_VOLTAGE_80MV			(0x1<<3)
#define INPUT_CURRENT_LIMIT			(0x07)
#define INPUT_CURRENT_100MA			(0x0<<0)
#define INPUT_CURRENT_150MA			(0x1<<0)
#define INPUT_CURRENT_500MA			(0x2<<0)
#define INPUT_CURRENT_900MA			(0x3<<0)
#define INPUT_CURRENT_1200MA		(0x4<<0)
#define INPUT_CURRENT_1500MA		(0x5<<0)  
#define INPUT_CURRENT_2000MA		(0x6<<0) 
#define INPUT_CURRENT_3000MA		(0x7<<0)

#define POWER_ON_CONFIG				REG01   
#define REGISTER_RESET				(0x80)																						  
#define WDG_TIMER_RESET				(0x40)																						  
#define CHARGER_CONFIG				(0x30)																							  
#define CHARGER_DISABLE				(0x0<<4)																					  
#define CHARGER_BATTERY				(0x1<<4)																						  
#define CHARGER_OTG					(0x2<<4)	
#define MINI_SYSTEM_VOLTAGE			(0x0E)
#define SYSTEM_VOLTAGE_3000MV		(0x0<<3)
#define SYSTEM_VOLTAGE_400MV		(0x1<<3)
#define SYSTEM_VOLTAGE_200MV		(0x1<<2)
#define SYSTEM_VOLTAGE_100MV		(0x1<<1) 

#define CHARGE_CURRENT_CONTROL		REG02 
#define FAST_CHARGE_CURRENT			(0xFC)
#define FAST_CHARGE_OFFSET_512MA	(0x0<<7)
#define FAST_CHARGE_2048MA			(0x1<<7)
#define FAST_CHARGE_1024MA			(0x1<<6)
#define FAST_CHARGE_512MA			(0x1<<5)
#define FAST_CHARGE_256MA			(0x1<<4)
#define FAST_CHARGE_128MA			(0x1<<3)	
#define FAST_CHARGE_64MA			(0x1<<2)
#define FORCE_20PEC					(0x01)
  
#define PRE_CHARGE_CURRENT_CONTROL	REG03 
#define PRE_CHARGE_CURRENT_LIMIT	(0xF0)
#define PRE_CHARGE_CURRENT_1024MA	(0x1<<7)
#define PRE_CHARGE_CURRENT_512MA	(0x1<<6)
#define PRE_CHARGE_CURRENT_256MA	(0x1<<5)
#define PRE_CHARGE_CURRENT_128MA	(0x1<<4)
#define TERMINATION_CURRENT_LIMIT	(0x0F)
#define TERMINATION_CURRENT_1024MA	(0x1<<3)
#define TERMINATION_CURRENT_512MA	(0x1<<2)
#define TERMINATION_CURRENT_256MA	(0x1<<1)
#define TERMINATION_CURRENT_128MA	(0x1<<0)
#define CHARGE_CURRENT_OFFSET_128MA	(0x0)
  
#define CHARGE_VOLTAGE_CONTROL		REG04 
#define CHARGE_VOLTAGE_LIMITE		(0xFC)
#define CHARGE_VOLTAGE_3504MV		(0x0<<7)
#define CHARGE_VOLTAGE_512MV		(0x1<<7)
#define CHARGE_VOLTAGE_256MV		(0x1<<6)
#define CHARGE_VOLTAGE_128MV		(0x1<<5)
#define CHARGE_VOLTAGE_64MV			(0x1<<4)
#define CHARGE_VOLTAGE_32MV			(0x1<<3)
#define CHARGE_VOLTAGE_16MV			(0x1<<2)
#define BATLOW_THRESHOLD			(0x02)	
#define BATLOW_2800MV				(0x0<<1)
#define BATLOW_3000MV				(0x1<<1)
#define BTATTERY_RECHARGE_THRESHOLD	(0x01)
#define BTATTERY_RECHARGE_100MV		(0x0<<0)
#define BTATTERY_RECHARGE_300MV		(0x1<<0)

#define CHARGE_TERMINATE_CONTROL	REG05
#define CHARGE_TERMINATE_ENABLE		(0x80)
#define CHARGE_TERMINATE_THRESHOLD	(0x40)
#define CHARGE_TERMINATE_MATCH_ITEM	(0x0<<6)
#define CHARGE_TERMINATE_STAT_HIGH	(0x1<<6)
#define WATCHER_DOG_TIMER			(0x30)
#define WATCHER_DOG_TIMER_DISABLE	(0x0<<4)
#define WATCHER_DOG_TIMER_40S		(0x1<<4)
#define WATCHER_DOG_TIMER_80S		(0x2<<4)
#define WATCHER_DOG_TIMER_160S		(0x3<<4)
#define ENABLE_CHARGING_TIMER		(0x08)
#define FAST_CHARGE_TIMER			(0x06)
#define FAST_CHARGE_5HOURS			(0x0<<1)
#define FAST_CHARGE_8HOURS			(0x1<<1)
#define FAST_CHARGE_12HOURS			(0x2<<1) 
#define FAST_CHARGE_20HOURS			(0x3<<1)
   
#define TERMAL_REGULATION_CONTROL	REG06
#define TERMAL_REGULATION_THRESHOLD	(0x03)
#define REGULATHION_THRESHOLD_60C	(0x0)
#define REGULATHION_THRESHOLD_80C	(0x1)
#define REGULATHION_THRESHOLD_100C	(0x2)
#define REGULATHION_THRESHOLD_120C	(0x3)
   
#define MISC_OPERATION_CONTROL		REG07
#define FORCE_DPDM_DETECTION		(0x80)
#define SLOWED_SAFELY_TIMER			(0x40)
#define FORCE_BATFET_OFF			(0x20)
#define CHARGE_FAULT_INT_ENABLE		(0x02)
#define BATTERY_FAULT_INT_ENABLE	(0x01)
   
#define SYSTEM_STATUE				REG08
#define VBUS_STATUE					(0xC0)
#define VBUS_UNKNOW					(0x0<<6)
#define VBUS_USB					(0x1<<6)
#define VBUS_ADAPTER				(0x2<<6)
#define VBUS_OTG					(0x3<<6)
#define CHARGE_STATUE 				(0x30)
#define CHARGE_NOT_CHARGING			(0x0<<4)
#define CHARGE_PRE_CHARGING			(0x1<<4)
#define CHARGE_FAST_CHARGING		(0x2<<4)
#define CHARGE_DONE_CHARGING		(0x3<<4)
#define DPM_STATUE					(0x08)	 
#define POWER_GOOD_STATUE			(0x04)  
#define THERM_STATUE				(0x02) 
#define VSYSTEM_STATUE				(0x01)
  
#define FAULT_REGISTER				REG09 
#define WATCHDOG_FAULT				(0x80)
#define CHARGE_FAULT				(0x30)
#define CHARGE_NORMAL				(0x0<<4)
#define CHARGE_INPUT_FAULT			(0x1<<4)
#define CHARGE_THERMAL_SHUTDOWN		(0x2<<4)
#define CHARGE_TIMER_EXPIRATION		(0x3<<4)
#define BATTERY_FAULT				(0x08)
#define NTC_FAULT					(0x07)
#define NTC_NORMAL					(0x0<<3) 
#define NTC_COLD					(0x5<<3) 
#define NTC_HOT						(0x6<<3)
  
#define REVISION_STATUS				REG0A  		

void InitPower(void);
void InitBq24195(void);
void PrintfBin(uint8_t d);
void EnableCharge(void);
uint8_t AnalysisRegisiter(uint8_t reg);
uint8_t SingleRead(uint8_t addr);
uint8_t SingleWrite(uint8_t addr,uint8_t data);	
uint8_t MultiRead(uint8_t addr,uint8_t len,uint8_t* data);
uint8_t MultiWrite(uint8_t addr,uint8_t len,uint8_t* data);
//reg00
uint8_t SetHighImpedanceMode(uint8_t control);
uint8_t SetInputVoltageLimit(uint8_t voltage);
uint8_t SetInputCurrentLimit(uint8_t current);
//reg01
uint8_t ResetDefault(void);
uint8_t ResetWdgTimer(void);
uint8_t SetChargeType(uint8_t type);
uint8_t SetMiniSysVoltage(uint8_t voltage);
//reg02
uint8_t SetFastCurrentLimit(uint8_t current);
uint8_t SetForce20PEC(uint8_t control);
//reg03
uint8_t SetPreChargeCurrentLimit(uint8_t current);
uint8_t SetTerminationCurrentLimit(uint8_t current);
//reg04
uint8_t SetChargeVoltageLimit(uint8_t voltage);
uint8_t SetBatteryLowThreshold(uint8_t voltage);
uint8_t SetBatteryRechargeThreshold(uint8_t voltage);
//reg05
uint8_t SetChargeTerminate(uint8_t control);
uint8_t SetChargeTerminateThreshold(uint8_t control);
uint8_t SetWdgTimer(uint8_t time);
uint8_t SetEnableChargeTimer(uint8_t control);
uint8_t SetFastChargeTimer(uint8_t time);
//reg06
uint8_t SetTermalRegulationThreshold(uint8_t termal);
//reg07
uint8_t SetForceDPDMDetection(uint8_t control);
uint8_t SetSlowedSatelyTimer(uint8_t control);
uint8_t SetForceBatfetOff(uint8_t control);
uint8_t SetEnableChargeFaultInterrupt(uint8_t control);
uint8_t SetEnableBatteryFaultInterrupt(uint8_t control);
//reg08
uint8_t ReadSystemStatue(void);
//reg09
uint8_t ReadFaultRegisiter(void);	
//reg0a										  
uint8_t ReadVision(void);


#ifdef __cplusplus
}
#endif

#endif
