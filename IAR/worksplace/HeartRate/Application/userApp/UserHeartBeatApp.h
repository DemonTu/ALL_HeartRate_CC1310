#ifndef __USERHEARTBEAT_H
	#define __USERHEARTBEAT_H
#ifndef INCLUDE_RF_MASTER
/*************************** Command Data Packet Format *********************

	Byte Format/Order for the PerformTek Protocol
	---------------------------------------------------------------------
	| Byte		|				Name				|		Value		|
	---------------------------------------------------------------------
	| 0x00		|	PerformTek Packet Start 		|	0x44			|
	---------------------------------------------------------------------
	| 0x01		|	Byte Count number of bytes, 	|	after this byte	|
	---------------------------------------------------------------------
	| 0x02		|	PerformTek Protocol Command ID	|	see tables below|
	---------------------------------------------------------------------
	| 0x03..0xNN|	PerformTek Protocol Payload Data|	see tables below|
	---------------------------------------------------------------------
*/

/*************************** Protocol Commands **********************************/
// PerformTek Protocol Command IDs	
#define CMDSTART 				0x01 
#define CMDSTOP 				0x02 
#define CMDSET 					0x04 
#define CMDGET 					0x08 
#define CMDACK 					0x10 
#define CMDDATA 				0x20 
	
	/*************************** Command: SET (0x04) ********************************/
	// SET Command Parameters 
#define Age 					0x10 /* Age Range: 60-1440 months
											Default: 360 (30 years) */
#define Gender 					0x11 /* Gender Range: Female (0)
											Male (1)
											Default: 1 (Male) */
#define Weight 					0x12 /* Weight Range: 100-5000
											(in 1/10 kg)
											Default: 816 (81.6 kg)*/
											
#define Height 					0x13 /* Height Range: 60-250 cm
											Default: 180 (~5’ 11”) */
											
#define HeartRateRest 			0x14 /* Resting heart rate Range: 1-220 BPM
											Default: 72 */
											
#define ClearValues 			0x1A /* Clear the specified internal
											values
											Range: 0x0001-0xFFFF
											(See Appendix A) */
	
#define SetActivityMode 		0x1B /* Activity Mode Range:
											MSB (0x8000) enables auto
											mode selection between
											running (1) and lifestyle (6).
											1 = Running
											2 = Low HR
											3 = Cycling
											4 = Weights & Sports
											5 = Aerobics
											6 = Lifestyle
											Default: 1 (Running) */
	
#define ActivityMode_Mi 		0x20 /* Activity Mode Mirrors Register 0x1B for 
											backwards compatibility */
#define WalkCal1 				0x21 /* Walk Speed calibration Default is 8192 */
#define RunCal1 				0x23 /* Run Speed calibration Default is 8192 */
#define HRRunningAvgTime 		0x2B /* Running Average Time (number
											of seconds to average over)
											Range: 0-7 secs
											Default: 0
											(single reading used) */
#define Processing_State        0x2C /* Algorithm Processing State Bits 0-1: 0 = HR Disabled
											Bits 0-1: 3 = HR Enabled
											Bit 6: RRI Enabled */
	
/*************************** command Get(0x08)***********************************/
// GET Command Parameters
#define ConfigID_U 				0x0E //Feature identifier bit array, Upper Word
#define ConfigID_L 				0x0F //Feature identifier bit array, Lower Word
#define ProcState 				0x10 //PerformTek processor state
	
#define SignalFlagAndQuality 	0x11 /* Bit 0: 0 = OK  1 = Out of Ear or Acquiring Signal
											Bit 1-Bit 7: Signal Quality Value (0%-100%)
											Bit 8-Bit 15: Reserved */
											
#define Optical_Input_DC_Level 	0x12 // Optical input DC level (average, % of full scale)
	
#define PostResults 			0x13 /* Result of the POST (Power-On Self-Test) check:
										  Bit 0: 0=OK, 1=System Failure
										  Bit 1: 0=OK, 1=Detector Failure
										  Bit 2: 0=OK, 1=Accelerometer Failure */
										  
#define GetActivityMode 		0x1B /* Range:
											MSB (0x8000) indicates auto mode selection between
											running (1) and lifestyle (6).
											1 = Running
											2 = Low HR
											3 = Cycling
											4 = Weights & Sports
											5 = Aerobics
											6 = Lifestyle
											Default: 1 (Running)
											*/
#define HeartRate 				 0x20 //Most recent heart rate, beats/min
#define HeartRateAvg 			 0x21 //Average heart rate for session, beats/min
#define HeartRateMin 			 0x22 //Minimum heart rate for session, beats/min
#define HeartRateMax 			 0x23 // Maximum heart rate for session, beats/min
#define Processor_State 		 0x2C /* Algorithm Processing State Bits 0-1: 0 = HR Disabled
											Bits 0-1: 3 = HR Enabled
											Bit 6: RRI Enabled */
											
#define StepRate 				 0x30 // Most recent step rate, steps/min
#define Distance 				 0x31 // Distance covered (meters)
#define ActivityCount 			 0x32 // Un-calibrated activity count (arbitrary units)
#define TotalSteps               0x33 // Total number of steps taken
#define Speed                    0x34 // Speed (1/1000’s of miles per hour) 1英里(mi)=1.609344千米(km)
#define RRI_Status               0x35 // Indicates the status of RRI processing (1 = On, 0 = Off)
#define RRI_Timestamp 			 0x36 // 1 second timestamp of the current RRI interval
	
#define RRI_Data_Register1       0x37 // Bit0 – Bit9 = 1st peak timestamp in ms (0-999)
										  // Bit15: 1 = Leading interval, 0 = Relative interval
#define RRI_Data_Register2       0x38 // Bit0 – Bit9 = 2nd peak timestamp in ms (0-999)	
										  //Bit15: 1 = Leading interval, 0 = Relative interval
#define RRI_Data_Register3		 0x39 // Bit0 – Bit9 = 3rd peak timestamp in ms (0-999)
										  //Bit15: 1 = Leading interval, 0 = Relative interval
#define RRI_Data_Register4 		 0x3A // Bit0 – Bit9 = 4th peak timestamp in ms (0-999)
										  //Bit15: 1 = Leading interval, 0 = Relative interval
#define RRI_Data_Register5       0x3B //Bit0 – Bit9 = 5th peak timestamp in ms (0-999)
										  //Bit15: 1 = Leading interval, 0 = Relative interval
#define VO2                      0x40 //Most recent VO2, 10x ml/kg/min
#define CALR                     0x41 //Active calorie burn rate, kCal/hr
#define CALS                     0x42 //Total calories, kCal
#define MAXVO2                   0x43 //Maximum VO2 in this session, 10x ml/kg/min
#define FirmwareVerNum           0x44 //Device Firmware Version Number (FVN)
#define MiscVerInfo              0x45 //Customer-supplied version number/information
#define ProcPartID               0x47 //Processor Part ID
#define ProcSerialID_U           0x48 //Processor Serial ID/Number, Upper Word
#define ProcSerialID_L           0x49 //Processor Serial ID/Number, Lower Word
#define DetectorPartID           0x4A //Light Detector Part ID
#define DetectorSerialID_U       0x4B //Light Detector Serial ID/Number, Upper Word
#define DetectorSerialID_L       0x4C //Light Detector Serial ID/Number, Lower Word
#define AccelPartID              0x4D //Accelerometer Part ID
#define AccelSerialID_U          0x4E //Accelerometer Serial ID/Number, Upper Word
#define AccelSerialID_L          0x4F //Accelerometer Serial ID/Number, lower Word
	
/******************* command ACK (0x10) **********************************/
typedef enum
{
	ACK  = 0x00, //Acknowledge with no errors.
	NACK = 0x01, //Acknowledge with unspecified major error (halt if possible).
	MEH  = 0x02, //Acknowledge with unspecified minor error (continue if desired).
	NOGO = 0x03, //Error processing the START command (don't assume sensors are active).
	UNK  = 0x04  //Invalid or unrecognized parameter from GET command or SET command.
}ACK_STR;

/****************** Command: DATA (0x20) ***********************************/
// PerformTek  Byte 	DATA
//	 Sta	   rtCount	Command
//	0x44	   0x0A 	0x20

// BPM		HR=114BPM	SPM 	 SR=135SPM	 CALS	   Total kCals=57
// Request				Request 			 Request
// 0x20 	0x00 0x72	 0x30	 0x00 0x87	  0x42		 0x00 0x39

//=============================================================================


// PT4100 sensor para
typedef struct
{
	uint16_t heartRate;
	uint16_t heartRateAvg;
	uint16_t stepRate;
	uint16_t distance;
	uint16_t totalSteps;
	uint16_t speed;
	uint16_t cals;
	uint16_t sVO2;			// 耗氧量  单位:10x ml/kg/min (ml/kg*min)
	
}SENSORPARA_STR;

extern Task_Handle HRTaskHandle;
#if 0
{
	1 = Running
	2 = Low HR
	3 = Cycling
	4 = Weights & Sports
	5 = Aerobics
	6 = Lifestyle

}
#endif

void HR_TaskInit(void);
void HR_GetSensorData(SENSORPARA_STR *snrData);
void HR_CloseSensor(void);

#endif
#endif
