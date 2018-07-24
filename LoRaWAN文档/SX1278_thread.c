static void Thread_entry_RFMgmt(void * parameters)
{

	static	SensorUploadPkt_t SensorUploadPkt;
	rt_uint8_t 								len,command;
	rt_uint8_t			 					msg;
	rt_uint16_t								DispRefresh_timevalue = RT_TICK_PER_SECOND * 5;
	rt_uint16_t								AvoidZombie_timevalue = RT_TICK_PER_SECOND * RF_AVOID_ZOMBIE_TIME;
	DoubleByte_t							SensorSn;
	rt_thread_delay(RT_TICK_PER_SECOND);
	LoRaRDO.pRadioDriver = RadioDriverInit();
	SX1276_Open();


	rt_timer_init(&Timer_AvoidSI4463Zombie, "TimerAvoidZombie" , CallBack_AvoidSI4463Zombie , \
																			RT_NULL , AvoidZombie_timevalue , \
	                                    RT_TIMER_FLAG_PERIODIC);
	#if defined(USE_AVOID_ZOMBIE)
		rt_timer_start(&Timer_AvoidSI4463Zombie);
	#endif
	rt_timer_init(&Timer_DispNodeInfoRefresh , "timerDispAckRefresh" , CallBack_DispNodeInfoRefresh , \
																			RT_NULL , DispRefresh_timevalue , RT_TIMER_FLAG_ONE_SHOT);
	//this timer will be started according program condition.

	LoRaRDO.pRadioDriver->Reset();
    LoRaRDO.pRadioDriver->Init();
	LoRaRDO.pRadioDriver->ModeReq(RFLR_OPMODE_RECEIVER);
    LoRaRDO.CurrentStatus = STATUS_LORA_RXCONTINUOUS;
	Disp_NodeInfo( SensorSn, 1 , FALSE);
	while(1)
	{
	 if(rt_mq_recv(&MsgQ_RFMgmt , &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
	  {
		switch(msg)
		{
			//the SI4463 interrupt process.
			case MSG_RF_INTERRUPT:
			{
				#if defined(USE_AVOID_ZOMBIE)
					rt_timer_stop(&Timer_AvoidSI4463Zombie);
					rt_timer_control(&Timer_AvoidSI4463Zombie , RT_TIMER_CTRL_SET_TIME , &AvoidZombie_timevalue);
					rt_timer_start(&Timer_AvoidSI4463Zombie);
				#endif
				//the package is transmitted finish.
				switch(LoRaRDO.CurrentStatus)
				{
					case STATUS_LORA_TX:
					{
						LoRaRDO.pRadioDriver->IrqClear(RFLR_IRQFLAGS_TXDONE);
						LoRaRDO.CurrentStatus = STATUS_LORA_RXCONTINUOUS;
						LoRaRDO.pRadioDriver->ModeReq( RFLR_OPMODE_RECEIVER);
					}
					break;
				//the cad detecting done.
					case STATUS_LORA_CAD:
					{
						LoRaRDO.pRadioDriver->IrqClear(RFLR_IRQFLAGS_CADDONE);
						LoRaRDO.CurrentStatus = STATUS_LORA_RXCONTINUOUS;
						LoRaRDO.pRadioDriver->ModeReq( RFLR_OPMODE_RECEIVER);
					}
					break;
				 //received a package.
				  case STATUS_LORA_RXCONTINUOUS:
				  case STATUS_LORA_RXSINGLE:
					{
						LoRaRDO.pRadioDriver->IrqClear(RFLR_IRQFLAGS_RXDONE);
						LoRaRDO.pRadioDriver->ModeReq(RFLR_OPMODE_STANDBY);
						//if RecPkt with correct payload CRC.
						if(LoRaRDO.pRadioDriver->RecPkt(&SensorUploadPkt.DeviceAddrHighByte , &len))
							{

								command  					= SensorUploadPkt.Cmd;  //the two mask bytes and one length byte is discarded.
								SensorSn.HighByte = SensorUploadPkt.Data.SensorSnHighByte;
								SensorSn.LowByte  = SensorUploadPkt.Data.SensorSnLowByte;
								switch(command)
								{
									//this message from sensor is the join package ,have no data to be processed.
									case CMD_SENSOR_PWRON_REPORT_ACK :
									{
										MakeSensorConfirmPkt(SensorSn);
										LoRaRDO.CurrentStatus = STATUS_LORA_TX;
										LoRaRDO.pRadioDriver->TransPkt(&SensorConfirmPkt.PktHead.SlaverAddr.HighByte, sizeof(SensorDownloadPkt_t));
										Disp_NodeInfo(SensorSn , 0 ,TRUE);
										rt_timer_stop(&Timer_DispNodeInfoRefresh);
										rt_timer_control(&Timer_DispNodeInfoRefresh , RT_TIMER_CTRL_SET_TIME , &DispRefresh_timevalue);
										rt_timer_start(&Timer_DispNodeInfoRefresh);
									}
								break;
								//this message from sensor is the oridinary data package.
								case CMD_SENSOR_DATA_REPORT_ACK:
									{
										MakeSensorConfirmPkt(SensorSn);
										LoRaRDO.CurrentStatus = STATUS_LORA_TX;
										LoRaRDO.pRadioDriver->TransPkt(&SensorConfirmPkt.PktHead.SlaverAddr.HighByte , sizeof(SensorDownloadPkt_t));

										Disp_NodeInfo(SensorSn , 1 ,TRUE);
										rt_timer_stop(&Timer_DispNodeInfoRefresh);
										rt_timer_control(&Timer_DispNodeInfoRefresh , RT_TIMER_CTRL_SET_TIME , &DispRefresh_timevalue);
										rt_timer_start(&Timer_DispNodeInfoRefresh);
							//send the record to data management thread ,where the data will be stored.
										MakeSensorCellPkt(&SensorUploadPkt.Data);
										rt_mq_send(&MsgQ_SensorDataMgmt ,&SensorDataCellPkt.SampleTime.Year , sizeof(SensorDataCellPkt_t));
									}
								break;
								default:
								{
									LoRaRDO.CurrentStatus = STATUS_LORA_RXCONTINUOUS;
									LoRaRDO.pRadioDriver->ModeReq(RFLR_OPMODE_RECEIVER);
								}
								break;
							}
						}
					 else
					 {
						 LoRaRDO.CurrentStatus = STATUS_LORA_RXCONTINUOUS;
						 LoRaRDO.pRadioDriver->ModeReq( RFLR_OPMODE_RECEIVER);
					 }
					}
					break;
					default:break;
			}
		}
			break;
			//if the avoid zombie msg is received, re-arm SI4463 into RX mode.
			case MSG_RF_AVOID_ZOMBIE:
				{
					;
				}
			break;
			default:break;
		}
  }
 }
}
