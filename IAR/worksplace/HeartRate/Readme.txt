//����ͨ�ŵ�Э��

//Э��ṹ
typedef struct
{
	uint8_t  cmdHead;   // Э��ͷ�ֽ�
	uint8_t  cmd;       // Э������
	uint8_t  len;		// ���ݳ���
	uint16_t crc;       // ����У�飬ֻ�������ݵ�У��
//  uint8_t *dat;	
}PROSTR;

//���ݽṹ
typedef struct
{
	uint16_t deviceID;	// ΨһID�����ڶԹ㲥�źŵ�����
	/* ����������*/	
	uint16_t heartRate;		// ʵʱ����	
	uint16_t heartRateAvg;  // ����ƽ��ֵ
	uint16_t stepRate;		// ����
	uint16_t distance;		// �˶���·��
	uint16_t totalSteps;	// �ܲ���
	uint16_t speed;			// �˶��ٶ�
	uint16_t cals;			// ���ĵĿ�·��
}DATASTR;
