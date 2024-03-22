/************************************************************************************************************
Module:       distance

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
#include "distance.h"
#include "timestamp.h"


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
	distance_cal_tof()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Calculate the time of flight between two nodes.
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
int64 distance_cal_tof(instance_data_t *inst, uwb_msg_info_t *p_msg)
{
	int64 Rb, Da, Ra, Db;
	uint64 tagFinalTxTime = 0;
	uint64 tagFinalRxTime = 0;
	uint64 tagPollTxTime = 0;
	uint64 anchorRespRxTime = 0;

	double RaRbxDaDb = 0;
	double RbyDb = 0;
	double RayDa = 0;

	// time of arrival of Final message
	tagFinalRxTime = inst->dwt_final_rx;

	// times measured at Tag extracted from the message buffer
	// extract 40bit times
	memcpy(&tagPollTxTime, &(p_msg->p_data[PTXT]), 5);
	memcpy(&anchorRespRxTime, &(p_msg->p_data[RRXT]), 5);
	memcpy(&tagFinalTxTime, &(p_msg->p_data[FTXT]), 5);

	// poll response round trip delay time is calculated as
	// (anchorRespRxTime - tagPollTxTime) - (anchorRespTxTime - tagPollRxTime)
	Ra = (int64)((anchorRespRxTime - tagPollTxTime) & MASK_40BIT);
	Db = (int64)((inst->anchorRespTxTime - inst->tagPollRxTime) & MASK_40BIT);

	// response final round trip delay time is calculated as
	// (tagFinalRxTime - anchorRespTxTime) - (tagFinalTxTime - anchorRespRxTime)
	Rb = (int64)((tagFinalRxTime - inst->anchorRespTxTime) & MASK_40BIT);
	Da = (int64)((tagFinalTxTime - anchorRespRxTime) & MASK_40BIT);

	RaRbxDaDb = (((double)Ra)) * (((double)Rb)) - (((double)Da)) * (((double)Db));
	RbyDb = ((double)Rb + (double)Db);
	RayDa = ((double)Ra + (double)Da);

	return (int64)(RaRbxDaDb / (RbyDb + RayDa));
}

/********************************************************************************
Function:
  getrangebias_rng()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get the range bias for the given range.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
double getrangebias_rng(uint8 channel, uint8 prf, double distance)
{
	float bias = 0;
	distance *= 1000.0; // convert to mm

	if (channel == 2)
	{
		if (prf == DWT_PRF_16M) // 00
		{
			if (distance < 40800.0)
			{
				double a1 = -0.536346023;
				double b1 = -0.0000935073535;
				double c1 = 6.47523309;
				double d1 = -85.2803987;

				bias = a1 * exp(b1 * distance + c1) + d1;
			}
			else if (distance < 91000.0)
			{
				double a2 = -0.279365897;
				double b2 = -0.0000136494099;
				double c2 = 7.21745083;
				double d2 = 171.189077;

				bias = a2 * exp(b2 * distance + c2) + d2;
			}
			else
			{
				double a2 = -0.279365897;
				double b2 = -0.0000136494099;
				double c2 = 7.21745083;
				double d2 = 171.189077;

				bias = a2 * exp(b2 * 91000.0 + c2) + d2;
			}
		}
		else if (prf == DWT_PRF_64M) // 10
		{
			double a = -0.746580470;
			double b = -0.000100581928;
			double c = 5.66511916;
			double d = -16.8215660;
			double e = 6.85427230;

			if (distance < 67000)
			{
				bias = a * exp(b * distance + c) + d;
			}
			else
			{
				bias = e;
			}
		}
	}
	else if (channel == 5)
	{
		if (prf == DWT_PRF_16M) // 01
		{
			double m = -0.0127206623;
			double bx = 589.971543;
			double by = -365.378639;
			double cx = 12543.2809;
			double cy = -163.200647;
			double a1 = -0.970942265;
			double b1 = -0.0000284505196;
			double c1 = 6.40529252;
			double d1 = 112.149301;

			double x40k = 40000.0;
			double x91k = 91000.0;

			if (distance < bx)
			{
				bias = (bx - distance) * m + by;
			}
			else if (distance < cx)
			{
				bias = (distance - bx) / (cx - bx) * (cy - by) + by;
			}
			else if (distance < x40k)
			{
				double y40k = a1 * exp(b1 * x40k + c1) + d1;
				bias = (distance - cx) / (x40k - cx) * (y40k - cy) + cy;
			}
			else if (distance < x91k)
			{
				bias = a1 * exp(b1 * distance + c1) + d1;
			}
			else
			{
				bias = a1 * exp(b1 * x91k + c1) + d1;
			}
		}
		else if (prf == DWT_PRF_64M) // 11
		{
			double a = -0.922756193;
			double b = -0.0000694702324;
			double c = 5.57166578;
			double d = 10.5099467;

			bias = a * exp(b * distance + c) + d;
		}
	}

	return bias / 1000.0; // convert to meters
}

/********************************************************************************
Function:
  getrangebias_rsl()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get range bias for RSL
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
double getrangebias_rsl(uint8 channel, uint8 prf, double rsl)
{
	float bias = 0;

	if (channel == 2)
	{
		if (prf == DWT_PRF_16M) // 00
		{
			float m = -62.04;
			float bx = -82.83;
			float by = -129.82;
			float cx = -90.05;
			float cy = 28.16;
			float dx = -92.40;
			float dy = 49.44;

			if (rsl > bx) // region 1
			{
				bias = (rsl - bx) * m + by;
			}
			else if (rsl > cx) // region 2
			{
				bias = (rsl - cx) / (bx - cx) * (by - cy) + cy;
			}
			else if (rsl > dx) // region 3
			{
				bias = (rsl - dx) / (cx - dx) * (cy - dy) + dy;
			}
			else // region 4
			{
				bias = dy;
			}
		}
		else if (prf == DWT_PRF_64M) // 10
		{
			double m = -45.48484848;
			double bx = -80.62270464;
			double by = -92.36287762;
			double cx = -82.03729358;
			double cy = -109.84714946;
			double dx = -82.83220438;
			double dy = -50.94116132;
			double ex = -85.42920938;
			double ey = -22.6320239;
			double fx = -89.88519826;
			double fy = -38.18332969;
			double gx = -91.7826361;
			double gy = 7.25759428;

			if (rsl > bx) // region 1
			{
				bias = (rsl - bx) * m + by;
			}
			else if (rsl > cx) // region 2
			{
				bias = (rsl - cx) / (bx - cx) * (by - cy) + cy;
			}
			else if (rsl > dx) // region 3
			{
				bias = (rsl - dx) / (cx - dx) * (cy - dy) + dy;
			}
			else if (rsl > ex) // region 4
			{
				bias = (rsl - ex) / (dx - ex) * (dy - ey) + ey;
			}
			else if (rsl > fx) // region 5
			{
				bias = (rsl - fx) / (ex - fx) * (ey - fy) + fy;
			}
			else if (rsl > gx) // region 6
			{
				bias = (rsl - gx) / (fx - gx) * (fy - gy) + gy;
			}
			else // region 7
			{
				bias = gy;
			}
		}
	}
	else if (channel == 5)
	{
		if (prf == DWT_PRF_16M) // 01
		{
			float m = -60.771777;
			float bx = -84.68474012;
			float by = -61.42658856;
			float cx = -88.40823804;
			float cy = 55.46391624;
			float dx = -90.59395975;
			float dy = 1.48597902;
			float ex = -95.42385593;
			float ey = -52.4786514;

			if (rsl > bx) // region 1
			{
				bias = (rsl - bx) * m + by;
			}
			else if (rsl > cx) // region 2
			{
				bias = (rsl - cx) / (bx - cx) * (by - cy) + cy;
			}
			else if (rsl > dx) // region 3
			{
				bias = (rsl - dx) / (cx - dx) * (cy - dy) + dy;
			}
			else if (rsl > ex) // region 4
			{
				bias = (rsl - ex) / (dx - ex) * (dy - ey) + ey;
			}
			else // region 5
			{
				bias = ey;
			}
		}
		else if (prf == DWT_PRF_64M) // 11
		{
			double m = -54.53010908;
			double bx = -81.65862959;
			double by = -4.6428097;
			double cx = -85.18994963;
			double cy = 12.25860862;
			double dx = -88.01254788;
			double dy = -27.64410538;

			if (rsl > bx) // region 1
			{
				bias = (rsl - bx) * m + by;
			}
			else if (rsl > cx) // region 2
			{
				bias = (rsl - bx) / (cx - bx) * (cy - by) + by;
			}
			else if (rsl > dx) // region 3
			{
				bias = (rsl - cx) / (dx - cx) * (dy - cy) + cy;
			}
			else // region 4
			{
				bias = dy;
			}
		}
	}

	return bias / 1000.0; // convert to meters
}

/********************************************************************************
Function:
  reportTOF()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  report time of flight
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
int reportTOF(instance_data_t *inst, uint8 uwb_index, double rsl)
{
	// no TOF to report if not from a uwb that we are tracking
	if (uwb_index >= UWB_LIST_SIZE)
	{
		return -1;
	}

	double distance_raw = 0;
	double distance_rsl = 0;
	double distance_rng = 0;
	double tof;
	int64 tofi;

	// check for negative results and accept them making them proper negative integers
	tofi = inst->tof[uwb_index]; // make it signed

	if (tofi > 0x007FFFFFFFFF) // MP counter is 40 bits,  close up TOF may be negative
	{
		tofi -= 0x010000000000; // subtract fill 40 bit range to make it negative
	}

	// convert to seconds (as floating point)
	tof = convert_device_time_to_sec(tofi);
	// inst->idistanceraw[uwb_index] = distance_raw = tof * SPEED_OF_LIGHT;
	distance_raw = tof * SPEED_OF_LIGHT;

#if (CORRECT_RANGE_BIAS == 1)
	distance_rng = distance_raw - getrangebias_rng(inst->configData.chan, inst->configData.prf, distance_raw);
	distance_rsl = distance_raw - getrangebias_rsl(inst->configData.chan, inst->configData.prf, rsl);
#else
	distance_rng = distance_raw;
	distance_rsl = distance_raw;
#endif

	int retval = 0;

	if (distance_rng > 20000.000) // discount any items with error
	{
		distance_rng = 0;
		retval = -1;
	}

	if (distance_rsl > 20000.000) // discount any items with error
	{
		distance_rsl = 0;
		retval = -1;
	}

	if (distance_rng < 0)
	{
		distance_rng = 0;
	}

	if (distance_rsl < 0)
	{
		distance_rsl = 0;
	}

	inst->idistance[uwb_index] = distance_rng;
	inst->idistancersl[uwb_index] = distance_rsl;
	// inst->iRSL[uwb_index] = rsl;

	return retval;
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
//      END OF distance.c
//###########################################################################################################
