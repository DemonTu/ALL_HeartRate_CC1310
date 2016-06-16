#ifndef __SYSTEMPARA_H
	#define __SYSTEMPARA_H

typedef struct 
{
	uint16_t sysFlag;
	uint16_t deviceID;
	uint16_t verNum;
	uint8_t  serNum[10];
}SYSTEMPARA;

extern SYSTEMPARA sysPara;

void systemParaInit(void); 

void systemParaSave(void);

#ifdef INCLUDE_RF_MASTER

void syncParaSave(uint16_t deviceID);
uint16_t syncParaRead(uint16_t *devIDBuf);

#endif
#endif	
