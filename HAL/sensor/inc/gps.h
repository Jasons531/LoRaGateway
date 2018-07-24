#ifndef __GPS_H
#define __GPS_H	 

#include <stdint.h>
#include "timer.h"
#include "timerserver.h"

extern TimerEvent_t GPSTimer;
extern void OnGpsTimerEvent( void );

void GpsInit(void);
void GpsEnable(void);
void GpsDisable(void);
uint8_t GpsSet(void);
void GetGpsPosition(void);

void GpsLocationLed(void);


//GPS NMEA-0183Э����Ҫ�����ṹ�嶨�� 
//������Ϣ
__packed typedef struct  
{										    
 	uint8_t num;		//���Ǳ��
	uint8_t eledeg;	//��������
	uint16_t azideg;	//���Ƿ�λ��
	uint8_t sn;		//�����		   
}nmea_slmsg; 
//���� NMEA-0183Э����Ҫ�����ṹ�嶨�� 
//������Ϣ
__packed typedef struct  
{	
 	uint8_t beidou_num;		//���Ǳ��
	uint8_t beidou_eledeg;	//��������
	uint16_t beidou_azideg;	//���Ƿ�λ��
	uint8_t beidou_sn;		//�����		   
}beidou_nmea_slmsg; 

//UTCʱ����Ϣ
__packed typedef struct  
{										    
 	uint16_t year;	//���
	uint8_t month;	//�·�
	uint8_t date;	//����
	uint8_t hour; 	//Сʱ
	uint8_t min; 	//����
	uint8_t sec; 	//����
}nmea_utc_time;   	   
//NMEA 0183 Э����������ݴ�Žṹ��
__packed typedef struct  
{										    
 	uint8_t svnum;					//�ɼ�GPS������
	uint8_t beidou_svnum;					//�ɼ�GPS������
	nmea_slmsg slmsg[12];		//���12��GPS����
	beidou_nmea_slmsg beidou_slmsg[12];		//���������12�ű�������
	nmea_utc_time utc;			//UTCʱ��
	uint32_t latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
	uint8_t nshemi;					//��γ/��γ,N:��γ;S:��γ				  
	uint32_t longitude;			    //���� ������100000��,ʵ��Ҫ����100000
	uint8_t ewhemi;					//����/����,E:����;W:����
	uint8_t gpssta;					//GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.				  
 	uint8_t posslnum;				//���ڶ�λ��GPS������,0~12.
 	uint8_t possl[12];				//���ڶ�λ�����Ǳ��
	uint8_t fixmode;					//��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
	uint16_t pdop;					//λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
	uint16_t hdop;					//ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
	uint16_t vdop;					//��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0 

	int altitude;			 	//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
	uint16_t speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ	 
}nmea_msg;

extern nmea_msg gpsx; 		

 	////////////////////////////////////////////////////////////////////////////////////////////////////
//SkyTra S1216F8 ���ò����ʽṹ��
__packed typedef struct
{
	uint16_t sos;            //�������У��̶�Ϊ0XA0A1
	uint16_t PL;             //��Ч���ݳ���0X0004�� 
	uint8_t id;             //ID���̶�Ϊ0X05
	uint8_t com_port;       //COM�ڣ��̶�Ϊ0X00����COM1   
	uint8_t Baud_id;       //�����ʣ�0~8,4800,9600,19200,38400,57600,115200,230400,460800,921600��
	uint8_t Attributes;     //�������ݱ���λ�� ,0���浽SRAM��1���浽SRAM&FLASH��2��ʱ����
	uint8_t CS;             //У��ֵ
	uint16_t end;            //������:0X0D0A  
}SkyTra_baudrate;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//SkyTra S1216F8 ���������Ϣ�ṹ��
__packed typedef struct
{
	uint16_t sos;            //�������У��̶�Ϊ0XA0A1
	uint16_t PL;             //��Ч���ݳ���0X0009�� 
	uint8_t id;             //ID���̶�Ϊ0X08
	uint8_t GGA;            //1~255��s��,0:disable
	uint8_t GSA;            //1~255��s��,0:disable
	uint8_t GSV;            //1~255��s��,0:disable
	uint8_t GLL;            //1~255��s��,0:disable
	uint8_t RMC;            //1~255��s��,0:disable
	uint8_t VTG;            //1~255��s��,0:disable
	uint8_t ZDA;            //1~255��s��,0:disable
	uint8_t Attributes;     //�������ݱ���λ�� ,0���浽SRAM��1���浽SRAM&FLASH��2��ʱ����
	uint8_t CS;             //У��ֵ
	uint16_t end;            //������:0X0D0A  
}SkyTra_outmsg;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//SkyTra S1216F8 ����λ�ø����ʽṹ��
__packed typedef struct
{
	uint16_t sos;            //�������У��̶�Ϊ0XA0A1
	uint16_t PL;             //��Ч���ݳ���0X0003�� 
	uint8_t id;             //ID���̶�Ϊ0X0E
	uint8_t rate;           //ȡֵ��Χ:1, 2, 4, 5, 8, 10, 20, 25, 40, 50
	uint8_t Attributes;     //�������ݱ���λ�� ,0���浽SRAM��1���浽SRAM&FLASH��2��ʱ����
	uint8_t CS;             //У��ֵ
	uint16_t end;            //������:0X0D0A  
}SkyTra_PosRate;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//SkyTra S1216F8 �����������(PPS)��Ƚṹ��
__packed typedef struct
{
	uint16_t sos;            //�������У��̶�Ϊ0XA0A1
	uint16_t PL;             //��Ч���ݳ���0X0007�� 
	uint8_t id;             //ID���̶�Ϊ0X65
	uint8_t Sub_ID;         //0X01
	uint32_t width;        //1~100000(us)
	uint8_t Attributes;     //�������ݱ���λ�� ,0���浽SRAM��1���浽SRAM&FLASH��2��ʱ����
	uint8_t CS;             //У��ֵ
	uint16_t end;            //������:0X0D0A 
}SkyTra_pps_width;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//SkyTra S1216F8 ACK�ṹ��
__packed typedef struct
{
	uint16_t sos;            //�������У��̶�Ϊ0XA0A1
	uint16_t PL;             //��Ч���ݳ���0X0002�� 
	uint8_t id;             //ID���̶�Ϊ0X83
	uint8_t ACK_ID;         //ACK ID may further consist of message ID and message sub-ID which will become 3 bytes of ACK message
	uint8_t CS;             //У��ֵ
	uint16_t end;            //������ 
}SkyTra_ACK;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//SkyTra S1216F8 NACK�ṹ��
__packed typedef struct
{
	uint16_t sos;            //�������У��̶�Ϊ0XA0A1
	uint16_t PL;             //��Ч���ݳ���0X0002�� 
	uint8_t id;             //ID���̶�Ϊ0X84
	uint8_t NACK_ID;         //ACK ID may further consist of message ID and message sub-ID which will become 3 bytes of ACK message
	uint8_t CS;             //У��ֵ
	uint16_t end;            //������ 
}SkyTra_NACK;


int NMEA_Str2num(uint8_t *buf,uint8_t*dx);
void GPS_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGLL_Analysis(nmea_msg *gpsx,uint8_t *buf);
uint8_t SkyTra_Cfg_Cfg_Save(void);
uint8_t SkyTra_Cfg_Msg(uint8_t msgid,uint8_t uart1set);
uint8_t SkyTra_Cfg_Prt(uint8_t baud_id);
uint8_t SkyTra_Cfg_Tp(uint32_t width);
uint8_t SkyTra_Cfg_Rate(uint8_t Frep);
void SkyTra_Send_Date(uint8_t* dbuf,uint16_t len);
uint8_t* data_Little_endian(uint8_t* data,uint16_t len);
#endif

 



