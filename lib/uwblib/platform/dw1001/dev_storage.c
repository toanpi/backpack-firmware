/************************************************************************************************************
Module:       dev_storage

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

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "dev_storage.h"

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
#define ENABLE_FLASH 1
#define RECORD_KEY 0x11
#define FILE_ID_LPS_FLASH 0x1338

//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static int devAddr = 0xA0;

#if !ENABLE_FLASH
uint8_t ram[256];
#endif

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################

static void env_evt_handler(fds_evt_t const *const p_fds_evt)
{
}

/********************************************************************************
Function:
  devStorageInit()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
void devStorageInit(void)
{
#if ENABLE_FLASH
  fds_register(env_evt_handler);

  fds_init();
#endif
}

/********************************************************************************
Function:
  devStorageTest()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
bool devStorageTest()
{
  uint8_t test;

  return devStorageRead(0, &test, 1);
}

/********************************************************************************
Function:
  devStorageRead()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
bool devStorageRead(int address, void *data, size_t length)
{
#if ENABLE_FLASH
  ret_code_t err_code;
  fds_flash_record_t record = {0};
  fds_record_desc_t desc = {0};
  fds_find_token_t ft = {0};

  err_code = fds_record_find_by_key(RECORD_KEY, &desc, &ft);

  err_code = fds_record_open(&desc, &record);

  if (err_code != NRF_SUCCESS)
  {
    fds_record_close(&desc);
    return false;
  }

  memcpy(data, record.p_data, length);

  err_code = fds_record_close(&desc);

  return err_code == NRF_SUCCESS;
#else
  memcpy(data, ram + address, length);
  return true;
#endif
}

/********************************************************************************
Function:
  devStorageHasData()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
bool devStorageHasData(int address)
{
#if ENABLE_FLASH
  ret_code_t err_code;
  fds_flash_record_t record = {0};
  fds_record_desc_t desc = {0};
  fds_find_token_t ft = {0};

  err_code = fds_record_find_by_key(RECORD_KEY, &desc, &ft);

  return err_code == NRF_SUCCESS;
#else
  return true;
#endif
}

/********************************************************************************
Function:
  devStorageWrite()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
bool devStorageWrite(int address, void *data, size_t length)
{
#if ENABLE_FLASH
  ret_code_t err_code;
  fds_flash_record_t record = {0};
  fds_record_desc_t desc = {0};
  fds_find_token_t ft = {0};
  fds_record_t record_to_write =
      {
          .data.p_data = data,
          .data.length_words = (length + 3) / 4, // Round up to 4
          .file_id = FILE_ID_LPS_FLASH,
          .key = RECORD_KEY};

  err_code = fds_record_find_by_key(RECORD_KEY, &desc, &ft);

  if (err_code == FDS_ERR_NOT_FOUND)
  {
    err_code = fds_record_write(&desc, &record_to_write);
  }

  else
  {
    err_code = fds_record_update(&desc, &record_to_write);
  }

  fds_gc();

  return err_code == NRF_SUCCESS;
#else
  memcpy(ram + address, data, length);
  return true;
#endif
}

/********************************************************************************
Function:
  devStorageClear()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
void devStorageClear(void)
{
  ret_code_t err_code;
  fds_record_desc_t desc = {0};
  fds_find_token_t ft = {0};

  err_code = fds_record_find_by_key(RECORD_KEY, &desc, &ft);

  err_code = fds_record_delete(&desc);
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
//      END OF dev_storage.c
//###########################################################################################################
