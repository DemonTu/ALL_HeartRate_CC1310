#ifndef __CHECKSUM_H
	#define __CHECKSUM_H

uint_fast8_t Add8Sum_To_U08(void const *src, uint_fast16_t len);
uint_fast16_t TcpIp_CheckSum(void const *src, uint_fast16_t length); //计算校验和
uint_fast16_t Crc16_1021_Sum(void const *src, uint_fast16_t len);

#endif	
