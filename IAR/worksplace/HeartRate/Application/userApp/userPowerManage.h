#ifndef __USERPOWERMANAGE_H
	#define __USERPOWERMANAGE_H
	
extern void powerKeyScan10ms(void);
extern void powerManegeTaskInit(void);
extern uint16_t powerGetChargeValue(void);
extern uint8_t powerShowChargePercent(void);

extern void powerOn(void);
extern void powerOff(void);

#endif	
