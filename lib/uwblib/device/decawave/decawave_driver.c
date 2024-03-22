/************************************************************************************************************
Module:       decawave_driver

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
#include <stdint.h>
#include <stdbool.h>
#include "tx_power.h"
#include "deca_device_api.h"
#include "deca_regs.h"




//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define MAX_DW_TX_POWER_DB       30.5


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
static bool set_smart_mode(void);
static uint8_t build_reg_db(float db);
static void write_tx_power(uint32_t value);
static bool set_manual_mode(void);
static bool tx_power_value_write(tx_power_mode_e mode, uint32_t value);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  set_tx_power()
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
bool set_tx_power(tx_power_t *cfg, uint32_t *p_power)
{
  uint32_t value = 0;

  if (cfg->mode == TX_POWER_SMART)
  {
    uint32_t boost_norm = build_reg_db(cfg->phr_db);
    uint32_t boost_125 = build_reg_db(cfg->payload_125us_db);
    uint32_t boost_250 = build_reg_db(cfg->payload_250us_db);
    uint32_t boost_500 = build_reg_db(cfg->payload_500us_db);

    value = boost_norm + (boost_500 << 8) + (boost_250 << 16) + (boost_125 << 24);

    return tx_power_value_write(cfg->mode, value);
  }
  else if (cfg->mode == TX_POWER_MANUAL)
  {
    uint32_t pow_phr = build_reg_db(cfg->phr_db);
    uint32_t pow_sd = build_reg_db(cfg->payload_db);

    value = (pow_sd << 16) + (pow_phr << 8);

    return tx_power_value_write(cfg->mode, value);
  }

  return false;
}

/********************************************************************************
Function:
  tx_power_write_txrf()
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
bool tx_power_write_txrf(dwt_txconfig_t *tx)
{
  /* configure the tx spectrum parameters (power and PG delay) */
  dwt_configuretxrf(tx);
  return true;
}

/********************************************************************************
Function:
  get_max_tx_power()
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
float get_max_tx_power(void)
{
  return MAX_DW_TX_POWER_DB;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  tx_power_value_write()
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
static bool tx_power_value_write(tx_power_mode_e mode, uint32_t value)
{
  if (mode == TX_POWER_SMART)
  {
    set_smart_mode();
  }
  else if (mode == TX_POWER_MANUAL)
  {
    set_manual_mode();
  }

  write_tx_power(value);

  return true;
}

/********************************************************************************
Function:
  build_reg_db()
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
static uint8_t build_reg_db(float db)
{
  db = min(db, get_max_tx_power());

  /*
   * [0:4]: Fine setting (step 0.5)
   * [5:7]: Coarse setting (step 2.5)
   * Coarse:
   *  000: 15 dB
   *  001: 12.5 dB
   *  010: 10 dB
   *  011: 7.5 dB
   *  100: 5 dB
   *  101: 2.5 dB
   *  110: 0 dB
   *  111: OFF (No output)
   *
   * Fine:
   *  00000: 0.0 dB
   *  00001: 0.5 dB
   *  00010: 1.5 dB
   *  00011: 1.5 dB
   *  ...
   *  11110: 15.0 dB
   *  11111: 15.5 dB
   */
  const float coarse_step = 2.5;
  const float coarse_max = 6;
  const float fine_step = 0.5;
  const float fine_max = 31;

  /* Count coarse value */
  uint8_t coarse = min((uint8_t)(db / coarse_step), coarse_max);

  /* Calculate remain db after subtract coarse */
  db -= coarse * coarse_step;

  /* Revert coarse from 0 -> 6 to 6 -> 0 (because 0 is max value) */
  coarse = coarse_max - coarse;

  /* Count fine value */
  uint8_t fine = min((uint8_t)(db / fine_step), fine_max);

  // Final register stop
  return (coarse << 5) + fine;
}

/********************************************************************************
Function:
  write_tx_power()
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
static void write_tx_power(uint32_t value)
{
  dwt_write32bitreg(TX_POWER_ID, value);
}

/********************************************************************************
Function:
  set_manual_mode()
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
static bool set_manual_mode(void)
{
  dwt_setsmarttxpower(0);
  return true;
}

/********************************************************************************
Function:
  set_smart_mode()
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
static bool set_smart_mode(void)
{
  dwt_setsmarttxpower(1);
  return true;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF decawave_driver.c
//###########################################################################################################
