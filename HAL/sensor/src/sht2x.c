/*
**************************************************************************************************************
*	@file	sht2x.c
*	@author 
*	@version 
*	@date    
*	@brief	�����ģ��I2C��ȡSHT2X���¶Ⱥ�ʪ�����ݣ���ΪSTM32��I2C�е�BUG���������
***************************************************************************************************************
*/

#include "sht2x.h"
#include "delay.h"
#include "i2c.h"

/*********************************************************************
 * ��SHT2xͨѶ�Ľӿ�
 */

#define SHT2x_WRITE_ADDR	0x80	// SHT2x���豸��ַ
#define SHT2x_NO_HOLD_T		0xF3	// NO HOLDģʽ�������¶�
#define SHT2x_NO_HOLD_H		0xF5	// NO HOLDģʽ������ʪ��
#define POLYNOMIAL 			0x31	//sht2xʹ�õ�crc8У�����ʽ

/*
 *	SHT2xCheckCrc:	��sht2x���������ݽ���У��
 *	measure��		�ɹ���1��ʧ�ܣ�0
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
 *	Sht2xReadTH:	��ȡSTH2x���¶Ⱥ�ʪ������
 *	measure��		�洢�¶Ⱥ�ʪ�����ݣ��Ŵ���10��
 */
void Sht2xReadTH(int32_t *measure)
{
	uint8_t result[3],i,num;
	uint16_t temperature, humidity;	 

	// ��ȡ�¶�
	measure[0] = 0;	//�������
	num = 0;		//��ȡ��������
	for(i=0;i<10;i++)
	{
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR);
		I2cCheckAck();
		I2cWritex(SHT2x_NO_HOLD_T);	// ����һ�β���
		I2cCheckAck();
		delay_us(20);	// �ֲ�˵��Ҫ20us��ʱ
		I2cStop();
		delay_ms(90);		// �����ʱ85ms������������ʱʱ�䣬����Ͳ��жϣ�ֱ�Ӷ�ȡ
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR+1);
		I2cCheckAck();
		result[0] = I2cReadx();		//��λ����
		I2cAck();
		result[1] = I2cReadx(); 	//��λ����
		I2cAck();
		result[2] = I2cReadx();		//crcУ������
			
		I2cNack();					// ���һ���ֽ�����Ӧ��	 
		I2cStop();
		DEBUG(4,"%d,sht2x raw data,%x,%x,%x\r\n",RTC_CNT,result[0],result[1],result[2]);	
		if (SHT2xCheckCrc(result))	//crcУ��ͨ��
		{
			temperature = ((result[0]<<8)|result[1])&(~0x3);
			measure[0] += (-46.85 + 175.72*temperature/65536)*10;	// �Ŵ�10������������С�����һλ
			num++;												   	//��ȡ�ɹ�����+1
		}
	}
	if (num>0)
		measure[0] = measure[0]/num;		  						//��ȡ����Ч�����������Ч�����Ĵ���
	
		
	// ��ȡʪ��
	measure[1] = 0;	//�������
	num = 0;
	for(i=0;i<10;i++)
	{
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR);
		I2cCheckAck();
		I2cWritex(SHT2x_NO_HOLD_H);	// ����һ�β���
		I2cCheckAck();
		delay_us(20);	// �ֲ�˵��Ҫ20us��ʱ
		I2cStop();
		delay_ms(35);	// �����ʱ29ms������������ʱʱ�䣬����Ͳ��жϣ�ֱ�Ӷ�ȡ
		I2cStart();
		I2cWritex(SHT2x_WRITE_ADDR+1);
		I2cCheckAck();
		result[0] = I2cReadx();		//��λ����
		I2cAck();
		result[1] = I2cReadx(); 	//��λ����
		I2cAck();
		result[2] = I2cReadx();		//crcУ������
				
		I2cNack();	// ���һ���ֽ�����Ӧ��	 
		I2cStop();
		DEBUG(4,"%d,sht2x raw data,%x,%x,%x\r\n",RTC_CNT,result[0],result[1],result[2]);	
		if (SHT2xCheckCrc(result))	  //crcУ��ͨ��
		{
			humidity = ((result[0]<<8)|result[1])&(~0x3);
			measure[1] += (-6 + 125.0*humidity/65536)*10;	// �Ŵ�10������������С�����һλ
			num++;											//��ȡ�ɹ�����+1
		}
	}
	if (num>0)
		measure[1] = measure[1]/num;		  				//��ȡ����Ч�����������Ч�����Ĵ���
}
