/*
**************************************************************************************************************
*	@file	LoRa-cad.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_CAD_H
#define __LORA_CAD_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdio.h>
#include <stdint.h>

extern uint32_t RfSend_time;

typedef struct LoRaCsmas
{
		bool Iq_Invert;///node to node 
		bool Listen;   ///侦听标志

		uint8_t  DisturbCounter; ///侦听计数
	  uint8_t  retry;          ///侦听最大次数
    uint32_t symbolTime;     ///preamblelen time
}LoRaCsma_t;

typedef struct //sLoRaMacCsma
{
	void ( *ChannelAddFun )(void);
	void ( *SymbolTime )(void);	
	void ( *ListenAagain )(void);
	
}LoRaMacCsma_t;

extern LoRaCsma_t Csma;
extern LoRaMacCsma_t LoRaMacCsma;

extern uint8_t Channel; ///信道号

extern TimerEvent_t CsmaTimer;

void OnCsmaTimerEvent( void );
void UserChannelAddFun( void );

void LoRaCadInit(void);

float UserSymbolTime(void);

void UserListenAagain(void);

#endif /* __LoRa-cad_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
