/*
**************************************************************************************************************
*	@file	main.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	NBI_LoRaWAN end node PING-PONG Mode
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "timer.h"
#include "delay.h"
#include "board.h"

#if defined( USE_BAND_433 )

#define RF_FREQUENCY                                433000000 // Hz 

#elif defined( USE_BAND_868 )

#define RF_FREQUENCY                                868000000 // Hz

#elif defined( USE_BAND_915 )

#define RF_FREQUENCY                                915000000 // Hz

#else
    #error "Please define a frequency band in the compiler options."
#endif

#define TX_OUTPUT_POWER                             20        // dBm

#if defined( USE_MODEM_LORA )

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         5         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#elif defined( USE_MODEM_FSK )

#define FSK_FDEV                                    25e3      // Hz
#define FSK_DATARATE                                50e3      // bps
#define FSK_BANDWIDTH                               50e3      // Hz
#define FSK_AFC_BANDWIDTH                           83.333e3  // Hz
#define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   false

#else
    #error "Please define a modem in the compiler options."
#endif

TimerEvent_t CadTimer;
volatile bool CadTimerEvent = false;

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000000
#define BUFFER_SIZE                                 64 // Define the payload size here

uint8_t PingMsg[] = "PING";
uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];
uint8_t buffer[BUFFER_SIZE] = "hello world LoRa";

States_t State = LOWPOWER;

int8_t RssiValue = 0;
int8_t SnrValue = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );


/*!***********************************分割线************************************/

extern UART_HandleTypeDef 			    UartHandle;
extern RTC_HandleTypeDef 				RtcHandle;

//TimerEvent_t ReportTimer;
//volatile bool ReportTimerEvent = false;


//void OnReportTimerEvent( void )
//{
//   DEBUG(2,"OnReportTimerEvent\r\n");
//	 ReportTimerEvent = true;
//	 TimerStop( &ReportTimer );
////	 TimerSetValue( &ReportTimer, 5000 ); ///1s
////	 TimerStart( &ReportTimer );		
//}

extern SPI_HandleTypeDef SPI1_Handler;  //SPI1

RTC_DateTypeDef sdatestructureget;
RTC_TimeTypeDef stimestructureget;
RTC_AlarmTypeDef sAlarmtructureget;

bool test_rtc_state = false;

int main(void)
{
    bool isMaster = true;
    uint8_t i;
	
    BoardInitMcu();	
	
	// Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( RF_FREQUENCY );
	
#if defined( USE_MODEM_LORA )

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000000 );
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );

#elif defined( USE_MODEM_FSK )

    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                                  FSK_DATARATE, 0,
                                  FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                                  true, 0, 0, 0, 3000000 );
    
    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                                  0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                                  0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, true,
                                  0, 0,false, true );

#else
    #error "Please define a frequency band in the compiler options."
#endif

  DEBUG(2,"hello world NBI test_loar_spi\r\n");
	DEBUG(2,"LORA_SPREADING_FACTOR = %d\r\n", LORA_SPREADING_FACTOR);
																	
//  TimerInit( &ReportTimer, OnReportTimerEvent );
//	TimerSetValue( &ReportTimer, 1000000 ); ///1s
//	TimerStart( &ReportTimer );			

	uint8_t TxData = 0X42;
  uint8_t pRxData = SX1276Read( TxData );

	DEBUG(2, "SX1276 ID = 0x%x\r\n",pRxData);  ///读取到0x12则正确，否则错误				
 
  Read_Flash_Data(  );																	
 
//  Radio.Rx( RX_TIMEOUT_VALUE );
  while (1)
  {
#if 	0
        switch( State )
        {
        case RX:
            DEBUG(2," The state is RX\r\n ");
            if( isMaster == true )
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
                    {                   
                        // Send the next PING frame            
                        Buffer[0] = 'P';
                        Buffer[1] = 'I';
                        Buffer[2] = 'N';
                        Buffer[3] = 'G';
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        HAL_Delay( 1000 ); 
                        Radio.Send( Buffer, BufferSize );
                       
                    }
                    else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    { // A master already exists then become a slave
                        isMaster = false;                 
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }
                    else // valid reception but neither a PING or a PONG message
                    {    // Set device as master ans start again
                        isMaster = true;
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }
                }
            }
            else
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    {
                        // Send the reply to the PONG string
                        Buffer[0] = 'P';
                        Buffer[1] = 'O';
                        Buffer[2] = 'N';
                        Buffer[3] = 'G';
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        DEBUG(2,"line = %d\r\n", __LINE__);
                        HAL_Delay( 1000 );
                        DEBUG(2,"line = %d\r\n", __LINE__);
                        Radio.Send( Buffer, BufferSize );
                    }
                    else // valid reception but not a PING as expected
                    {    // Set device as master and start again
                        isMaster = true;
                        Radio.Rx( RX_TIMEOUT_VALUE );
                    }   
                }
            }
            State = LOWPOWER;
            break;
        case TX:
            DEBUG(2," The state is TX\r\n");
						HAL_Delay(2);
            Radio.Rx( RX_TIMEOUT_VALUE );
            
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            DEBUG(2," The state is RX_ERROR\r\n");
				    HAL_Delay(2);
            if( isMaster == true )
            {
                // Send the next PING frame
                Buffer[0] = 'P';
                Buffer[1] = 'I';
                Buffer[2] = 'N';
                Buffer[3] = 'G';
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                HAL_Delay( 1000 ); 
                Radio.Send( Buffer, BufferSize );
            }
            else
            {
                Radio.Rx( RX_TIMEOUT_VALUE );
            }
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            DEBUG(2," The state is TX_TIMEOUT\r\n");
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case LOWPOWER:
        default:
            // Set low power
            break;
        }
#else
				DEBUG(2, "buffer\r\n");  ///读取到0x12则正确，否则错误
        Radio.Send( buffer, BufferSize );  
				HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_15);
				DelayMs( 3000 ); 	
			
        Radio.Rx( RX_TIMEOUT_VALUE );		
				DelayMs( 1000 ); 	///????,??SPI?????,SPI????
		
//	if(ReportTimerEvent == true)
//	{
//		ReportTimerEvent = false;		
//		
//		HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
//			
//		/* Get the RTC current Date */
//		HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
//		
//		HAL_RTC_WaitForSynchro(&RtcHandle);	
//		DEBUG(3, "year - %d mouth - %d date - %d, hour : %d min : %d second : %d\r\n",sdatestructureget.Year,sdatestructureget.Month,sdatestructureget.Date,
//					stimestructureget.Hours,stimestructureget.Minutes,stimestructureget.Seconds);
//		TimerSetValue( &ReportTimer, 1000000 ); ///1s
//		TimerStart( &ReportTimer );
//	}	
	
//	uint8_t TxData = 0X42;
//  uint8_t pRxData = SX1276Read( TxData );

//	DEBUG(2, "SX1276 ID = 0x%x\r\n",pRxData);  ///读取到0x12则正确，否则错误
//	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);	//PB13置1 
//	  DelayMs(1500); 	
				
#endif		
  }
 

}


void OnTxDone( void )
{
    Radio.Sleep( );
    State = TX;
	  DEBUG(2,"OnTxDone\r\n");
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    Radio.Sleep( );
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
	  DEBUG(2,"Buffer = %s\r\n",Buffer);
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
	  DEBUG(2,"TX_TIMEOUT\r\n");
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
	  DEBUG(2,"RX_TIMEOUT\r\n");
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
	  DEBUG(2,"OnRxError\r\n");
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{ 
	DEBUG(2,"error\r\n");
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

