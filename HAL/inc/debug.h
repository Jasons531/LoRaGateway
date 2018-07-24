/*
**************************************************************************************************************
*	@file		debug.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief	debug
***************************************************************************************************************
*/
#ifndef __DEBUG_H
#define __DEBUG_H	 
#include <stdio.h>
#include <stdint.h>
#include "stm32l0xx_hal.h"
#include "usart.h"

#ifdef __cplusplus
	extern "C" {
#endif
			
#define DEBUG__						1
#define DEBUG_LEVEL	  		        2					//���Եȼ������DEBUG���Ժ���Ƶ��������Χ,���ڸõȼ��ĵ��Բ����
		
		
#ifdef DEBUG__				  	//���Ժ궨��  
	#include <stdio.h>
	#include <string.h>  
	#define DEBUG(level, fmt, arg...)  if(level <= DEBUG_LEVEL)	printf(fmt,##arg);  
	#define DEBUG_NOW(level, fmt, arg...)	
	#define REDIRECT_SEND()	
	#define REDIRECT_RECIVE()
	#define REDIRECT_RECORD()
#endif //end of DEBUG__							
//extern void DEBUG(uint8_t level, const int8_t *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
