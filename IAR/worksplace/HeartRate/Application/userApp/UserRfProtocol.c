#include "includes.h"

#include "UserRfProtocol.h"



//����ͨ�ŵ�Э��

//Э��ṹ
typedef struct
{
	uint16_t crc;       // ����У�飬ֻ�������ݵ�У��
	
	uint8_t  cmdHead;   // Э��ͷ�ֽ�
	uint8_t  cmd;       // Э������
	uint16_t deviceID;	// ΨһID�����ڶԹ㲥�źŵ�����
	
	uint16_t len;		// ���ݳ���
//  uint8_t *dat;	
}PROSTR;

//�ϴ����������ݽṹ
typedef struct
{
	/* ��ص��� */
	uint8_t  batteryCharge; // ����
	/* ����������*/	
	uint16_t heartRate;		// ʵʱ����	
	uint16_t heartRateAvg;  // ����ƽ��ֵ
	uint16_t stepRate;		// ����
	uint16_t distance;		// �˶���·��
	uint16_t totalSteps;	// �ܲ���
	uint16_t speed;			// �˶��ٶ�
	uint16_t cals;			// ���ĵĿ�·��
	uint16_t sVO2;			// ������  ��λ:10x ml/kg/min
}DATASTR;

#define PROTOCOLHEADBYTE	0xaa

enum
{
	CMDNONE    = 0x00,
	CMDREQUEST = 0x01,
	CMDSYNC    = 0x02,
	CMDSACK    = 0x10,
};

static void rfDataPacking(uint8_t cmd, uint8_t *packBuf, uint16_t len)
{
	uint8_t sendBufTemp[31];
	uint16_t sendLen = 0;
	uint16_t crc;

	sendBufTemp[2] = PROTOCOLHEADBYTE;
	sendBufTemp[3] = cmd;
	
	/*device ID*/
	U16_To_BigEndingBuf(&sendBufTemp[4], 0xabcd);
	
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
		case CMDREQUEST:					 // �����ȡ�����¼�
			HR_GetSensorData(&sendDatTemp);
			/* ��ȡ��ص��� */
			// powerGetChargeValue();
			U16_To_BigEndingBuf(sendBuf, powerGetChargeValue());	// ��ص���
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
		case CMDSYNC:						 // ͬ�������¼�
			ackFlag = CMDSACK | CMDREQUEST;  
			break;
		case (CMDSACK | CMDREQUEST):	     // Ӧ���¼� 
		#ifdef INCLUDE_RF_MASTER
			uartWriteDebug("ok",2);
		#endif
			ackFlag = CMDNONE;		
			break;
		default:
			break;
	}
	if (CMDNONE != ackFlag)
	{
		rfDataPacking(ackFlag, sendBuf, sendLen);
	}

	free(sendBuf);
}

