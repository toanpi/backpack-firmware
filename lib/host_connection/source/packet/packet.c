/************************************************************************************************************
Module:       packet

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/28/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "packet.h"
#include "crc32.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define DEV_ADDR          (0x00)
#define FOOTER            {'e', 'n', 'd', '\n', '\r'}


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
static bool packet_verify(hc_package_t *p_package);



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  packet_parse_header()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Parse the header of the packet
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
hc_header_t *packet_parse_header(uint8_t *p_data, uint32_t len)
{
  if(len < sizeof(hc_header_t) || p_data == NULL)
  {
    return NULL;
  }

  hc_header_t *p_header = NULL;

  p_header = (hc_header_t *)p_data;

  return p_header;
}

/********************************************************************************
Function:
  packet_parse_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Parse the package of the packet
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
hc_package_t packet_parse_data(uint8_t *p_data, uint32_t len)
{
  hc_package_t package = {0};

  if(len < sizeof(hc_header_t) || p_data == NULL)
  {
    return package;
  }

  hc_header_t *p_header = NULL;

  p_header = packet_parse_header(p_data, len);
  package.p_header = p_header;
  package.p_data = p_data + sizeof(hc_header_t);
  package.data_len = len - sizeof(hc_header_t);

  if (package.data_len > p_header->package_len)
  {
    // Correct package length
    package.data_len = p_header->package_len;
  }

  package.total_len = packet_get_header_len() + package.data_len;
  package.package_len = package.total_len + packet_get_footer_len();

  if (!packet_verify(&package))
  {
    package.p_data = NULL;
    package.data_len = 0;
  }

  return package;
}

/********************************************************************************
Function:
  packet_get_header_len()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get the length of the header
Notes:
  ---
Author, Date:
  Toan Huynh, 05/01/2022
*********************************************************************************/
uint32_t packet_get_header_len(void)
{
  return sizeof(hc_header_t);
}

uint32_t packet_get_footer_len(void)
{
  uint8_t footer[] = FOOTER;
  return sizeof(footer);
}

/********************************************************************************
Function:
  packet_format_header()
Input Parameters:
  ---
Output Parameters:
  Return length of the header + data
Description:
  Format the header of the packet
Notes:
  ---
Author, Date:
  Toan Huynh, 05/01/2022
*********************************************************************************/
uint32_t packet_format_header(uint8_t *p_buf, uint32_t buf_len, uint32_t data_len)
{
  if (p_buf == NULL || buf_len < sizeof(hc_header_t))
  {
    return 0;
  }

  hc_header_t *p_header = (hc_header_t *)p_buf;

  p_header->desAddr = 0x00; // TODO: Set destination address
  p_header->srcAddr = DEV_ADDR; // TODO: Input source address
  p_header->package_len = data_len;
  p_header->crc32 = crc32_calc(CRC_START_NORMAL, data_len, p_buf + sizeof(hc_header_t));

  return sizeof(hc_header_t) + data_len;
}

/********************************************************************************
Function:
  packet_format_footer()
Input Parameters:
  ---
Output Parameters:
  Return the length of the footer
Description:
  Format the footer of the packet
Notes:
  ---
Author, Date:
  Toan Huynh, 05/01/2022
*********************************************************************************/
uint32_t packet_format_footer(uint8_t *buf, uint32_t buf_len, uint32_t data_len)
{
  uint32_t packet_len = sizeof(hc_header_t) + data_len;
  uint8_t footer[] = FOOTER;

  if (packet_len + sizeof(footer) > buf_len)
  {
    return 0;
  }

  memcpy(buf + packet_len, footer, sizeof(footer));

  return sizeof(footer);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  packet_verify()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Check if packet is valid
Notes:
  ---
Author, Date:
  Toan Huynh, 05/01/2022
*********************************************************************************/
static bool packet_verify(hc_package_t *p_package)
{
  bool ret = false;

  do
  {
    // Check length
    if (p_package->data_len == 0 || p_package->data_len != p_package->p_header->package_len)
    {
      break;
    }

    // Check CRC
    uint32_t crc = crc32_calc(CRC_START_NORMAL, p_package->data_len, p_package->p_data);
    if (crc != p_package->p_header->crc32)
    {
      break;
    }

    ret = true;

  } while (0);

  return ret;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF packet.c
//###########################################################################################################
