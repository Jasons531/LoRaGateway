/*
**************************************************************************************************************
*	@file	bq24195.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	用于配置bq24195电源管理芯片
***************************************************************************************************************
*/

#include "user_bq24195.h"
#include "debug.h"

extern I2C_HandleTypeDef hi2c2;

/*
 * Bq24195Init:			初始化供电电源芯片
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195Init(void)
{
	//MX_I2C2_Init();	//主要是I2C速度、地址、模式以及引脚映射的配置，用于读写相关寄存器
	return 1;
}
/*
 * EnableCharge:		使芯片开始对电池充电
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195EnableCharge(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_RESET);
	return 1;
}
/*
 * EnableCharge:		使芯片对电池不充电
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195DisableCharge(void)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, GPIO_PIN_SET);
	return 1;
}

void Bq24195Sleep(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	GPIO_InitStructure.Pin = GPIO_PIN_9;  /// PB13 PB14保留默认状态 0x9DE7 : PB3 4 9 13 14
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

}

/*
 * Bq24195WriteData:	对芯片某寄存器写入数据
 * 参数RegAdd:			芯片寄存器地址
 * data:				要写入的数据指针
 * len:					要写入的数据的长度
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195WriteData(uint8_t RegAdd,uint8_t* data,uint8_t len)
{	
	if(RegAdd>REG0A)
		return 0;
	if(HAL_I2C_Mem_Write(&hi2c2, BQ24195_WRITE_ADDR, RegAdd, 1, data, len,  1000)!=HAL_OK)
		return 0;
	return 1;
}			   

/*
 * Bq24195ReadData:		对芯片某寄存器写入数据
 * 参数RegAdd:			芯片寄存器地址
 * data:				要写入的数据指针
 * len:					要写入的数据的长度
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195ReadData(uint8_t RegAdd,uint8_t* data,uint8_t len)
{	
	if(RegAdd>REG0A)
		return 0;
	if(HAL_I2C_Mem_Read(&hi2c2,BQ24195_READ_ADDR,RegAdd,1,data,len,1000)!=HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Mem_Read error\r\n");

		return 0;
	}
	return 1;

} 

/*
 * Bq24195WriteByte:	对芯片某寄存器写入1字节数据
 * 参数RegAdd:			芯片寄存器地址
 * data_byte:			要写入的字节数据
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195WriteByte(uint8_t RegAdd,uint8_t *data_byte)
{	
	if(RegAdd>REG0A)
		return 0;
	if(HAL_I2C_Mem_Write(&hi2c2, BQ24195_WRITE_ADDR, RegAdd, 1, data_byte, 1,  1000)!=HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Mem_Write error\r\n");
		return 0;
	}
	return 1;
}			   

/*
 * Bq24195ReadByte:		读芯片某寄存器1字节数据
 * 参数RegAdd:			芯片寄存器地址
 * data_byte:			读出的字节数据
 * 返回值:				1成功 0失败
*/
uint8_t Bq24195ReadByte(uint8_t RegAdd,uint8_t *data_byte)
{	
	if(RegAdd>REG0A)
		return 0;
	if(HAL_I2C_Mem_Read(&hi2c2,BQ24195_READ_ADDR,RegAdd,1,data_byte,1,1000)!=HAL_OK)
	{
		DEBUG(2,"HAL_I2C_Mem_Read error\r\n");
		return 0;
	}
	DEBUG(3,"data = %02x\r\n",*data_byte);
	return *data_byte;
} 

//reg00
uint8_t SetHighImpedanceMode(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(INPUT_SOURCE_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~ENABLE_HIGH_IMPEDANCE_MODE;
	else if (control==ENABLE_CONTROL)  
		reg |= ENABLE_HIGH_IMPEDANCE_MODE;	
	else
		return 0;
	return Bq24195WriteByte(INPUT_SOURCE_CONTROL,&reg);
}

uint8_t SetInputVoltageLimit(uint8_t voltage)
{
	uint8_t reg;
	if (Bq24195ReadByte(INPUT_SOURCE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~INPUT_VOLTAGE_LIMIT;  
	reg |= voltage;	
	return Bq24195WriteByte(INPUT_SOURCE_CONTROL,&reg);
} 

uint8_t SetInputCurrentLimit(uint8_t current)
{
	uint8_t reg;
	if (Bq24195ReadByte(INPUT_SOURCE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~INPUT_CURRENT_LIMIT; 
	reg |= current;	 
	return Bq24195WriteByte(INPUT_SOURCE_CONTROL,&reg);
}

//reg01
uint8_t ResetDefault(void)
{
	uint8_t reg;
	if (Bq24195ReadByte(POWER_ON_CONFIG,&reg) == 0)
		return 0;
	reg |= REGISTER_RESET; 
	return Bq24195WriteByte(POWER_ON_CONFIG,&reg);
}	   	   

uint8_t ResetWdgTimer(void)
{
	uint8_t reg;
	if (Bq24195ReadByte(POWER_ON_CONFIG,&reg) == 0)
		return 0;
	reg |= WDG_TIMER_RESET; 
	return Bq24195WriteByte(POWER_ON_CONFIG,&reg);
}

uint8_t SetChargeType(uint8_t type)
{
	uint8_t reg;
	if (Bq24195ReadByte(POWER_ON_CONFIG,&reg) == 0)
		return 0;
	reg &= ~CHARGER_CONFIG;
	reg |= type; 
	return Bq24195WriteByte(POWER_ON_CONFIG,&reg);
}	   

uint8_t SetMiniSysVoltage(uint8_t voltage)
{
	uint8_t reg;
	if (Bq24195ReadByte(POWER_ON_CONFIG,&reg) == 0)
		return 0;
	reg &= ~MINI_SYSTEM_VOLTAGE;
	reg |= voltage;
	return Bq24195WriteByte(POWER_ON_CONFIG,&reg);
}	   

//reg02
uint8_t SetFastCurrentLimit(uint8_t current)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_CURRENT_CONTROL,&reg) == 0)
		return 0;
	reg &= ~FAST_CHARGE_CURRENT;  
	reg |= current;	
	return Bq24195WriteByte(CHARGE_CURRENT_CONTROL,&reg);
}

uint8_t SetForce20PEC(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_CURRENT_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~FAST_CHARGE_CURRENT;
	else if (control==ENABLE_CONTROL)  
		reg |= FAST_CHARGE_CURRENT;	
	else
		return 0;
	return Bq24195WriteByte(CHARGE_CURRENT_CONTROL,&reg);
}	  

//reg03
uint8_t SetPreChargeCurrentLimit(uint8_t current)
{
	uint8_t reg;
	if (Bq24195ReadByte(PRE_CHARGE_CURRENT_CONTROL,&reg) == 0)
		return 0;
	reg &= ~PRE_CHARGE_CURRENT_LIMIT;  
	reg |= current;	
	return Bq24195WriteByte(PRE_CHARGE_CURRENT_CONTROL,&reg);
}	    

uint8_t SetTerminationCurrentLimit(uint8_t current)
{
	uint8_t reg;
	if (Bq24195ReadByte(PRE_CHARGE_CURRENT_CONTROL,&reg) == 0)
		return 0;
	reg &= ~TERMINATION_CURRENT_LIMIT;  
	reg |= current;	
	return Bq24195WriteByte(PRE_CHARGE_CURRENT_CONTROL,&reg);
}	   	    

//reg04
uint8_t SetChargeVoltageLimit(uint8_t voltage)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_VOLTAGE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~CHARGE_VOLTAGE_LIMITE;  
	reg |= voltage;	
	return Bq24195WriteByte(CHARGE_VOLTAGE_CONTROL,&reg);
}	   	    

uint8_t SetBatteryLowThreshold(uint8_t voltage)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_VOLTAGE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~BATLOW_THRESHOLD;  
	reg |= voltage;	
	return Bq24195WriteByte(CHARGE_VOLTAGE_CONTROL,&reg);
}		  	    

uint8_t SetBatteryRechargeThreshold(uint8_t voltage)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_VOLTAGE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~BTATTERY_RECHARGE_THRESHOLD;  
	reg |= voltage;	
	return Bq24195WriteByte(CHARGE_VOLTAGE_CONTROL,&reg);
}		  	    	   		  	    

//reg05
uint8_t SetChargeTerminate(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_TERMINATE_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~CHARGE_TERMINATE_ENABLE;
	else if (control==ENABLE_CONTROL)  
		reg |= CHARGE_TERMINATE_ENABLE;	
	else
		return 1;
	return Bq24195WriteByte(CHARGE_TERMINATE_CONTROL,&reg);
} 	  	    

uint8_t SetChargeTerminateThreshold(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_TERMINATE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~CHARGE_TERMINATE_THRESHOLD;  
	reg |= control;	
	return Bq24195WriteByte(CHARGE_TERMINATE_CONTROL,&reg);
}		  	    

uint8_t SetWdgTimer(uint8_t time)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_TERMINATE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~WATCHER_DOG_TIMER;  
	reg |= time;	
	return Bq24195WriteByte(CHARGE_TERMINATE_CONTROL,&reg);
}	

uint8_t SetEnableChargeTimer(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_TERMINATE_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~ENABLE_CHARGING_TIMER;
	else if (control==ENABLE_CONTROL)  
		reg |= ENABLE_CHARGING_TIMER;	
	else
		return 0;
	return Bq24195WriteByte(CHARGE_TERMINATE_CONTROL,&reg);
} 	  	    

uint8_t SetFastChargeTimer(uint8_t time)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_TERMINATE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~FAST_CHARGE_TIMER;  
	reg |= time;	
	return Bq24195WriteByte(CHARGE_TERMINATE_CONTROL,&reg);
}	

//reg06
uint8_t SetTermalRegulationThreshold(uint8_t termal)
{
	uint8_t reg;
	if (Bq24195ReadByte(CHARGE_TERMINATE_CONTROL,&reg) == 0)
		return 0;
	reg &= ~TERMAL_REGULATION_THRESHOLD;  
	reg |= termal;	
	return Bq24195WriteByte(TERMAL_REGULATION_CONTROL,&reg);
}

//reg07
uint8_t SetForceDPDMDetection(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(MISC_OPERATION_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~FORCE_DPDM_DETECTION;
	else if (control==ENABLE_CONTROL)  
		reg |= FORCE_DPDM_DETECTION;	
	else
		return 0;
	return Bq24195WriteByte(MISC_OPERATION_CONTROL,&reg);
}

uint8_t SetSlowedSatelyTimer(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(MISC_OPERATION_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~SLOWED_SAFELY_TIMER;
	else if (control==ENABLE_CONTROL)  
		reg |= SLOWED_SAFELY_TIMER;	
	else
		return 0;
	return Bq24195WriteByte(MISC_OPERATION_CONTROL,&reg);
}	   

uint8_t SetForceBatfetOff(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(MISC_OPERATION_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~FORCE_BATFET_OFF;
	else if (control==ENABLE_CONTROL)  
		reg |= FORCE_BATFET_OFF;	
	else
		return 0;
	return Bq24195WriteByte(MISC_OPERATION_CONTROL,&reg);
}		 

uint8_t SetEnableChargeFaultInterrupt(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(MISC_OPERATION_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~CHARGE_FAULT_INT_ENABLE;
	else if (control==ENABLE_CONTROL)  
		reg |= CHARGE_FAULT_INT_ENABLE;	
	else
		return 0;
	return Bq24195WriteByte(MISC_OPERATION_CONTROL,&reg);
}		   

uint8_t SetEnableBatteryFaultInterrupt(uint8_t control)
{
	uint8_t reg;
	if (Bq24195ReadByte(MISC_OPERATION_CONTROL,&reg) == 0)
		return 0;
	if (control==DISABLE_CONTROL)
		reg &= ~BATTERY_FAULT_INT_ENABLE;
	else if (control==ENABLE_CONTROL)  
		reg |= BATTERY_FAULT_INT_ENABLE;	
	else
		return 0;
	return Bq24195WriteByte(MISC_OPERATION_CONTROL,&reg);
}

//reg08
uint8_t ReadSystemStatue(void)
{
	uint8_t reg;
	Bq24195ReadByte(SYSTEM_STATUE,&reg);
	return reg;
}

//reg09
uint8_t ReadFaultRegisiter(void)
{ 
	uint8_t reg;
	Bq24195ReadByte(FAULT_REGISTER,&reg);
	return reg;
}

//reg0a
uint8_t ReadVision(void)
{ 
	uint8_t reg;
	Bq24195ReadByte(REVISION_STATUS,&reg);
	return reg;
}

//tool
void PrintfBin(uint8_t d)
{
	uint8_t i;
	for (i=0;i<8;i++)
		printf("%d",(d&(1<<(7-i)))>>(7-i));
	printf("\r\n");
}

uint8_t AnalysisRegisiter(uint8_t reg)
{
	uint8_t result,aux;
	if (reg>0x0a)
		return 1;
	printf("\r\nreg%02d:",reg);
	Bq24195ReadByte(reg,&result);
	printf("%02x,",result);
	PrintfBin(result);
	switch(reg)
	{
		case INPUT_SOURCE_CONTROL:
			printf("high impedance mode:%s\r\n",result&ENABLE_HIGH_IMPEDANCE_MODE?"enable":"disable");
			printf("input voltage limit:%d\r\n",(3880
				+(result&INPUT_VOLTAGE_640MV?640:0)
				+(result&INPUT_VOLTAGE_320MV?320:0)
				+(result&INPUT_VOLTAGE_160MV?160:0)
				+(result&INPUT_VOLTAGE_80MV?80:0))); 
			aux = result&INPUT_CURRENT_LIMIT;
			printf("input current limit:%d\r\n",
				aux==INPUT_CURRENT_100MA?100:
				aux==INPUT_CURRENT_150MA?150:
				aux==INPUT_CURRENT_500MA?500:
				aux==INPUT_CURRENT_900MA?900:
				aux==INPUT_CURRENT_1200MA?1200:
				aux==INPUT_CURRENT_1500MA?1500:
				aux==INPUT_CURRENT_2000MA?2000:
				aux==INPUT_CURRENT_3000MA?3000:0);
			break; 
		case POWER_ON_CONFIG:
			aux = result&CHARGER_CONFIG;
			printf("charge config:%s\r\n",
				aux==CHARGER_DISABLE?"disable":
				aux==CHARGER_BATTERY?"battery":
				aux==CHARGER_OTG?"otg":"data fault"); 
			printf("mini system voltage:%d\r\n",(3000
				+(result&SYSTEM_VOLTAGE_400MV?400:0)
				+(result&SYSTEM_VOLTAGE_200MV?200:0)
				+(result&SYSTEM_VOLTAGE_100MV?100:0))); 
			break;
		case CHARGE_CURRENT_CONTROL:
			printf("Force20pct:%s\r\n",result&FORCE_20PEC?"enable":"disable");
			printf("fast charge current limit:%d\r\n",(512
				+(result&FAST_CHARGE_2048MA?2048:0)
				+(result&FAST_CHARGE_1024MA?1024:0)
				+(result&FAST_CHARGE_512MA?512:0)
				+(result&FAST_CHARGE_256MA?256:0)
				+(result&FAST_CHARGE_128MA?128:0)
				+(result&FAST_CHARGE_64MA?64:0))); 
			break;
		case PRE_CHARGE_CURRENT_CONTROL:
			printf("precharge current limit:%d\r\n",(128
				+(result&PRE_CHARGE_CURRENT_1024MA?1024:0)
				+(result&PRE_CHARGE_CURRENT_512MA?512:0)
				+(result&PRE_CHARGE_CURRENT_256MA?256:0)
				+(result&PRE_CHARGE_CURRENT_128MA?128:0))); 
			printf("termination current limit:%d\r\n",(128
				+(result&TERMINATION_CURRENT_1024MA?1024:0)
				+(result&TERMINATION_CURRENT_512MA?512:0)
				+(result&TERMINATION_CURRENT_256MA?256:0)
				+(result&TERMINATION_CURRENT_128MA?128:0)));
			break;
		case CHARGE_VOLTAGE_CONTROL:
			printf("charge voltage limit:%d\r\n",(3504
				+(result&CHARGE_VOLTAGE_512MV?512:0)
				+(result&CHARGE_VOLTAGE_256MV?256:0)
				+(result&CHARGE_VOLTAGE_128MV?128:0)
				+(result&CHARGE_VOLTAGE_64MV?64:0)
				+(result&CHARGE_VOLTAGE_32MV?32:0)
				+(result&CHARGE_VOLTAGE_16MV?16:0)));
			printf("battery low threshold:%dmv\r\n",result&BATLOW_THRESHOLD?3000:2800); 
			printf("battery recharge threshold:%dmv\r\n",result&BTATTERY_RECHARGE_THRESHOLD?300:100); 
			break;
		case CHARGE_TERMINATE_CONTROL:
			printf("charge termination:%s\r\n",result&CHARGE_TERMINATE_ENABLE?"enable":"disable");
			printf("charge termination threshold:%s\r\n",result&CHARGE_TERMINATE_THRESHOLD?"stat high":"match item"); 
			aux = result&CHARGER_CONFIG;
			printf("wdg timer:%s\r\n",
				aux==WATCHER_DOG_TIMER_40S?"40s":
				aux==WATCHER_DOG_TIMER_80S?"80s":	
				aux==WATCHER_DOG_TIMER_160S?"160s":
				aux==WATCHER_DOG_TIMER_DISABLE?"disable":"fault");	
			printf("charge timer:%s\r\n",result&ENABLE_CHARGING_TIMER?"enable":"disable"); 
			aux = result&FAST_CHARGE_TIMER;
			printf("charge timer:%s\r\n",
				aux==FAST_CHARGE_5HOURS?"5hs":
				aux==FAST_CHARGE_8HOURS?"8hs":	
				aux==FAST_CHARGE_12HOURS?"12hs":
				aux==FAST_CHARGE_20HOURS?"20hs":"fault");
			break;
		case TERMAL_REGULATION_CONTROL:	
			aux = result&TERMAL_REGULATION_THRESHOLD;
			printf("termal regulation threshold:%s\r\n",
				aux==REGULATHION_THRESHOLD_60C?"60c":
				aux==REGULATHION_THRESHOLD_80C?"80c":	
				aux==REGULATHION_THRESHOLD_100C?"100c":
				aux==REGULATHION_THRESHOLD_120C?"120c":"fault");
			break;
		case MISC_OPERATION_CONTROL:
			printf("force dpm:%s\r\n",result&FORCE_DPDM_DETECTION?"enable":"disable");
			printf("slowed sately timer:%s\r\n",result&SLOWED_SAFELY_TIMER?"enable":"disable");
			printf("force batfet off:%s\r\n",result&FORCE_BATFET_OFF?"enable":"disable");
			printf("charge fault interrupt:%s\r\n",result&CHARGE_FAULT_INT_ENABLE?"enable":"disable");
			printf("battery fault interrupt:%s\r\n",result&BATTERY_FAULT_INT_ENABLE?"enable":"disable");
			break;
		case SYSTEM_STATUE:	
			aux = result&VBUS_STATUE;
			printf("vbus statue:%s\r\n",
				aux==VBUS_USB?"usb":
				aux==VBUS_ADAPTER?"adapter":	
				aux==VBUS_OTG?"otg":"uknow");	  
			aux = result&CHARGE_STATUE;
			printf("charge statue:%s\r\n",
				aux==CHARGE_PRE_CHARGING?"pre charging":
				aux==CHARGE_FAST_CHARGING?"fast charging":	
				aux==CHARGE_DONE_CHARGING?"charge done":"not charging");
			printf("dpm statue:%s\r\n",result&DPM_STATUE?"in dpm statue":"not in dpm statue");
			printf("power statue:%s\r\n",result&POWER_GOOD_STATUE?"good":"not good");
			printf("thremal statue:%s\r\n",result&THERM_STATUE?"in thremal regulation":"normal");
			printf("vsystem statue:%s\r\n",result&VSYSTEM_STATUE?"bat<vsys":"bat>vsys");		
			break;
		case FAULT_REGISTER:
			printf("wdg:%s\r\n",result&WATCHDOG_FAULT?"wdg timer expirtion":"normal");	  
			aux = result&CHARGE_FAULT;
			printf("charge:%s\r\n",
				aux==CHARGE_NORMAL?"normal":
				aux==CHARGE_INPUT_FAULT?"input fault":	
				aux==CHARGE_THERMAL_SHUTDOWN?"thremal shutdown":
				"timer expiration");
			printf("battery:%s\r\n",result&BATTERY_FAULT?"over voltage protect":"normal");	  
			aux = result&NTC_FAULT;
			printf("ntc:%s\r\n",
				aux==NTC_COLD?"cold":
				aux==NTC_HOT?"hot":	
				aux==NTC_NORMAL?"normal":
				"data fault");
			break;
	}
	return result;
}

