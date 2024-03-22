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

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "dev_cfg.h"
#include "dev_storage.h"
#include "port_mcu.h"

//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################

//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
// Temporary fix, just buffer the first 100 bytes
#define CONFIG_BUF_SIZE           (256)
#define MAGIC                     ((uint8_t)0xBC)

#define SIZE_HEADER               (5)
#define SIZE_TAIL                 (1)

//###########################################################################################################
//      MACROS
//###########################################################################################################

//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################
typedef struct
{
  uint8_t magic;
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint16_t tlvSize;
} __attribute__((packed)) CfgHeader;

typedef struct
{
  uint8_t *data;
} TlvArea;

//###########################################################################################################
//      CONSTANTS
//###########################################################################################################

//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static uint8_t buffer[CONFIG_BUF_SIZE];
static TlvArea tlv;
static CfgHeader *cfgHeader = (CfgHeader *)buffer;
static bool binaryMode = false;

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static int tlvFindType(TlvArea *tlv, ConfigField type);
static bool readData(void);
static void write_crc(void);
static bool check_crc(void);
static bool check_magic(void);
static bool check_content(void);
static bool write_defaults(void);
static bool cfgWriteField(ConfigField field, uint16_t pos, uint8_t *p_data, uint8_t data_len);
static bool cfgRemoveField(ConfigField field, uint16_t pos);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  cfgInit()
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
void cfgInit(void)
{
  /* Initialize the storage flash memory */
  devStorageInit();

  // devStorageClear();

  // Has data --> Check content
  if (readData() || !devStorageHasData(0))
  {
    cfgHeader = (CfgHeader *)buffer;
    tlv.data = &buffer[SIZE_HEADER];
    if (check_content())
    {
      sys_printf("CONFIG: READ AND VERIFIED PASS\r\n");
    }
    else
    {
      sys_printf("CONFIG: WRITE DEFAULT CONFIG\r\n");
      if (!write_defaults())
      {
        sys_printf("CONFIG: WRITE ERROR\r\n");
      }
    }
  }
  else
  {
    sys_printf("CONFIG: READ ERROR\r\n");
  }
}

/********************************************************************************
Function:
  cfgReset()
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
bool cfgReset(void)
{
  bool ret = true;
  devStorageClear();
  return ret;
}

/********************************************************************************
Function:
  cfgSet()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Set configuration value in volatile memory
Notes:
  ---
Author, Date:
  Toan Huynh, 05/09/2022
*********************************************************************************/
bool cfgSet(ConfigField field, uint8_t *p_data, uint8_t size)
{
  int pos = tlvFindType(&tlv, field);
  bool ret = true;
  bool createNew = true;

  if (pos > -1)
  {
    uint8_t curSize = tlv.data[pos + 1];

    if (curSize == size)
    {
      ret = cfgWriteField(field, pos, p_data, size);
      createNew = false;
    }
    else
    {
      // Size is mismatch --> Remove old one
      ret = cfgRemoveField(field, pos);
    }
  }

  if (createNew && ret)
  {
    // Add new field at the end of the tlv
    ret = cfgWriteField(field, cfgHeader->tlvSize, p_data, size);
  }

  return ret;
}

/********************************************************************************
Function:
  cfgStoreNew()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Store configuration value in flash memory
Notes:
  ---
Author, Date:
  Toan Huynh, 05/09/2022
*********************************************************************************/
bool cfgStore(bool need_clear)
{
  if (need_clear)
  {
    cfgReset();
  }

  write_crc();

  devStorageWrite(0, buffer, CONFIG_BUF_SIZE);

  readData();

  return true;
}

/********************************************************************************
Function:
  cfgReadU8()
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
bool cfgReadU8(ConfigField field, uint8_t *value)
{
  int pos = tlvFindType(&tlv, field);

  if (pos > -1)
  {
    *value = tlv.data[pos + 2];
  }

  return (pos > -1);
}

/********************************************************************************
Function:
  cfgWriteU8()
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
bool cfgWrite(ConfigField field, uint8_t *p_data, uint8_t size)
{
  bool ret = cfgSet(field, p_data, size);

  if (ret)
  {
    cfgStore(true);
  }

  return ret;
}

/********************************************************************************
Function:
  cfgReadU32()
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
bool cfgReadU32(ConfigField field, uint32_t *value)
{
  int pos = tlvFindType(&tlv, field);

  if (pos > -1)
  {
    memcpy(value, &tlv.data[pos + 2], sizeof(uint32_t));
  }

  return (pos > -1);
}

/********************************************************************************
Function:
  cfgWriteU32()
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
bool cfgWriteU32(ConfigField field, uint32_t value)
{
  return cfgWrite(field, (uint8_t *)&value, sizeof(uint32_t));
}

/********************************************************************************
Function:
  cfgReadU8list()
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
bool cfgReadU8list(ConfigField field, uint8_t *list, uint8_t length)
{
  int pos = tlvFindType(&tlv, field);

  if (pos > -1)
  {
    memcpy(list, &tlv.data[pos + 2], length);
  }

  return (pos > -1);
}

/********************************************************************************
Function:
  cfgFieldSize()
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
bool cfgFieldSize(ConfigField field, uint8_t *size)
{
  int pos = tlvFindType(&tlv, field);

  if (pos > -1)
  {
    *size = tlv.data[pos + 1];
  }

  return (pos > -1);
}

/********************************************************************************
Function:
  cfgWriteU8list()
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
bool cfgWriteU8list(ConfigField field, uint8_t* list, uint8_t length)
{
  return cfgWrite(field, list, length);
}

/********************************************************************************
Function:
  cfgReadFP32listLength()
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
bool cfgReadFP32listLength(ConfigField field, uint8_t *size)
{
  bool success = cfgFieldSize(field, size);
  *size /= 4;
  return success;
}

/********************************************************************************
Function:
  cfgReadFP32list()
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
bool cfgReadFP32list(ConfigField field, float list[], uint8_t length)
{
  int pos = tlvFindType(&tlv, field);

  if (pos > -1)
  {
    memcpy(list, &tlv.data[pos + 2], length * sizeof(float));
  }

  return (pos > -1);
}

/********************************************************************************
Function:
  cfgWriteFP32list()
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
bool cfgWriteFP32list(ConfigField field, float list[], uint8_t length)
{
  return cfgWrite(field, (uint8_t *)list, length * sizeof(float));
}

/********************************************************************************
Function:
  deckTlvHasElement()
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
bool deckTlvHasElement(TlvArea *tlv, ConfigField type)
{
  return tlvFindType(tlv, type) >= 0;
}

/********************************************************************************
Function:
  cfgSetBinaryMode()
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
void cfgSetBinaryMode(bool enable)
{
  binaryMode = enable;
}

/********************************************************************************
Function:
  cfgIsBinaryMode()
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
bool cfgIsBinaryMode()
{
  return binaryMode;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  tlvFindType()
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
static int tlvFindType(TlvArea *tlv, ConfigField type)
{
  uint16_t pos = 0;

  while (pos < cfgHeader->tlvSize)
  {
    if (tlv->data[pos] == type)
    {
      return pos;
    }
    else
    {
      pos += tlv->data[pos + 1] + 2;
    }
  }
  return -1;
}

/********************************************************************************
Function:
  cfgWriteField()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Writes a field to config structure
Notes:
  ---
Author, Date:
  Toan Huynh, 05/09/2022
*********************************************************************************/
static bool cfgWriteField(ConfigField field, uint16_t pos, uint8_t *p_data, uint8_t data_len)
{
  if ((pos + data_len) > CONFIG_BUF_SIZE || !p_data)
  {
    return false;
  }

  /* Clear the field */
  // uint8_t dataSize = tlv.data[pos + 1];
  // memset(tlv.data + pos + 2, 0, dataSize);

  // Add new field
  tlv.data[pos] = field;
  tlv.data[pos + 1] = data_len;
  memcpy(&tlv.data[pos + 2], p_data, data_len);

  /* Update the size of the tlv */
  if (pos == cfgHeader->tlvSize)
  {
    cfgHeader->tlvSize += data_len + 2;
  }

  return true;
}

/********************************************************************************
Function:
  cfgRemoveField()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Removes a field from config structure
Notes:
  ---
Author, Date:
  Toan Huynh, 05/09/2022
*********************************************************************************/
static bool cfgRemoveField(ConfigField field, uint16_t pos)
{
  if (pos > CONFIG_BUF_SIZE)
  {
    return false;
  }

  uint8_t filedSize = tlv.data[pos + 1] + 2;
  uint8_t temp[CONFIG_BUF_SIZE];

  // Copy the first part
  memcpy(temp, tlv.data, pos);

  // Copy the second part
  memcpy(temp + pos, tlv.data + pos + filedSize, CONFIG_BUF_SIZE - pos - filedSize);

  memcpy(tlv.data, temp, CONFIG_BUF_SIZE);

  cfgHeader->tlvSize -= filedSize;

  return true;
}
/********************************************************************************
Function:
  readData()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Reads all the data from the devStorage
Notes:
  ---
Author, Date:
  Toan Huynh, 03/03/2022
*********************************************************************************/
static bool readData(void)
{
  if (devStorageRead(0, buffer, CONFIG_BUF_SIZE))
  {
    return true;
  }
  else
  {
    sys_printf("CONFIG\t: Failed to read data from devStorage!\r\n");
    return false;
  }
}

/********************************************************************************
Function:
  write_crc()
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
static void write_crc(void)
{
  int i;
  uint8_t checksum = 0;

  for (i = 0; i < SIZE_HEADER + cfgHeader->tlvSize; i++)
  {
    checksum += buffer[i];
  }
  buffer[SIZE_HEADER + cfgHeader->tlvSize] = checksum;
}

/********************************************************************************
Function:
  check_crc()
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
static bool check_crc(void)
{
  int total_len = SIZE_HEADER + SIZE_TAIL + cfgHeader->tlvSize;
  uint8_t ref_checksum = buffer[total_len - SIZE_TAIL];
  int i;
  uint8_t checksum = 0;

  for (i = 0; i < SIZE_HEADER + cfgHeader->tlvSize; i++)
  {
    checksum += buffer[i];
  }
  if (checksum == ref_checksum)
  {
    return true;
  }
  else
  {
    sys_printf("CONFIG\t: devStorage configuration checksum not correct (0x%02X vs 0x%02X)!\r\n", ref_checksum, checksum);
    return false;
  }
}

/********************************************************************************
Function:
  check_magic()
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
static bool check_magic(void)
{
  return (cfgHeader->magic == MAGIC);
}

/********************************************************************************
Function:
  check_version()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Check if the version in storage is compatible with the current version
Notes:
  ---
Author, Date:
  Toan Huynh, 03/18/2022
*********************************************************************************/
static bool check_version(void)
{
  return (cfgHeader->majorVersion == DEV_STORAGE_MAJOR_VERSION &&
          cfgHeader->minorVersion == DEV_STORAGE_MINOR_VERSION);
}

/********************************************************************************
Function:
  check_content()
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
static bool check_content(void)
{
  bool status = false;

  do
  {
    if (!check_magic())
    {
      break;
    }

    if (cfgHeader->tlvSize >= (CONFIG_BUF_SIZE - SIZE_HEADER - SIZE_TAIL))
    {
      break;
    }

    if (!check_crc())
    {
      break;
    }

    if (!check_version())
    {
      break;
    }

    status = true;

  } while (0);

  return status;
}

/********************************************************************************
Function:
  write_defaults()
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
static bool write_defaults(void)
{
  memset(buffer, 0, CONFIG_BUF_SIZE);

  buffer[0] = MAGIC;
  buffer[1] = DEV_STORAGE_MAJOR_VERSION; // Major version
  buffer[2] = DEV_STORAGE_MINOR_VERSION; // Minor version
  buffer[3] = 0; // Length of TLV
  buffer[4] = 0; // Length of TLV
  buffer[5] = buffer[0] + buffer[1];

  // Clear all previous data
  devStorageClear();

  write_crc();

  if (!devStorageWrite(0, buffer, CONFIG_BUF_SIZE))
  {
    return false;
  }

  if (readData())
  {
    if (check_content())
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################

//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################

//###########################################################################################################
//      END OF dev_cfg.c
//###########################################################################################################
