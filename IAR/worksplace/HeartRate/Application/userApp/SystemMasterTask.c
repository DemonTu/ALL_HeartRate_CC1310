#include "includes.h"

#include "UserRfProtocol.h"

/*****************************************************************************************
* 本文档主要是创建一个系统的主线任务，包括了对时钟软定时的事件处理，还有对系统其他任务的
* 消息事件进行处理分析，由此对外提供一个消息队列。
* 文档由system Task、Click和Queue三部分组成的
* Demon
* 2016.06.01
***************************************************************************************/
static uint8_t systemPowerFlag;// 开关机状态标志

//======================== Queue =================================================
// Queue object used for app messages
Queue_Struct systemMsg;
Queue_Handle systemQueue;


typedef struct _queueRec_ 
{
  Queue_Elem _elem;          // queue element
  uint8_t *pData;            // pointer to app data
} queueRec_t;


/*********************************************************************
 * @fn      systemConstructQueue
 *
 * @brief   Initialize an RTOS queue to hold messages to be processed.
 *
 * @param   pQueue - pointer to queue instance structure.
 *
 * @return  A queue handle.
 */
Queue_Handle systemConstructQueue(Queue_Struct *pQueue)
{
  // Construct a Queue instance.
  Queue_construct(pQueue, NULL);
  
  return Queue_handle(pQueue);
}

/*********************************************************************
 * @fn      systemEnqueueMsg
 *
 * @brief   Creates a queue node and puts the node in RTOS queue.
 *
 * @param   msgQueue - queue handle.

 * @param   pMsg - pointer to message to be queued
 *
 * @return  TRUE if message was queued, FALSE otherwise.
 */
uint8_t systemEnqueueMsg(Queue_Handle msgQueue, uint8_t *pMsg)
{
  queueRec_t *pRec;
  
  // Allocated space for queue node.

  if (pRec = (queueRec_t *)malloc(sizeof(queueRec_t)))
  {
    pRec->pData = pMsg;
  
    Queue_enqueue(msgQueue, &pRec->_elem);
    
    return TRUE;
  }
  
  // Free the message.

  free(pMsg);
  return FALSE;
}

/*********************************************************************
 * @fn      systemDequeueMsg
 *
 * @brief   Dequeues the message from the RTOS queue.
 *
 * @param   msgQueue - queue handle.
 *
 * @return  pointer to dequeued message, NULL otherwise.
 */
uint8_t *systemDequeueMsg(Queue_Handle msgQueue)
{
  if (!Queue_empty(msgQueue))
  {
    queueRec_t *pRec = Queue_dequeue(msgQueue);
    uint8_t *pData = pRec->pData;
    
    // Free the queue node
    // Note:  this does not free space allocated by data within the node.

    free(pRec);
    
    return pData;
  }
  
  return NULL;
}

/*
 * 用户自定义数据封包
 */
void systemUserEnqueue(uint8_t evt, uint8_t datLen, uint8_t *dat)
{
	SYS_stEvt_t *tempMsg;
	if (tempMsg = malloc(sizeof(SYS_stEvt_t)))
	{
		tempMsg->sysEvent = evt;
		tempMsg->eData = dat;
		tempMsg->len = datLen;
		systemEnqueueMsg(systemQueue, (uint8_t *)tempMsg);
	}
}
// ==================== End Queue ================================================

// ==================== Clock ====================================================
static Clock_Struct periodicClock_1s;
static Clock_Struct periodicClock_100ms;
static Clock_Struct periodicClock_500ms;
static Clock_Struct periodicClock_SendTimeout;

static Semaphore_Struct mutexMaster;

#define TIME350MS_EVENT        0x0001
#define TIME100MS_EVENT        0x0002
#define TIME500MS_EVENT        0x0004
#define TIME1S_EVENT           0x0008

/*  ======== systemMasterClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void systemMasterClockFuncCB(uintptr_t arg)
{
	switch(arg)
	{	

		case TIME100MS_EVENT:
			systemUserEnqueue(EVENT_TIME_100MS, 0, NULL);
			break;
		case TIME500MS_EVENT:
			systemUserEnqueue(EVENT_TIME_500MS, 0, NULL);
			break;			
		case TIME1S_EVENT:
			systemUserEnqueue(EVENT_TIME_1S, 0, NULL);
			break;
#ifdef INCLUDE_RF_MASTER
		case TIME350MS_EVENT:
			systemUserEnqueue(EVENT_REQUES_OK, 0, NULL);
			break;
#endif
		default:
			break;

	}
	//Semaphore_post(Semaphore_handle(&mutexMaster));
}

/*
 * 系统定时时间设置 时间单位1ms
 */
static void systemClockCreate(Clock_Struct *clockHandle, uint32_t userTime, uintptr_t event)
{
	Clock_Params clockParams;
	
	Clock_Params_init(&clockParams);
    clockParams.period = 0;
    clockParams.startFlag = FALSE;
    clockParams.arg = event;
	// Create one-shot clocks for internal periodic events.
	Clock_construct(clockHandle, &systemMasterClockFuncCB,
	                  userTime*100, &clockParams);
}

//======================== End Clock =====================================

// ======================= System Task ===================================

/***** Variable declarations *****/
static Task_Params masterTaskParams;
Task_Struct masterTask;              /* not static so you can see in ROV */
static uint8_t masterTaskStack[1024];

#ifdef INCLUDE_RF_MASTER
#define DEVICEIDNUM			50
#endif

enum
{
	SYSTEMIDLE = 0,
	SYSTEMSYNC,
};
/*
 * 系统任务
 */
static void systemMasterTaskFunc(UArg arg0, UArg arg1)
{
	#ifdef INCLUDE_RF_MASTER
	uint16_t deviceIDBuf[DEVICEIDNUM];
	uint16_t devIDNum = 0;	
    uint16_t devCnt = 0;
	#else
	uint8_t systemShowStateFlag = HeartRate;	// range = sizeof(SENSORPARA_STR)/2 default: heart beat mode
	#endif
	uint8_t sysMode = SYSTEMIDLE;
	
	Semaphore_Params semParamsMutex;
    
    uartWriteDebug("system", 6);
	systemPowerFlag = 0;// 处于关机状态
	
    // Create protection semaphore
	Semaphore_Params_init(&semParamsMutex);
	semParamsMutex.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&mutexMaster, 1, &semParamsMutex);

	systemQueue = systemConstructQueue(&systemMsg);

	systemClockCreate(&periodicClock_100ms, 100, TIME100MS_EVENT);
//	systemClockCreate(&periodicClock_500ms, 500, TIME500MS_EVENT);
	systemClockCreate(&periodicClock_1s, 1000, TIME1S_EVENT);
#ifdef INCLUDE_RF_MASTER
	systemClockCreate(&periodicClock_SendTimeout, 350, TIME350MS_EVENT);
#endif
	while(1)
	{
	
		while (!Queue_empty(systemQueue))
		{
			SYS_stEvt_t *pMsg = (SYS_stEvt_t *)systemDequeueMsg(systemQueue);
			if (pMsg)
			{
				// Process message.
				switch(pMsg->sysEvent)
				{
					case EVENT_POWERKEY_DOWN:
						if (sysMode == SYSTEMSYNC)
						{
							rfDataPacking(CMDSYNC, sysPara.deviceID, NULL, 0);								
						}
						uartWriteDebug("KEYPOWER", 7);
						break;
					case EVENT_POWERKEY_LONG:
						if (0 == systemPowerFlag)
						{
							// long key to power on
							powerOn();
							systemParaInit();
						#ifdef INCLUDE_RF_MASTER
							// 读取已经配对了几个设备
							devIDNum = syncParaRead(deviceIDBuf);
                            devCnt = devIDNum;
							// 延时启动发送无线请求命令
							Clock_start(Clock_handle(&periodicClock_SendTimeout));
							
							OLED_ProcessTaskInit();
							RF_TaskInit();
						#else							
							OLED_ProcessTaskInit();
							delay_ms(100);
							HR_TaskInit();
							RF_TaskInit();
						#endif	
							systemPowerFlag = 1;
							
						    /* 启动软件定时器 */	
							Clock_start(Clock_handle(&periodicClock_100ms));
						//	Clock_start(Clock_handle(&periodicClock_500ms));
							Clock_start(Clock_handle(&periodicClock_1s));
						}
						else
						{
							// long key to power off
							systemPowerFlag = 0;
						#ifndef INCLUDE_RF_MASTER	
							HR_CloseSensor();
						#endif
							OLED_Clear();
							powerOff();
						}
						break;
					case EVENT_RF_RECV:
						RfDataProcess(pMsg->eData, pMsg->len);
						//uartWriteDebug(pMsg->eData, pMsg->len);
						break;
					case EVENT_UART_RECV:
						if (pMsg->len)
						{
							if (memcmp(pMsg->eData, "setID=", 6) == 0)
							{
								sysPara.deviceID = BigEndingBuf_To_U16(&pMsg->eData[6]);
								systemParaSave();
								bspUartWrite("set OK", 6);
							}
						}
						break;
				#ifndef INCLUDE_RF_MASTER			
					case EVENT_SENSOR_HIDE:
						if (1 == systemPowerFlag && sysMode==SYSTEMIDLE)
						{
							OLED_ShowString(0, 16, "                ");
						}
						break;
					case EVENT_SENSOR_SHOW:
						if (1 == systemPowerFlag && sysMode==SYSTEMIDLE)
						{
							uint8_t showBuf[20]={0};
							SENSORPARA_STR *sensorDataT;
							
							sensorDataT = (SENSORPARA_STR *)pMsg->eData;
							
							//OLED_ShowString(0, 16, "                ");
							switch(systemShowStateFlag)
							{
								case HeartRate:
									sprintf((char *)showBuf, " %3dbpm", sensorDataT->heartRate);
									break;
								case StepRate:
									sprintf((char *)showBuf, " %3dsteps/min", sensorDataT->stepRate);
									break;
								case Distance:
									sprintf((char *)showBuf, " %4dm", sensorDataT->distance);
									break;
								case TotalSteps:
									sprintf((char *)showBuf, " %3dsteps", sensorDataT->totalSteps);
									break;
								case Speed:
									sprintf((char *)showBuf, " %3dkm/h", sensorDataT->speed);
									break;
								case CALS:
									sprintf((char *)showBuf, " %3dkCal", sensorDataT->cals);
									break;
								case VO2:
									sprintf((char *)showBuf, " %3dml/kg*min", sensorDataT->sVO2);
									break;		
								default:
									break;
							}
							
							OLED_ShowString(0, 16, showBuf);
						}
						break;
					case EVENT_MENUKEY_DOWN:
						if (1 == systemPowerFlag && sysMode==SYSTEMIDLE)
						{
							static uint8_t i = 0;
							const uint8_t stateTbl[]={\
								HeartRate, StepRate, Distance,\
								TotalSteps, Speed, CALS, VO2\
								};

							if (++i >= sizeof(stateTbl))
							{
								i = 0;
							}
							systemShowStateFlag = stateTbl[i];
							OLED_ShowString(0, 0, "         ");
							OLED_ShowString(0, 16, "                ");
							switch(systemShowStateFlag)
							{
								case HeartRate:
									OLED_ShowString(0, 0, "HR");
									break;
								case StepRate:
									OLED_ShowString(0, 0, "StepRate");
									break;
								case Distance:
									OLED_ShowString(0, 0, "Distance");
									break;
								case TotalSteps:
									OLED_ShowString(0, 0, "AllSteps");
									break;
								case Speed:
									OLED_ShowString(0, 0, "Speed");
									break;
								case CALS:
									OLED_ShowString(0, 0, "CALS");
									break;
								case VO2:
									OLED_ShowString(0, 0, "VO2");
									break;		
								default:
									break;
							}
						}				
						break;
						
					case EVENT_MENUKEY_LONG:
						/* 长按菜单键，切换工作模式 */
						{
							static uint8_t flag = 0;
							
							flag = !flag; 
							if (flag)
							{
								sysMode = SYSTEMSYNC;
								OLED_ShowString(0, 0, "         ");
								OLED_ShowString(0, 16, "     SYNC       ");
							}
							else
							{
								sysMode = SYSTEMIDLE;
								systemUserEnqueue(EVENT_MENUKEY_DOWN, 0, NULL);
							}
						}
						break;
				#endif			
			//======================================================
			//=========== 系统定时器 ===============================
					case EVENT_TIME_100MS:		
						Clock_start(Clock_handle(&periodicClock_100ms));
						break;
					case EVENT_TIME_500MS:
						Clock_start(Clock_handle(&periodicClock_500ms));
						break;
					case EVENT_TIME_1S:
						if (1 == systemPowerFlag)
						{
							powerShowChargePercent();
						}
						Clock_start(Clock_handle(&periodicClock_1s));

						break;
			//============== end ==========================================			
			//=============================================================		
				/* 主设备 请求无线数据 */		
				#ifdef INCLUDE_RF_MASTER
					case EVENT_REQUES_OK:
						Clock_stop(Clock_handle(&periodicClock_SendTimeout));
					case EVRNT_SEND_NEXT:
						if (1 == systemPowerFlag)
						{
							// 读取已经配对的ID 
							if (devIDNum)
							{						
								rfDataPacking(CMDREQUEST, deviceIDBuf[devCnt-1], NULL, 0);
								Clock_start(Clock_handle(&periodicClock_SendTimeout));
								if (devCnt == 0)
								{
									devCnt = devIDNum;
								}
							}							
						}
						break;
				#endif	
			//=========== end =======================================			
					default:
						break;
				}
				free(pMsg);
			}
		}
		
		delay_ms(arg0);
	}

}

void systemMasterTaskInit(void)
{
	Task_Params_init(&masterTaskParams);
    masterTaskParams.stackSize = 1024;
    masterTaskParams.priority = 2;
    masterTaskParams.stack = &masterTaskStack;
    masterTaskParams.arg0 = (UInt)1;			// task sleep 10ms

    Task_construct(&masterTask, systemMasterTaskFunc, &masterTaskParams, NULL);
}
// =============================End System Task ===================================
