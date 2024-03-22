/************************************************************************************************************
Module:       dev_cfg

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
03/03/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _DEV_CFG_H
#define _DEV_CFG_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "application_definitions.h"
#include <stdbool.h>
#include <stdint.h>



//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################
#define DEFAULT_MODE    CONFIG_OPERATION_MODE
#define DEFAULT_ADDR    CONFIG_DEFAULT_EUI64



//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef enum {
  cfgUnkown = 0,
  cfgAddress = 1,
  cfgMode,
  cfgAnchorlist,
  cfgNodePos,
  cfgUwbSetting,
  cfgRangingIntervalMs,
  cfgRxDetectThreshold,
  cfgTxDelay,
  cfgRxDelay,
  cfgTxPower,
  cfgLowBitrate,
  cfgLongPreamble,
  cfgEnSmoothingAlgo,
  cfgSmoothMaxSpeed,
  cfgSmoothE1,
  cfgPanId,
} ConfigField;



//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################


void cfgInit();

bool cfgReset(void);

bool cfgFieldSize(ConfigField field, uint8_t * size);

bool cfgReadU8(ConfigField field, uint8_t * value);

bool cfgWriteU8(ConfigField field, uint8_t data);

bool cfgReadU32(ConfigField field, uint32_t * value);

bool cfgWriteU32(ConfigField field, uint32_t value);

bool cfgReadU8list(ConfigField field, uint8_t *list, uint8_t lenth);

bool cfgWriteU8list(ConfigField field, uint8_t *list, uint8_t lenth);

bool cfgReadFP32listLength(ConfigField field, uint8_t * size);

bool cfgReadFP32list(ConfigField field, float list[], uint8_t length);

bool cfgWriteFP32list(ConfigField field, float list[], uint8_t length);

void cfgSetBinaryMode(bool enable);

bool cfgIsBinaryMode();

bool cfgSet(ConfigField field, uint8_t *p_data, uint8_t size);

bool cfgStore(bool need_clear);


//###########################################################################################################
// END OF dev_cfg.h
//###########################################################################################################
#endif
