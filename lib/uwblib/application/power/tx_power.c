/************************************************************************************************************
Module:       tx_power

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/07/2023    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2023 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "tx_power.h"
#include "application_definitions.h"
#include "decawave_driver.h"
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

// The table below specifies the default TX spectrum configuration parameters... this has been tuned for DW EVK hardware units
// the table is set for smart power - see below in the instance_config function how this is used when not using smart power
const tx_struct txSpectrumConfig[8] =
{
		// Channel 0 ----- this is just a place holder so the next array element is channel 1
		{
				0x0, // 0
				{
						0x0, // 0
						0x0	 // 0
				}},
		// Channel 1
		{
				0xc9, // PG_DELAY
				{
						0x15355575, // 16M prf power
						0x07274767	// 64M prf power
				}

		},
		// Channel 2
		{
				0xc2, // PG_DELAY
				{
						0x15355575, // 16M prf power
						0x07274767	// 64M prf power
				}},
		// Channel 3
		{
				0xc5, // PG_DELAY
				{
						0x0f2f4f6f, // 16M prf power
						0x2b4b6b8b	// 64M prf power
				}},
		// Channel 4
		{
				0x95, // PG_DELAY
				{
						0x1f1f3f5f, // 16M prf power
						0x3a5a7a9a	// 64M prf power
				}},
		// Channel 5
		{
				0xc0, // PG_DELAY
				{
						0x0E082848, // 16M prf power
						0x25456585	// 64M prf power
				}},
		// Channel 6 ----- this is just a place holder so the next array element is channel 7
		{
				0x0, // 0
				{
						0x0, // 0
						0x0	 // 0
				}},
		// Channel 7
		{
				0x93, // PG_DELAY
				{
						0x32527292, // 16M prf power
						0x5171B1d1	// 64M prf power
				}}
};


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool tx_power_calibrated_set(instanceConfig_t *configChannel, uint32_t *p_power);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################

/********************************************************************************
Function:
	tx_power_init()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/07/2023
*********************************************************************************/
bool tx_power_init(tx_power_percent_t *cfgTx, instanceConfig_t *configChannel, uint32_t *p_power)
{
  if (cfgTx->mode == TX_POWER_CALIBRATED_SMART)
  {
    return tx_power_calibrated_set(configChannel, p_power);
  }
  else
  {
    return tx_power_relative_set(cfgTx);
  }
}

/********************************************************************************
Function:
	tx_power_relative_set()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/07/2023
*********************************************************************************/
bool tx_power_relative_set(tx_power_percent_t *cfg)
{
	if (cfg->mode == TX_POWER_CALIBRATED_SMART)
	{
		return false;
	}

	tx_power_t power = tx_power_convert_absolute_config(cfg);

	return tx_power_absolute_set(&power);
}

/********************************************************************************
Function:
	tx_power_absolute_set()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/07/2023
*********************************************************************************/
bool tx_power_absolute_set(tx_power_t *cfg)
{
  return set_tx_power(cfg, NULL);
}

/********************************************************************************
Function:
	tx_power_get_dg_delay()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/07/2023
*********************************************************************************/
uint8_t tx_power_get_dg_delay(instanceConfig_t *configChannel)
{
  return txSpectrumConfig[configChannel->channelNumber].PGdelay;
}

/********************************************************************************
Function:
	tx_power_convert_absolute_config()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/07/2023
*********************************************************************************/
tx_power_t tx_power_convert_absolute_config(tx_power_percent_t *cfg)
{
	float max_db = get_max_tx_power();

	tx_power_t power = {
			.mode = cfg->mode,
			.phr_db = cfg->phr_percent * max_db / 100,
			.payload_db = cfg->payload_percent * max_db / 100,
			.payload_125us_db = cfg->payload_125us_percent * max_db / 100,
			.payload_250us_db = cfg->payload_250us_percent * max_db / 100,
			.payload_500us_db = cfg->payload_500us_percent * max_db / 100
		};

	return power;
}

/********************************************************************************
Function:
	showTxPower()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/08/2023
*********************************************************************************/
void showTxPower(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	uint32 chan = inst->chConfig.channelNumber;
	uint32 prf = (inst->chConfig.pulseRepFreq == DWT_PRF_16M) ? 16 : 64;
	tx_power_t power = tx_power_convert_absolute_config(&inst->txPower);

	sys_printf("TX Channel: %ld - PRF: %ldM\n\r", chan, prf);
	sys_printf("Tx Power: \n\r");

	if (power.mode == TX_POWER_MANUAL)
	{
		sys_printf("Manual Mode - PHR Power %.2fdB - Payload Power: %.2fdB\r\n",
							 power.phr_db, power.payload_db);
	}
	else
	{
		sys_printf("Smart Power Mode \n\r");
		sys_printf("PHR Power %.2fdB - Boost 500us: %.2fdB - Boost 250us: %.2fdB - Boost 125us: %.2fdB\r\n",
							 power.phr_db, power.payload_500us_db, power.payload_250us_db, power.payload_125us_db);
	}
}

/********************************************************************************
Function:
	tx_power_max_db()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/08/2023
*********************************************************************************/
float tx_power_max_db(void)
{
	return get_max_tx_power();
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 04/07/2023
*********************************************************************************/
static bool tx_power_calibrated_set(instanceConfig_t *configChannel, uint32_t *p_power)
{
	uint32 power = txSpectrumConfig[configChannel->channelNumber].txPwr[configChannel->pulseRepFreq - DWT_PRF_16M];

	dwt_txconfig_t tx = {
			.power = power,
			.PGdly = tx_power_get_dg_delay(configChannel),
	};

	// configure the tx spectrum parameters (power and PG delay)
	tx_power_write_txrf(&tx);

	if (p_power)
	{
		*p_power = power;
	}

	return true;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF tx_power.c
//###########################################################################################################
