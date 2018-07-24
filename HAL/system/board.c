/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * Battery level ratio (battery dependent)
 */
#define BATTERY_STEP_LEVEL                0.23

#define CLOCK_16MHZ						  1


#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

/*!
 * Flag to indicate if the MCU is Initialized
 */

bool sendwkup = false;

static bool McuInitialized = false;

extern void app_mac_cb (mac_evt_t evt, void *msg);

extern RadioEvents_t *RadioEvents;

void BoardInitMcu( void )
{
   if( McuInitialized == false )
   {
        HAL_Init( );

        /***************ʱ�ӳ�ʼ��********************/
        SystemClockConfig( );
      
        RtcInit( );  
       
        TimerHwInit(  );
       
        /***************SX1276 I/O��ʼ��********************/
        SX1276IoInit( );

        /***************LoRa��Դ���� I/O��ʼ��********************/
        LoRaPower_Init( );

        /****************SPI��ʼ��*******************/
        SPI1_NSS( );						///Ƭѡ��ʼ��
                    
        SPI1_Init( );					///SPI��ʼ��	
     
        McuInitialized = true;        
   }
   else
   {
        SystemClockReConfig( );
       
        TimerHwInit(  );
       
        /***************SX1276 I/O��ʼ��********************/
        SX1276IoInit( );

        /***************LoRa��Դ���� I/O��ʼ��********************/
        LoRaPower_Init( );

        /****************SPI��ʼ��*******************/
        SPI1_NSS( );						///Ƭѡ��ʼ��
                    
        SPI1_Init( );	
//        Radio.Init( RadioEvents );
   }
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE(); ///����ʱ��

    /***************���ڳ�ʼ��********************/
    MX_USART1_UART_Init( );  

    MX_USART2_UART_Init(  );

    /*****************I2C��ʼ��********************/
    MX_I2C2_Init( );

    /****************ADC��ʼ��*******************/
    MX_ADC_Init();

    /*****************��Դ����********************/
    InitPower( );
   
    /*******************����RTC�ж�*******************/
    HAL_NVIC_SetPriority(RTC_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);

    HAL_NVIC_SetPriority(USART4_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART4_5_IRQn);
    
    /******************���ٳ�ʼ��*****************/
		ReadFlashData(  );

		clock_init();
		
		LoRaMacCsma.ChannelAddFun = UserChannelAddFun; ///��ֵ
		LoRaMacCsma.ListenAagain = UserListenAagain;   ///��ֵ

    printf("BoardInitMcu\r\n");       
}

uint32_t BoardGetRandomSeed( void )
{
    return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
    id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[0] = ( ( *( uint32_t* )ID2 ) );
}


/**
  * @brief ϵͳʱ�ӳ�ʼ��
  * @param �ⲿʱ��72MHZ
  * @retval None
  */
void SystemClockConfig( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
#if CLOCK_16MHZ

  /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
 

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  delay_init(16);
	
 #endif
}

void SystemClockReConfig( void )
{
    __HAL_RCC_PWR_CLK_ENABLE( );
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    /* Enable HSE */
    __HAL_RCC_HSE_CONFIG( RCC_HSE_ON );

    /* Wait till HSE is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    /* Enable PLL */
    __HAL_RCC_PLL_ENABLE( );

    /* Wait till PLL is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
    {
    }

    /* Select PLL as system clock source */
    __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
    {
    }
    /* Resume Tick interrupt if disabled prior to sleep mode entry*/
    HAL_ResumeTick();
}


/*
 *	BoardSleep:	����͹���ģʽ��SLEEP ģʽ
 *	����ֵ: 				��
 */
void BoardSleep( void )
{ 
    GPIO_InitTypeDef GPIO_InitStructure;

      /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    /****************************************/
    /* Disable the Peripheral */	
    HAL_ADC_MspInit(&hadc);  ///OK
    hadc.State = HAL_ADC_STATE_RESET;

     /* Disable the selected I2C peripheral */
    HAL_I2C_MspInit(&hi2c2);
    hi2c2.State = HAL_I2C_STATE_RESET;

    GPIO_InitStructure.Pin = 0xf903;  ///0xF0FF�������д�ӡ  0xFDFF 0xFF9F  0xFB0F--- UART1-PA10(RX) SPI ������Ϊģ������   0xFF0F GPIO_PIN_All
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
    
	///PB3 4 5 6 10 11 13 14 15����IO����Ϊģ������ģʽ
	GPIO_InitStructure.Pin = 0xEC78;  
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_5;  ///  PA8 15  |GPIO_PIN_9|GPIO_PIN_13|GPIO_PIN_14
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD; //��©
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure); 
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_RESET); ///SYSCLK	
}

/*
 *	BoardDeInitMcu:	����͹���ģʽ��ͣ������Ҫ������ӦIOģʽ
 *	����ֵ: 				��
 */
void BoardDeInitMcu( void )
{ 
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

      /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    
    /****************************************/
    /* Disable the Peripheral */	
    HAL_ADC_DeInit(&hadc);  ///OK
    hadc.State = HAL_ADC_STATE_RESET;

     /* Disable the selected I2C peripheral */
    HAL_I2C_DeInit(&hi2c2);
    hi2c2.State = HAL_I2C_STATE_RESET;

    ///�ر�UART1ʱ��
    HAL_UART_DeInit(&huart1);
    huart1.gState = HAL_UART_STATE_RESET;

    ///�ر�UART2ʱ��
    HAL_UART_DeInit(&huart2);
    huart2.gState = HAL_UART_STATE_RESET;

    ///�ر�UART5ʱ��
    HAL_UART_DeInit(&huart5);
    huart5.gState = HAL_UART_STATE_RESET;

    /*******************�ر�SPI*********************/
    /* Disble the selected SPI peripheral */
    HAL_SPI_DeInit(&hspi1);
    hspi1.State = HAL_SPI_STATE_RESET;
    
    HAL_TIM_Base_MspDeInit(&htim2);
    htim2.State = HAL_TIM_STATE_RESET;
    
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);             //ʹ���ж���2
	
	//�ж���3-PC3
	HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);             //ʹ���ж���3
	
	//�ж���4-PC15
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);     
       
#if 1

    GPIO_InitStructure.Pin = GPIO_PIN_All;   ///GPIO_PIN_All
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG; ///low_power,�����ϸ�
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
    HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
            
    GPIO_InitStructure.Pin = 0xEDFF;  /// PB9��CH_CE(ok)/PB5��485_DE/PB0: RESET/PB1: DIO0 �������IO����
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    
#endif
        
    /*Suspend Tick increment to prevent wakeup by Systick interrupt. 
    Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
    HAL_SuspendTick();

    /* Disable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOH_CLK_DISABLE();
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif
