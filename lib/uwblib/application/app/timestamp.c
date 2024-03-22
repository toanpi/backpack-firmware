/************************************************************************************************************
Module:       timestamp

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/15/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include "timestamp.h"
#include "instance.h"


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	timestamp_get_ms()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get the current timestamp with resolution in milliseconds.
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
uint32 timestamp_get_ms(void)
{
	return portGetTickCnt();
}

/********************************************************************************
Function:
	timestamp_get_us()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get the current timestamp with resolution in microseconds.
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
uint64 timestamp_get_us(void)
{
	return portGetTickCntMicro();
}

/********************************************************************************
Function:
  get_dt32()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get the time difference between two between two 32-bit unsigned timestamps
  t1 is the first timestamp
  t2 is the second timetamp that occured after t1
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 get_dt32(uint32 t1, uint32 t2)
{
	if (t2 >= t1)
	{
		return t2 - t1;
	}
	else
	{
		// handle timestamp roleover
		return 4294967295 - t1 + t2;
	}
}

/********************************************************************************
Function:
  timestamp_add32()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Add a duration to a 32 bit timestamp. This function handles number wrapping
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 timestamp_add32(uint32 timestamp, uint32 duration)
{
	uint32 to_wrap = 4294967295 - timestamp;
	if (duration > to_wrap)
	{
		return duration - to_wrap;
	}
	else
	{
		return timestamp + duration;
	}
}

/********************************************************************************
Function:
  timestamp_subtract32()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Subtract a duration from a 32 bit timestamp. This function handles number wrapping
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 timestamp_subtract32(uint32 timestamp, uint32 duration)
{
	if (duration > timestamp)
	{
		return 4294967295 - (duration - timestamp);
	}
	else
	{
		return timestamp - duration;
	}
}

/********************************************************************************
Function:
  get_dt64()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get the time difference between two between two 64-bit unsigned timestamps
  t1 is the first timestamp
  t2 is the second timetamp that occured after t1
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint64 get_dt64(uint64 t1, uint64 t2)
{
	if (t2 >= t1)
	{
		return t2 - t1;
	}
	else
	{
		// handle timestamp rollover
		return 4294967295999 - t1 + t2;
	}
}

/********************************************************************************
Function:
  timestamp_add64()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  add a duration to a 64 bit timestamp. This function handles number wrapping
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint64 timestamp_add64(uint64 timestamp, uint64 duration)
{
	uint64 to_wrap = (uint64)4294967295999 - timestamp;
	if (duration > to_wrap)
	{
		return duration - to_wrap;
	}
	else
	{
		return timestamp + duration;
	}
}

/********************************************************************************
Function:
  timestamp_subtract64()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Subtract a duration from a 64 bit timestamp. This function handles number wrapping
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint64 timestamp_subtract64(uint64 timestamp, uint64 duration)
{
	if (duration > timestamp)
	{
		return (uint64)4294967295999 - (duration - timestamp);
	}
	else
	{
		return timestamp - duration;
	}
}


/********************************************************************************
Function:
	convert_microsec_to_device_time_u64()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Convert microseconds to device time
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
uint64 convert_microsec_to_device_time_u64(double microsecu)
{
	uint64 dt;
	long double dtime;

	dtime = (microsecu / (double)DWT_TIME_UNITS) / 1e6;

	dt = (uint64)(dtime);

	return dt;
}

/********************************************************************************
Function:
	convert_microsec_to_device_time_u32()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Convert microseconds to device time
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 convert_microsec_to_device_time_u32(double microsecu)
{
	uint32 dt;
	long double dtime;

	dtime = (microsecu / (double)DWT_TIME_UNITS) / 1e6;

	dt = (uint32)(dtime);

	return dt;
}

/********************************************************************************
Function:
	convert_device_time_to_sec()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
double convert_device_time_to_sec(int32 dt)
{
	double f = 0;

	f = dt * DWT_TIME_UNITS; // seconds #define TIME_UNITS          (1.0/499.2e6/128.0) = 15.65e-12

	return f;
}

/********************************************************************************
Function:
	convert_device_time_bytes_to_sec()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
double convert_device_time_bytes_to_sec(uint8 *dt)
{
	double f = 0;

	uint32 lo = 0;
	int8 hi = 0;

	memcpy(&lo, dt, 4);
	hi = dt[4];

	f = ((hi * 65536.00 * 65536.00) + lo) * DWT_TIME_UNITS; // seconds #define TIME_UNITS          (1.0/499.2e6/128.0) = 15.65e-12

	return f;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################


//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF timestamp.c
//###########################################################################################################
