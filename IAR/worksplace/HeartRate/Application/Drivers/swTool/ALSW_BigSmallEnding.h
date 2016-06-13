#ifndef __BIGSMALLENDING_H
	#define __BIGSMALLENDING_H

uint_fast16_t BigEndingBuf_To_U16(void const *src);	
uint_fast32_t BigEndingBuf_To_U32(void const *src);
void *U16_To_BigEndingBuf(void *dest, uint_fast16_t bin);
void *U32_To_BigEndingBuf(void *dest, uint_fast32_t bin);

#endif	
