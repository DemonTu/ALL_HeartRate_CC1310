#include "includes.h"

#include "EasyLink.h"
#include "UserRfProtocol.h"


//========================================================================

#define RF_TASK_STACK_SIZE    1024
#define RF_TASK_PRIORITY      2

Task_Struct rfTask;    /* not static so you can see in ROV */
static uint8_t rfTaskStack[RF_TASK_STACK_SIZE];

static Semaphore_Handle rfRxDoneSem;
static Semaphore_Handle rfTxDoneSem;

#ifdef INCLUDE_RFSEND_MALLOC
//======================== Queue =================================================
// Queue object used for app messages
Queue_Struct rfSendMsg;
Queue_Handle rfSendQueue;

#define RFPAYLOAD_LENGTH      30


typedef struct _queueRec_ 
{
  Queue_Elem _elem;          // queue element
  uint8_t *pData;            // pointer to app data
} queueRec_t;

typedef struct  
{
  uint8_t len;          // queue element
  uint8_t *Data;            // pointer to app data
} SendQueue;

/*********************************************************************
 * @fn      RF_SendConstructQueue
 *
 * @brief   Initialize an RTOS queue to hold messages to be processed.
 *
 * @param   pQueue - pointer to queue instance structure.
 *
 * @return  A queue handle.
 */
Queue_Handle RF_SendConstructQueue(Queue_Struct *pQueue)
{
  // Construct a Queue instance.
  Queue_construct(pQueue, NULL);
  
  return Queue_handle(pQueue);
}

/*********************************************************************
 * @fn      RF_SendEnqueueMsg
 *
 * @brief   Creates a queue node and puts the node in RTOS queue.
 *
 * @param   msgQueue - queue handle.

 * @param   pMsg - pointer to message to be queued
 *
 * @return  TRUE if message was queued, FALSE otherwise.
 */
uint8_t RF_SendEnqueueMsg(Queue_Handle msgQueue, uint8_t *pMsg)
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
 * @fn      RF_SendDequeueMsg
 *
 * @brief   Dequeues the message from the RTOS queue.
 *
 * @param   msgQueue - queue handle.
 *
 * @return  pointer to dequeued message, NULL otherwise.
 */
uint8_t *RF_SendDequeueMsg(Queue_Handle msgQueue)
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

#else
#define RF_BURST_SIZE         10
#define RFPAYLOAD_LENGTH      30

#define SENDQUEUESIZE         5

#define RBUF_NEXT_PT(dat, offset, size)     (((dat) + (offset)) % (size))

typedef struct  
{
  uint8_t len;          // queue element
  uint8_t dat[RFPAYLOAD_LENGTH];            // pointer to app data
} DATASEND_STR;

typedef struct  
{
  uint8_t head;          
  uint8_t tail;          
} QUEUE_T;

static QUEUE_T queue;
static DATASEND_STR sendQueue[SENDQUEUESIZE];
#endif
static uint8_t rfRxFlag = 0;

static void RF_SendData(uint8_t *dat, uint8_t len);

/*
 * 发送后的状态回调函数
 */
void rfTxDoneCb(EasyLink_Status status)
{
    if (status == EasyLink_Status_Success)
    {
        /* Toggle LED1 to indicate TX */
		bspUartWrite("TX", 2);
    }
    else if(status == EasyLink_Status_Aborted)
    {
        /* Toggle LED2 to indicate command aborted */
		bspUartWrite("aborted", 7);
        
    }
    else
    {
        /* Toggle LED1 and LED2 to indicate error */
		bspUartWrite("error", 5);

    }

    Semaphore_post(rfTxDoneSem);
}


void rfRxDoneCb(EasyLink_RxPacket * rxPacket, EasyLink_Status status)
{
    if (status == EasyLink_Status_Success)
    {
        /* Toggle LED2 to indicate RX */
		bspUartWrite("RX=", 3);
        //bspUartWrite(&rxPacket->payload[0], rxPacket->len);
		#if 0
		{
			SYS_stEvt_t *tempMsg;

			if (tempMsg = malloc(sizeof(SYS_stEvt_t)))
			{
				tempMsg->sysEvent = EVENT_RF_RECV;
				tempMsg->len = rxPacket->len;
				tempMsg->eData = rxPacket->payload;
				
				systemEnqueueMsg(systemQueue, (uint8_t *)tempMsg);
			}
		}
		#endif
		systemUserEnqueue(EVENT_RF_RECV, rxPacket->len, rxPacket->payload);
		//RfDataProcess(&rxPacket->payload[0], rxPacket->len);
    }
    else if(status == EasyLink_Status_Aborted)
    {
        /* Toggle LED1 to indicate command aborted */

    }
    else
    {
        /* Toggle LED1 and LED2 to indicate error */

    }

    Semaphore_post(rfRxDoneSem);
}

static void rfTaskFunction(UArg arg0, UArg arg1)
{
	uint8_t addrFilter = 0xaa;
	
    /* Create a semaphore for Async*/
    Semaphore_Params params;
    Error_Block eb;

    /* Init params */
    Semaphore_Params_init(&params);
    Error_init(&eb);

    /* Create semaphore instance */
    rfRxDoneSem = Semaphore_create(0, &params, &eb);
	rfTxDoneSem = Semaphore_create(0, &params, &eb);
#ifdef INCLUDE_RFSEND_MALLOC
	rfSendQueue = RF_SendConstructQueue(&rfSendMsg);
#endif
    EasyLink_init(EasyLink_Phy_Custom);
    EasyLink_setFrequency(433000000);
	/* Set output power to 12dBm */
	EasyLink_setRfPwr(12);
	
    EasyLink_enableRxAddrFilter(&addrFilter, 1, 1);


    while(1) 
	{
		#ifdef INCLUDE_RFSEND_MALLOC
		while(!Queue_empty(rfSendQueue))
		{
			SendQueue *sendBuf = (SendQueue *)RF_SendDequeueMsg(rfSendQueue);
			if (sendBuf)
			{
				RF_SendData(sendBuf->Data, sendBuf->len);
				free(sendBuf);
			}
		}
		#else
		while(queue.head != queue.tail)
		{
			
			//uartWriteDebug(sendQueue[queue.tail].dat, sendQueue[queue.tail].len);
			
			RF_SendData(sendQueue[queue.tail].dat, sendQueue[queue.tail].len);
			
			queue.tail = RBUF_NEXT_PT(queue.tail, 1, SENDQUEUESIZE);
		}
		#endif
		//delay_ms(100);
		EasyLink_receiveAsync(rfRxDoneCb, 0);
		rfRxFlag = 1;
		Semaphore_pend(rfRxDoneSem, BIOS_WAIT_FOREVER);
    }
}


void RF_TaskInit(void)
{
	Task_Params rfTaskParams;
	
	Task_Params_init(&rfTaskParams);
    rfTaskParams.stackSize = RF_TASK_STACK_SIZE;
    rfTaskParams.priority = RF_TASK_PRIORITY;
    rfTaskParams.stack = &rfTaskStack;
    rfTaskParams.arg0 = (UInt)BIOS_WAIT_FOREVER;

    Task_construct(&rfTask, rfTaskFunction, &rfTaskParams, NULL);

}

/*
 * 无线底层发送
 */
static void RF_SendData(uint8_t *dat, uint8_t len)
{
	EasyLink_TxPacket txPacket = {0};
	uint8_t i;
	
	for (i=0; i<len; i++)
	{
	  txPacket.payload[i] = dat[i];
	}

	txPacket.len = len;
	txPacket.dstAddr[0] = 0xaa;

	/* Set Tx absolute time 0 for send immediate */
	txPacket.absTime = 0;


	EasyLink_transmitAsync(&txPacket, rfTxDoneCb);
	/* Wait 300ms for Tx to complete */
	if(Semaphore_pend(rfTxDoneSem, (500000 / Clock_tickPeriod)) == FALSE)
	{
		bspUartWrite("timou", 5);
		/* TX timed out, abort */
		if(EasyLink_abort() == EasyLink_Status_Success)
		{
			bspUartWrite("abort", 5);
			/*
			 * Abort will cause the rfTxDoneCb to be called, and the txDoneSem ti
			 * Be released. So we must consume the txDoneSem
			 * */
		   //Semaphore_pend(rfTxDoneSem, BIOS_WAIT_FOREVER);
		}
	}
}


/*********************************************************************
* FunctionName : RF_Send
* Description  : 无线数据发送(应用层)
* Input        : 要发送的数据 dat; 数据长度 len 
* Output       : None
* Return       : None
**********************************************************************/
void RF_Send(uint8_t *dat, uint8_t len)
{
#ifdef INCLUDE_RFSEND_MALLOC
	SendQueue *tempMsg;
	if (tempMsg = malloc(sizeof(SendQueue)))
	{
		tempMsg->len = len;
		tempMsg->Data = dat;
		RF_SendEnqueueMsg(rfSendQueue, (uint8_t *)tempMsg);
	}
#else
	memcpy((char *)&sendQueue[queue.head].dat, dat, len);
	sendQueue[queue.head].len = len;
	queue.head = RBUF_NEXT_PT(queue.head, 1, SENDQUEUESIZE);
#endif
	if (1 == rfRxFlag) // 处在接收模式，切换到发送模式
	{
		rfRxFlag = 0;
		EasyLink_abort();
		Semaphore_post(rfRxDoneSem);
	}
	
}

