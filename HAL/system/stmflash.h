#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "stm32l0xx_hal.h"

#ifdef __cplusplus
	extern "C" {
#endif
										
#define STM32_FLASH_SIZE 		128	 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_BASE 		0x08000000 		//flash����ַ
									 							  
#define READ_FLASH(faddr)		(*(volatile uint16_t*)faddr) 	
#define STMFLASH_ReadWord(faddr)		(*(volatile uint32_t*)faddr) 	
					  
#if STM32_FLASH_SIZE<256							
#define STM_SECTOR_SIZE 		1024 			//�е�����flash���ͺ�ҳ���С��1024�ֽ�
#else												
#define STM_SECTOR_SIZE 		2048 			//������flash���ͺ�ҳ���С��2048�ֽ�
#endif												
															
#define UNLOCK_FLASH()			{FLASH->KEYR=0X45670123;FLASH->KEYR=0XCDEF89AB;}//FLASH����
#define LOCK_FLASH()			(FLASH->CR|=1<<7)								//FLASH����
#define FLASH_STATUS 			(FLASH->SR)										//FLASH״̬�Ĵ���
#define FLASH_DONE				0
#define FLASH_BUSY				1
#define FLASH_PROGRAM_ERROR		1<<2
#define FLASH_WRITE_ERROR		1<<4
#define FLASH_TIMEOUT			0xee
#define FLASH_ERROR				0xff	

uint8_t STMFLASH_WaitDone(uint16_t time);				  	//�ȴ���������
uint8_t STMFLASH_ErasePage(uint32_t paddr);			  		//����ҳ
uint8_t STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
uint8_t STMFLASH_Write32(uint32_t WriteAddr,uint32_t *pBuffer,uint16_t NumToWrite);
void StmFlashRead(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
#ifdef __cplusplus
}
#endif		   

#endif	

