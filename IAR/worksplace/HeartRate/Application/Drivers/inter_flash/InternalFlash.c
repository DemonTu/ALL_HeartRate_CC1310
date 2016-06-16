#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Task.h>

#include <driverlib/flash.h>

#define BASELOADER_IPAGESIZE 4096


/*******************************************************************************
 * @fn     int_FlashErasePage
 *
 * @brief  Erase an internal flash page.
 *
 * @param  page - page number
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
int int_FlashErasePage(uint_least16_t page)
{
  /* Note that normally flash */
  if (FlashSectorErase(page * BASELOADER_IPAGESIZE) == FAPI_STATUS_SUCCESS)
  {
    return 0;
  }
  
  return -1;
}

/*******************************************************************************
 * @fn     int_FlashWriteWord
 *
 * @brief  Write a word to internal flash.
 *
 * @param  addr - address
 * @param  word - value to write
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
int int_FlashWriteWord(uint_least32_t addr, uint32_t word)
{
  if (FlashProgram((uint8_t *) &word, addr, 4) == FAPI_STATUS_SUCCESS)
  {
    return 0;
  }
  
  return -1;
}

/*********************************************************************
 * @fn      int_FlashRead
 *
 * @brief   Read data from internal flash.
 *
 * @param   addr   - read address of internal flash
 * @param   pBuf   - pointer to buffer into which data is read.
 * @param   len    - length of data to read in bytes.
 *
 * @return  None.
 */
void int_FlashRead(uint_least32_t addr, uint8_t *pBuf, uint16_t len)
{

	uint8_t *ptr = (uint8_t *)addr;

	// Read from pointer into buffer.
	while (len--)
	{
		*pBuf++ = *ptr++;
	}
}


/*******************************************************************************
 * @fn     int_FlashWriteBuf
 *
 * @brief  Write buf to internal flash.
 *
 * @param  addr - address
 * @param  buf - value to write
 * @param  len - buf length
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
int int_FlashWritebuf(uint_least32_t addr, uint8_t *buf, uint16_t len)
{
	int flashstatus = -1;
	uint32_t adr;
    uint16_t i=0;
    uint32_t *pBuffer;

    pBuffer = (uint32_t *)&buf[0];

    adr = addr;
    while((adr < (len + addr)))
    {
        flashstatus = int_FlashWriteWord(adr, *pBuffer);
        adr = adr + 4;
        pBuffer++;
        i++;
    }
	return flashstatus;
}


