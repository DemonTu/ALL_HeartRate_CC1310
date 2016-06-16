#ifndef __INCLUDES_H
	#define __INCLUDES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
		
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
		
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>

/* Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>
	

#include "config.h"
#include "st_defs.h"

/* Board Header files */	
#include "Board.h"

// Drivers	
#include "bsp_i2c.h"
//#include "bsp_spi.h"
#include "bsp_uart.h"
#include "OLED_Driver.h"
#include "UserHeartBeatApp.h"

//software tool
#include "ALSW_BigSmallEnding.h"
#include "ALSW_CheckSum.h"


//user app
#include "systemPara.h"

#include "UserRfApp.h"
#include "userPowerManage.h"

#include "SystemMasterTask.h"
#include "OLED_UserApp.h"


#endif	

