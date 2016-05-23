#include "includes.h"
#include "OLED_Driver.h"

/*
 * batter manage
 */
/* Pin driver handle */
static PIN_Handle ADCPinHandle;
static PIN_State ADCPinState;

/*
 * Application LED pin configuration table:
 *	 - All LEDs board LEDs are off.
 */
PIN_Config ADCpinTable[] =
{
	Board_STAT    | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_DIS,
	Board_ADC_BAT | PIN_INPUT_EN | PIN_GPIO_OUTPUT_DIS,	
	PIN_TERMINATE
};


static void batteryDetectionInit(void)
{
	// ADC    I/O
	// charge I/O
	//ADCPinHandle = PIN_open(&ADCPinState, ADCpinTable);
}

/*
 * switch control
 */

/* Pin driver handle */
static PIN_Handle swPinHandle;
static PIN_State swPinState;

/*
 * Application LED pin configuration table:
 *	 - All LEDs board LEDs are off.
 */
PIN_Config swpinTable[] =
{
	Board_ON_OFF | PIN_INPUT_EN       | PIN_PULLUP   | PIN_DRVSTR_MAX,
	Board_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH| PIN_PUSHPULL | PIN_DRVSTR_MAX,	
	PIN_TERMINATE
};

static void switchControlInit(void)
{
	swPinHandle = PIN_open(&swPinState, swpinTable);
}

/*
 * 按键扫描
 */

void powerKeyScan10ms(void)
{
	static uint8_t keyValue = 0;

	keyValue <<= 1;
	if (0 == PIN_getInputValue(Board_ON_OFF))
	{
		keyValue |= 1; 
	}
	switch(keyValue)
	{
		case 0x0f:  // 按下
			uartWriteDebug("down", 4);
			break;
		case 0xff:  // 长按
			uartWriteDebug("long", 4);
			break;
		case 0xf0:	// 放开
			uartWriteDebug("up\r\n", 4);
			break;
		default:
			break;
	}
}

//void powerManegeInit(void)
//{
//	batteryDetectionInit();
//	switchControlInit();
//}

#if 1
//===============================================================================================
/***** Defines *****/
#define POWER_TASK_STACK_SIZE 512
#define POWER_TASK_PRIORITY   2

/***** Variable declarations *****/
static Task_Params powerTaskParams;
Task_Struct powerTask;    /* not static so you can see in ROV */

static uint8_t powerTaskStack[POWER_TASK_STACK_SIZE];

static void powerManegeInit(void)
{
	batteryDetectionInit();
	switchControlInit();
}

static Clock_Struct periodicClock_10ms;
static Semaphore_Struct mutexPower;

/*  ======== emptyClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void powerClockFunc(uintptr_t arg)
{
	Semaphore_post(Semaphore_handle(&mutexPower));
}

static void powerTaskFunction(UArg arg0, UArg arg1)
{
	Clock_Params clockParams;
	Semaphore_Params semParamsMutex;
	
	powerManegeInit();

	// Create protection semaphore
	Semaphore_Params_init(&semParamsMutex);
	semParamsMutex.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&mutexPower, 1, &semParamsMutex);
	
	Clock_Params_init(&clockParams);
    clockParams.period = 0;
    clockParams.startFlag = FALSE;
    clockParams.arg = 5;
	// Create one-shot clocks for internal periodic events.
	Clock_construct(&periodicClock_10ms, &powerClockFunc,
	                  100000, &clockParams);
	Clock_stop(Clock_handle(&periodicClock_10ms));
	
	while(1)
	{
		Semaphore_pend(Semaphore_handle(&mutexPower), BIOS_WAIT_FOREVER);
        Clock_start(Clock_handle(&periodicClock_10ms));
		
        powerKeyScan10ms();
		uartWriteDebug("ADC\r\n", 5);
		//Task_sleep(3000);
	}
}

void powerManegeTaskInit(void)
{
	Task_Params_init(&powerTaskParams);
    powerTaskParams.stackSize = POWER_TASK_STACK_SIZE;
    powerTaskParams.priority = POWER_TASK_PRIORITY;
    powerTaskParams.stack = &powerTaskStack;
    powerTaskParams.arg0 = (UInt)1000000;

    Task_construct(&powerTask, powerTaskFunction, &powerTaskParams, NULL);
}
#endif

