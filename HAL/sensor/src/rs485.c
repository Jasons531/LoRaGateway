/*
**************************************************************************************************************
*	@file	rs485.c
*	@author Jason_531@163.com 
*	@version 
*	@date    
*	@brief	��RS485ʹ�ò�ѯ��ʽ
***************************************************************************************************************
*/ 
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rs485.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*********************************************************************
 * RS485/�����������
 */
#define RS485_TO_TX()	 ( HAL_GPIO_WritePin(UART_485_IO, UART_485_DE, GPIO_PIN_SET) )	// RS485�����л�������ģʽ
#define RS485_TO_RX()	 ( HAL_GPIO_WritePin(UART_485_IO, UART_485_DE, GPIO_PIN_RESET) )	// RS485�����л�������ģʽ		

#define TX_BUF_LEN	32	// ���ͻ���������
#define RX_BUF_LEN	32	// ���ջ���������
uint8_t rs485_rxbuf[RX_BUF_LEN+1] = {0};	// ���������ڽ��ջ�����	   
uint8_t rs485_txbuf[TX_BUF_LEN+1] = {0};	// ���������ڷ��ͻ�����

/*********************************************************************
 * �������������
 */
#define SENSOR_CMD_LEN			8  	// �����
#define SENSOR_REPLY_TIMEOUT	40	// ���ճ�ʱʱ��,ʵ����ʱ����sensor_info_tag.delay


/*
 *	InitUsart3:		������9600�����ͺͽ��ն�ΪDMAģʽ
 *	������			��
 *	����ֵ��		��	
 */
void InitUsart5(void)
{
	MX_USART5_UART_Init();
	InitPowerPin();
	InitRs( );
}

/*
 *	InitRs:		��ʼ��������
 *	������			��
 *	����ֵ��		��	
 */
void InitRs(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
	
  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = UART_485_DE;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(UART_485_IO, &GPIO_InitStruct);
  HAL_GPIO_WritePin(UART_485_IO, UART_485_DE, GPIO_PIN_RESET);
}

/*
 *	InitPowerPin:	��ʼ���������Ĺ�������
 *	������			��
 *	����ֵ��		��	
 */
void InitPowerPin(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin : PA8 */
	GPIO_InitStruct.Pin = POWER_12V_ON;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(POWER_IO, &GPIO_InitStruct);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(POWER_IO, POWER_12V_ON, GPIO_PIN_RESET);
}

/*
 *	Enble485Power:	ʹ��485��Դ����
 *	������			��
 *	����ֵ��		��	
 */
void Enble485Power(void)
{
	HAL_GPIO_WritePin(POWER_IO, POWER_12V_ON, GPIO_PIN_SET);
}

/*
 *	Disable485Power:	ʹ��485��Դ����
 *	������			��
 *	����ֵ��		��	
 */
void Disable485Power(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin : PA8 */
	GPIO_InitStruct.Pin = POWER_12V_ON;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; //GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(POWER_IO, &GPIO_InitStruct);
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(POWER_IO, POWER_12V_ON, GPIO_PIN_SET);
}

/*
 *	CalcCRC16:	����CRC16У��ֵ
 *	data:		����ָ��
 *	len:		���ݳ���
 *	����ֵ��	16λ��CRCУ��ֵ
 */
static uint16_t CalcCRC16(uint8_t *data, uint8_t len)
{
	uint16_t result = 0xffff;
	uint8_t i, j;

	for (i=0; i<len; i++)
	{
		result ^= data[i];
		for (j=0; j<8; j++)
		{
			if ( result&0x01 )
			{
					result >>= 1;
					result ^= 0xa001;
			}
			else
			{
					result >>= 1;
			}
		}
	}
	return result;
}

/*
 *	ExcuteRs485Command:		��ѯ485����������
 *	command��				Rs485_Command�ṹ�壬���ؽ��յ������ݽ����Ƿ��͵�data������
 *	����ֵ��				EXCUTE_OK|CRC_ERROR|RECIVE_TIMEOUT
 */
int8_t ExcuteRs485Command(Rs485Command command)
{
	uint8_t i,recive_len=0;
	uint16_t crc_val;
	crc_val = CalcCRC16(command.data, command.data_len-2); //����crc
	command.data[command.data_len-2] = crc_val&0xff;		//CRC��λ	
	command.data[command.data_len-1] = crc_val>>8;			//CRC��λ
	for (i=0;i<command.data_len;i++)
	DEBUG(3,"%02x ",command.data[i]);
	DEBUG(3,"tx\r\n");
    
	RS485_TO_TX();	  ///����ΪTX
	HAL_Delay(100);    ///������ʱ�ٷ��ͣ����������쳣
    
	HAL_UART_Transmit(&huart5, command.data, command.data_len, 0xFFFF); 
    
    RS485_TO_RX(); ///����ΪRx	
	memset(command.data,0,command.recive_len);			//������ջ�����		
    memset(UART_RX_DATA5.USART_RX_BUF, 0, 516);
    UART_RX_DATA5.USART_RX_Len = 0;

	DEBUG(3,"RX_BUF[0] : %02x \r\n",UART_RX_DATA5.USART_RX_BUF[0]);

	HAL_Delay(100);//�ȴ�485ģ���л�Ϊ���գ�ȥ�����쳣��ȡ
    DEBUG(3,"RX_BUF[0] : %02x \r\n",UART_RX_DATA5.USART_RX_BUF[0]);
 
	uint32_t uart_over_time = HAL_GetTick( ); ///���ڲ�ѯ�������ݱ������ƣ���ֹû��485���쳣

	while( (HAL_GetTick( ) - UART_RX_DATA5.rxtime <= 20) || (HAL_GetTick( ) - uart_over_time < 100) ); //�ȴ�485����������ɣ����11Bit = 10ms//

	if(HAL_GetTick( ) - UART_RX_DATA5.rxtime > 20 && UART_RX_DATA5.USART_RX_Len != 0)//20msRX��ʱ���� 
	{
		HAL_NVIC_DisableIRQ(USART4_5_IRQn);
        DEBUG(3,"RX_BUF[0] : %02x ",UART_RX_DATA5.USART_RX_BUF[0]);
		memcpy(command.data,UART_RX_DATA5.USART_RX_BUF,UART_RX_DATA5.USART_RX_Len);
		memset(UART_RX_DATA5.USART_RX_BUF, 0, sizeof(UART_RX_DATA5.USART_RX_BUF));
		recive_len = UART_RX_DATA5.USART_RX_Len;
		for(uint8_t i = 0; i<UART_RX_DATA5.USART_RX_Len; i++)
		DEBUG(3,"%02x ",command.data[i]);
		DEBUG(3,"\r\n");
		UART_RX_DATA5.USART_RX_Len = 0;
		
		HAL_NVIC_EnableIRQ(USART4_5_IRQn);
	}
    DEBUG(3,"recive_len :%d command : %d",recive_len,command.recive_len);
	if (recive_len!=command.recive_len)					//û�н��յ�Ԥ�����ȣ���Ϊ��ʱ
    {
        DEBUG(3,"RECIVE_TIMEOUT : %d ",RECIVE_TIMEOUT);
		return RECIVE_TIMEOUT;
    }
	else if (0!=CalcCRC16(command.data, command.recive_len))		// CRCУ�飬��λ��ǰ������ȷcrc����У����ӦΪ0
	{
		printf("line = %d\r\n",__LINE__);
		return CRC_ERROR;
	}
	return EXCUTE_OK;
}

