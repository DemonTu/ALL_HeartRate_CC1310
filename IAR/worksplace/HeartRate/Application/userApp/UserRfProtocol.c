#include "includes.h"

#include "UserRfProtocol.h"

//无线通信的协议

//协议结构
typedef struct
{
	uint16_t crc;       // 数据校验，只包含数据的校验
	
	uint8_t  cmdHead;   // 协议头字节
	uint8_t  cmd;       // 协议命令
	uint16_t deviceID;	// 唯一ID号用于对广播信号的区别
	
	uint16_t len;		// 数据长度
//  uint8_t *dat;	
}PROSTR;

//上传参数的数据结构
typedef struct
{
	/* 电池电量 */
	uint8_t  batteryCharge; // 电量
	/* 传感器数据*/	
	uint16_t heartRate;		// 实时心率	
	uint16_t heartRateAvg;  // 心率平均值
	uint16_t stepRate;		// 步率
	uint16_t distance;		// 运动的路程
	uint16_t totalSteps;	// 总步数
	uint16_t speed;			// 运动速度
	uint16_t cals;			// 消耗的卡路里
	uint16_t sVO2;			// 耗氧量  单位:10x ml/kg/min
}DATASTR;

#define PROTOCOLHEADBYTE	0xaa

/*
 * 无线数据封包，发送 
 */
void rfDataPacking(uint8_t cmd, uint16_t deviceID, uint8_t *packBuf, uint16_t len)
{
	uint8_t sendBufTemp[31];
	uint16_t sendLen = 0;
	uint16_t crc;

	sendBufTemp[2] = PROTOCOLHEADBYTE;
	sendBufTemp[3] = cmd;
	
	/*device ID*/
	U16_To_BigEndingBuf(&sendBufTemp[4], deviceID);
	
	/* data length */
	sendBufTemp[6] = 0;
	sendBufTemp[7] = len;
	
	/* data */
	memcpy(&sendBufTemp[8], packBuf, len);
	sendLen = sizeof(PROSTR)+len;

	/* crc */
	crc = Crc16_1021_Sum(&sendBufTemp[2], sendLen-2);
	U16_To_BigEndingBuf(sendBufTemp, crc);
	
	RF_Send(sendBufTemp, sendLen);
}


void RfDataProcess(uint8_t *buf, uint8_t len)
{
	PROSTR *datHead;
	uint16_t cmpCrc;
	uint8_t ackFlag = CMDNONE; 
	SENSORPARA_STR sendDatTemp;
	uint8_t *sendBuf = (uint8_t *)malloc(sizeof(DATASTR));	
	uint8_t sendLen = 0;
	
	datHead = (PROSTR *)buf;
	datHead->crc = BigEndingBuf_To_U16(buf);
	if (datHead->cmdHead != PROTOCOLHEADBYTE)
	{
		return;
	}
	cmpCrc = Crc16_1021_Sum(&buf[2], len-2);
	if (datHead->crc != cmpCrc)
	{
		uartWriteDebug("crc", 3);
		return;
	}
	switch(datHead->cmd)
	{
		case CMDREQUEST:					 // 请求获取心率事件
			HR_GetSensorData(&sendDatTemp);
			/* 读取电池电量 */
			// powerGetChargeValue();
			U16_To_BigEndingBuf(sendBuf, powerGetChargeValue());	// 电池电量
			U16_To_BigEndingBuf(&sendBuf[2],  sendDatTemp.heartRate);
			U16_To_BigEndingBuf(&sendBuf[4],  sendDatTemp.heartRateAvg);
			U16_To_BigEndingBuf(&sendBuf[6],  sendDatTemp.stepRate);
			U16_To_BigEndingBuf(&sendBuf[8],  sendDatTemp.distance);
			U16_To_BigEndingBuf(&sendBuf[10], sendDatTemp.totalSteps);
			U16_To_BigEndingBuf(&sendBuf[12], sendDatTemp.speed);
			U16_To_BigEndingBuf(&sendBuf[14], sendDatTemp.cals);
			//U16_To_BigEndingBuf(&sendBuf[16], 0x64);
			sendLen = sizeof(DATASTR);
			ackFlag = CMDSACK | CMDREQUEST;  
			break;
#ifdef INCLUDE_RF_MASTER
		case CMDSYNC:						 // 主设备收到同步事件
		
			ackFlag = CMDSACK | CMDSYNC;  
			break;
		case (CMDSACK | CMDREQUEST):	     // 主设备收到请求应答事件 
			systemUserEnqueue(EVENT_REQUES_OK, 0,  NULL);
			uartWriteDebug("ok",2);

			ackFlag = CMDNONE;		
			break;
#endif			
		default:
			break;
	}
	if (CMDNONE != ackFlag)
	{
		rfDataPacking(ackFlag, 0xabcd, sendBuf, sendLen);
	}

	free(sendBuf);
}

