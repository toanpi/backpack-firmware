/*! ----------------------------------------------------------------------------
 *  @file    instance_calib.c
 *  @brief   DecaWave application level configuration and calibration functions
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
// #include "compiler.h"
#include "deca_device_api.h"
#include "instance.h"
#include "port_mcu.h"

#define DWT_PRF_64M_RFDLY (514.462f)
#define DWT_PRF_16M_RFDLY (513.9067f)

// -------------------------------------------------------------------------------------------------------------------

// these are default antenna delays for EVB1000, these can be used if there is no calibration data in the DW1000,
// or instead of the calibration data
const uint16 rfDelays[2] = {
		(uint16)((DWT_PRF_16M_RFDLY / 2.0) * 1e-9 / DWT_TIME_UNITS), // PRF 16
		(uint16)((DWT_PRF_64M_RFDLY / 2.0) * 1e-9 / DWT_TIME_UNITS)};

// -------------------------------------------------------------------------------------------------------------------
extern instance_data_t instance_data[NUM_INST];

int instance_starttxtest(int framePeriod)
{
	// define some test data for the tx buffer
	uint8 msg[127] = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the l";

	// NOTE: SPI frequency must be < 3MHz
	port_set_dw1000_slowrate(); // max SPI before PLLs configured is ~4M

	// the value here 0x1000 gives a period of 32.82 �s
	// this is setting 0x1000 as frame period (125MHz clock cycles) (time from Tx en - to next - Tx en)
	dwt_configcontinuousframemode(framePeriod);

	dwt_writetxdata(127, (uint8 *)msg, 0);
	dwt_writetxfctrl(127, 0, 0);

	// to start the first frame - set TXSTRT
	dwt_starttx(DWT_START_TX_IMMEDIATE);

	// measure the power
	// Spectrum Analyser set:
	// FREQ to be channel default e.g. 3.9936 GHz for channel 2
	// SPAN to 1GHz
	// SWEEP TIME 1s
	// RBW and VBW 1MHz
	// measure channel power

	return DWT_SUCCESS;
}

/* ==========================================================

Notes:

Previously code handled multiple instances in a single console application

Now have changed it to do a single instance only. With minimal code changes...(i.e. kept [instance] index but it is always 0.

Windows application should call instance_init() once and then in the "main loop" call instance_run().

*/
