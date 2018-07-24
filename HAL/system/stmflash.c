#include "stmflash.h"
#include "delay.h"

/*
 * StmFlashRead：	从指定地址开始读出指定长度的数据
 * ReadAddr:		起始地址(此地址必须为2的倍数!!)
 * pBuffer:			要写入的数据指针
 * NumToRead：		读出半字（16位）数
 * 返回值：			无
 */ 
void StmFlashRead(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=READ_FLASH(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}


/*
 * STMFLASH_Read_Word：	从指定地址开始读出指定长度的数据
 * ReadAddr:						起始地址(此地址必须为2的倍数!!)
 * pBuffer:							要写入的数据指针
 * NumToRead：					读出字（32位）数
 * 返回值：							无
 */ 
void STMFLASH_Read_Word(uint32_t ReadAddr, uint32_t *pBuffer,uint32_t NumToRead)   	
{
	uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//读取4个字节.
		ReadAddr+=4;//偏移4个字节.	
	}
}
