/*
**************************************************************************************************************
*	@file	uart.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief usart串口1，print串口测试doma
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
 /**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);  ///不需要库原因：从速度考虑直接使用寄存器更好
	return ch;
}

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart5;

UART_RX UART_RX_DATA1 = {0, {0}, 0, {0}, false, false};
UART_RX UART_RX_DATA2 = {0, {0}, 0, {0}, false, false};
UART_RX UART_RX_DATA5 = {0, {0}, 0, {0}, false, false};
Get_Gps_Ack_t Get_Gps_Ack = {false, false, false, false, false, 0};

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
	
	HAL_UART_Receive_IT(&huart1,UART_RX_DATA1.aRxBuffer, RXBUFFERSIZE);

  HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_UART_Receive_IT(&huart2,UART_RX_DATA2.aRxBuffer, RXBUFFERSIZE);
	
  HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}
/* USART5 init function */

void MX_USART5_UART_Init(void)
{
  huart5.Instance = USART5;
  huart5.Init.BaudRate = 9600;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
	
  HAL_UART_Receive_IT(&huart5, UART_RX_DATA5.aRxBuffer, RXBUFFERSIZE);
}


void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = USART1_TX|USART1_RX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
    HAL_GPIO_Init(USART1_IO, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = USART2_TX|USART2_RX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(USART2_IO, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART5)
  {
  /* USER CODE BEGIN USART5_MspInit 0 */

  /* USER CODE END USART5_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART5_CLK_ENABLE();
  
    /**USART5 GPIO Configuration    
    PB3     ------> USART5_TX
    PB4     ------> USART5_RX 
    */
    GPIO_InitStruct.Pin = USART5_TX|USART5_RX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_USART5;
    HAL_GPIO_Init(USART5_IO, &GPIO_InitStruct);

  /* USER CODE BEGIN USART5_MspInit 1 */

  /* USER CODE END USART5_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(USART1_IO, USART1_TX|USART1_RX);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(USART2_IO, USART2_TX|USART2_RX);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART2_IRQn);

  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART5)
  {
  /* USER CODE BEGIN USART5_MspDeInit 0 */

  /* USER CODE END USART5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART5_CLK_DISABLE();
  
    /**USART5 GPIO Configuration    
    PB3     ------> USART5_TX
    PB4     ------> USART5_RX 
    */
    HAL_GPIO_DeInit(USART5_IO, USART5_TX|USART5_RX);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART4_5_IRQn);

  /* USER CODE BEGIN USART5_MspDeInit 1 */

  /* USER CODE END USART5_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//如果是串口1
	{	
		if(UART_RX_DATA1.aRxBuffer[0] == 0x0d)  ///以'\r''\n'作为结束标记
		{
			UART_RX_DATA1.Rx_State = true;			
		}else if(UART_RX_DATA1.aRxBuffer[0] == 0x0a) ///以'\n'作为结束标记
		{
			if(UART_RX_DATA1.Rx_State)
			{			
				UART_RX_DATA1.Rx_State = false;
							 
				DEBUG(2,"%s\r\n",UART_RX_DATA1.USART_RX_BUF);
								
				memset(UART_RX_DATA1.USART_RX_BUF, 0, UART_RX_DATA1.USART_RX_Len);
				UART_RX_DATA1.USART_RX_Len = 0;
				
				Wakeup_post(  );
				RFTXDONE(  );
				
			}
		}
		else
		{
			UART_RX_DATA1.USART_RX_BUF[UART_RX_DATA1.USART_RX_Len]=UART_RX_DATA1.aRxBuffer[0];	
			DEBUG(3,"%c",UART_RX_DATA1.USART_RX_BUF[UART_RX_DATA1.USART_RX_Len]);	
			UART_RX_DATA1.USART_RX_Len++;
		}	
		
	 if(UART_RX_DATA1.USART_RX_Len >= 516)
			UART_RX_DATA1.USART_RX_Len = 0;
	}	
	else if(huart->Instance==USART2)//如果是串口2
	{	
		if(UART_RX_DATA2.aRxBuffer[0] == 0x0d)  ///以'\r''\n'作为结束标记
		{
			UART_RX_DATA2.Rx_State = true;
			
		}else if(UART_RX_DATA2.aRxBuffer[0] == 0x0a) ///以'\n'作为结束标记
		{
			if(UART_RX_DATA2.Rx_State)
			{			
				UART_RX_DATA2.Rx_State = false;
				
				int8_t GLL_State = 0;
				
			 if(GLL_State == strcmp((char *)UART_RX_DATA2.USART_RX_BUF, "$PMTK010,002*2D"))
					Get_Gps_Ack.START = true;
				else if(GLL_State == strcmp((char *)UART_RX_DATA2.USART_RX_BUF, "$PMTK001,314,3*36"))
					Get_Gps_Ack.GPGLL = true;
				else if(GLL_State == strcmp((char *)UART_RX_DATA2.USART_RX_BUF, "$PMTK001,220,3*30"))
				{
					Get_Gps_Ack.POS_FIX = true;	
					Get_Gps_Ack.GPS_OVER_TIME = HAL_GetTick( );  ///记录GPS定位时间
				}

				if(UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len-6] == 'A')
				{
					DEBUG(3,"get poation\r\n");
					Get_Gps_Ack.Get_PATION = true;
					
					memcpy(Get_Gps_Ack.GLL, UART_RX_DATA2.USART_RX_BUF, UART_RX_DATA2.USART_RX_Len);
					Get_Gps_Ack.GLL[UART_RX_DATA2.USART_RX_Len++]='\r';
				    Get_Gps_Ack.GLL[UART_RX_DATA2.USART_RX_Len++]='\n';

					DEBUG(2,"GLL--%s",Get_Gps_Ack.GLL);

				}
				else if(UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len-6] == 'V')
				{
					DEBUG(3,"get poation false\r\n"); 
					Get_Gps_Ack.Get_PATION = false;
					memcpy(Get_Gps_Ack.GLL, UART_RX_DATA2.USART_RX_BUF, UART_RX_DATA2.USART_RX_Len);
				}	
			}		
			 
			DEBUG(2,"%s\r\n",UART_RX_DATA2.USART_RX_BUF);
			memset(UART_RX_DATA2.USART_RX_BUF, 0, UART_RX_DATA2.USART_RX_Len);
			UART_RX_DATA2.USART_RX_Len = 0;
		}
		else
		{
			UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len]=UART_RX_DATA2.aRxBuffer[0];	
			DEBUG(3,"%c",UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len]);	
			UART_RX_DATA2.USART_RX_Len++;
		}	
		
	 if(UART_RX_DATA2.USART_RX_Len >= 516)
			UART_RX_DATA2.USART_RX_Len = 0;
	}	

	
	else if(huart->Instance==USART5)//如果是串口5
	{		
		UART_RX_DATA5.USART_RX_BUF[UART_RX_DATA5.USART_RX_Len] = UART_RX_DATA5.aRxBuffer[0];
		DEBUG(3,"%02x ",UART_RX_DATA5.USART_RX_BUF[UART_RX_DATA5.USART_RX_Len]);	
		UART_RX_DATA5.USART_RX_Len++;
		UART_RX_DATA5.rxtime = HAL_GetTick( );

	 if(UART_RX_DATA5.USART_RX_Len >= 516)
        UART_RX_DATA5.USART_RX_Len = 0;		
    }	
}


/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
