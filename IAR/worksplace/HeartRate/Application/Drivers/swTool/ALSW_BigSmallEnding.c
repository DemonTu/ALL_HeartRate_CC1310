/**************************************************************************************************/
#include <stdint.h>
#include <stddef.h>
/* #include "ALSW_BigSmallEnding.h" */
/**************************************************************************************************/
uint_fast16_t BigEndingBuf_To_U16(void const *src)
{
	uint8_t const * const p = src;
	return (0xFFFFU & (((uint_fast16_t)p[0]<<8) | ((uint_fast16_t)p[1]<<0)));
}
/**************************************************************************************************/
uint_fast32_t BigEndingBuf_To_U32(void const *src)
{
	uint8_t const * const p = src;
	return (0xFFFFFFFFU &(((uint_fast32_t)p[0]<<24)|((uint_fast32_t)p[1]<<16)\
                		| ((uint_fast32_t)p[2]<<8)|((uint_fast32_t)p[3]<<0)));
}
/**************************************************************************************************/
void *U16_To_BigEndingBuf(void *dest, uint_fast16_t bin)
{
	uint8_t  * const p = dest;
	if (NULL != dest)
	{
		p[0] = (bin>>8)&0xFF;
		p[1] = (bin>>0)&0xFF;
		return (p+2);
	}
	else
	{
		return NULL;
	}
}
/**************************************************************************************************/
void *U32_To_BigEndingBuf(void *dest, uint_fast32_t bin)
{
	uint8_t * const p = dest;
	if (NULL != dest)
	{
		p[0] = (bin>>24)&0xFF;
		p[1] = (bin>>16)&0xFF;
		p[2] = (bin>>8)&0xFF;
		p[3] = (bin>>0)&0xFF;
		return (p+4);
	}
	else
	{
		return (NULL);
	}

}
/**************************************************************************************************/

