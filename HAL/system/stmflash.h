#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "stm32l0xx_hal.h"

#ifdef __cplusplus
	extern "C" {
#endif
										
#define STM32_FLASH_SIZE 		128	 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_BASE 		0x08000000 		//flash基地址
									 							  
#define READ_FLASH(faddr)		(*(volatile uint16_t*)faddr) 	
#define STMFLASH_ReadWord(faddr)		(*(volatile uint32_t*)faddr) 	
					  
#if STM32_FLASH_SIZE<256							
#define STM_SECTOR_SIZE 		1024 			//中等容量flash的型号页面大小是1024字节
#else												
#define STM_SECTOR_SIZE 		2048 			//大容量flash的型号页面大小是2048字节
#endif												
															
#define UNLOCK_FLASH()			{FLASH->KEYR=0X45670123;FLASH->KEYR=0XCDEF89AB;}//FLASH解锁
#define LOCK_FLASH()			(FLASH->CR|=1<<7)								//FLASH上锁
#define FLASH_STATUS 			(FLASH->SR)										//FLASH状态寄存器
#define FLASH_DONE				0
#define FLASH_BUSY				1
#define FLASH_PROGRAM_ERROR		1<<2
#define FLASH_WRITE_ERROR		1<<4
#define FLASH_TIMEOUT			0xee
#define FLASH_ERROR				0xff	

uint8_t STMFLASH_WaitDone(uint16_t time);				  	//等待操作结束
uint8_t STMFLASH_ErasePage(uint32_t paddr);			  		//擦除页
uint8_t STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//从指定地址开始写入指定长度的数据
uint8_t STMFLASH_Write32(uint32_t WriteAddr,uint32_t *pBuffer,uint16_t NumToWrite);
void StmFlashRead(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//从指定地址开始读出指定长度的数据
#ifdef __cplusplus
}
#endif		   

#endif	

