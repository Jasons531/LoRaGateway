/*
**************************************************************************************************************
*	@file			main.c
*	@author 	Jason_531@163.com
*	@version 	V1.1
*	@date    	2017/12/13
*	@brief	
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "timerserver.h"
#include "delay.h"
#include "board.h"
#include "user-app.h"
#include "etimer.h"
#include "autostart.h"


#ifndef SUCCESS
#define SUCCESS                         1
#endif

#ifndef FAIL
#define FAIL                            0
#endif


/*!***********************************���м���************************************/

#if  OVER_THE_AIR_ACTIVATION

extern uint8_t DevEui[8];
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

extern TimerEvent_t JoinReqTimer;
extern volatile bool IsNetworkJoined;
extern bool JoinReq_flag;

#endif

LoRaMacRxInfo *loramac_rx_info;
mac_evt_t loramac_evt;

void app_mac_cb (mac_evt_t evt, void *msg)
{
    switch(evt){
    case MAC_STA_TXDONE:                
    case MAC_STA_RXDONE:
    case MAC_STA_RXTIMEOUT:
    case MAC_STA_ACK_RECEIVED:
    case MAC_STA_ACK_UNRECEIVED:
    case MAC_STA_CMD_JOINACCEPT:         
    case MAC_STA_CMD_RECEIVED:
         loramac_rx_info = msg;   ///mac�����������Ϣ��rssi �˿ڵ�
         loramac_evt = evt;
         
         break;
    }
}


/*!***********************************�ָ���************************************/

extern UART_HandleTypeDef 			    UartHandle;
extern RTC_HandleTypeDef 						RtcHandle;
extern SPI_HandleTypeDef            SPI1_Handler;  

PROCESS(Wakeup_process,"Wakeup_process");
PROCESS(SX1278Send_process,"SX1278Send_process");
AUTOSTART_PROCESSES(&SX1278Send_process,&Wakeup_process);  
void RFTXDONE(void)
{
	process_poll(&Wakeup_process); ///�������ȼ�
}

static process_event_t LoRaReceiveDone;
void Netsend_post(void)
{
    process_post(&SX1278Send_process,LoRaReceiveDone,NULL); ///�첽
}

static process_event_t LoRaSendWakeup;
void Wakeup_post(void)
{
    process_post(&Wakeup_process,LoRaSendWakeup,NULL); ///�첽
}

extern uint32_t UpLinkCounter;

bool Gps_Send_Stae = false;

/*!
 * Channels default datarate
 */
extern int8_t ChannelsDefaultDatarate;

PROCESS_THREAD(SX1278Send_process,ev,data)
{
	static struct etimer et;
        
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System SX1278Send Process..."); 
	LoRaReceiveDone = process_alloc_event();
	
//	etimer_set(&et,CLOCK_SECOND);	
    
	while(1)
	{		
		
		PROCESS_YIELD();

		if( ev == LoRaReceiveDone )///�첽����
		{  
						
			LoRapp_Handle.Send_Buf = "#";
			LoRapp_Handle.Tx_Len = 1;
			
			RfSend_time = HAL_GetTick(  );
			
			HAL_Delay( 1000 );

			for(uint8_t i = 0; i < 3; ) ///�������ݻ��ƣ�����ʧ�����ط���������
			{
				///�������ݣ����Ӧ�����ģʽ��ȷ�������ȶ���
				if(UserAppSend(UNCONFIRMED, LoRapp_Handle.Send_Buf, LoRapp_Handle.Tx_Len, 2) == 0) ///���ͳɹ����л��������ģʽFreq + 30mhz
				{
					DEBUG(2,"Wait ACK app_send UpLinkCounter = %d\r\n", LoRaMacGetUpLinkCounter( ));

					PROCESS_YIELD_UNTIL(LoRapp_Handle.Loramac_evt_flag == 1);
					LoRapp_Handle.Loramac_evt_flag = 0;
					LoRapp_Handle.MhdrAck = false;
					
					break;
				}
				else
				{
					DEBUG(2,"app_send again\r\n");
					Radio.Standby( );
					etimer_set(&et,CLOCK_SECOND*4 + randr(-CLOCK_SECOND*4,CLOCK_SECOND*4));
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
					i++;
				}
			}
			ev = PROCESS_EVENT_NONE;		
		}  
			
//		etimer_reset(&et);
	}
	PROCESS_END();
}

extern bool rx_start;

PROCESS_THREAD(Wakeup_process,ev,data)
{	
  static struct etimer et;

	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System Wakeup Process..."); 
	
	LoRaSendWakeup = process_alloc_event();

	etimer_set(&et,CLOCK_SECOND*0.5);	

	while(1)
	{	
//			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
//			
//		 etimer_reset(&et);
		
		PROCESS_YIELD();
		
		if( ev == LoRaSendWakeup )///�첽����
		{	
			LoRapp_Handle.Send_Buf = "helloworld";
			LoRapp_Handle.Tx_Len = 16;
		
			for(uint8_t i = 0; i < 3; ) ///�������ݻ��ƣ�����ʧ�����ط���������
			{
				///�������ݣ����Ӧ�����ģʽ��ȷ�������ȶ���
				if(UserAppSend(UNCONFIRMED, LoRapp_Handle.Send_Buf, LoRapp_Handle.Tx_Len, 2) == 0) ///���ͳɹ����л��������ģʽFreq + 30mhz
				{
					DEBUG(2,"Wait ACK app_send UpLinkCounter = %d\r\n", LoRaMacGetUpLinkCounter( ));

					PROCESS_YIELD_UNTIL(LoRapp_Handle.Loramac_evt_flag == 1);
					LoRapp_Handle.Loramac_evt_flag = 0;
					LoRapp_Handle.MhdrAck = false;
					ev = PROCESS_EVENT_NONE;	
					
					break;
				}
				else
				{
					DEBUG(2,"app_send again\r\n");
					Radio.Standby( );
					etimer_set(&et,CLOCK_SECOND*4 + randr(-CLOCK_SECOND*4,CLOCK_SECOND*4));
					PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
					i++;
				}
			}
		}
		 etimer_reset(&et);
	}
  PROCESS_END(); 
}

void processset(void)
{
	process_init();
	process_start(&etimer_process,NULL); ///�Զ�����������߳�
	autostart_start(autostart_processes);
}


/*******************************************************************************
  * @��������		main
  * @����˵��   ������ 
  * @�������   ��
  * @�������   ��
  * @���ز���   ��

	�汾˵����
	��1����V0.1��MCU---stm32L0�����ز��Դ����Զ�ͨѶ

	�Ż����ܣ�
	��1���� ʵ��LORAWAN��С����ͨ�š�
	��2���� RTCͣ�����ѻ��ơ�
  ��3���� ˫Ƶ��FreqTX = FreqRX2  FreqRX1 = FreqTX+30MHZ
  ��4���� ˫ģ����ͨѶ���֣���ֹ�ŵ�й¶
  ��5���� ģ���װΪ͸�������ýӿ�Ԥ������
  ��6���� ������������CADģʽ��ֻ����preamblelen���ڲ��ȶ��ԣ���˲�ʹ�øû���
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	
int main(void)
{	
	BoardInitMcu(  );	
	DEBUG(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__);
		
	//	MX_IWDG_Init(  );

	//	HAL_IWDG_Refresh(&hiwdg); ///���Ź�ι��
	
	UserAppInit(app_mac_cb);

	Channel = 3; ///��ȡ�ŵ�ID flash��ȡ
	
	LoRaMacCsma.ChannelAddFun(  );
	
	//��������ģʽ
	Radio.Standby( );
	LoRaMacSetDeviceClass( CLASS_C );

	LoRapp_Handle.Loramac_evt_flag = 0;

	LoRapp_Handle.FPort = randr( 1, 0xDF );

	LoRapp_Handle.Send_Buf = (uint8_t *)malloc(sizeof(uint8_t)*56); ///ʹ��ָ���������ַ�ռ䣬��������HardFault_Handler����

	processset( );

	USR_UsrLog("System Contiki InitSuccess...");	

	TimerInit( &CsmaTimer, OnCsmaTimerEvent );
	    	
	while (1)
	{		
		do
		{
		}while(process_run() > 0);     
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{ 
	DEBUG(2,"error\r\n");
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/*--------------------------------------------------------------------------------------------------------
                   									     0ooo											
                   								ooo0     (   )
                								(   )     ) /
                								 \ (     (_/
                								  \_)
----------------------------------------------------------------------------------------------------------*/

