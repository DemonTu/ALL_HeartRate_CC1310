#include "includes.h"
#include "OLED_Driver.h"
#include "scif.h"


/***** Defines *****/
#define POWER_TASK_STACK_SIZE 512
#define POWER_TASK_PRIORITY   2

/***** Variable declarations *****/
static Task_Params powerTaskParams;
Task_Struct powerTask;    /* not static so you can see in ROV */

static uint8_t powerTaskStack[POWER_TASK_STACK_SIZE];

static Clock_Struct periodicClock_10ms;
static Semaphore_Struct mutexPower;

/*
 * batter manage
 */
/* Pin driver handle */
static PIN_Handle ADCPinHandle;
static PIN_State ADCPinState;

static uint16_t chargeAvg;
static uint8_t chargingFlag = 0;

/*
 * Application LED pin configuration table:
 *	 - All LEDs board LEDs are off.
 */
PIN_Config ADCpinTable[] =
{
	Board_STAT    | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_DIS,
	Board_ADC_BAT | PIN_INPUT_DIS| PIN_GPIO_OUTPUT_DIS,	
	PIN_TERMINATE
};


static void batteryDetectionInit(void)
{
	// ADC    I/O
	// charge I/O
	ADCPinHandle = PIN_open(&ADCPinState, ADCpinTable);
}


static void powerAdcProcess(void) // 计算每个循环的AD平均值
{
	static uint8_t onlyoneFlag = 0;
	uint16_t chargeSum=0;
	uint16_t chargeMax=0;
	uint16_t chargeMin=0;
	uint8_t i;
	if (scifTaskData.adcDataLogger.output.head == 0)
	{
		if (0 == onlyoneFlag)
		{
			/* 读取ADC数据队列完成 计算平均值*/
			chargeMax = scifTaskData.adcDataLogger.output.pSamples[0];
			if (chargeMax>scifTaskData.adcDataLogger.output.pSamples[1])
			{
				chargeMin = scifTaskData.adcDataLogger.output.pSamples[1]; 
			}
			else
			{
				chargeMax = scifTaskData.adcDataLogger.output.pSamples[1];
				chargeMin = scifTaskData.adcDataLogger.output.pSamples[0]; 
			}
			
			for (i=2; i<(SCIF_ADC_DATA_LOGGER_BUFFER_SIZE); i++)
			{
				if (scifTaskData.adcDataLogger.output.pSamples[i] > chargeMax)
				{
					chargeSum += chargeMax;
					chargeMax = scifTaskData.adcDataLogger.output.pSamples[i];
				}
				else if (scifTaskData.adcDataLogger.output.pSamples[i] < chargeMin)
				{
					chargeSum += chargeMin;
					chargeMin = scifTaskData.adcDataLogger.output.pSamples[i];
				}
				else
				{
					chargeSum += scifTaskData.adcDataLogger.output.pSamples[i];
				}
				
			}
			uartWriteDebug((uint8_t *)scifTaskData.adcDataLogger.output.pSamples, SCIF_ADC_DATA_LOGGER_BUFFER_SIZE*2);
			chargeAvg = chargeSum/(SCIF_ADC_DATA_LOGGER_BUFFER_SIZE-2);
			uartWriteDebug("AD=", 3);
			uartWriteDebug((uint8_t *)&chargeAvg, 2);
		}
		onlyoneFlag = 1;
	}
	else
	{
		onlyoneFlag = 0;
	}
}

// Currently, we assume that 1/3
  //		  Max Vol : 4.2V  (1.4V)
  //		  Min Vol : 2.75V  (0.92V)
  // 
  //  4.38	  : 4095
  //  4.30	  : 4020
  //  4.20	  : 3926
  //  2.70	  : 2524
  //  1.40	  : 1309
  //  0.92    : 860
  //uint16_t vol = ( - 860) * 100 / 449;
  
/*
 * 读取电池电量
 */  
uint16_t powerGetChargeValue(void)
{
	uint16_t charge;
	//powerAdcProcess();
	uartWriteDebug((uint8_t *)&scifTaskData.adcDataLogger.output.head, 2);
	if (chargeAvg < 860)
	{
		return 0;
	}
	else
	{
		charge = (chargeAvg - 860) * 100 / 449;
		uartWriteDebug((uint8_t *)&charge, 2);
		if (100 < charge)
		{
			charge = 100;
		}
	}
	return charge;
}

/*
 * 充放电状态判断
 */
uint8_t isChargePowerUp(void)
{
	return chargingFlag;
}

/*******************************************************************************
 * @fn      userAppShowCharge
 *
 * @brief   电池图标和百分比的显示
 *
 * @param   void
 *
 * @return  0 有电，1 没电
 */
uint8_t powerShowChargePercent(void)
{
	uint8_t smbBuf[4];
	uint8_t charge;
	uint8_t chargeState = 6;
	uint8_t bmpMov = 0;
	
	charge = powerGetChargeValue();
	
	OLED_ShowString(80,0, "       ");	// 清电池显示区域
	
	if (charge >100)
	{
		return 2;
	}
	if (100 == charge)
	{
		uartWriteDebug("T", 1);
		bmpMov = 8;
		chargeState = 0;
	}
	else if (charge > 90)
	{
		chargeState = 0;
	}
	else if(charge > 76)
	{
		chargeState = 1;
	}
	else if(charge > 62)
	{
		chargeState = 2;
	}
	else if(charge > 48)
	{
		chargeState = 3;
	}
	else if(charge > 34)
	{
		chargeState = 4;
	}
	else if(charge > 10)
	{
		chargeState = 5;
	}
	else
	{
		sprintf((char *)smbBuf, "%02d%", charge);
		OLED_ShowString(104,0, smbBuf);	
		return 1;
	}
	
	sprintf((char *)smbBuf, "%02d%", charge);
	if (0 == isChargePowerUp())
	{
		
		OLED_showBatteryBmp(0, 88-bmpMov, chargeState);

	}
	else
	{
		if (charge == 100)
		{
			
			uartWriteDebug("I", 1);
			OLED_showBatteryBmp(0, 88-bmpMov, chargeState);
			//chargedLedState();
		}
		else
		{
			uartWriteDebug("O", 1);
			OLED_showBatteryBmp(0, 88-bmpMov, 7);	// 显示充电图标
		}
		
	}
	OLED_ShowString(104-bmpMov, 0, smbBuf);	
	return 0;
}


/*
 * switch control
 */
#define KEYNUM		2

/* Pin driver handle */
static PIN_Handle swPinHandle;
static PIN_State swPinState;

/*
 * Application LED pin configuration table:
 *	 - All LEDs board LEDs are off.
 */
const PIN_Config swpinTable[] =
{
	Board_ON_OFF | PIN_INPUT_EN       | PIN_PULLUP   | PIN_DRVSTR_MAX,
	Board_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH| PIN_PUSHPULL    | PIN_DRVSTR_MAX,	
	Board_SW2    | PIN_INPUT_EN       | PIN_PULLUP   | PIN_DRVSTR_MAX,	
	PIN_TERMINATE
};

const SYSTEM_EVENT keyEvt[KEYNUM*3]=
{
	EVENT_POWERKEY_DOWN,
	EVENT_POWERKEY_LONG,  
	EVENT_POWERKEY_UP,

	EVENT_MENUKEY_DOWN,
	EVENT_MENUKEY_LONG,  
	EVENT_MENUKEY_UP
};

const PIN_Id pinID[KEYNUM]=
{ 
	Board_ON_OFF,
	Board_SW2
};

static void switchControlInit(void)
{
	swPinHandle = PIN_open(&swPinState, swpinTable);
}



/*
 * 按键扫描
 */
void powerKeyScan(void)
{
	static uint8_t shortKeyFlag[KEYNUM] = {0};
	static uint8_t keyValue[KEYNUM] = {0};
	static uint16_t keyLongCnt[KEYNUM] = {0};
	int i;
	
	for (i=0; i<KEYNUM; i++)
	{
		keyValue[i] <<= 1;
		if (0 == PIN_getInputValue(pinID[i]))
		{
			keyValue[i] |= 1; 
		}

		switch(keyValue[i])
		{
			case 0x0f:
				shortKeyFlag[i] = 1;
				//systemUserEnqueue(EVENT_KEY_DOWN, 0, NULL);
				uartWriteDebug("keydown", 7);
				// 短按
				break;
			case 0xff:
				// 长按 第一次大概80ms， 后面是每次30ms
				if (++keyLongCnt[i] == 100)
				{
					systemUserEnqueue(keyEvt[i*3+1], 0, NULL);
					uartWriteDebug("keylong", 7);
				}
				else if (keyLongCnt[i] > 100)
				{
					keyLongCnt[i] = 256;
					// 长按事件已产生
				}
				else if (keyLongCnt[i] > 2)
				{
					shortKeyFlag[i] = 0;	// 已经是属于长按的范畴了
				}
				break;
			case 0xf0:
				keyLongCnt[i] = 0;
				if (1 == shortKeyFlag[i])
				{
					shortKeyFlag[i] = 0;
					systemUserEnqueue(keyEvt[i*3], 0, NULL);
				}
				systemUserEnqueue(keyEvt[i*3+2], 0, NULL);
				
				uartWriteDebug("keyup", 5);
				// 放开
				break;
			default:
				//tempMsg->GPIOName = KEY_NAME_NONE;
				break;
		}
	}
}

/*
 * open system power
 */
void powerOn(void)
{
	PIN_setOutputValue(swPinHandle, Board_POWER, 1);
}

/*
 * close system power
 */
void powerOff(void)
{
	PIN_setOutputValue(swPinHandle, Board_POWER, 0);
}

#if 1
//===============================================================================================


static void powerManegeInit(void)
{
	batteryDetectionInit();		// ADC I/O 初始化
	switchControlInit();        // power key 初始化
}

/*  ======== powerClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void powerClockFunc(uintptr_t arg)
{
	Semaphore_post(Semaphore_handle(&mutexPower));
}

static void scCtrlReadyCallback(void)
{
	//uartWriteDebug("re", 2);
}

static void powerTaskFunction(UArg arg0, UArg arg1)
{
	Clock_Params clockParams;
	Semaphore_Params semParamsMutex;
	
	powerManegeInit();
	powerOff();			// 关闭系统电源
	
	/* ADC init*/
	scifOsalInit();
	scifOsalRegisterCtrlReadyCallback(scCtrlReadyCallback);
	//scifOsalRegisterTaskAlertCallback(scCtrlReadyCallback);
	scifInit(&scifDriverSetup);

	// Set the Sensor Controller task tick 
	// First start : 3 second; Interval to 500 ms.
	// It should be half of BAT_POLLING_PERIOD_MS.
	scifStartRtcTicks(0x00008000, 0x00008000);
	//scifStartRtcTicksNow
	scifStartTasksNbl(1 << (SCIF_ADC_DATA_LOGGER_TASK_ID));

	delay_ms(1000);
	chargeAvg = scifTaskData.adcDataLogger.output.pSamples[0];
		
	// Create protection semaphore
	Semaphore_Params_init(&semParamsMutex);
	semParamsMutex.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&mutexPower, 1, &semParamsMutex);
	
	Clock_Params_init(&clockParams);
    clockParams.period = 0;
    clockParams.startFlag = FALSE;
    clockParams.arg = 5;			// event 

	// Create one-shot clocks for internal periodic events.
 	Clock_construct(&periodicClock_10ms, &powerClockFunc, arg0, &clockParams); // 30ms
 	
	Clock_start(Clock_handle(&periodicClock_10ms));
	
	while(1)
	{
		Semaphore_pend(Semaphore_handle(&mutexPower), BIOS_WAIT_FOREVER);
        Clock_start(Clock_handle(&periodicClock_10ms));

        powerKeyScan();
		powerAdcProcess();
		//uartWriteDebug("ADC\r\n", 5);
	}
}

/*
 * Power Task Create 
 */
void powerManegeTaskInit(void)
{
	Task_Params_init(&powerTaskParams);
    powerTaskParams.stackSize = POWER_TASK_STACK_SIZE;
    powerTaskParams.priority = POWER_TASK_PRIORITY;
    powerTaskParams.stack = &powerTaskStack;
    powerTaskParams.arg0 = (UInt)3000;

    Task_construct(&powerTask, powerTaskFunction, &powerTaskParams, NULL);
}

#endif

