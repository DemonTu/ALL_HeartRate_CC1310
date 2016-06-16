#ifndef __INTERNALFLASH_H
	#define __INTERNALFLASH_H

extern int int_FlashErasePage(uint_least16_t page);
extern int int_FlashWriteWord(uint_least32_t addr, uint32_t word);
extern int int_FlashWritebuf(uint_least32_t addr, uint8_t *buf, uint16_t len);

extern void int_FlashRead(uint_least32_t addr, uint8_t *pBuf, uint16_t len);

#endif	
