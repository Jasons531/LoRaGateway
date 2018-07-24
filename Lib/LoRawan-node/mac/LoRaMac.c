/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel J盲ckle ( STACKFORCE )
*/
#include "board.h"

#include "LoRaMacCrypto.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"

/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Maximum MAC commands buffer size
 */
#define LORA_MAC_COMMAND_MAX_LENGTH                 15

/*!
 * Device IEEE EUI
 */
static uint8_t *LoRaMacDevEui;

/*!
 * Application IEEE EUI
 */
static uint8_t *LoRaMacAppEui;

/*!
 * AES encryption/decryption cipher application key
 */
static uint8_t *LoRaMacAppKey;

/*!
 * AES encryption/decryption cipher network session key
 */
static uint8_t LoRaMacNwkSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t LoRaMacAppSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
static uint16_t LoRaMacDevNonce;

/*!
 * Network ID ( 3 bytes )
 */
static uint32_t LoRaMacNetID;

/*!
 * Mote Address
 */
static uint32_t LoRaMacDevAddr;

/*!
 * Multicast channels linked list
 */
static MulticastParams_t *MulticastChannels = NULL;

/*!
 * Actual device class
 */
static DeviceClass_t LoRaMacDeviceClass;

/*!
 * Indicates if the node is connected to a private or public network
 */
static bool PublicNetwork;

/*!
 * Indicates if the node supports repeaters
 */
static bool RepeaterSupport;

/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

/*!
 * Buffer containing the upper layer data.
 */
static uint8_t LoRaMacPayload[LORAMAC_PHY_MAXPAYLOAD];
static uint8_t LoRaMacRxPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
uint32_t UpLinkCounter = 1;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
static uint32_t DownLinkCounter = 0;

/*!
 * IsPacketCounterFixed enables the MIC field tests by fixing the
 * UpLinkCounter value
 */
static bool IsUpLinkCounterFixed = false;

/*!
 * Used for test purposes. Disables the opening of the reception windows.
 */
static bool IsRxWindowsEnabled = true;

/*!
 * Indicates if the MAC layer has already joined a network.
 */
static bool IsLoRaMacNetworkJoined = false;

/*!
 * LoRaMac ADR control status
 */
static bool AdrCtrlOn = false;

/*!
 * Counts the number of missed ADR acknowledgements
 */
static uint32_t AdrAckCounter = 0;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
static bool NodeAckRequested = false;

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
static bool SrvAckRequested = false;

/*!
 * Indicates if the MAC layer wants to send MAC commands
 */
static bool MacCommandsInNextTx = false;

/*!
 * Contains the current MacCommandsBuffer index
 */
static uint8_t MacCommandsBufferIndex = 0;

/*!
 * Buffer containing the MAC layer commands
 */
static uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_MAX_LENGTH];

#if defined( USE_BAND_433 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 59, 59, 59, 123, 250, 250, 250, 250 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 59, 59, 59, 123, 230, 230, 230, 230 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_780 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 59, 59, 59, 123, 250, 250, 250, 250 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 59, 59, 59, 123, 230, 230, 230, 230 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_868 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 242, 242, 242, 242 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
    BAND1,
    BAND2,
    BAND3,
    BAND4,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 10, 9, 8,  7,  8,  0,  0, 0, 12, 11, 10, 9, 8, 7, 0, 0 };

/*!
 * Up/Down link data rates offset definition
 */
const int8_t datarateOffsets[16][4] =
{
    { DR_10, DR_9 , DR_8 , DR_8  }, // DR_0
    { DR_11, DR_10, DR_9 , DR_8  }, // DR_1
    { DR_12, DR_11, DR_10, DR_9  }, // DR_2
    { DR_13, DR_12, DR_11, DR_10 }, // DR_3
    { DR_13, DR_13, DR_12, DR_11 }, // DR_4
    { 0xFF , 0xFF , 0xFF , 0xFF  },
    { 0xFF , 0xFF , 0xFF , 0xFF  },
    { 0xFF , 0xFF , 0xFF , 0xFF  },
    { DR_8 , DR_8 , DR_8 , DR_8  },
    { DR_9 , DR_8 , DR_8 , DR_8  },
    { DR_10, DR_9 , DR_8 , DR_8  },
    { DR_11, DR_10, DR_9 , DR_8  },
    { DR_12, DR_11, DR_10, DR_9  },
    { DR_13, DR_12, DR_11, DR_10 },
    { 0xFF , 0xFF , 0xFF , 0xFF  },
    { 0xFF , 0xFF , 0xFF , 0xFF  },
};

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 11, 53, 129, 242, 242, 0, 0, 0, 53, 129, 242, 242, 242, 242, 0, 0 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 11, 53, 129, 242, 242, 0, 0, 0, 33, 103, 222, 222, 222, 222, 0, 0 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
ChannelParams_t Channels[LORA_MAX_NB_CHANNELS];

/*!
 * Contains the channels which remain to be applied.
 */
static uint16_t ChannelsMaskRemaining[6];

#else
    #error "Please define a frequency band in the compiler options."
#endif

/*!
 * LoRaMAC 2nd reception window settings
 */
static Rx2ChannelParams_t Rx2Channel = RX_WND_2_CHANNEL;

/*!
 * Datarate offset between uplink and downlink on first window
 */
static uint8_t Rx1DrOffset = 0;

/*!
 * Mask indicating which channels are enabled
 */
static uint16_t ChannelsMask[6];

/*!
 * Channels Tx output power
 */
static int8_t ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;

/*!
 * Channels datarate  
 */
static int8_t ChannelsDatarate = 0;//LORAMAC_DEFAULT_DATARATE;

/*!
 * Channels default datarate
 */
static int8_t ChannelsDefaultDatarate = 0;//LORAMAC_DEFAULT_DATARATE;

/*!
 * Number of uplink messages repetitions [1:15] (unconfirmed messages only)
 */
static uint8_t ChannelsNbRep = 1;

/*!
 * Uplink messages repetitions counter
 */
static uint8_t ChannelsNbRepCounter = 0;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device.
 */
static uint8_t MaxDCycle = 0;

/*!
 * Aggregated duty cycle management
 */
static uint16_t AggregatedDCycle;
static TimerTime_t AggregatedLastTxDoneTime;
static TimerTime_t AggregatedTimeOff;

/*!
 * Enables/Disables duty cycle management (Test only)
 */
static bool DutyCycleOn;

/*!
 * Current channel index
 */
uint8_t Channel;

/*!
 * LoRaMac internal states
 */
enum eLoRaMacState
{
    MAC_IDLE          = 0x00000000,
    MAC_TX_RUNNING    = 0x00000001,
    MAC_RX            = 0x00000002,
    MAC_ACK_REQ       = 0x00000004,
    MAC_ACK_RETRY     = 0x00000008,
    MAC_TX_DELAYED    = 0x00000010,
    MAC_TX_CONFIG     = 0x00000020,
};

/*!
 * LoRaMac internal state
 */
uint32_t LoRaMacState = MAC_IDLE;

/*!
 * LoRaMac timer used to check the LoRaMacState (runs every second)
 */
static TimerEvent_t MacStateCheckTimer;

/*!
 * LoRaMac upper layer event functions
 */
static LoRaMacPrimitives_t *LoRaMacPrimitives;

/*!
 * LoRaMac upper layer callback functions
 */
static LoRaMacCallback_t *LoRaMacCallbacks;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
static TimerEvent_t TxDelayedTimer;

/*!
 * LoRaMac reception windows timers
 */
static TimerEvent_t RxWindowTimer1;
static TimerEvent_t RxWindowTimer2;

/*!
 * LoRaMac reception windows delay from end of Tx
 */
static uint32_t ReceiveDelay1;
static uint32_t ReceiveDelay2;
static uint32_t JoinAcceptDelay1;
static uint32_t JoinAcceptDelay2;

/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
static uint32_t RxWindow1Delay;
static uint32_t RxWindow2Delay;

/*!
 * LoRaMac maximum time a reception window stays open
 */
static uint32_t MaxRxWindow;

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
static TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetries = 1;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetriesCounter = 1;

/*!
 * Indicates if the AckTimeout timer has expired or not
 */
static bool AckTimeoutRetry = false;

/*!
 * Last transmission time on air
 */
TimerTime_t TxTimeOnAir = 0;

/*!
 * Structure to hold an MCPS indication data.
 */
static McpsIndication_t McpsIndication;

/*!
 * Structure to hold MCPS confirm data.
 */
static McpsConfirm_t McpsConfirm;

/*!
 * Structure to hold MLME confirm data.
 */
static MlmeConfirm_t MlmeConfirm;

/*!
 * Holds the current rx window slot
 */
static uint8_t RxSlot = 0;

/*!
 * LoRaMac tx/rx operation state
 */
LoRaMacFlags_t LoRaMacFlags;

/*!
 * \brief Function to be executed on Radio Cad Done event
 */
void OnRadioCadDone( bool channelActivityDetected );

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * \brief This function prepares the MAC to abort the execution of function
 *        OnRadioRxDone in case of a reception error.
 */
static void PrepareRxDoneAbort( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx error event
 */
static void OnRadioRxError( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void );

/*!
 * \brief Function executed on Resend Frame timer event.
 */
static void OnMacStateCheckTimerEvent( void );

/*!
 * \brief Function executed on duty cycle delayed Tx  timer event
 */
void OnTxDelayedTimerEvent( void );

/*!
 * \brief Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * \brief Searches and set the next random available channel
 *
 * \param [OUT] Time to wait for the next transmission according to the duty
 *              cycle.
 *
 * \retval status  Function status [1: OK, 0: Unable to find a channel on the
 *                                  current datarate]
 */
static bool SetNextChannel( TimerTime_t* time );

/*!
 * \brief Sets the network to public or private. Updates the sync byte.
 *
 * \param [IN] enable if true, it enables a public network
 */
static void SetPublicNetwork( bool enable );

/*!
 * \brief Initializes and opens the reception window
 *
 * \param [IN] freq window channel frequency
 * \param [IN] datarate window channel datarate
 * \param [IN] bandwidth window channel bandwidth
 * \param [IN] timeout window channel timeout
 */
static void RxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous );

/*!
 * \brief Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 );

/*!
 * \brief Validates if the payload fits into the frame, taking the datarate
 *        into account.
 *
 * \details Refer to chapter 4.3.2 of the LoRaWAN specification, v1.0
 *
 * \param lenN Length of the application payload. The length depends on the
 *             datarate and is region specific
 *
 * \param datarate Current datarate
 *
 * \param fOptsLen Length of the fOpts field
 *
 * \retval [false: payload does not fit into the frame, true: payload fits into
 *          the frame]
 */
static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * \brief Counts the number of enabled 125 kHz channels in the channel mask.
 *        This function can only be applied to US915 band.
 *
 * \param channelsMask Pointer to the first element of the channel mask
 *
 * \retval Number of enabled channels in the channel mask
 */
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask );
#endif

/*!
 * \brief Limits the Tx power according to the number of enabled channels
 *
 * \retval Returns the maximum valid tx power
 */
static int8_t LimitTxPower( int8_t txPower );

/*!
 * \brief Verifies, if a value is in a given range.
 *
 * \param value Value to verify, if it is in range
 *
 * \param min Minimum possible value
 *
 * \param max Maximum possible value
 *
 * \retval Returns the maximum valid tx power
 */
static bool ValueInRange( int8_t value, int8_t min, int8_t max );

/*!
 * \brief Calculates the next datarate to set, when ADR is on or off
 *
 * \param [IN] adrEnabled Specify whether ADR is on or off
 *
 * \param [IN] updateChannelMask Set to true, if the channel masks shall be updated
 *
 * \param [OUT] datarateOut Reports the datarate which will be used next
 *
 * \retval Returns the state of ADR ack request
 */
static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut );

/*!
 * \brief Disables channel in a specified channel mask
 *
 * \param [IN] id - Id of the channel
 *
 * \param [IN] mask - Pointer to the channel mask to edit
 *
 * \retval [true, if disable was successful, false if not]
 */
static bool DisableChannelInMask( uint8_t id, uint16_t* mask );

/*!
 * \brief Decodes MAC commands in the fOpts field and in the payload
 */
static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr );

/*!
 * \brief LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * \brief LoRaMAC layer frame buffer initialization
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );


/*!
 * \brief LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark PrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel parameters
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel );

void OnRadioCadDone( bool channelActivityDetected )
{
	if(channelActivityDetected == true)
	{
        LoRapp_Handle.Cad_Detect = true;
		DEBUG(2,"Cad_State3333 = Cad_Detect \r\n");
	}else
	{
        LoRapp_Handle.Cad_Done = true;
		DEBUG(2,"Cad_State3333 = CadDone \r\n");
	}
}

uint32_t rx1time = 0;
static void OnRadioTxDone( void )
{
    TimerTime_t curTime = TimerGetCurrentTime( );
   if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
	else
    {
        OnRxWindow2TimerEvent( );
    }

    // Update Band Time OFF
    Bands[Channels[Channel].Band].LastTxDoneTime = curTime;
    if( DutyCycleOn == true )
    {
        Bands[Channels[Channel].Band].TimeOff = TxTimeOnAir * Bands[Channels[Channel].Band].DCycle - TxTimeOnAir;
    }
    else
    {
        Bands[Channels[Channel].Band].TimeOff = 0;
    }
    // Update Aggregated Time OFF
    AggregatedLastTxDoneTime = curTime;
    AggregatedTimeOff = AggregatedTimeOff + ( TxTimeOnAir * AggregatedDCycle - TxTimeOnAir );

    if( IsRxWindowsEnabled == true )
    {
    	DEBUG(2, "IsRxWindowsEnabled is true RxWindow1Delay : %d\r\n",RxWindow1Delay);
        TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
        TimerStart( &RxWindowTimer1 );		
		
		
       	if( LoRaMacDeviceClass != CLASS_C )
       	{
            TimerSetValue( &RxWindowTimer2, RxWindow2Delay ); 					
            TimerStart( &RxWindowTimer2 );
			DEBUG(2, "LoRaMacDeviceClass : %d, NodeAckRequested : %d\r\n",LoRaMacDeviceClass,NodeAckRequested);
		}
		if( ( LoRaMacDeviceClass == CLASS_C ) || ( NodeAckRequested == true ) )
		{
			DEBUG(3,"AckTimeoutTimer\r\n");
			TimerSetValue( &AckTimeoutTimer, RxWindow2Delay + ACK_TIMEOUT +
												 randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
			TimerStart( &AckTimeoutTimer );
		}
        
    }
    else
    {
    	DEBUG(2, "IsRxWindowsEnabled is false\r\n");
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;

        if( LoRaMacFlags.Value == 0 )
        {
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        LoRaMacFlags.Bits.MacDone = 1;
    }

    if( NodeAckRequested == false )
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        ChannelsNbRepCounter++;
    }
}

void LoRaMacflag(void)
{
    LoRaMacFlags.Bits.MacDone = 0;
}

static void PrepareRxDoneAbort( void )
{
    LoRaMacState &= ~MAC_TX_RUNNING;

	DEBUG(2,"LoRaMacState &= ~MAC_TX_RUNNING--1\r\n");

    Radio.Standby( );

    if( NodeAckRequested )
    {
        OnAckTimeoutTimerEvent( );
    }

    if( ( RxSlot == 0 ) && ( LoRaMacDeviceClass == CLASS_C ) )
    {
        OnRxWindow2TimerEvent( );
    }

    LoRaMacFlags.Bits.McpsInd = 1;
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 );  //1ms
    TimerStart( &MacStateCheckTimer );
}

bool rx_start = false;

static uint8_t CurMulticastNwkSKey[] = {
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t CurMulticastAppSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    DEBUG(2,"%s\r\n",__func__);
	
    rx_start = true;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;

    uint16_t sequenceCounter = 0;
    uint16_t sequenceCounterPrev = 0;
    uint16_t sequenceCounterDiff = 0;
    uint32_t downLinkCounter = 0;

    MulticastParams_t *curMulticastParams = NULL;
    uint8_t *nwkSKey = CurMulticastNwkSKey; ///LoRaMacNwkSKey;
    uint8_t *appSKey = LoRaMacAppSKey;

    uint8_t multicast = 0;

    bool isMicOk = false;

    McpsConfirm.AckReceived = false;
    McpsIndication.Rssi = rssi;
    McpsIndication.Snr = snr;
    McpsIndication.RxSlot = RxSlot;
    McpsIndication.Port = 0;
    McpsIndication.Multicast = 0;
    McpsIndication.FramePending = 0;
    McpsIndication.Buffer = NULL;
    McpsIndication.BufferSize = 0;
    McpsIndication.RxData = false;
    McpsIndication.AckReceived = false;
    McpsIndication.DownLinkCounter = 0;
    McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    TimerStop( &RxWindowTimer2 );

    macHdr.Value = payload[pktHeaderLen++];

		DEBUG(2,"---macHdr.Value---0x%02x\r\n",macHdr.Value);
					
		if( macHdr.Value == 0x80 )
		{
			DEBUG(2,"---macHdr.Value-22--0x%02x\r\n",macHdr.Value);
			LoRapp_Handle.MhdrAck = true;
		}
			
		DEBUG(2,"---macHdr---%d\r\n",LoRapp_Handle.MhdrAck);

	
    if(Csma.Iq_Invert)
    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_DOWN;

    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            if( IsLoRaMacNetworkJoined == true )
            {
                break;
            }
            DEBUG(2,"join accept \r\n");
            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, LoRaMacRxPayload + 1 );

            LoRaMacRxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacRxPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );

            micRx |= ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );

            if( micRx == mic )
            {
                LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacRxPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                LoRaMacNetID = ( uint32_t )LoRaMacRxPayload[4];
                LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[5] << 8 );
                LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[6] << 16 );

                LoRaMacDevAddr = ( uint32_t )LoRaMacRxPayload[7];
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[8] << 8 );
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[9] << 16 );
                LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[10] << 24 );

                // DLSettings
                Rx1DrOffset = ( LoRaMacRxPayload[11] >> 4 ) & 0x07;
                Rx2Channel.Datarate = LoRaMacRxPayload[11] & 0x0F;
							  DEBUG(2,"Rx2Channel.Datarate = %d\r\n",Rx2Channel.Datarate);
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                /*
                 * WARNING: To be removed once Semtech server implementation
                 *          is corrected.
                 */
                if( Rx2Channel.Datarate == DR_3 )
                {
                    Rx2Channel.Datarate = DR_8;
                }
#endif
                // RxDelay
                ReceiveDelay1 = ( LoRaMacRxPayload[12] & 0x0F );
                if( ReceiveDelay1 == 0 )
                {
                    ReceiveDelay1 = 1;
                }
                ReceiveDelay1 *= 1e3;
                ReceiveDelay2 = ReceiveDelay1 + 1e3;

#if !( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                //CFList
                if( ( size - 1 ) > 16 )
                {
                    ChannelParams_t param;
                    param.DrRange.Value = ( DR_5 << 4 ) | DR_0;

                    LoRaMacState |= MAC_TX_CONFIG;
                    for( uint8_t i = 3, j = 0; i < ( 5 + 3 ); i++, j += 3 )
                    {
                        param.Frequency = ( ( uint32_t )LoRaMacRxPayload[13 + j] | ( ( uint32_t )LoRaMacRxPayload[14 + j] << 8 ) | ( ( uint32_t )LoRaMacRxPayload[15 + j] << 16 ) ) * 100;
                        LoRaMacChannelAdd( i, param );
                    }
                    LoRaMacState &= ~MAC_TX_CONFIG;
                }
#endif
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                IsLoRaMacNetworkJoined = true;
                ChannelsDatarate = ChannelsDefaultDatarate;
            }
            else
            {
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
            }
            break;
						
				case FRAME_TYPE_DATA_CONFIRMED_UP:  ///���ӽڵ��ͨ�����ݽ��մ���		
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {					
                address = payload[pktHeaderLen++];
                address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

								DEBUG(2,"FRAME_TYPE_DATA_UNCONFIRMED_DOWN %08x, %08x\r\n",address, LoRaMacDevAddr);

							  /**************************** add buy Jason Start **********************************/
							
                if( address != LoRaMacDevAddr ) 
                {
										DEBUG(2,"22--Address : %08x address : %08x\r\n",MulticastChannels->Address,address);
                    MulticastChannels->Address = address;
                    curMulticastParams = MulticastChannels;
                    
                    LoRaMacDevAddr = address; ///��������ID=��������ID		

										while( curMulticastParams != NULL )
                    {
                        DEBUG(2,"----------------1111------------\r\n");
                        if( address == curMulticastParams->Address )
                        {
                            DEBUG(2,"----------------2222------------\r\n");
                            multicast = 1;
                            
                            downLinkCounter = curMulticastParams->DownLinkCounter;
                            break;
                        }
                        DEBUG(2,"33--Address : %08x address : %08x, %d\r\n",curMulticastParams->Address,address,curMulticastParams->DownLinkCounter);
                        curMulticastParams = curMulticastParams->Next;
                    }	
									
									if( multicast == 0 )
									{
											// We are not the destination of this frame.
											McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
											PrepareRxDoneAbort( );
										
											DEBUG(2,"LoRaCsma.Listen RxDoneAbort return\r\n");
											return;
									}
								}
								else
								{
									multicast = 0;
									nwkSKey = LoRaMacNwkSKey;
									appSKey = LoRaMacAppSKey;
									downLinkCounter = DownLinkCounter;  ///Ĭ�϶���0
								}
								
						/**************************** add buy Jason End *********************************/

						for(uint8_t i = 0; i < size; i++)
						DEBUG(3,"%02x",payload[i]);
						DEBUG(3,"BufferSize = %d DownLinkCounter = %d\r\n",size,DownLinkCounter);

						fCtrl.Value = payload[pktHeaderLen++]; ///FCtrl bits: 0x20��ʾΪACK

						DEBUG(2,"pktHeaderLen = %d %02x\r\n",pktHeaderLen,fCtrl.Value);

						sequenceCounter = ( uint16_t )payload[pktHeaderLen++];     ///���յ�����֡��
						sequenceCounter |= ( uint16_t )payload[pktHeaderLen++] << 8;									

										appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;
						DEBUG(2,"sequenceCounter = %d downLinkCounter = %d appPayloadStartIndex = %d\r\n ", sequenceCounter,downLinkCounter,appPayloadStartIndex);

						micRx |= payload[size - LORAMAC_MFR_LEN];
						micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 1] << 8 );
						micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 2] << 16 );
						micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 3] << 24 );

						DEBUG(3,"payload[ %d] = %02x \r\n",size - LORAMAC_MFR_LEN,payload[size - LORAMAC_MFR_LEN]);
						DEBUG(3,"payload[ %d] = %02x \r\n",size - LORAMAC_MFR_LEN + 1,payload[size - LORAMAC_MFR_LEN + 1]);
						DEBUG(3,"payload[ %d] = %02x \r\n",size - LORAMAC_MFR_LEN + 2,payload[size - LORAMAC_MFR_LEN + 2]);
						DEBUG(3,"payload[ %d] = %02x \r\n",size - LORAMAC_MFR_LEN + 3,payload[size - LORAMAC_MFR_LEN + 3]);
						DEBUG(3,"micRx = %02x mic = %02x\r\n",( uint32_t )micRx, mic);
						DEBUG(3,"size = %d LORAMAC_MFR_LEN = %d size - LORAMAC_MFR_LEN = %d\r\n",size, LORAMAC_MFR_LEN, size - LORAMAC_MFR_LEN);
						
						DEBUG(3,"sequenceCounterPrev = %d sequenceCounter = %d \r\n",sequenceCounterPrev,sequenceCounter);

						sequenceCounterPrev = downLinkCounter; ///�����ϴν��յ���֡�ţ�ֻ����һ���ն��豸��������

						DEBUG(3,"sequenceCounterPrev111 = %d\r\n",sequenceCounterPrev);
						
						if(sequenceCounter < sequenceCounterPrev) ///�����յ�����ն˷��͵�������ʱ���������ϴν���֡�ţ����������쳣MIC����ʧ��
						{      	
											sequenceCounterPrev = downLinkCounter = 0;  
							DEBUG(3,"sequenceCounterPrev = %d\r\n",sequenceCounterPrev);
						}
						
						sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );  ///������

						DEBUG(3,"sequenceCounterPrev = %d sequenceCounter = %d sequenceCounterDiff = %d %d\r\n",sequenceCounterPrev,sequenceCounter,sequenceCounter - sequenceCounterPrev, 1 << 15);

            if( sequenceCounterDiff < ( 1 << 15 ) ) ///:16λ
					{
						downLinkCounter += sequenceCounterDiff; ///++1
						DEBUG(3,"DOWN_LINK = %d downLinkCounter = %d \r\n",DOWN_LINK,downLinkCounter);
						LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );///MICУ��: У��mac+paload----MIC֮ǰ����
						DEBUG(3,"mic --- %02x\r\n",mic);

						if( micRx == mic )
           {
							isMicOk = true;
							DEBUG(3,"isMicOk mic = %d\r\n",mic);
						}
					}					
					else ///32λ
					{
							// check for sequence roll-over
							uint32_t  downLinkCounterTmp = downLinkCounter + 0x10000 + ( int16_t )sequenceCounterDiff;
							DEBUG(2,"DOWN_LINK = %d downLinkCounter = %d downLinkCounterTmp = %d\r\n",DOWN_LINK,downLinkCounter,downLinkCounterTmp);
							LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounterTmp, &mic );
						
							if( micRx == mic )
							{
									isMicOk = true;
									downLinkCounter = downLinkCounterTmp;
							}
					}

					DEBUG(3,"isMicOk = %d\r\n",isMicOk);
					if( isMicOk == true )
					{
							McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
							McpsIndication.Multicast = multicast;
							McpsIndication.FramePending = fCtrl.Bits.FPending;
							McpsIndication.Buffer = NULL;
							McpsIndication.BufferSize = 0;
							McpsIndication.DownLinkCounter = downLinkCounter;

							McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;

							AdrAckCounter = 0;

							// Update 32 bits downlink counter
							if( multicast == 1 )
							{
									McpsIndication.McpsIndication = MCPS_MULTICAST;

									if( ( curMulticastParams->DownLinkCounter == downLinkCounter ) &&
											( curMulticastParams->DownLinkCounter != 0 ) )
									{
											McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
											McpsIndication.DownLinkCounter = downLinkCounter;
											PrepareRxDoneAbort( );
											return;
									}
									curMulticastParams->DownLinkCounter = downLinkCounter;
							}
							else
							{
									DEBUG(4,"macHdr.Bits.MType : %d\r\n",macHdr.Bits.MType);
									if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
									{
											SrvAckRequested = true;
											McpsIndication.McpsIndication = MCPS_CONFIRMED;
									}
									else
									{
											SrvAckRequested = false;
											McpsIndication.McpsIndication = MCPS_UNCONFIRMED;
									}
									if( ( DownLinkCounter == downLinkCounter ) &&
											( DownLinkCounter != 0 ) )
									{
											McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
											McpsIndication.DownLinkCounter = downLinkCounter;
											PrepareRxDoneAbort( );
											return;
									}
									DownLinkCounter = downLinkCounter;
							}
							
							DEBUG(3,"fCtrl.Bits.Ack : %d\r\n",fCtrl.Bits.Ack);
							// Check if the frame is an acknowledgement
							if( fCtrl.Bits.Ack == 1 )
							{
									McpsConfirm.AckReceived = true;
									McpsIndication.AckReceived = true;
			
									// Stop the AckTimeout timer as no more retransmissions
									// are needed.
									TimerStop( &AckTimeoutTimer );
							}
							else
							{
									McpsConfirm.AckReceived = false;

									if( AckTimeoutRetriesCounter > AckTimeoutRetries )
									{
											// Stop the AckTimeout timer as no more retransmissions
											// are needed.
											TimerStop( &AckTimeoutTimer );
									}
							}

							if( fCtrl.Bits.FOptsLen > 0 )
							{
									// Decode Options field MAC commands
									ProcessMacCommands( payload, 8, appPayloadStartIndex, snr );
							}
							
							if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
							{
																					DEBUG(3,"appPayloadStartIndex = %d\r\n",( ( size - 4 ) - appPayloadStartIndex ));
									port = payload[appPayloadStartIndex++];
									frameLen = ( size - 4 ) - appPayloadStartIndex;

									McpsIndication.Port = port;

									if( port == 0 )
									{
											LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
																						 frameLen,
																						 nwkSKey,
																						 address,
																						 DOWN_LINK,
																						 downLinkCounter,
																						 LoRaMacRxPayload );

											// Decode frame payload MAC commands
											ProcessMacCommands( LoRaMacRxPayload, 0, frameLen, snr );
									}
									else
									{
											LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
																						 frameLen,
																						 appSKey,
																						 address,
																						 DOWN_LINK,
																						 downLinkCounter,
																						 LoRaMacRxPayload );

											McpsIndication.Buffer = LoRaMacRxPayload;
											McpsIndication.BufferSize = frameLen;
											McpsIndication.RxData = true;                        
									}

									for(uint8_t i = 0; i < McpsIndication.BufferSize; i++)
									DEBUG(3,"%02x",McpsIndication.Buffer[i]);
									DEBUG(3,"BufferSize = %d\r\n",McpsIndication.BufferSize);
								}
							LoRaMacFlags.Bits.McpsInd = 1;
								
							macHdr.Value = 0;
           }
					else
					{
							McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;

				//	McpsIndication.DownLinkCounter = downLinkCounter;
				
							DEBUG(2,"INFO_STATUS_MIC_FAIL mic = %02x\r\n",mic);
							//	AddMacCommand( MOTE_MAC_LINK_ADR_ANS, 0x07, snr ); ///������������ط���ADR_req,�ڵ�û�Ϸ�ADR_ANS��ᵼ�����ز����·����ݣ��������
							//��MIC��ȷ�󣬷�����������ʾ�ն�ANS����
							PrepareRxDoneAbort( );
              return;
            }
         }
            break;
        case FRAME_TYPE_PROPRIETARY:
            {
                memcpy1( LoRaMacRxPayload, &payload[pktHeaderLen], size );

                McpsIndication.McpsIndication = MCPS_PROPRIETARY;
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                McpsIndication.Buffer = LoRaMacRxPayload;
                McpsIndication.BufferSize = size - pktHeaderLen;

                LoRaMacFlags.Bits.McpsInd = 1;
                break;
            }
        default:
            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            PrepareRxDoneAbort( );
            break;
    }

    if( ( RxSlot == 0 ) && ( LoRaMacDeviceClass == CLASS_C ) )
    {
        OnRxWindow2TimerEvent( );
    }
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 ); //1ms
    TimerStart( &MacStateCheckTimer );
}

static void OnRadioTxTimeout( void )
{
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    LoRaMacFlags.Bits.MacDone = 1;
}


static void OnRadioRxError( void ) ///ͨѶ������ײ
{
	///����A/C���ͣ������ܳ���rxerror��
	///A: �������ݷ�����ײ: �����ط�����
	///C: ����/������������ײ: �ٴ�����/�����ط�����
	if( LoRaMacDeviceClass != CLASS_C )
    {
    	DEBUG(2,"OnRadioRxError\r\n");
      Radio.Sleep( );
    }
    else
    {
    /**********************add buy Jason Start************************/
		///�ȴ�ACK�׶�: ������ֻ���������׶�		
		if( ( LoRaMacState & MAC_TX_RUNNING ) != MAC_TX_RUNNING )
		{
			///�����׶�����:
			DEBUG(2,"CsmaTimerEvent OnRadioRxError\r\n");	 
		}
        
			///RF���ܴ����쳣����Ҫ����״̬�л���ȷ��RF����������,���FIFO
			SX1276.Settings.State = RF_IDLE;
      Radio.Standby( );
      OnRxWindow2TimerEvent( );
	  /**********************add buy Jason End************************/
    }

    if( RxSlot == 1 )
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static void OnRadioRxTimeout( void )
{
    if( LoRaMacDeviceClass != CLASS_C ) 
    {
        ///ֻ���RX1��־λ�������������˲�ʹ��
			if(LoRapp_Handle.OnRxWindow1)
			{
				DEBUG(2,"OnRadioRxTimeout ACK Fail\r\n");
				LoRapp_Handle.OnRxWindow1 = false;

				LoRapp_Handle.Send_Counter ++;
									
				if(LoRapp_Handle.Send_Counter <= 1)
				LoRapp_Handle.Send_again = true;
			}
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    if( RxSlot == 1 )
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static void OnMacStateCheckTimerEvent( void ) 
{
    TimerStop( &MacStateCheckTimer );
    bool txTimeout = false;
	
		DEBUG(2, "%s\r\n",__func__);

    if( LoRaMacFlags.Bits.MacDone == 1 )
    {
        DEBUG(2, "line = %d\r\n",__LINE__);
        if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
        {
            if( ( McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                ( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) )
            {
                // Stop transmit cycle due to tx timeout.
                LoRaMacState &= ~MAC_TX_RUNNING;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                McpsConfirm.AckReceived = false;
                McpsConfirm.TxTimeOnAir = 0;
                txTimeout = true;
				DEBUG(2, "LoRaMacState &= ~MAC_TX_RUNNING--2\r\n");
            }
        }
		
        if( ( NodeAckRequested == false ) && ( txTimeout == false ) )
        {
       		DEBUG(2, "line = %d\r\n",__LINE__);
            if( LoRaMacFlags.Bits.MlmeReq == 1 )
            {
                if( MlmeConfirm.MlmeRequest == MLME_JOIN )
                {
                    if( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_OK )
                    {
                        UpLinkCounter = 0;
                    }
                    // Join messages aren't repeated automatically
                    ChannelsNbRepCounter = ChannelsNbRep;
                }
            }
            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
            {
            	DEBUG(2, "line = %d\r\n",__LINE__);
                if( ( ChannelsNbRepCounter >= ChannelsNbRep ) || ( LoRaMacFlags.Bits.McpsInd == 1 ) )
                {
               		DEBUG(2, "line = %d\r\n",__LINE__);
                    ChannelsNbRepCounter = 0;

                    AdrAckCounter++;
                    if( IsUpLinkCounterFixed == false )
                    {
                        UpLinkCounter++;
                    }
										DEBUG(2,"LoRaMacState &= ~MAC_TX_RUNNING--3\r\n");
                    LoRaMacState &= ~MAC_TX_RUNNING;
                }
                else
                {
                    LoRaMacFlags.Bits.MacDone = 0;
                    // Sends the same frame again
                    ScheduleTx( );
										DEBUG(2, " ---ScheduleTx again---\r\n");
                }
            }
        }
				DEBUG(2, "line = %d\r\n",__LINE__);

        if( LoRaMacFlags.Bits.McpsInd == 1 )
        {
            if( ( McpsConfirm.AckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) )
            {
                AckTimeoutRetry = false;
                NodeAckRequested = false;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;

								DEBUG(2,"LoRaMacState &= ~MAC_TX_RUNNING--4\r\n");
                LoRaMacState &= ~MAC_TX_RUNNING;
            }
        }

        if( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & MAC_TX_DELAYED ) == 0 ) )
        {
						DEBUG(2, "line = %d\r\n",__LINE__);
            AckTimeoutRetry = false;
            if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                AckTimeoutRetriesCounter++;

                if( (( AckTimeoutRetriesCounter % 2 ) == 1 ) )
                {
                    ChannelsDatarate = MAX( ChannelsDatarate - 1, LORAMAC_MIN_DATARATE );
										DEBUG(3," MAX( ChannelsDatarate - 1, LORAMAC_MIN_DATARATE ) = %d\r\n", MAX( ChannelsDatarate - 1, LORAMAC_MIN_DATARATE ));
                }
                LoRaMacFlags.Bits.MacDone = 0;

							 // Sends the same frame again                
							 DEBUG(2,"---ScheduleTx---\r\n");       
							 RfSend_time = HAL_GetTick(  );
							 ScheduleTx( );                
            }
            else
            {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                // Re-enable default channels LC1, LC2, LC3
                ChannelsMask[0] = ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
#elif defined( USE_BAND_915 )
                // Re-enable default channels
                ChannelsMask[0] = 0xFFFF;
                ChannelsMask[1] = 0xFFFF;
                ChannelsMask[2] = 0xFFFF;
                ChannelsMask[3] = 0xFFFF;
                ChannelsMask[4] = 0x00FF;
                ChannelsMask[5] = 0x0000;
#elif defined( USE_BAND_915_HYBRID )
                // Re-enable default channels
                ChannelsMask[0] = 0x00FF;
                ChannelsMask[1] = 0x0000;
                ChannelsMask[2] = 0x0000;
                ChannelsMask[3] = 0x0000;
                ChannelsMask[4] = 0x0001;
                ChannelsMask[5] = 0x0000;
#else
    #error "Please define a frequency band in the compiler options."
#endif
                LoRaMacState &= ~MAC_TX_RUNNING;

                NodeAckRequested = false;
                McpsConfirm.AckReceived = false;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }

				DEBUG(2,"LoRaMacState &= ~MAC_TX_RUNNING--5 %d\r\n",LoRaMacState);
            }
        }
    }
    // Handle reception for Class B and Class C
    if( ( LoRaMacState & MAC_RX ) == MAC_RX )
    {
        LoRaMacState &= ~MAC_RX;
    }
    if( LoRaMacState == MAC_IDLE )
    {
        if( LoRaMacFlags.Bits.McpsReq == 1 )
        {
            LoRaMacPrimitives->MacMcpsConfirm( &McpsConfirm );
            LoRaMacFlags.Bits.McpsReq = 0;
        }

        if( LoRaMacFlags.Bits.MlmeReq == 1 )
        {
            LoRaMacPrimitives->MacMlmeConfirm( &MlmeConfirm );
            LoRaMacFlags.Bits.MlmeReq = 0;
        }

        LoRaMacFlags.Bits.MacDone = 0;
    }
    else
    {
        // Operation not finished restart timer
        TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
        TimerStart( &MacStateCheckTimer );
		DEBUG(3," LoRaMacState = %d\r\n",LoRaMacState);
    }

    if( LoRaMacFlags.Bits.McpsInd == 1 )
    {
        LoRaMacPrimitives->MacMcpsIndication( &McpsIndication );
        LoRaMacFlags.Bits.McpsInd = 0;
    }
}

void OnTxDelayedTimerEvent( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;   
    TimerStop( &TxDelayedTimer );
    LoRaMacState &= ~MAC_TX_DELAYED;
    DEBUG(2,"%s\r\n",__func__);
    
    if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_JOIN ) )
    {
        macHdr.Value = 0;
        macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;

        fCtrl.Value = 0;
        fCtrl.Bits.Adr = AdrCtrlOn;

        /* In case of a join request retransmission, the stack must prepare
         * the frame again, because the network server keeps track of the random
         * LoRaMacDevNonce values to prevent reply attacks. */
        PrepareFrame( &macHdr, &fCtrl, 0, NULL, 0 );
    }

    ScheduleTx( );
}

void OnRxWindow1TimerEvent( void )
{
    uint16_t symbTimeout = 5; // DR_2, DR_1, DR_0
    int8_t datarate = 0;
    uint32_t bandwidth = 0; // LoRa 125 kHz

    DEBUG(2,"%s\r\n",__func__);

    TimerStop( &RxWindowTimer1 );
    RxSlot = 0;
    
    LoRapp_Handle.OnRxWindow1 = true;

    if( LoRaMacDeviceClass == CLASS_C )
    {
        Radio.Standby( );
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    datarate = ChannelsDatarate - Rx1DrOffset;
    if( datarate < 0 )
    {
        datarate = DR_0;
    }

    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( datarate >= DR_3 )
    { // DR_6, DR_5, DR_4, DR_3
        symbTimeout = 8;
    }
    if( datarate == DR_6 )
    {// LoRa 250 kHz
        bandwidth  = 1;
    }

    RxWindowSetup( Channels[Channel].Frequency, datarate, bandwidth, symbTimeout, false );//false---true��������
    DEBUG(2,"RX1Frequency %d, datarate %d\r\n",Channels[Channel].Frequency,datarate);
    
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    datarate = datarateOffsets[ChannelsDatarate][Rx1DrOffset];
    if( datarate < 0 )
    {
        datarate = DR_0;
    }
    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( datarate > DR_0 )
    { // DR_1, DR_2, DR_3, DR_4, DR_8, DR_9, DR_10, DR_11, DR_12, DR_13
        symbTimeout = 8;
    }
    if( datarate >= DR_4 )
    {// LoRa 500 kHz
        bandwidth  = 2;
    }
    RxWindowSetup( 923.3e6 + ( Channel % 8 ) * 600e3, datarate, bandwidth, symbTimeout, false );
#else
    #error "Please define a frequency band in the compiler options."
#endif
}

void OnRxWindow2TimerEvent( void )
{
	DEBUG(2,"func: %s\r\n",__func__);
	int8_t datarate = 0;
    uint16_t symbTimeout = 5; // DR_2, DR_1, DR_0
    uint32_t bandwidth = 0; // LoRa 125 kHz

    TimerStop( &RxWindowTimer2 );
    RxSlot = 1;
	
	///add by Ysheng Start

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    datarate = ChannelsDatarate - Rx1DrOffset;
    if( datarate < 0 )
    {
        datarate = DR_0;
    }

#endif

///add by Ysheng End
	
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( Rx2Channel.Datarate >= DR_3 )
    { // DR_6, DR_5, DR_4, DR_3
        symbTimeout = 8;
    }
    if( Rx2Channel.Datarate == DR_6 )
    {// LoRa 250 kHz
        bandwidth  = 1;
    }
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    // For higher datarates, we increase the number of symbols generating a Rx Timeout
    if( Rx2Channel.Datarate > DR_0 )
    { // DR_1, DR_2, DR_3, DR_4, DR_8, DR_9, DR_10, DR_11, DR_12, DR_13
        symbTimeout = 8;
    }
    if( Rx2Channel.Datarate >= DR_4 )
    {// LoRa 500 kHz
        bandwidth  = 2;
    }
#else
    #error "Please define a frequency band in the compiler options."
#endif
		
		uint32_t Freq = Channels[Channel].Frequency;
		Freq += 3e7;
		
    if( LoRaMacDeviceClass != CLASS_C ) ///����Ӧ��A��
    {
        RxWindowSetup( Freq, datarate, bandwidth, symbTimeout, false );
				DEBUG(2,"RX2Frequency = %d, Datarate = %d\r\n",Freq,datarate);
    }
    else
    {			
			 RxWindowSetup( Channels[Channel].Frequency, datarate, bandwidth, symbTimeout, true );
			 DEBUG(2,"RX2Frequency = %d, Datarate = %d\r\n",Channels[Channel].Frequency,datarate);
					
    }
}

static void OnAckTimeoutTimerEvent( void )
{
    TimerStop( &AckTimeoutTimer );
		DEBUG(2,"%s\r\n",__func__);
    if( NodeAckRequested == true )
    {
        AckTimeoutRetry = true;
        LoRaMacState &= ~MAC_ACK_REQ;
        DEBUG(2,"MAC_ACK_REQ\r\n");
    }
    if( LoRaMacDeviceClass == CLASS_C )
    {
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static bool SetNextChannel( TimerTime_t* time )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTx = 0;
    uint8_t enabledChannels[LORA_MAX_NB_CHANNELS];
    TimerTime_t curTime = TimerGetCurrentTime( );
    TimerTime_t nextTxDelay = ( TimerTime_t )( -1 );

    memset1( enabledChannels, 0, LORA_MAX_NB_CHANNELS );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( CountNbEnabled125kHzChannels( ChannelsMaskRemaining ) == 0 )
    { // Restore default channels
        memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) ChannelsMask, 8 );
    }
    if( ( ChannelsDatarate >= DR_4 ) && ( ( ChannelsMaskRemaining[4] & 0x00FF ) == 0 ) )
    { // Make sure, that the channels are activated
        ChannelsMaskRemaining[4] = ChannelsMask[4];
    }
#else
    uint8_t chanCnt = 0;
    for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
    {
        if( ChannelsMask[k] != 0 )
        {
            chanCnt++;
            break;
        }
    }
    if( chanCnt == 0 )
    {
        // Re-enable default channels, if no channel is enabled
        ChannelsMask[0] = ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
    }
#endif

    // Update Aggregated duty cycle
    if( AggregatedTimeOff < ( curTime - AggregatedLastTxDoneTime ) )
    {
        AggregatedTimeOff = 0;

        // Update bands Time OFF
        for( uint8_t i = 0; i < LORA_MAX_NB_BANDS; i++ )
        {
            if( DutyCycleOn == true )
            {
                if( Bands[i].TimeOff < ( curTime - Bands[i].LastTxDoneTime ) )
                {
                    Bands[i].TimeOff = 0;
                }
                if( Bands[i].TimeOff != 0 )
                {
                    nextTxDelay = MIN( Bands[i].TimeOff -
                                       ( curTime - Bands[i].LastTxDoneTime ),
                                       nextTxDelay );
                }
            }
            else
            {
                nextTxDelay = 0;
                Bands[i].TimeOff = 0;
            }
        }

        // Search how many channels are enabled
        for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
        {
            for( uint8_t j = 0; j < 16; j++ )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                if( ( ChannelsMaskRemaining[k] & ( 1 << j ) ) != 0 )
#else
                if( ( ChannelsMask[k] & ( 1 << j ) ) != 0 )
#endif
                {
                    if( Channels[i + j].Frequency == 0 )
                    { // Check if the channel is enabled
                        continue;
                    }
                    if( ( ( Channels[i + j].DrRange.Fields.Min <= ChannelsDatarate ) &&
                          ( ChannelsDatarate <= Channels[i + j].DrRange.Fields.Max ) ) == false )
                    { // Check if the current channel selection supports the given datarate
                     	  DEBUG(3,"continue \r\n");
                        continue;
                    }
                    if( Bands[Channels[i + j].Band].TimeOff > 0 )
                    { // Check if the band is available for transmission
                        delayTx++;
                        continue;
                    }
                    enabledChannels[nbEnabledChannels++] = i + j;
                }
            }
        }
    }
    else
    {
        delayTx++;
        nextTxDelay = AggregatedTimeOff - ( curTime - AggregatedLastTxDoneTime );
    }

    if( nbEnabledChannels > 0 )
    {
//        Channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];
				DEBUG(3,"Channel = %d \r\n",Channel);
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
        if( Channel < ( LORA_MAX_NB_CHANNELS - 8 ) )
        {
            DisableChannelInMask( Channel, ChannelsMaskRemaining );
        }
#endif
        *time = 0;
        return true;
    }
    else
    {
        if( delayTx > 0 )
        {
            // Delay transmission due to AggregatedTimeOff or to a band time off
            *time = nextTxDelay;
            return true;
        }
        // Datarate not supported by any channel
        *time = 0;
        return false;
    }
}

static void SetPublicNetwork( bool enable )
{
    PublicNetwork = enable;
    Radio.SetModem( MODEM_LORA );
    if( PublicNetwork == true )
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

static void RxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous )
{
    uint8_t downlinkDatarate = Datarates[datarate];
    RadioModems_t modem;

    if( Radio.GetStatus( ) == RF_IDLE )
    {
        Radio.SetChannel( freq );

        // Store downlink datarate
        McpsIndication.RxDatarate = ( uint8_t ) datarate;

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
        if( datarate == DR_7 )
        {
            modem = MODEM_FSK;
            Radio.SetRxConfig( modem, 50e3, downlinkDatarate * 1e3, 0, 83.333e3, 5, 0, false, 0, true, 0, 0, false, rxContinuous );
        }
        else
        {
            modem = MODEM_LORA;
					
			if(Csma.Iq_Invert) ///��Ե�
			{
				 Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 20, timeout, false, 0, false, 0, 0, false, rxContinuous ); //false -- P2P
				 DEBUG(3,"Radio.SetRxConfig\r\n");
			}
			else
			{
				Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 20, timeout, false, 0, false, 0, 0, true, rxContinuous ); //true---GW
			}
//            Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
        }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
        modem = MODEM_LORA;
        Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
#endif

        if( RepeaterSupport == true )
        {
            Radio.SetMaxPayloadLength( modem, MaxPayloadOfDatarateRepeater[datarate] );
        }
        else
        {
            Radio.SetMaxPayloadLength( modem, MaxPayloadOfDatarate[datarate] );
        }

        if( rxContinuous == false )
        {
			DEBUG(2,"Radio.MaxRxWindow: %d\r\n",MaxRxWindow);
            Radio.Rx( MaxRxWindow ); 
        }
        else
        {
			DEBUG(2,"Radio.Rx\r\n");
            Radio.Rx( 0 ); // Continuous mode
        }
    }
}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Get the maximum payload length
    if( RepeaterSupport == true )
    {
        maxN = MaxPayloadOfDatarateRepeater[datarate];
				LoRapp_Handle.ADR_Datarate = datarate;
    }
    else
    {
        maxN = MaxPayloadOfDatarate[datarate];
				LoRapp_Handle.ADR_Datarate = datarate; ///datarate�ص�ʹ��
    }

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );

    // Validation of the application payload size
    if( ( payloadSize <= maxN ) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) )
    {
        return true;
    }
    return false;
}

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask )
{
    uint8_t nb125kHzChannels = 0;

    for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS - 8; i += 16, k++ )
    {
        for( uint8_t j = 0; j < 16; j++ )
        {// Verify if the channel is active
            if( ( channelsMask[k] & ( 1 << j ) ) == ( 1 << j ) )
            {
                nb125kHzChannels++;
            }
        }
    }

    return nb125kHzChannels;
}
#endif

static int8_t LimitTxPower( int8_t txPower )
{
    int8_t resultTxPower = txPower;
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( ( ChannelsDatarate == DR_4 ) ||
        ( ( ChannelsDatarate >= DR_8 ) && ( ChannelsDatarate <= DR_13 ) ) )
    {// Limit tx power to max 26dBm
        resultTxPower =  MAX( txPower, TX_POWER_26_DBM );
    }
    else
    {
        if( CountNbEnabled125kHzChannels( ChannelsMask ) < 50 )
        {// Limit tx power to max 21dBm
            resultTxPower = MAX( txPower, TX_POWER_20_DBM );
        }
    }
#endif
    return resultTxPower;
}

static bool ValueInRange( int8_t value, int8_t min, int8_t max )
{
    if( ( value >= min ) && ( value <= max ) )
    {
        return true;
    }
    return false;
}

static bool DisableChannelInMask( uint8_t id, uint16_t* mask )
{
    uint8_t index = 0;
    index = id / 16;

    if( ( index > 4 ) || ( id >= LORA_MAX_NB_CHANNELS ) )
    {
        return false;
    }

    // Deactivate channel
    mask[index] &= ~( 1 << ( id % 16 ) );

    return true;
}

static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut )
{
    bool adrAckReq = false;
    int8_t datarate = ChannelsDatarate;

    if( adrEnabled == true )
    {
        if( datarate == LORAMAC_MIN_DATARATE )
        {
            AdrAckCounter = 0;
            adrAckReq = false;
        }
        else
        {
            if( AdrAckCounter >= ADR_ACK_LIMIT )
            {
                adrAckReq = true;
            }
            else
            {
                adrAckReq = false;
            }
            if( AdrAckCounter >= ( ADR_ACK_LIMIT + ADR_ACK_DELAY ) )
            {
                if( ( ( AdrAckCounter - ADR_ACK_DELAY ) % ADR_ACK_LIMIT ) == 0 )
                {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                    if( datarate > LORAMAC_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {

                            // Re-enable default channels LC1, LC2, LC3
                            ChannelsMask[0] = ChannelsMask[0] | (LC( 1 ) + LC( 2 ) + LC( 3 ) );
                        }
                    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    if( ( datarate > LORAMAC_MIN_DATARATE ) && ( datarate == DR_8 ) )
                    {
                        datarate = DR_4;
                    }
                    else if( datarate > LORAMAC_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
#if defined( USE_BAND_915 )
                            // Re-enable default channels
                            ChannelsMask[0] = 0xFFFF;
                            ChannelsMask[1] = 0xFFFF;
                            ChannelsMask[2] = 0xFFFF;
                            ChannelsMask[3] = 0xFFFF;
                            ChannelsMask[4] = 0x00FF;
                            ChannelsMask[5] = 0x0000;
#else // defined( USE_BAND_915_HYBRID )
                            // Re-enable default channels
                            ChannelsMask[0] = 0x00FF;
                            ChannelsMask[1] = 0x0000;
                            ChannelsMask[2] = 0x0000;
                            ChannelsMask[3] = 0x0000;
                            ChannelsMask[4] = 0x0001;
                            ChannelsMask[5] = 0x0000;
#endif
                        }
                    }
#else
#error "Please define a frequency band in the compiler options."
#endif
                }
            }
        }
    }

    *datarateOut = datarate;

    return adrAckReq;
}

static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_BUSY;

    switch( cmd )
    {
        case MOTE_MAC_LINK_CHECK_REQ:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this command
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate ACK, Channel ACK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 2 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // 1st byte Battery
                // 2nd byte Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }
    if( status == LORAMAC_STATUS_OK )
    {
        MacCommandsInNextTx = true;
    }
    return status;
}

static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr )
{
    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                MlmeConfirm.DemodMargin = payload[macIndex++];
                MlmeConfirm.NbGateways = payload[macIndex++];
                break;
            case SRV_MAC_LINK_ADR_REQ:
                {
                    uint8_t i;
                    uint8_t status = 0x07;
                    uint16_t chMask;
                    int8_t txPower = 0;
                    int8_t datarate = 0;
                    uint8_t nbRep = 0;
                    uint8_t chMaskCntl = 0;
                    uint16_t channelsMask[6] = { 0, 0, 0, 0, 0, 0 };

                    // Initialize local copy of the channels mask array
                    for( i = 0; i < 6; i++ )
                    {
                        channelsMask[i] = ChannelsMask[i];
                    }
                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    if( ( AdrCtrlOn == false ) &&
                        ( ( ChannelsDatarate != datarate ) || ( ChannelsTxPower != txPower ) ) )
                    { // ADR disabled don't handle ADR requests if server tries to change datarate or txpower
                        // Answer the server with fail status
                        // Power ACK     = 0
                        // Data rate ACK = 0
                        // Channel mask  = 0
                        AddMacCommand( MOTE_MAC_LINK_ADR_ANS, 0, 0 );
                        macIndex += 3;  // Skip over the remaining bytes of the request
                        break;
                    }
                    chMask = ( uint16_t )payload[macIndex++];
                    chMask |= ( uint16_t )payload[macIndex++] << 8;

                    nbRep = payload[macIndex++];
                    chMaskCntl = ( nbRep >> 4 ) & 0x07;
                    nbRep &= 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                    if( ( chMaskCntl == 0 ) && ( chMask == 0 ) )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else if( ( ( chMaskCntl >= 1 ) && ( chMaskCntl <= 5 )) ||
                             ( chMaskCntl >= 7 ) )
                    {
                        // RFU
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
                        {
                            if( chMaskCntl == 6 )
                            {
                                if( Channels[i].Frequency != 0 )
                                {
                                    chMask |= 1 << i;
                                }
                            }
                            else
                            {
                                if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                    ( Channels[i].Frequency == 0 ) )
                                {// Trying to enable an undefined channel
                                    status &= 0xFE; // Channel mask KO
                                }
                            }
                        }
                        channelsMask[0] = chMask;
                    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    if( chMaskCntl == 6 )
                    {
                        // Enable all 125 kHz channels
                        for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS - 8; i += 16, k++ )
                        {
                            for( uint8_t j = 0; j < 16; j++ )
                            {
                                if( Channels[i + j].Frequency != 0 )
                                {
                                    channelsMask[k] |= 1 << j;
                                }
                            }
                        }
                    }
                    else if( chMaskCntl == 7 )
                    {
                        // Disable all 125 kHz channels
                        channelsMask[0] = 0x0000;
                        channelsMask[1] = 0x0000;
                        channelsMask[2] = 0x0000;
                        channelsMask[3] = 0x0000;
                    }
                    else if( chMaskCntl == 5 )
                    {
                        // RFU
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( uint8_t i = 0; i < 16; i++ )
                        {
                            if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                ( Channels[chMaskCntl * 16 + i].Frequency == 0 ) )
                            {// Trying to enable an undefined channel
                                status &= 0xFE; // Channel mask KO
                            }
                        }
                        channelsMask[chMaskCntl] = chMask;

                        if( CountNbEnabled125kHzChannels( channelsMask ) < 6 )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
                    }
#else
    #error "Please define a frequency band in the compiler options."
#endif
                    if( ValueInRange( datarate, LORAMAC_MIN_DATARATE, LORAMAC_MAX_DATARATE ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    //
                    // Remark MaxTxPower = 0 and MinTxPower = 5
                    //
                    if( ValueInRange( txPower, LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) == false )
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        ChannelsDatarate = datarate;
                        ChannelsTxPower = txPower;
#if defined( USE_BAND_915_HYBRID )
                        ChannelsMask[0] = channelsMask[0] & 0x00FF;
                        ChannelsMask[1] = channelsMask[1] & 0x0000;
                        ChannelsMask[2] = channelsMask[2] & 0x0000;
                        ChannelsMask[3] = channelsMask[3] & 0x0000;
                        ChannelsMask[4] = channelsMask[4] & 0x0001;
                        ChannelsMask[5] = channelsMask[5] & 0x0000;
#else
                        ChannelsMask[0] = channelsMask[0];
                        ChannelsMask[1] = channelsMask[1];
                        ChannelsMask[2] = channelsMask[2];
                        ChannelsMask[3] = channelsMask[3];
                        ChannelsMask[4] = channelsMask[4];
                        ChannelsMask[5] = channelsMask[5];
#endif
                        ChannelsNbRep = nbRep;
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    uint8_t status = 0x07;
                    int8_t datarate = 0;
                    int8_t drOffset = 0;
                    uint32_t freq = 0;

                    drOffset = ( payload[macIndex] >> 4 ) & 0x07;
                    datarate = payload[macIndex] & 0x0F;
                    macIndex++;

                    freq =  ( uint32_t )payload[macIndex++];
                    freq |= ( uint32_t )payload[macIndex++] << 8;
                    freq |= ( uint32_t )payload[macIndex++] << 16;
                    freq *= 100;

                    if( Radio.CheckRfFrequency( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ValueInRange( datarate, LORAMAC_MIN_DATARATE, LORAMAC_MAX_DATARATE ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }
#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                    if( ( ValueInRange( datarate, DR_5, DR_7 ) == true ) ||
                        ( datarate > DR_13 ) )
                    {
                        status &= 0xFD; // Datarate KO
                    }
#endif
                    if( ValueInRange( drOffset, LORAMAC_MIN_RX1_DR_OFFSET, LORAMAC_MAX_RX1_DR_OFFSET ) == false )
                    {
                        status &= 0xFB; // Rx1DrOffset range KO
                    }

                    if( ( status & 0x07 ) == 0x07 )
                    {
                        Rx2Channel.Datarate = datarate;
                        Rx2Channel.Frequency = freq;
                        Rx1DrOffset = drOffset;
                    }
                    AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                {
                    uint8_t batteryLevel = BAT_LEVEL_NO_MEASURE;
                    if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetBatteryLevel != NULL ) )
                    {
                        batteryLevel = LoRaMacCallbacks->GetBatteryLevel( );
                    }
                    AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, batteryLevel, snr );
                    break;
                }
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;

#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                    status &= 0xFC; // Channel frequency and datarate KO
                    macIndex += 5;
#else
                    int8_t channelIndex = 0;
                    ChannelParams_t chParam;

                    channelIndex = payload[macIndex++];
                    chParam.Frequency = ( uint32_t )payload[macIndex++];
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.DrRange.Value = payload[macIndex++];

                    LoRaMacState |= MAC_TX_CONFIG;
                    if( chParam.Frequency == 0 )
                    {
                        if( channelIndex < 3 )
                        {
                            status &= 0xFC;
                        }
                        else
                        {
                            if( LoRaMacChannelRemove( channelIndex ) != LORAMAC_STATUS_OK )
                            {
                                status &= 0xFC;
                            }
                        }
                    }
                    else
                    {
                        switch( LoRaMacChannelAdd( channelIndex, chParam ) )
                        {
                            case LORAMAC_STATUS_OK:
                            {
                                break;
                            }
                            case LORAMAC_STATUS_FREQUENCY_INVALID:
                            {
                                status &= 0xFE;
                                break;
                            }
                            case LORAMAC_STATUS_DATARATE_INVALID:
                            {
                                status &= 0xFD;
                                break;
                            }
                            case LORAMAC_STATUS_FREQ_AND_DR_INVALID:
                            {
                                status &= 0xFC;
                                break;
                            }
                            default:
                            {
                                status &= 0xFC;
                                break;
                            }
                        }
                    }
                    LoRaMacState &= ~MAC_TX_CONFIG;
#endif
                    AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                }
                break;
            case SRV_MAC_RX_TIMING_SETUP_REQ:
                {
                    uint8_t delay = payload[macIndex++] & 0x0F;

                    if( delay == 0 )
                    {
                        delay++;
                    }
                    DEBUG(2,"line = %d delay = %d\r\n", __LINE__,delay);
                    
                    ReceiveDelay1 = delay * 1e3;
                    ReceiveDelay2 = ReceiveDelay1 + 1e3;
                    AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

		if(LoRapp_Handle.MhdrAck) ///���յ�����ȷ������
		{
			fCtrl.Value = 0x20;
			fCtrl.Bits.Ack           = true;
		}
		else	
		{
		    fCtrl.Value = 0;
				fCtrl.Bits.Ack           = false;
		}			
						
    fCtrl.Bits.FOptsLen      = 0;
    fCtrl.Bits.FPending      = 0;
   
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    // Prepare the frame
    status = PrepareFrame( macHdr, &fCtrl, fPort, fBuffer, fBufferSize );
    
    // Validate status
    if( status != LORAMAC_STATUS_OK )
    {
        return status;
    }

    // Reset confirm parameters
    McpsConfirm.NbRetries = 0;
    McpsConfirm.AckReceived = false;
    McpsConfirm.UpLinkCounter = UpLinkCounter;

    status = ScheduleTx( );

    return status;
}

LoRaMacStatus_t ScheduleTx( )
{
    TimerTime_t dutyCycleTimeOff = 0;

    // Check if the device is off
    if( MaxDCycle == 255 )
    {
        return LORAMAC_STATUS_DEVICE_OFF;
    }
    if( MaxDCycle == 0 )
    {
        AggregatedTimeOff = 0;
    }

    // Select channel
    while( SetNextChannel( &dutyCycleTimeOff ) == false )
    {
        // Set the default datarate
        ChannelsDatarate = ChannelsDefaultDatarate;

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
        // Re-enable default channels LC1, LC2, LC3
        ChannelsMask[0] = ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
#endif
    }

    // Schedule transmission of frame
    if( dutyCycleTimeOff == 0 )
    {
        // Try to send now
        DEBUG(3,"SendFrameOnChannel\r\n");
        return SendFrameOnChannel( Channels[Channel] );
    }
    else
    {
        // Send later - prepare timer
        LoRaMacState |= MAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, dutyCycleTimeOff );
        TimerStart( &TxDelayedTimer );
        DEBUG(2,"---TxDelayedTimer---\r\n");

        return LORAMAC_STATUS_OK;
    }
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    const void* payload = fBuffer;
    uint8_t payloadSize = fBufferSize;
    uint8_t framePort = fPort;

    LoRaMacBufferPktLen = 0;

    NodeAckRequested = false;

    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:
            RxWindow1Delay = JoinAcceptDelay1 - RADIO_WAKEUP_TIME;
            RxWindow2Delay = JoinAcceptDelay2 - RADIO_WAKEUP_TIME;

            LoRaMacBufferPktLen = pktHeaderLen;

            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacAppEui, 8 );
            LoRaMacBufferPktLen += 8;
            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacDevEui, 8 );
            LoRaMacBufferPktLen += 8;

            LoRaMacDevNonce = Radio.Random( );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            NodeAckRequested = true;
            //Intentional falltrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( IsLoRaMacNetworkJoined == false )
            {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            fCtrl->Bits.AdrAckReq = AdrNextDr( fCtrl->Bits.Adr, true, &ChannelsDatarate );

            if( ValidatePayloadLength( fBufferSize, ChannelsDatarate, MacCommandsBufferIndex ) == false )
            {
                return LORAMAC_STATUS_LENGTH_ERROR;
            }

            RxWindow1Delay = ReceiveDelay1 - RADIO_WAKEUP_TIME;
            RxWindow2Delay = ReceiveDelay2 - RADIO_WAKEUP_TIME;

            DEBUG(3,"SrvAckRequested : %d\r\n",SrvAckRequested);
            if( SrvAckRequested == true )
            {
                SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
                DEBUG(3,"fCtrl->Bits.Ack : %d\r\n",fCtrl->Bits.Ack);
            }

            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

            LoRaMacBuffer[pktHeaderLen++] = fCtrl->Value;

            LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            if( ( payload != NULL ) && ( payloadSize > 0 ) )
            {
                if( ( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_LENGTH ) && ( MacCommandsInNextTx == true ) )
                {
                    DEBUG(2,"fCtrl->Bits.FOptsLen : %d Index :%d\r\n",fCtrl->Bits.FOptsLen,MacCommandsBufferIndex);
                    fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;


                    // Update FCtrl field with new value of OptionsLength
                    LoRaMacBuffer[0x05] = fCtrl->Value;
                    for( i = 0; i < MacCommandsBufferIndex; i++ )
                    {
                        LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                    }
                }
            }
            else
            {
                if( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx ) )
                {
                    payloadSize = MacCommandsBufferIndex;
                    payload = MacCommandsBuffer;
                    framePort = 0;
                }
            }
            MacCommandsInNextTx = false;
            MacCommandsBufferIndex = 0;

            if( ( payload != NULL ) && ( payloadSize > 0 ) )
            {
                LoRaMacBuffer[pktHeaderLen++] = framePort;

                if( framePort == 0 )
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, payloadSize, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, LoRaMacPayload );
                }
                else
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, payloadSize, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, LoRaMacPayload );
                }
                memcpy1( LoRaMacBuffer + pktHeaderLen, LoRaMacPayload, payloadSize );
            }
            LoRaMacBufferPktLen = pktHeaderLen + payloadSize;

            LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

            LoRaMacBufferPktLen += LORAMAC_MFR_LEN;

            break;
        case FRAME_TYPE_PROPRIETARY:
            if( ( fBuffer != NULL ) && ( fBufferSize > 0 ) )
            {
                memcpy1( LoRaMacBuffer + pktHeaderLen, ( uint8_t* ) fBuffer, fBufferSize );
                LoRaMacBufferPktLen = pktHeaderLen + fBufferSize;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SendFrameOnChannel( ChannelParams_t channel )
{
    int8_t datarate = Datarates[ChannelsDatarate];
    int8_t txPower = 0;

    ChannelsTxPower = LimitTxPower( ChannelsTxPower );
    txPower = TxPowers[ChannelsTxPower];

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Datarate = ChannelsDatarate;
    McpsConfirm.TxPower = ChannelsTxPower;

	DEBUG(3,"McpsConfirm.Datarate = %d\r\n",McpsConfirm.Datarate);

	uint32_t Freq = channel.Frequency;	
	Freq += 3e7;
    
    Radio.SetChannel( Freq );
    DEBUG(2, "Txchannel.Frequency = %d\r\n",Freq);

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( ChannelsDatarate == DR_7 )
    { // High Speed FSK channel
        Radio.SetMaxPayloadLength( MODEM_FSK, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_FSK, txPower, 25e3, 0, datarate * 1e3, 0, 5, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_FSK, LoRaMacBufferPktLen );
    }
    else if( ChannelsDatarate == DR_6 )
    { // High speed LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 1, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 202, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
    if( ChannelsDatarate >= DR_4 )
    { // High speed LoRa channel BW500 kHz
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 2, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
#else
    #error "Please define a frequency band in the compiler options."
#endif

    // Store the time on air
    McpsConfirm.TxTimeOnAir = TxTimeOnAir;
    MlmeConfirm.TxTimeOnAir = TxTimeOnAir;

	DEBUG(2,"McpsConfirm.TxTimeOnAir = %d, MlmeConfirm.TxTimeOnAir = %d\r\n",McpsConfirm.TxTimeOnAir,MlmeConfirm.TxTimeOnAir);

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    // Send now
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

    LoRaMacState |= MAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks )
{
    if( primitives == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    if( ( primitives->MacMcpsConfirm == NULL ) ||
        ( primitives->MacMcpsIndication == NULL ) ||
        ( primitives->MacMlmeConfirm == NULL ))
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    LoRaMacPrimitives = primitives;
    LoRaMacCallbacks = callbacks;

    LoRaMacFlags.Value = 0;

    LoRaMacDeviceClass = CLASS_A;

    UpLinkCounter = 1;
    DownLinkCounter = 0;
    AdrAckCounter = 0;

    RepeaterSupport = false;
    IsRxWindowsEnabled = true;
    IsLoRaMacNetworkJoined = false;
    LoRaMacState = MAC_IDLE;

#if defined( USE_BAND_433 )
    ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_780 )
    ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_868 )
    ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_915 )
    ChannelsMask[0] = 0xFFFF;
    ChannelsMask[1] = 0xFFFF;
    ChannelsMask[2] = 0xFFFF;
    ChannelsMask[3] = 0xFFFF;
    ChannelsMask[4] = 0x00FF;
    ChannelsMask[5] = 0x0000;

    memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) ChannelsMask, sizeof( ChannelsMask ) );
#elif defined( USE_BAND_915_HYBRID )
    ChannelsMask[0] = 0x00FF;
    ChannelsMask[1] = 0x0000;
    ChannelsMask[2] = 0x0000;
    ChannelsMask[3] = 0x0000;
    ChannelsMask[4] = 0x0001;
    ChannelsMask[5] = 0x0000;

    memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) ChannelsMask, sizeof( ChannelsMask ) );
#else
    #error "Please define a frequency band in the compiler options."
#endif

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    // 125 kHz channels
    for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS - 8; i++ )
    {
        Channels[i].Frequency = 902.3e6 + i * 200e3;
        Channels[i].DrRange.Value = ( DR_3 << 4 ) | DR_0;
        Channels[i].Band = 0;
    }
    // 500 kHz channels
    for( uint8_t i = LORA_MAX_NB_CHANNELS - 8; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        Channels[i].Frequency = 903.0e6 + ( i - ( LORA_MAX_NB_CHANNELS - 8 ) ) * 1.6e6;
        Channels[i].DrRange.Value = ( DR_4 << 4 ) | DR_4;
        Channels[i].Band = 0;
    }
#endif

    ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;
    ChannelsDefaultDatarate = ChannelsDatarate = LoRapp_Handle.default_datarate; //LORAMAC_DEFAULT_DATARATE;
		
		DEBUG(2,"RF_Send_Data = %d\r\n",LoRapp_Handle.default_datarate);
    ChannelsNbRep = 1;
    ChannelsNbRepCounter = 0;

    MaxDCycle = 0;
    AggregatedDCycle = 1;
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

#if defined( USE_BAND_433 )
    DutyCycleOn = false;
#elif defined( USE_BAND_780 )
    DutyCycleOn = false;
#elif defined( USE_BAND_868 )
    DutyCycleOn = true;
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    DutyCycleOn = false;
#else
    #error "Please define a frequency band in the compiler options."
#endif

    MaxRxWindow = MAX_RX_WINDOW;
    ReceiveDelay1 = RECEIVE_DELAY1;
    ReceiveDelay2 = RECEIVE_DELAY2;
    JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;
    JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;

    TimerInit( &MacStateCheckTimer, OnMacStateCheckTimerEvent );
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent );

    // Initialize Radio driver
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );

    // Random seed initialization
    srand1( Radio.Random( ) );

    // Initialize channel index.
    Channel = LORA_MAX_NB_CHANNELS;

    PublicNetwork = true;
    SetPublicNetwork( PublicNetwork );
    Radio.Sleep( );
    
    MulticastChannels = (MulticastParams_t *)malloc(sizeof(MulticastChannels));
    
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo )
{
    int8_t datarate = ChannelsDefaultDatarate;

    if( txInfo == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    AdrNextDr( AdrCtrlOn, false, &datarate );

    if( RepeaterSupport == true )
    {
        txInfo->CurrentPayloadSize = MaxPayloadOfDatarateRepeater[datarate];
    }
    else
    {
        txInfo->CurrentPayloadSize = MaxPayloadOfDatarate[datarate];
    }

    if( txInfo->CurrentPayloadSize >= MacCommandsBufferIndex )
    {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize - MacCommandsBufferIndex;
    }
    else
    {
        return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    if( ValidatePayloadLength( size, datarate, 0 ) == false )
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }

    if( ValidatePayloadLength( size, datarate, MacCommandsBufferIndex ) == false )
    {
        return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( mibGet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    switch( mibGet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            mibGet->Param.Class = LoRaMacDeviceClass;
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            mibGet->Param.IsNetworkJoined = IsLoRaMacNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            mibGet->Param.AdrEnable = AdrCtrlOn;
            break;
        }
        case MIB_NET_ID:
        {
            mibGet->Param.NetID = LoRaMacNetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            mibGet->Param.DevAddr = LoRaMacDevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            mibGet->Param.NwkSKey = LoRaMacNwkSKey;
            break;
        }
        case MIB_APP_SKEY:
        {
            mibGet->Param.AppSKey = LoRaMacAppSKey;
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            mibGet->Param.EnablePublicNetwork = PublicNetwork;
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
            mibGet->Param.EnableRepeaterSupport = RepeaterSupport;
            break;
        }
        case MIB_CHANNELS:
        {
            mibGet->Param.ChannelList = Channels;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            mibGet->Param.Rx2Channel = Rx2Channel;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            mibGet->Param.ChannelsMask = ChannelsMask;
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            mibGet->Param.ChannelNbRep = ChannelsNbRep;
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            mibGet->Param.MaxRxWindow = MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            mibGet->Param.ReceiveDelay1 = ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            mibGet->Param.ReceiveDelay2 = ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            mibGet->Param.JoinAcceptDelay1 = JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            mibGet->Param.JoinAcceptDelay2 = JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            mibGet->Param.ChannelsDatarate = ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            mibGet->Param.ChannelsTxPower = ChannelsTxPower;
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            mibGet->Param.UpLinkCounter = UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            mibGet->Param.DownLinkCounter = DownLinkCounter;
            break;
        }
        case MIB_MULTICAST_CHANNEL:
        {
            mibGet->Param.MulticastList = MulticastChannels;
            printf("MIB_MULTICAST_CHANNEL\r\n");
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( mibSet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    switch( mibSet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            LoRaMacDeviceClass = mibSet->Param.Class;
            switch( LoRaMacDeviceClass )
            {
                case CLASS_A:
                {
                    // Set the radio into sleep to setup a defined state
                    Radio.Sleep( );
                    break;
                }
                case CLASS_B:
                {
                    break;
                }
                case CLASS_C:
                {
                    // Set the NodeAckRequested indicator to default
                    NodeAckRequested = false;
                    OnRxWindow2TimerEvent( );
                    break;
                }
            }
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            IsLoRaMacNetworkJoined = mibSet->Param.IsNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            AdrCtrlOn = mibSet->Param.AdrEnable;
            break;
        }
        case MIB_NET_ID:
        {
            LoRaMacNetID = mibSet->Param.NetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            LoRaMacDevAddr = mibSet->Param.DevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            if( mibSet->Param.NwkSKey != NULL )
            {
                memcpy1( LoRaMacNwkSKey, mibSet->Param.NwkSKey,
                               sizeof( LoRaMacNwkSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_SKEY:
        {
            if( mibSet->Param.AppSKey != NULL )
            {
                memcpy1( LoRaMacAppSKey, mibSet->Param.AppSKey,
                               sizeof( LoRaMacAppSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            SetPublicNetwork( mibSet->Param.EnablePublicNetwork );
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
             RepeaterSupport = mibSet->Param.EnableRepeaterSupport;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            Rx2Channel = mibSet->Param.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            if( mibSet->Param.ChannelsMask )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                if( ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) < 6 ) &&
                    ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) > 0 ) )
                {
                    status = LORAMAC_STATUS_PARAMETER_INVALID;
                }
                else
                {
                    memcpy1( ( uint8_t* ) ChannelsMask,
                             ( uint8_t* ) mibSet->Param.ChannelsMask, sizeof( ChannelsMask ) );
                    for ( uint8_t i = 0; i < sizeof( ChannelsMask ) / 2; i++ )
                    {
                        ChannelsMaskRemaining[i] &= ChannelsMask[i];
                    }
                }
#else
                memcpy1( ( uint8_t* ) ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsMask, 2 );
#endif
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            if( ( mibSet->Param.ChannelNbRep >= 1 ) &&
                ( mibSet->Param.ChannelNbRep <= 15 ) )
            {
                ChannelsNbRep = mibSet->Param.ChannelNbRep;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            MaxRxWindow = mibSet->Param.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            ReceiveDelay1 = mibSet->Param.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            ReceiveDelay2 = mibSet->Param.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            JoinAcceptDelay1 = mibSet->Param.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            JoinAcceptDelay2 = mibSet->Param.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            if( ValueInRange( mibSet->Param.ChannelsDatarate,
                              LORAMAC_MIN_DATARATE, LORAMAC_MAX_DATARATE ) )
            {
                ChannelsDatarate = mibSet->Param.ChannelsDatarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            if( ValueInRange( mibSet->Param.ChannelsTxPower,
                              LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                int8_t txPower = LimitTxPower( mibSet->Param.ChannelsTxPower );
                if( txPower == mibSet->Param.ChannelsTxPower )
                {
                    ChannelsTxPower = mibSet->Param.ChannelsTxPower;
                }
                else
                {
                    status = LORAMAC_STATUS_PARAMETER_INVALID;
                }
#else
                ChannelsTxPower = mibSet->Param.ChannelsTxPower;
#endif
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params )
{
#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    return LORAMAC_STATUS_PARAMETER_INVALID;
#else
    bool datarateInvalid = false;
    bool frequencyInvalid = false;
    uint8_t band = 0;

    // The id must not exceed LORA_MAX_NB_CHANNELS
    if( id >= LORA_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    // Validate if the MAC is in a correct state
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        if( ( LoRaMacState & MAC_TX_CONFIG ) != MAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }
    // Validate the datarate
    if( ( params.DrRange.Fields.Min > params.DrRange.Fields.Max ) ||
        ( ValueInRange( params.DrRange.Fields.Min, LORAMAC_MIN_DATARATE,
                        LORAMAC_MAX_DATARATE ) == false ) ||
        ( ValueInRange( params.DrRange.Fields.Max, LORAMAC_MIN_DATARATE,
                        LORAMAC_MAX_DATARATE ) == false ) )
    {
        datarateInvalid = true;
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( id < 3 )
    {
        if( params.Frequency != Channels[id].Frequency )
        {
            frequencyInvalid = true;
        }

        if( params.DrRange.Fields.Min > LORAMAC_DEFAULT_DATARATE )
        {
            datarateInvalid = true;
        }
        if( ValueInRange( params.DrRange.Fields.Max, DR_5, LORAMAC_MAX_DATARATE ) == false )
        {
            datarateInvalid = true;
        }
    }
#endif

    // Validate the frequency
    if( ( Radio.CheckRfFrequency( params.Frequency ) == true ) && ( params.Frequency > 0 ) && ( frequencyInvalid == false ) )
    {
#if defined( USE_BAND_868 )
        if( ( params.Frequency >= 865000000 ) && ( params.Frequency <= 868000000 ) )
        {
            band = BAND_G1_0;
        }
        else if( ( params.Frequency > 868000000 ) && ( params.Frequency <= 868600000 ) )
        {
            band = BAND_G1_1;
        }
        else if( ( params.Frequency >= 868700000 ) && ( params.Frequency <= 869200000 ) )
        {
            band = BAND_G1_2;
        }
        else if( ( params.Frequency >= 869400000 ) && ( params.Frequency <= 869650000 ) )
        {
            band = BAND_G1_3;
        }
        else if( ( params.Frequency >= 869700000 ) && ( params.Frequency <= 870000000 ) )
        {
            band = BAND_G1_4;
        }
        else
        {
            frequencyInvalid = true;
        }
#endif
    }
    else
    {
        frequencyInvalid = true;
    }

    if( ( datarateInvalid == true ) && ( frequencyInvalid == true ) )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }
    if( datarateInvalid == true )
    {
        return LORAMAC_STATUS_DATARATE_INVALID;
    }
    if( frequencyInvalid == true )
    {
        return LORAMAC_STATUS_FREQUENCY_INVALID;
    }

    // Every parameter is valid, activate the channel
    Channels[id] = params;
    Channels[id].Band = band;
    ChannelsMask[0] |= ( 1 << id );
   	
    return LORAMAC_STATUS_OK;
#endif
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        if( ( LoRaMacState & MAC_TX_CONFIG ) != MAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    if( id < 3 )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    else
    {
        // Remove the channel from the list of channels
        Channels[id] = ( ChannelParams_t ){ 0, { 0 }, 0 };
        
        // Disable the channel as it doesn't exist anymore
        if( DisableChannelInMask( id, ChannelsMask ) == false )
        {
            return LORAMAC_STATUS_PARAMETER_INVALID;
        }
    }
    return LORAMAC_STATUS_OK;
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    return LORAMAC_STATUS_PARAMETER_INVALID;
#endif
}

LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
    	printf("MulticastChannels NULL\r\n");
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
    	printf("LORAMAC_STATUS_BUSY NULL\r\n");
        return LORAMAC_STATUS_BUSY;
    }

    // Reset downlink counter
    channelParam->DownLinkCounter = 0;

    if( MulticastChannels == NULL )
    {
        // New node is the fist element
        MulticastChannels = channelParam;
		printf("MulticastChannels == NULL\r\n");
    }
    else
    {
        printf("MulticastChannels\r\n");
        MulticastParams_t *cur = MulticastChannels;

        // Search the last node in the list
        while( cur->Next != NULL )
        {
            cur = cur->Next;
        }
        // This function always finds the last node
        cur->Next = channelParam;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( MulticastChannels != NULL )
    {
        if( MulticastChannels == channelParam )
        {
          // First element
          MulticastChannels = channelParam->Next;
        }
        else
        {
            MulticastParams_t *cur = MulticastChannels;

            // Search the node in the list
            while( cur->Next && cur->Next != channelParam )
            {
                cur = cur->Next;
            }
            // If we found the node, remove it
            if( cur->Next )
            {
                cur->Next = channelParam->Next;
            }
        }
        channelParam->Next = NULL;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;

    if( mlmeRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    memset1( ( uint8_t* ) &MlmeConfirm, 0, sizeof( MlmeConfirm ) );

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mlmeRequest->Type )
    {
        case MLME_JOIN:
        {
            if( ( LoRaMacState & MAC_TX_DELAYED ) == MAC_TX_DELAYED )
            {
                status = LORAMAC_STATUS_BUSY;
            }

            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

            if( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppKey == NULL ) )
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }

            LoRaMacFlags.Bits.MlmeReq = 1;

            LoRaMacDevEui = mlmeRequest->Req.Join.DevEui;
            LoRaMacAppEui = mlmeRequest->Req.Join.AppEui;
            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;

            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            IsLoRaMacNetworkJoined = false;

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
#if defined( USE_BAND_915 )
            // Re-enable 500 kHz default channels
            ChannelsMask[4] = 0x00FF;
#else // defined( USE_BAND_915_HYBRID )
            // Re-enable 500 kHz default channels
            ChannelsMask[4] = 0x0001;
#endif

            static uint8_t drSwitch = 0;

            if( ( ++drSwitch & 0x01 ) == 0x01 )
            {
                ChannelsDatarate = DR_0;
            }
            else
            {
                ChannelsDatarate = DR_4;
            }
#endif

            status = Send( &macHdr, 0, NULL, 0 );
            break;
        }
        case MLME_LINK_CHECK:
        {
            LoRaMacFlags.Bits.MlmeReq = 1;
            // LoRaMac will send this command piggy-pack
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

            status = AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
            break;
        }
        default:
            break;
    }

    if( status != LORAMAC_STATUS_OK )
    {
        NodeAckRequested = false;
        LoRaMacFlags.Bits.MlmeReq = 0;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    uint8_t fPort = 0;
    void *fBuffer;
    uint16_t fBufferSize;
    int8_t datarate;
    bool readyToSend = false;

    DEBUG(3,"%s\r\n",__func__);

    if( mcpsRequest == NULL )
    {
        DEBUG(3,"LORAMAC_STATUS_PARAMETER_INVALID\r\n");
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING ) ||
        ( ( LoRaMacState & MAC_TX_DELAYED ) == MAC_TX_DELAYED ) )
    {
        DEBUG(2,"LoRaMacState �� %d\r\n",LoRaMacState);
        return LORAMAC_STATUS_BUSY;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t* ) &McpsConfirm, 0, sizeof( McpsConfirm ) );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mcpsRequest->Type )
    {
        case MCPS_UNCONFIRMED:
        {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
            fPort = mcpsRequest->Req.Unconfirmed.fPort;
            fBuffer = mcpsRequest->Req.Unconfirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Unconfirmed.fBufferSize;
            datarate = mcpsRequest->Req.Unconfirmed.Datarate;
            break;
        }
        case MCPS_CONFIRMED:
        {
            readyToSend = true;
            AckTimeoutRetriesCounter = 1;
            AckTimeoutRetries = mcpsRequest->Req.Confirmed.NbTrials;

            macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
            fPort = mcpsRequest->Req.Confirmed.fPort;
            fBuffer = mcpsRequest->Req.Confirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Confirmed.fBufferSize;
            datarate = mcpsRequest->Req.Confirmed.Datarate;
            break;
        }
        case MCPS_PROPRIETARY:
        {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_PROPRIETARY;
            fBuffer = mcpsRequest->Req.Proprietary.fBuffer;
            fBufferSize = mcpsRequest->Req.Proprietary.fBufferSize;
            datarate = mcpsRequest->Req.Proprietary.Datarate;
            break;
        }
        default:
            break;
    }

    if( readyToSend == true )
    {
        if( AdrCtrlOn == false )
        {
            if( ValueInRange( datarate, LORAMAC_MIN_DATARATE, LORAMAC_MAX_DATARATE ) == true )
            {
                ChannelsDatarate = datarate;
            }
            else
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }
        }

        DEBUG(3, "start send\r\n");
        status = Send( &macHdr, fPort, fBuffer, fBufferSize );
        if( status == LORAMAC_STATUS_OK )
        {
            McpsConfirm.McpsRequest = mcpsRequest->Type;
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        else
        {
            NodeAckRequested = false;
        }
    }

    return status;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

void LoRaMacTestSetMic( uint16_t txPacketCounter )
{
    UpLinkCounter = txPacketCounter;
    IsUpLinkCounterFixed = true;
}

void LoRaMacTestSetDutyCycleOn( bool enable )
{
    DutyCycleOn = enable;
}
