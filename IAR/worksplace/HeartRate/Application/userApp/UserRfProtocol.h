#ifndef __USERRFPROTOCOL_H
	#define __USERRFPROTOCOL_H

enum
{
	CMDNONE      = 0x00,
	CMDREQUEST   = 0x01,
	CMDSYNC      = 0x02,
	CMDCANCESYNC = 0x03,
	CMDSETID     = 0x04,
	CMDSACK    	 = 0x10,
	
};


extern void RfDataProcess(uint8_t *buf, uint8_t len);
extern void rfDataPacking(uint8_t cmd, uint16_t deviceID, uint8_t *packBuf, uint16_t len);
	
#endif

