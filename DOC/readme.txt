
	
1：硬件IO接口图：

								stm32L072CBT6
SX1278						 _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
SPI     NSS	  --------	PA4 |					  		|
		SCK	  --------	PA5 |    				  		|
		MISO  --------	PA6 |					 		|
		MOSI  --------	PA7 |					  		|
						    |					  		|
EXti	DIO0  --------	PB1 |                    		|
		DIO1  --------	PB2 |					  		|
		DIO2  --------	PB10|					  		|
		DIO3  --------	PB11|					 		|
		DIO4  --------	NC	|					  		|
		DIO5  --------	NC	|					 		|
							|					  		|
CPIO	RESET --------	PB0 |					  		|
		LoRa_Power ---  PB12|					 		|
							|					 		|
							|					 		|
GPS	(UART2)					|					 		|	
		TX	  --------  PA2	|					  		|	
		RX	  --------  PA3	|					  		|	
GPS_Power_ON  --------  PB7	|					  		|									
							|					  		|
485	(UART5)					|					 		|	
		485_TX	------	PB3	|					  		|	
		485_RX	------	PB4	|					  		|	
		485_DE	------	PB5	|					  		|
		12V_ON	------  PA8	|					  		|	
							|					  		|
							|					 		|	
DEBUG(UART1)				|					  		|
		TX   ---------	PA9	|					  		|
		RX	 ---------  PA10|					  		|
							|					  		|
I2C							|					  		|
		I2C2_SDA ----- PB14	|					  		|
		I2C2_SCL ----- PB13	|					  		|
							|					  		|
电源管理使能  -------- PB9	|					  		|
							|					  		|
							|					  		|
							|					  		|
							|_ _ _ _ _ _ _ _ _ _ _ _ _ _|	



关于重发机制：
重发机制有两种模式：
1：采用CONFIRMED模式：默认协议重发模式

2：采用UNCONFIRMED模式，自定义重发机制，采用CSMA机制模式

需要考虑实际应用时要求响应重发机制速率

空中唤醒时间分两部分处理：

1：终端设备节点定时休眠唤醒，切换进CAD模式

2：网关端唤醒处理：>sleep_time + 2*cad_time，cad唤醒需要的前导码如下：

TS=2^SF/BW=2^9/125K=4.1ms/symb 
symb = TS/4.1ms


Radio.SetTxConfig-->SendFrameOnChannel-->ScheduleTx-->(LoRaMacSendFrameOnChannel-->LoRaMacSendOnChannel)

Radio.SetRxConfig-->RxWindowSetup

采用class c模式


LoRa+4G/2G：4G/2G间采用统一数据格式，LoRa间只提取实际有效数据，去除冗余




------------------------------------------------------------------------------------
框架：采用函数指针进行扩展使用，
      
	  1：测试侦听，唤醒机制：定时SLeep + 进入cadtime + 成功则执行唤醒后操作，否则再进循环操作流程
 	  2：添加休眠唤醒机制: 唤醒处于cad_mode--->rx2 = freq+30mhz,preamblelen可保持一致
	  网关下发preamblelen = symb = TS/2^SF/BW


注意: CLASS C模式下上行数据应答模式，不支持接收窗口超时退出机制，从协议框架完整性考虑，不重新加入新协议模式，直接采用A C模式切换使用，相应改动使用模式所需要频段优化模式

	  侦听应答机制：兼容上行应答模式，ACK处理机制，确保协议完整性

.\Objects\LoRaWAN_Pro.axf: Error: L6200E: Symbol test_t multiply defined (by lora-cad.o and usart.o).

.\Objects\SloveV1.2: Error: L6218E: Undefined symbol Channels (referred from main.o).


4012121212000100d1db4e3ac827

MHDR:0x40 (1B)  ///0X80：确认上行，0x40：非确认上行
DevAddr:0x12121212 (4B)
FCtrl: 0x00(1B)    ///0x20：网关ACK
FCnt:  0X01  (2B)帧计数
FOpts: 0x00    (0--15B)
FPort: 0xd1  (1B)
FRMPayload:0xdb
MIC: 0x4e3ac827(4B)

应答更改机制，可以解决上行确认数据，下行应答失败问题
LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

		if(LoRapp_Handle.MhdrAck)
		{
			fCtrl.Value = 0x20;
			fCtrl.Bits.Ack           = true;
		}
		else	
		{
		    fCtrl.Value = 0;
				fCtrl.Bits.Ack           = false;
		}			



Radio.SetTxConfig