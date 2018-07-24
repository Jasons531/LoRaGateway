/*
**************************************************************************************************************
*	@file	power.c
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief 延时函数
***************************************************************************************************************
*/

#include <stdint.h>
#include "power.h"
#include "debug.h"
#include "adc.h"

#define VREFINT_CAL_ADDR                   							((uint16_t*) ((uint32_t)0x1FF80078U)) /* Internal voltage reference, address of parameter VREFINT_CAL: VrefInt ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV). */
#define VREFINT_CAL_VREF                   							((uint32_t) 3U)                    /* Analog voltage reference (Vref+) value with which temperature sensor has been calibrated in production (tolerance: +-10 mV) (unit: mV). */
#define VDD_APPLI                      		 						((uint32_t) 1220U)    /* Value of analog voltage supply Vdda (unit: mV) */
#define VFULL																						((uint32_t) 4095U)

uint8_t CheckBattery(void)
{
	uint16_t adc[3] = {0};
	
	adc[0] = AdcReadParameter(ADC_CHANNEL_0, 10);
	float Rechargeing = VREFINT_CAL_VREF*(*VREFINT_CAL_ADDR)*adc[0];
	
	adc[1] = AdcReadParameter(ADC_CHANNEL_1, 10);
	float VBAT = VREFINT_CAL_VREF*(*VREFINT_CAL_ADDR)*adc[1]; 
	
	adc[2] = AdcReadParameter(ADC_CHANNEL_VREFINT, 10);
	
	float temp = adc[2] * VFULL;
	
	DEBUG(2, "BAT = %d adc17 = %d adc1 = %d, adc0 = %d, VBAT = %.2fmV Rechargeing = %.2fmV\r\n", *VREFINT_CAL_ADDR, adc[2], adc[1], adc[0], (VBAT/temp)*2000, (Rechargeing/temp)*6100);  ///100:510
	
	if((Rechargeing/temp)*6100 > 4500)
		LoRapp_Handle.Rechargeing = 0x01; ///充电状态反馈
	else
		LoRapp_Handle.Rechargeing = 0x00;
	
	LoRapp_Handle.Battery = (((VBAT/temp)*2000 - 3600)/6);
	
	if(LoRapp_Handle.Battery >= 100)
		LoRapp_Handle.Battery = 100;
	else if(LoRapp_Handle.Battery <= 5)
		LoRapp_Handle.Battery = 3;
	return LoRapp_Handle.Battery;
}
