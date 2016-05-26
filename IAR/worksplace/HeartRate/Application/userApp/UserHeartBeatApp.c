#include "includes.h"
/********************************************************************************
 * heart Beat driver interface 
 *
 *
 ********************************************************************************/
 
static PIN_Handle IICGpioPin;
static PIN_State pinGpioState;
// Pins that are actively used by the application
static PIN_Config IICPinTable[] =
{
    Board_POST | PIN_INPUT_EN 		| PIN_PULLDOWN  | PIN_DRVSTR_MAX,     
    Board_WAKE | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
 
    PIN_TERMINATE
};

#define  I2C_SLAVE_Address			0x44 

#define HEART_SELECT()               bspI2cSelect(BSP_I2C_INTERFACE_0,I2C_SLAVE_Address) 
#define HEART_DESELECT()             bspI2cDeselect()


typedef struct
{
	uint8_t registerPT;
	uint8_t highValue;
	uint8_t lowValue;
	
}SETCMDPARA_STR;

// 默认参数: 年纪、性别、体重、身高、活动方式 ....
const SETCMDPARA_STR setCommand[]=
{
    Age,    		 0x01, 0x9E, // 34岁
	Gender, 		 0x00, 0x00, // 女
	Weight, 		 0x01, 0xF4, // 50Kg
	Height, 		 0x00, 0xA8, // 168cm
	ActivityMode_Mi, 0x00, 0x01, // running
	ClearValues,	 0x00, 0x03, // clear all health assessments     
};	

//================== Declare  ========================
static ACK_STR getACK(void);
static void sendConmandSet(SETCMDPARA_STR *parameterBuf, uint8_t parameterNum);
static void sendConmandStart(void);
static void sendConmandStop(void);

//================== function ========================

static uint8_t heartBeatInterfaceInit(void)
{
	uint8_t temp = NOGO;

	IICGpioPin = PIN_open(&pinGpioState, IICPinTable);
	PIN_setOutputValue(IICGpioPin, Board_WAKE, 1);
	delay_ms(200);
	
	if (0 == PIN_getInputValue(Board_POST))
	{
		//初始化未完成
		uartWriteDebug("error", 5);
		delay_ms(120);
		
		if (0 == PIN_getInputValue(Board_POST))
		{
			uartWriteDebug("fail\n", 5);
			return 1;
		}
		else
		{
			uartWriteDebug("ok1\r\n", 5); 
			
		}
	}
	else
	{		
		uartWriteDebug("ok2\r\n", 5);
	}
	
	// 设置检测参数
	sendConmandSet((SETCMDPARA_STR *)setCommand, sizeof(setCommand)/sizeof(SETCMDPARA_STR));
	temp = getACK();
	if (ACK != temp)
	{
		uartWriteDebug("cmno", 4);
		return 1;
	}
	
// 启动 HR 检测
	sendConmandStart();	
	temp = getACK();
	if (ACK != temp)
	{
		uartWriteDebug("stno", 4);
		return 1;
	}	
	return 0;
}

/**************************************************************************
 *
 * get ACK from the PerformTek after sending cmd set、cmd start or cmd stop 
 *
 *************************************************************************/
static ACK_STR getACK(void)
{
	uint8_t wTemp[4]={0};
	
	if (HEART_SELECT())
	{
		bspI2cRead(wTemp, 4);
		HEART_DESELECT();
		return (ACK_STR)wTemp[3];
	}
    else
    {
        return (ACK_STR)0xff;
    } 
}

static void sendConmandStart(void)
{
	if (HEART_SELECT())
	{
		uint8_t wTemp[3]={0};
		wTemp[0] = 0x44;
		wTemp[1] = 0x01;		// 长度 1
		wTemp[2] = CMDSTART;
		bspI2cWrite(wTemp, 3);
		
		HEART_DESELECT();
	}
}

static void sendConmandStop(void)
{
	if (HEART_SELECT())
	{
		uint8_t wTemp[3]={0};
		wTemp[0] = 0x44;
		wTemp[1] = 0x01;		// 长度 1
		wTemp[2] = CMDSTOP;
		bspI2cWrite(wTemp, 3);
		
		HEART_DESELECT();
	}
}

static uint8_t getDataLen = 0;
static void sendConmandGet(uint8_t *parameterBuf, uint8_t parameterNum)
{
	

	uint8_t comDataTempBuf[40]={0};

	if (HEART_SELECT())
	{
		comDataTempBuf[0] = 0x44;
		comDataTempBuf[1] = parameterNum+1;		// 长度 1
		comDataTempBuf[2] = CMDGET;
		for(int i=0; i<parameterNum; i++)
		{
			comDataTempBuf[3+i] = parameterBuf[i];
		}
		bspI2cWrite(comDataTempBuf, comDataTempBuf[1]+2);
		
		// 记录要读取的参数个数 N = parameterNum*3;
		getDataLen = parameterNum*3;
		HEART_DESELECT();
	}
	
}

/********************************************************************
* FunctiongName : ReceiveConmandGetData
* Description   : Receive date
* Input         : Data buf
* Output        : None
* Return        : Data len
*********************************************************************/
static uint8_t ReceiveConmandGetData(uint8_t *dataBuf)
{
	
	if (0 == getDataLen)
	{
		return 0;
	}
	if (HEART_SELECT())
	{
		bspI2cRead(dataBuf, getDataLen+1+1+1); // 一个0x44 一个 byte count; 一个data command

		//uartWriteDebug("data=", 5);
		uartWriteDebug(dataBuf, getDataLen+1+1+1);
		//uartWriteDebug("\r\n", 2);

		HEART_DESELECT();
		return (getDataLen+1+1+1);
	}
	return 0;
	
}

/****************************************************************************
* FunctiongName : sendConmandSet
* Description   : send cmd that set register
* Input         : Register data; number of Register
* Output        : None
* Return        : None
*****************************************************************************/
static void sendConmandSet(SETCMDPARA_STR *parameterBuf, uint8_t parameterNum)
{
	

	uint8_t comDataTempBuf[40]={0};

	if (HEART_SELECT())
	{
		comDataTempBuf[0] = 0x44;
		comDataTempBuf[1] = parameterNum*3+1;		// 长度 1
		comDataTempBuf[2] = CMDSET;
		for(int i=0; i<parameterNum; i++)
		{
			comDataTempBuf[3+i*3] = parameterBuf->registerPT;
			comDataTempBuf[4+i*3] = parameterBuf->highValue;
			comDataTempBuf[5+i*3] = parameterBuf->lowValue;
            parameterBuf++;
		}
		bspI2cWrite(comDataTempBuf, comDataTempBuf[1]+2);
		
		HEART_DESELECT();
	}
	
}


//============ Task process =================================

#define HRTASKSTACKSIZE  	512

/***** Variable declarations *****/
Task_Struct HRTask;              /* not static so you can see in ROV */
static uint8_t HRTaskStack[HRTASKSTACKSIZE];

static SENSORPARA_STR sensorData;

// 要获取参数的寄存器
const uint8_t getRegister[]=
{
	Processing_State,
	SignalFlagAndQuality,
	Optical_Input_DC_Level,
	PostResults,
	
	GetActivityMode,
	
	HeartRate,
	HeartRateAvg,
	
	StepRate,
	Distance,
	TotalSteps,
	Speed,
	
	CALS,

};

/*
 * 心率检测任务
 */
static void HRTaskFunc(UArg arg0, UArg arg1)
{
	uint8_t registerData[100];
	
	heartBeatInterfaceInit();	

	while(1)
	{

		sendConmandGet((uint8_t *)getRegister, sizeof(getRegister));
		
		ReceiveConmandGetData(registerData);

		sensorData.stepRate   = (registerData[3+7*3+1]<<8)|(registerData[3+7*3+2]);
		sensorData.distance   = (registerData[3+8*3+1]<<8)|(registerData[3+8*3+2]);
		sensorData.totalSteps = (registerData[3+9*3+1]<<8)|(registerData[3+9*3+2]);
		sensorData.speed      = (registerData[3+10*3+1]<<8)|(registerData[3+10*3+2]);
		sensorData.cals       = (registerData[3+11*3+1]<<8)|(registerData[3+11*3+2]);

		if ((registerData[3+2*3+1]>0x30) || ((registerData[3+1*3+2]&0x01)==0))
		{
			uint8_t showBuf[20]={0};

			sensorData.heartRate = (registerData[3+5*3+1]<<8)|(registerData[3+5*3+2]);
			sensorData.heartRateAvg = (registerData[3+6*3+1]<<8)|(registerData[3+6*3+2]);
			
			sprintf((char *)showBuf, "HR: %3dbpm", sensorData.heartRate);
			OLED_ShowString(0, 16, showBuf);
		}
		else
		{
			OLED_ShowString(0, 16, "                 ");
		}
		
		delay_ms(arg0);
	}
}

void HR_TaskInit(void)
{
	Task_Params HRTaskParams;
	
	Task_Params_init(&HRTaskParams);
    HRTaskParams.stackSize = HRTASKSTACKSIZE;
    HRTaskParams.priority = 2;
    HRTaskParams.stack = &HRTaskStack;
    HRTaskParams.arg0 = (UInt)2000;

    Task_construct(&HRTask, HRTaskFunc, &HRTaskParams, NULL);
}

void HR_GetSensorData(SENSORPARA_STR *snrData)
{
	memcpy((char *)snrData, (char *)&sensorData, sizeof(SENSORPARA_STR));
}
