#ifndef __SYSTEMMASTERTASK_H
	#define __SYSTEMMASTERTASK_H

// system event
typedef enum{

	EVENT_POWERKEY_DOWN,
	EVENT_POWERKEY_LONG,  
	EVENT_POWERKEY_UP,

	EVENT_MENUKEY_DOWN,
	EVENT_MENUKEY_LONG,  
	EVENT_MENUKEY_UP,

	EVENT_RF_RECV,

	EVENT_SENSOR_SHOW,
	EVENT_SENSOR_HIDE,
//====timer===============
	EVENT_TIME_100MS,
	EVENT_TIME_500MS,
	EVENT_TIME_1S,
	
	EVENT_MAX,	
} SYSTEM_EVENT;
	
// system event passed from profiles.
typedef struct
{
  uint8_t sysEvent;   // system event
  uint8_t len;
  uint8_t *eData; // event para
} SYS_stEvt_t;

extern Queue_Handle systemQueue;

extern uint8_t systemEnqueueMsg(Queue_Handle msgQueue, uint8_t *pMsg);
extern void systemUserEnqueue(uint8_t evt, uint8_t datLen, uint8_t *dat);
extern void systemMasterTaskInit(void);


#endif	
