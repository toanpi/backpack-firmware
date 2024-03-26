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

#ifndef _TX_POWER_H
#define _TX_POWER_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <application_definitions.h>



//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef enum
{
  TX_POWER_MANUAL,
  TX_POWER_SMART,
  TX_POWER_CALIBRATED_SMART,
} tx_power_mode_e;

typedef struct
{
  tx_power_mode_e mode;

  float phr_db;
  float payload_db;

  // For smart power
  float payload_500us_db;
  float payload_250us_db;
  float payload_125us_db;
} tx_power_t;

typedef struct
{
  tx_power_mode_e mode;

  uint8_t phr_percent;
  uint8_t payload_percent;

  // For smart power
  uint8_t payload_500us_percent;
  uint8_t payload_250us_percent;
  uint8_t payload_125us_percent;

} tx_power_percent_t;


#define DEFAULT_TX_POWER    TX_POWER_CALIBRATED_SMART

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################
#if !defined(min)
#define min(a, b)     (((a) < (b)) ? (a) : (b))
#endif



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
bool tx_power_relative_set(tx_power_percent_t *cfg);
bool tx_power_absolute_set(tx_power_t *cfg);
tx_power_t tx_power_convert_absolute_config(tx_power_percent_t *cfg);
void showTxPower(void);
float tx_power_max_db(void);
bool tx_power_init(tx_power_percent_t *cfgTx, instanceConfig_t *configChannel, uint32_t *p_power);
uint8_t tx_power_get_dg_delay(instanceConfig_t* configChannel);


//###########################################################################################################
// END OF tx_power.h
//###########################################################################################################
#endif
