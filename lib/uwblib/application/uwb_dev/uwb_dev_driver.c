/************************************************************************************************************
Module:       uwb_dev_deriver

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
#include "uwb_dev_driver.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "uwb_dev_setting.h"


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
// OTP address at which the TX power calibration value is stored
#define TXCFG_ADDRESS (0x10)	

// The TX power configuration read from OTP (6 channels consecutively with PRF16 then 64, e.g. Ch 1 PRF 16 is index 0 and PRF 64 index 1)
// OTP address at which the antenna delay calibration value is stored
#define ANTDLY_ADDRESS (0x1C) 

#define CONFIG_SUPPOER_CALIB_OTP			(0)


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
extern instance_data_t instance_data[NUM_INST];
extern const uint16 rfDelays[2];
// extern const tx_struct txSpectrumConfig[8];
extern const uint8 chan_idx[];



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	instance_config()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Function to allow application configuration be passed into instance and affect underlying device opetation
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_config(instance_data_t *inst)
{
	int instance = 0;
	uint32 power = 0;
	instanceConfig_t *config = &inst->chConfig;

	inst->configData.chan = config->channelNumber;
	inst->configData.rxCode = config->preambleCode;
	inst->configData.txCode = config->preambleCode;
	inst->configData.prf = config->pulseRepFreq;
	inst->configData.dataRate = config->dataRate;
	inst->configData.txPreambLength = config->preambleLen;
	inst->configData.rxPAC = config->pacSize;
	inst->configData.nsSFD = config->nsSFD;
	inst->configData.phrMode = DWT_PHRMODE_EXT;
	inst->configData.sfdTO = config->sfdTO;

	// configure the channel parameters
	dwt_configure(&inst->configData);

#if CONFIG_SUPPOER_CALIB_OTP
	// check if to use the antenna delay calibration values as read from the OTP
	if (dwt_otprevision() <= 1) // in revision 0, 1 of EVB1000/EVK1000
	{
		uint32 otpPower[12];

		// MUST change the SPI to < 3MHz as the dwt_otpread will change to XTAL clock
		port_set_dw1000_slowrate(); // reduce SPI to < 3MHz

		// read any data from the OTP for the TX power
		dwt_otpread(TXCFG_ADDRESS, otpPower, 12);

		port_set_dw1000_fastrate(); // increase SPI to max

		power = otpPower[(config->pulseRepFreq - DWT_PRF_16M) + (chan_idx[inst->configData.chan] * 2)];
	}
#endif

	inst->configTX.PGdly = tx_power_get_dg_delay(config);

	if ((power == 0x0) || (power == 0xFFFFFFFF)) // if there are no calibrated values... need to use defaults
	{
		tx_power_init(&inst->txPower, config, &inst->configTX.power);
	}
	else
	{
			inst->configTX.power = power;
			// configure the tx spectrum parameters (power and PG delay)
			dwt_configuretxrf(&inst->configTX);
	}

	inst->defaultAntennaDelay = rfDelays[config->pulseRepFreq - DWT_PRF_16M];

	// -------------------------------------------------------------------------------------------------------------------
	// set the antenna delay, we assume that the RX is the same as TX.
	dwt_setrxantennadelay(inst->rxAntennaDelay);
	dwt_settxantennadelay(inst->txAntennaDelay);

	inst->antennaDelayChanged = 0;

	if (config->preambleLen == DWT_PLEN_64) // if preamble length is 64
	{
		port_set_dw1000_slowrate(); // reduce SPI to < 3MHz

		dwt_loadopsettabfromotp(0);

		port_set_dw1000_slowrate(); // increase SPI to max
	}

	/* Set receiver detech threshold */
	uwb_dev_set_rx_detect_threshold(inst, inst->rxDetectThreshold);
}

/********************************************************************************
Function:
  instance_read_accumulator_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Read the accumulator data from the device.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_read_accumulator_data(void)
{
#if DECA_SUPPORT_SOUNDING == 1
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  uint16 len = 992; // default (16M prf)

  if (inst->configData.prf == DWT_PRF_64M) // Figure out length to read
    len = 1016;

  inst->buff.accumLength = len; // remember Length, then read the accumulator data

  len = len * 4 + 1; // extra 1 as first byte is dummy due to internal memory access delay

  dwt_readaccdata((uint8 *)&(inst->buff.accumData->dummy), len, 0);
#endif // support_sounding
}

/********************************************************************************
Function:
	instance_config_sleep()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Configure the device to sleep
Notes:
	---
Author, Date:
	Toan Huynh, 03/18/2022
*********************************************************************************/
bool instance_config_sleep(instance_data_t * inst)
{
	bool status = true;

	if (inst->lowPowerEnable)
	{
		uint16 sleep_mode = (DWT_PRESRV_SLEEP | DWT_CONFIG | DWT_TANDV);

		if (inst->configData.txPreambLength == DWT_PLEN_64)
		{
			sleep_mode |= DWT_LOADOPSET;
		}
		// configure the on wake parameters (upload the IC config settings)
		dwt_configuresleep(sleep_mode, DWT_WAKE_WK | DWT_WAKE_CS | DWT_SLP_EN);
	}

	return status;
}

/********************************************************************************
Function:
	instance_enter_sleep()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Enter sleep
Notes:
	---
Author, Date:
	Toan Huynh, 03/18/2022
*********************************************************************************/
bool instance_enter_sleep(instance_data_t *inst)
{
	bool status = true;

	if (inst->lowPowerEnable)
	{
		/* Put device into low power mode */
		dwt_entersleep();
		inst->isInDeepSleep = true;
		
		/* Disable pin connection to prevent current leak */
		port_disable_pin_connection();

		// sys_printf("[%u]DEVICE ENTER DEEP SLEEP\r\n", portGetTickCnt());
	}
	else
	{
		/* Turn off the receiver to save power */
		dwt_setrxtimeout(0);
		dwt_forcetrxoff();
		dwt_rxreset();
	}

	return status;
}

/********************************************************************************
Function:
	instance_exit_sleep()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Exit sleep mode.
Notes
	---
Author, Date:
	Toan Huynh, 03/18/2022
*********************************************************************************/
bool instance_exit_sleep(instance_data_t *inst)
{
	bool status = true;

	if (inst->lowPowerEnable)
	{
		/* Enable pin connection*/
		port_enable_pin_connection();

		// port_wakeup_dw1000();
		port_wakeup_dw1000_fast();

		dwt_forcetrxoff();
		dwt_rxreset();

		/* MP bug - TX antenna delay needs reprogramming as it is not preserved (only RX) */
		dwt_settxantennadelay(inst->txAntennaDelay);

		/* set EUI (not preserved) */
		dwt_seteui(inst->eui64);

		/* Config device to low power mode */
		instance_config_sleep(inst);

		inst->isInDeepSleep = false;
		// sys_printf("[%u]DEVICE EXIT DEEP SLEEP\r\n", portGetTickCnt());
	}

	return status;
}


/********************************************************************************
Function:
	instance_rxon()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Turn on the receiver with/without delay
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_rxon(instance_data_t *inst, int delayed, uint64 delayedReceiveTime)
{
	if (delayed)
	{
		uint32 dtime;
		dtime = (uint32)(delayedReceiveTime >> 8);
		dwt_setdelayedtrxtime(dtime);
	}

	int dwt_rx_enable_return = dwt_rxenable(delayed);
	inst->lateRX -= dwt_rx_enable_return;

}

/********************************************************************************
Function:
	instance_send_packet()
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
int instance_send_packet(uint16 length, uint8 txmode, uint32 dtime)
{
	int result = 0;

	dwt_writetxfctrl(length, 0, 1);
	if (txmode & DWT_START_TX_DELAYED)
	{
		dwt_setdelayedtrxtime(dtime);
	}

	// begin delayed TX of frame
	if (dwt_starttx(txmode)) // delayed start was too late
	{
		// sys_printf("[ERR][TX] FAILED\r\n");
		//while (1); // Need to take care if this happens
		result = 1; // late/error
	}

	return result; // state changes
}

/********************************************************************************
Function:
	instance_read_device_id()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Return the Device ID register value, enables higher level validation of physical device presence
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 instance_read_device_id(void)
{
	return dwt_readdevid();
}

/********************************************************************************
Function:
	uwb_dev_off_transceiver()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Turn of the transceiver of the device
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
bool uwb_dev_off_transceiver(void)
{
	bool status = true;

	dwt_forcetrxoff();

	return status;
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
//      END OF uwb_dev_deriver.c
//###########################################################################################################
