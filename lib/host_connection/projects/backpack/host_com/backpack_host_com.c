
/************************************************************************************************************
Module:       backpack_host_com

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/04/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "backpack_host_com.h"
#include "host_connection.h"
#include "backpack_parser.h"
#include "sensor_parser.h"
#include "uwb_dev_parser.h"
#include "file_transfer.h"
#include "cmd_parser.h"
#include "controller.h"




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
static bool host_conn_send(uint8_t *pData, uint32_t length);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  backpack_host_com_init()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/04/2022
*********************************************************************************/
void backpack_host_com_init(void)
{
  host_connection_init(host_conn_send, HOST_CONNECTION_MODE_PROTOBUF);

  /* For sensor data */
  sensor_parser_init();

  /* For uwb data */
  uwb_dev_parser_init();

  /* For file transfer */
  file_transfer_init();

  /* Command message */
  cmd_parser_init();

}

/********************************************************************************
Function:
  backpack_rx_process()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/04/2022
*********************************************************************************/
bool backpack_rx_process(uint8_t *p_data, uint32_t length)
{
  bool ret = false;

  if(!host_connection_parse(p_data, length, backpack_process_msg))
  {
    ret = true;
  }

  return ret;
}

/********************************************************************************
Function:
  backpack_send_fusion_data()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/04/2022
*********************************************************************************/
void backpack_send_fusion_data(void *p_fusion_data)
{
  host_connection_send(p_fusion_data, sizeof(p_fusion_data), sensor_encode_fusion_msg);
}

void backpack_send_position_data(void *p_data)
{
  host_connection_send(p_data, sizeof(p_data), sensor_encode_position_msg);
}

void backpack_send_sti_data(void)
{
  uint8_t dummy = 0;
  host_connection_send(&dummy, sizeof(dummy), sti_encode_state_info_msg);
}

void backpack_send_nav_state_data(void)
{
  uint8_t dummy = 0;
  host_connection_send(&dummy, sizeof(dummy), nav_encode_state_info_msg);
}

void backpack_send_dev_info(void)
{
  uint8_t dummy = 0;
  host_connection_send(&dummy, sizeof(dummy), sensor_encode_dev_info_msg);
}

/********************************************************************************
Function:
  backpack_stimulation_init_param()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 07/28/2022
*********************************************************************************/
bool backpack_stimulation_init_param(void)
{
  static bool is_init = false;

  if (!is_init)
  {
    is_init = true;
    return stimulation_init_param();
  }

  return is_init;
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
  Toan Huynh, 05/04/2022
*********************************************************************************/
static bool host_conn_send(uint8_t *pData, uint32_t length)
{
    return Controller_Send(pData, length);
}



//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF backpack_host_com.c
//###########################################################################################################
