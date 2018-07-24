/*
**************************************************************************************************************
*	@file	spi.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief spi����mode
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "debug.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi1_rx;

/**
  * @brief SPIģ��ĳ�ʼ�����룬���ó�����ģʽ 
  * @param SPI�ڳ�ʼ��
  * @retval SPI1 init function
  */
void SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
		DEBUG(2,"HAL_SPI_Init ERROR\r\n");
    Error_Handler();
  }

}

/**
  * @brief SPIƬѡ��ʼ��
  * @param SPI NSS���ų�ʼ��---PA4
  * @retval None
  */
void SPI1_NSS(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOD_CLK_ENABLE();			 ///�����ⲿʱ�ӣ�RCC_OSC_IN RCC_OSC_OUT ����SPI������
  __HAL_RCC_GPIOA_CLK_ENABLE();            ///����GPIOBʱ��
	
  GPIO_Initure.Pin=GPIO_PIN_4; //PA4
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
  GPIO_Initure.Pull=GPIO_PULLUP;          //����
  GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
  HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);	//PB4��1 
}

/**
  * @brief SPI�ײ�������ʱ��ʹ�ܣ���������
  * @param HAL_SPI_Init()���� //PA5,6,7
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* Peripheral clock enable */
	  __HAL_RCC_GPIOA_CLK_ENABLE();       //ʹ��GPIOAʱ��

    __HAL_RCC_SPI1_CLK_ENABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */
			
  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();
  
    /**SPI1 GPIO Configuration    
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI 
    */
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
	GPIO_InitTypeDef  GPIO_InitStruct;

  GPIO_InitStruct.Pin       = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode      = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


  }
  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

/**
  * @brief 	SPI������
  * @param	SPI1_Read
  * @retval ���شӻ�����
  */
uint32_t SPI1_Read(void)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t readvalue = 0;

  status = HAL_SPI_Receive(&hspi1, (uint8_t*) &readvalue, 1, 0xFFFFFFFF);

	/* Check the communication status */
	if(status != HAL_OK)
	{
		/* Re-Initiaize the BUS */
		DEBUG(2,"SPI1_Read_error = %d\r\n",status);
		Error_Handler();
	}

  return readvalue;
}


/**
  * @brief 	SPIд����
  * @param	SPI1_Write
  * @retval None
  */
void SPI1_Write(uint8_t Value)
{
	HAL_StatusTypeDef status = HAL_OK;

  status = HAL_SPI_Transmit(&hspi1, (uint8_t*) &Value, 1, 0xFFFFFFFF);
	
	/* Check the communication status */
	if(status != HAL_OK)
	{
	/* Re-Initiaize the BUS */
		 DEBUG(2,"SPI1_Write_error\r\n");
		 Error_Handler();
	}
}


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
