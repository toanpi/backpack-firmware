/************************************************************************************************************
Module:       collector_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/03/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include <pb_encode.h>
#include "host_connection.h"
#include "algorithm.pb.h"
#include "backpack.pb.h"
#include "backpack_parser.h"
#include "proto_utilities.h"
#include "algo_parser.h"
#include "human_detection_thread.h"



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
static bool process_dev_msg(void *p_msg);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  collector_parser_init()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
void algo_parser_init(void)
{
   backpack_decode_register(backpackPkg_backpackMsg_algoMsg_tag, process_dev_msg);
}

/*********************************************************************************
Function:
  encode_collector_list_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Uwb anchor log encode by protobuf
Notes:
  ---
Author, Date:
  Toan Huynh, 04/15/2022
*********************************************************************************/
bool encode_algo_blob_detection_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
   algoPkg_algoMsg message = algoPkg_algoMsg_init_zero;
   uint32_t *ret = (uint32_t *)p_data;

   message.which_type = algoPkg_algoMsg_blobDetection_tag;

   message.type.blobDetection.resultX = ret[0];
   message.type.blobDetection.resultY = ret[1];

   return backpack_encode_algo_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  encode_collector_loss_result_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/11/2022
*********************************************************************************/
bool encode_algo_human_detection_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
   algoPkg_algoMsg message = algoPkg_algoMsg_init_zero;
   message.which_type = algoPkg_algoMsg_humanDetection_tag;

   human_etection_t *info = (human_etection_t *)p_data;

   message.type.humanDetection.hdResult = info->hdResult;
   message.type.humanDetection.hdProcessTimeUs = info->hdProcessTimeUs;
   message.type.humanDetection.captureIntervalMs = info->captureIntervalMs;
   message.type.humanDetection.camOnTimeMs = info->camOnTimeMs;
   message.type.humanDetection.camCircleTimeMs = info->camCircleTimeMs;

   return backpack_encode_algo_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  collector_process_dev_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
static bool process_dev_msg(void *p_msg)
{
   if (!p_msg)
   {
     return false;
   }

   algoPkg_algoMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.algoMsg);
   bool ret = true;

   switch (msg->which_type)
   {
   case algoPkg_algoMsg_humanDetection_tag:
     break;
   case algoPkg_algoMsg_blobDetection_tag:
     break;
   case algoPkg_algoMsg_hdConfig_tag:
      human_detection_algo_enable(msg->type.hdConfig.enable);
     break;
   default:
     ret = false;
     break;
   }

   return ret;
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
//      END OF collector_parser.c
//###########################################################################################################

