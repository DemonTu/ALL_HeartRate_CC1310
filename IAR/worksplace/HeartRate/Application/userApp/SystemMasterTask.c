#include "includes.h"

/***** Variable declarations *****/
static Task_Params masterTaskParams;
Task_Struct masterTask;              /* not static so you can see in ROV */
static uint8_t masterTaskStack[1024];

static Clock_Struct periodicClock_10ms;
static Clock_Struct periodicClock_100ms;
static Clock_Struct periodicClock_500ms;
static Clock_Struct periodicClock_1s;

static Semaphore_Struct mutexMaster;

#define TIME10MS_EVENT         0x0001
#define TIME100MS_EVENT        0x0002
#define TIME500MS_EVENT        0x0004
#define TIME1S_EVENT           0x0008

static uint16_t systemEvent = 0;

/*  ======== systemMasterClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void systemMasterClockFuncCB(uintptr_t arg)
{
	systemEvent = arg;
	Semaphore_post(Semaphore_handle(&mutexMaster));
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

/*
 * 系统任务
 */
static void systemMasterTaskFunc(UArg arg0, UArg arg1)
{
	
	Semaphore_Params semParamsMutex;
    
   // OLED_Init();
	//OLED_ShowString(0, 0, "Heart Check");
	
    // Create protection semaphore
	Semaphore_Params_init(&semParamsMutex);
	semParamsMutex.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&mutexMaster, 1, &semParamsMutex);

//	systemClockCreate(&periodicClock_10ms, 10, TIME10MS_EVENT);
//	systemClockCreate(&periodicClock_100ms, 100, TIME100MS_EVENT);
//	systemClockCreate(&periodicClock_500ms, 500, TIME500MS_EVENT);
	systemClockCreate(&periodicClock_1s, 1000, TIME1S_EVENT);
	
//	Clock_stop(Clock_handle(&periodicClock_10ms));
//	Clock_stop(Clock_handle(&periodicClock_100ms));
//	Clock_stop(Clock_handle(&periodicClock_500ms));
	Clock_start(Clock_handle(&periodicClock_1s));
	
	while(1)
	{
		Semaphore_pend(Semaphore_handle(&mutexMaster), BIOS_WAIT_FOREVER);
#if 0
		if (systemEvent & TIME10MS_EVENT)
		{
			systemEvent &= ~TIME10MS_EVENT;
			Clock_start(Clock_handle(&periodicClock_10ms));	

			powerKeyScan10ms();
		}
		if (systemEvent & TIME100MS_EVENT)
		{
			systemEvent &= ~TIME100MS_EVENT;
			Clock_start(Clock_handle(&periodicClock_100ms));
		}
		if (systemEvent & TIME500MS_EVENT)
		{
			systemEvent &= ~TIME500MS_EVENT;
			Clock_start(Clock_handle(&periodicClock_500ms));
			
			OLED_Refresh_Gram();
		}
		
		if (systemEvent & TIME1S_EVENT)
		{
			systemEvent &= ~TIME1S_EVENT;
			Clock_start(Clock_handle(&periodicClock_1s));

			bspUartWrite("OLED", 4);
		}
    #endif    
    Clock_start(Clock_handle(&periodicClock_1s));    
    bspUartWrite("oled", 4);
 //   delay_ms(1000);
	}

}

void systemMasterTaskInit(void)
{
	Task_Params_init(&masterTaskParams);
    masterTaskParams.stackSize = 1024;
    masterTaskParams.priority = 2;
    masterTaskParams.stack = &masterTaskStack;
    masterTaskParams.arg0 = (UInt)1000000;

    Task_construct(&masterTask, systemMasterTaskFunc, &masterTaskParams, NULL);
}

