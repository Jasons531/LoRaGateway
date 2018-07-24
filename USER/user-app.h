/*
**************************************************************************************************************
*	@file	user-app.h
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	Ӧ�ò�ͷ�ļ�������MAC��
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_APP_H
#define __USER_APP_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdio.h>
#include <stdint.h>

#define AT_MODULE_PORT					(10)    ///FPort

#define AESKEY_LEN                  	(16)


///128KB==0X1F400    0x0800000 ---- 0x0801F400
#define DEV_ADDR                        0x0801F3E8        ////UID 0x0801F3EC ---- 0x0801F3FC
#define DEV_ADDR_SIZE                   0x12

/*!
 * Default datarate used by the node
 */
#define LORAMAC_DEFAULT_DATARATE        0x0801F3D8

#define SET_SLEEPT_ADDR					0x0801F3E0  		///��������ʱ�� 0x0801F3E8 ---- 0x0801F3E9
#define SET_ADR_ADDR                    0x0801F3DC	    ///ADR



/*!
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION         0

/*!
 * Application IEEE EUI (big endian)
 */
#define LORAWAN_APPLICATION_EUI         { 0x12,0x34,0x56,0x78,0x90,0xAB,0xCD,0xEF }


/*!
 * AES encryption/decryption cipher application key
 */
#define LORAWAN_APPLICATION_KEY         { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }

typedef enum{
	IDLE,
	BUSY,
	DONE,
}LoRaStatus_t;

typedef enum{
	CONFIRMED,
	UNCONFIRMED
}LoRaFrameType_t;

typedef enum{
	INVALID,
	RECEIVED,
	UNRECEIVED,
}LoRaTxAckReceived_t;

typedef enum{
    LORAMAC_NWKSKEY=0,
    LORAMAC_APPSKEY=1,
    LORAMAC_APPKEY=2,
}LoRaMacKey_t;

typedef struct{
    int16_t rssi;
    uint8_t snr;
    uint8_t win;
    uint8_t port;
    uint16_t size;
    uint8_t buf[256];
}LoRaMacRxInfo;

typedef enum{
    MAC_STA_TXDONE,
    MAC_STA_RXDONE,
    MAC_STA_RXTIMEOUT,
    MAC_STA_ACK_RECEIVED,
    MAC_STA_ACK_UNRECEIVED,
    MAC_STA_CMD_JOINACCEPT,
    MAC_STA_CMD_RECEIVED,
}mac_evt_t;

typedef enum Rx_State{
	RFWRITE = 0, ///�ȴ�״̬
	RADIO,       ///����������״̬��һ֡���ݻ�����������������ݣ������ٴν�������
	RFSTART,     ///��������״̬
	RFTX,        ///�����ϱ�
	RFREADY,     ///׼��״̬,�л�λ�ȴ�
}Rx_State_t;

typedef enum Work_Modes
{
	CSMA = 0, ///ͬƵ
	CAD,      ///��Ƶ
}Work_Mode_t;

/*
 *	Ӧ�ò�״̬��־λ
 */
typedef struct LoRapp_Handles
{
	bool   		Ack_Recived;   ///ACKӦ���־
	bool 			Hardware_Exist_GPS;	
	bool			Send_again;	///RF ACK Fail Send Again 
  bool    	OnRxWindow1;
	bool 			Cad_Done;
  bool 			Cad_Detect;
	bool 			MhdrAck;  ///����ȷ������
	
	uint8_t 	Loramac_evt_flag;          ///lora����״̬λ
	uint8_t 	Tx_Len;	///�������ݳ���
	uint8_t   FPort; ///FPort
	uint8_t 	*Send_Buf;
	uint8_t		ADR_Datarate;   ///��ȡ��ADRʱSF����
	uint8_t   default_datarate;  ///Ĭ��datarate
	uint8_t   Rechargeing;  ///�Ƿ����־
	uint8_t   Battery;  ///����
  uint8_t   Send_Counter;
	
	Work_Mode_t Work_Mode;
	Rx_State_t 	Rx_States;     ///��Ƶ����״̬	
}LoRapp_Handle_t;

/*****************��ȡflash��ȡADR״̬********************/

typedef struct{
	bool LoRaMacSetAdrOnState;
	char ReadSetAdar_Addr;
	uint8_t datarate;
	uint8_t sleep_times;   ///����ʱ��Ĭ��24S
	uint8_t min_datarate;
	uint8_t max_datarate;
}Get_Flash_Data;

extern Get_Flash_Data Get_Flash_Datas;  ///��ȡflash����

extern LoRapp_Handle_t LoRapp_Handle;

typedef void (*mac_callback_t) (mac_evt_t mac_evt, void *msg);

int PowerXY(int x, int y);

int Convert16To10(int number);

uint32_t ReadDecNumber(char *str);

void StringConversion(char *str, uint8_t *src, uint8_t len);

uint8_t ReadFlashData(void);

void UserAppInit(mac_callback_t mac);

int UserAppSend( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry);

uint32_t app_get_devaddr(void);

void OnReportTimerEvent( void );

void IntoLowPower(void);

void SendDoneLed(void);

void RxLed(void);

void ErrorLed(void);

void PowerEnableLed(void);

void PowerDisbleLed(void);

void RFTXDONE(void);

void Wakeup_post(void);

extern void Netsend_post(void);


#endif /* __USER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
