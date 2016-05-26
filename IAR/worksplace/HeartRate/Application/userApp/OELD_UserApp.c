#include "includes.h"
#include "OLED_Driver.h"
/***** Variable declarations *****/
static Task_Params OLEDTaskParams;
Task_Struct proTask;    /* not static so you can see in ROV */
static uint8_t OLEDTaskStack[512];

static Clock_Struct periodicClock_500ms;
static Semaphore_Struct mutexOLED;

/*  ======== emptyClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void OLED_UpdataFunc(uintptr_t arg)
{
	Semaphore_post(Semaphore_handle(&mutexOLED));
}


static void TaskFunction(UArg arg0, UArg arg1)
{
	Clock_Params clockParams;
	Semaphore_Params semParamsMutex;
    
    OLED_Init();
	OLED_ShowString(0, 0, "Heart Check");
	
    // Create protection semaphore
	Semaphore_Params_init(&semParamsMutex);
	semParamsMutex.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&mutexOLED, 1, &semParamsMutex);
	
	Clock_Params_init(&clockParams);
    clockParams.period = 0;
    clockParams.startFlag = FALSE;
    clockParams.arg = 5;
	// Create one-shot clocks for internal periodic events.
	Clock_construct(&periodicClock_500ms, &OLED_UpdataFunc,
	                  arg0, &clockParams);             // 500ms
	Clock_stop(Clock_handle(&periodicClock_500ms));
	//OLED_Refresh_Gram();
	while(1)
	{
		Semaphore_pend(Semaphore_handle(&mutexOLED), BIOS_WAIT_FOREVER);
		Clock_start(Clock_handle(&periodicClock_500ms));
		//uartWriteDebug("OLED", 4);
		OLED_Refresh_Gram();
		//delay_ms(1000);
	}
}

void OLED_ProcessTaskInit(void)
{
	Task_Params_init(&OLEDTaskParams);
    OLEDTaskParams.stackSize = 512;
    OLEDTaskParams.priority = 2;
    OLEDTaskParams.stack = &OLEDTaskStack;
    OLEDTaskParams.arg0 = (UInt)50000;

    Task_construct(&proTask, TaskFunction, &OLEDTaskParams, NULL);
}
