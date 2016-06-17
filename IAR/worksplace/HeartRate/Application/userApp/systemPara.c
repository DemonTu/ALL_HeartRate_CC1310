#include "includes.h"
#include "internalFlash.h"
#ifdef INCLUDE_RF_MASTER
#define SYSTEMPARA_START_ADDRESS	(0x00010000-4096*2)	// 第56K
#define SYNCPARA_START_ADDR			(0x00010000-4096)	// 第60K

typedef struct 
{
	uint8_t  saveFlag;
	uint8_t  reserve[1];
	uint16_t devoceId;
}SYNCPARA_STR;

SYNCPARA_STR syncPara;

#else
#define SYSTEMPARA_START_ADDRESS	(0x00010000-4096)	// 第60K
#endif

SYSTEMPARA sysPara;


/*************************************************************
* FuntionName : systemParaSave
* Description : 保存系统参数
* Input		  : None
* Output      : None
* return      : None
***************************************************************/
void systemParaSave(void)
{
	int_FlashErasePage(SYSTEMPARA_START_ADDRESS/4096);
	int_FlashWritebuf(SYSTEMPARA_START_ADDRESS, (uint8_t *)&sysPara, sizeof(SYSTEMPARA));
}

/*************************************************************
* FuntionName : systemParaInit
* Description : 系统参数初始化
* Input		  : None
* Output      : None
* return      : None
***************************************************************/
void systemParaInit(void)
{

	int_FlashRead(SYSTEMPARA_START_ADDRESS,(uint8_t *)&sysPara, sizeof(SYSTEMPARA));

	uartWriteDebug((uint8_t *)&sysPara, sizeof(SYSTEMPARA));
	if (sysPara.sysFlag != 0x55aa)
	{
		uartWriteDebug("err", 3);
		sysPara.sysFlag = 0x55aa;
		sysPara.deviceID = 0xabcd;
		systemParaSave();
	}
}

#ifdef INCLUDE_RF_MASTER

void syncParaSave(uint16_t deviceID)
{
	uint16_t i;
	SYNCPARA_STR syncParaT;
	
	for (i=0; i<1024; i++)
	{
		int_FlashRead((SYNCPARA_START_ADDR+i*4),(uint8_t *)&syncParaT, sizeof(SYNCPARA_STR));
		if (0xff == syncParaT.saveFlag)
		{
			syncParaT.devoceId = deviceID;
			syncParaT.saveFlag = 0x55;		// 有效标志
			int_FlashWritebuf((SYNCPARA_START_ADDR+i*4), (uint8_t *)&syncParaT, sizeof(SYNCPARA_STR));			
			break;
		}
		else if ((0x55==syncParaT.saveFlag) && (syncParaT.devoceId = deviceID))
		{
			break;
		}
		else
		{
			// 继续查询
		}
	}
}

uint16_t syncParaRead(uint16_t *devIDBuf)
{
	uint16_t i;
	uint16_t len=0;

	SYNCPARA_STR syncParaT;
	
	for (i=0; i<1024; i++)	// 4K 空间
	{
		int_FlashRead((SYNCPARA_START_ADDR+i*4),(uint8_t *)&syncParaT, sizeof(SYNCPARA_STR));
		if (0xff == syncParaT.saveFlag)
		{
			return len;
		}
		else if (0x55==syncParaT.saveFlag)
		{
			*(devIDBuf+len) = syncParaT.devoceId;
			len++;
		}
		else
		{
			// 继续查询
		}
	}
	return len;
}
#endif
