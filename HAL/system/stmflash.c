#include "stmflash.h"
#include "delay.h"

/*
 * StmFlashRead��	��ָ����ַ��ʼ����ָ�����ȵ�����
 * ReadAddr:		��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
 * pBuffer:			Ҫд�������ָ��
 * NumToRead��		�������֣�16λ����
 * ����ֵ��			��
 */ 
void StmFlashRead(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=READ_FLASH(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}


/*
 * STMFLASH_Read_Word��	��ָ����ַ��ʼ����ָ�����ȵ�����
 * ReadAddr:						��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
 * pBuffer:							Ҫд�������ָ��
 * NumToRead��					�����֣�32λ����
 * ����ֵ��							��
 */ 
void STMFLASH_Read_Word(uint32_t ReadAddr, uint32_t *pBuffer,uint32_t NumToRead)   	
{
	uint32_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadWord(ReadAddr);//��ȡ4���ֽ�.
		ReadAddr+=4;//ƫ��4���ֽ�.	
	}
}
