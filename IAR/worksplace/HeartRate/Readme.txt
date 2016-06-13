//无线通信的协议

//协议结构
typedef struct
{
	uint8_t  cmdHead;   // 协议头字节
	uint8_t  cmd;       // 协议命令
	uint8_t  len;		// 数据长度
	uint16_t crc;       // 数据校验，只包含数据的校验
//  uint8_t *dat;	
}PROSTR;

//数据结构
typedef struct
{
	uint16_t deviceID;	// 唯一ID号用于对广播信号的区别
	/* 传感器数据*/	
	uint16_t heartRate;		// 实时心率	
	uint16_t heartRateAvg;  // 心率平均值
	uint16_t stepRate;		// 步率
	uint16_t distance;		// 运动的路程
	uint16_t totalSteps;	// 总步数
	uint16_t speed;			// 运动速度
	uint16_t cals;			// 消耗的卡路里
}DATASTR;
