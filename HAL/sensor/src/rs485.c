/*
**************************************************************************************************************
*	@file	rs485.c
*	@author Jason_531@163.com 
*	@version 
*	@date    
*	@brief	用RS485使用查询方式
***************************************************************************************************************
*/ 
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rs485.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*********************************************************************
 * RS485/串口相关配置
 */
#define RS485_TO_TX()	 ( HAL_GPIO_WritePin(UART_485_IO, UART_485_DE, GPIO_PIN_SET) )	// RS485总线切换到发送模式
#define RS485_TO_RX()	 ( HAL_GPIO_WritePin(UART_485_IO, UART_485_DE, GPIO_PIN_RESET) )	// RS485总线切换到接收模式		

#define TX_BUF_LEN	32	// 发送缓冲区长度
#define RX_BUF_LEN	32	// 接收缓冲区长度
uint8_t rs485_rxbuf[RX_BUF_LEN+1] = {0};	// 传感器串口接收缓冲区	   
uint8_t rs485_txbuf[TX_BUF_LEN+1] = {0};	// 传感器串口发送缓冲区

/*********************************************************************
 * 传感器相关配置
 */
#define SENSOR_CMD_LEN			8  	// 命令长度
#define SENSOR_REPLY_TIMEOUT	40	// 接收超时时间,实际延时加上sensor_info_tag.delay


/*
 *	InitUsart3:		波特率9600，发送和接收都为DMA模式
 *	参数：			无
 *	返回值：		无	
 */
void InitUsart5(void)
{
	MX_USART5_UART_Init();
	InitPowerPin();
	InitRs( );
}

/*
 *	InitRs:		初始化传感器
 *	参数：			无
 *	返回值：		无	
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
 *	InitPowerPin:	初始化传感器的供电引脚
 *	参数：			无
 *	返回值：		无	
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
 *	Enble485Power:	使能485电源引脚
 *	参数：			无
 *	返回值：		无	
 */
void Enble485Power(void)
{
	HAL_GPIO_WritePin(POWER_IO, POWER_12V_ON, GPIO_PIN_SET);
}

/*
 *	Disable485Power:	使能485电源引脚
 *	参数：			无
 *	返回值：		无	
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
 *	CalcCRC16:	计算CRC16校验值
 *	data:		数据指针
 *	len:		数据长度
 *	返回值：	16位的CRC校验值
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
 *	ExcuteRs485Command:		查询485传感器数据
 *	command：				Rs485_Command结构体，返回接收到的数据将覆盖发送的data数据域
 *	返回值：				EXCUTE_OK|CRC_ERROR|RECIVE_TIMEOUT
 */
int8_t ExcuteRs485Command(Rs485Command command)
{
	uint8_t i,recive_len=0;
	uint16_t crc_val;
	crc_val = CalcCRC16(command.data, command.data_len-2); //计算crc
	command.data[command.data_len-2] = crc_val&0xff;		//CRC低位	
	command.data[command.data_len-1] = crc_val>>8;			//CRC高位
	for (i=0;i<command.data_len;i++)
	DEBUG(3,"%02x ",command.data[i]);
	DEBUG(3,"tx\r\n");
    
	RS485_TO_TX();	  ///设置为TX
	HAL_Delay(100);    ///必须延时再发送，否则会出现异常
    
	HAL_UART_Transmit(&huart5, command.data, command.data_len, 0xFFFF); 
    
    RS485_TO_RX(); ///设置为Rx	
	memset(command.data,0,command.recive_len);			//清零接收缓冲区		
    memset(UART_RX_DATA5.USART_RX_BUF, 0, 516);
    UART_RX_DATA5.USART_RX_Len = 0;

	DEBUG(3,"RX_BUF[0] : %02x \r\n",UART_RX_DATA5.USART_RX_BUF[0]);

	HAL_Delay(100);//等待485模块切换为接收：去除会异常读取
    DEBUG(3,"RX_BUF[0] : %02x \r\n",UART_RX_DATA5.USART_RX_BUF[0]);
 
	uint32_t uart_over_time = HAL_GetTick( ); ///串口查询不到数据保护机制，防止没接485下异常

	while( (HAL_GetTick( ) - UART_RX_DATA5.rxtime <= 20) || (HAL_GetTick( ) - uart_over_time < 100) ); //等待485接收数据完成：最多11Bit = 10ms//

	if(HAL_GetTick( ) - UART_RX_DATA5.rxtime > 20 && UART_RX_DATA5.USART_RX_Len != 0)//20msRX超时机制 
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
	if (recive_len!=command.recive_len)					//没有接收到预定长度，则为超时
    {
        DEBUG(3,"RECIVE_TIMEOUT : %d ",RECIVE_TIMEOUT);
		return RECIVE_TIMEOUT;
    }
	else if (0!=CalcCRC16(command.data, command.recive_len))		// CRC校验，低位在前，带正确crc进行校验结果应为0
	{
		printf("line = %d\r\n",__LINE__);
		return CRC_ERROR;
	}
	return EXCUTE_OK;
}

