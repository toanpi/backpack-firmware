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
#include "collector.pb.h"
#include "backpack.pb.h"
#include "backpack_parser.h"
#include "proto_utilities.h"
#include "collector_parser.h"
#include "collector_host_com.h"
#include "collector.h"




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
static bool uwb_process_indetify_msg(void *p_msg);



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
void collector_parser_init(void)
{
   backpack_decode_register(backpackPkg_backpackMsg_collectorMsg_tag, collector_process_dev_msg);
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
bool encode_collector_list_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  collectorPkg_collectorMsg message = collectorPkg_collectorMsg_init_zero;
  uint16_t max_size = sizeof(message.type.devList.address) / sizeof(uint32_t);

  message.which_type = collectorPkg_collectorMsg_devList_tag;
  
  message.type.devList.address_count = Cllc_copyDevList(message.type.devList.address, max_size);

  message.type.devList.addrBlockSize = MAX_NUM_CHILDREN_ADDR;

  return backpack_encode_collector_msg(p_buf, buf_size, p_written_len, &message);
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
bool encode_collector_loss_result_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  collectorPkg_collectorMsg message = collectorPkg_collectorMsg_init_zero;
  message.which_type = collectorPkg_collectorMsg_lossTestResult_tag;
  loss_test_result_t *p_result = (loss_test_result_t *)p_data;

  message.type.lossTestResult.nodeAddr = p_result->nodeAddr;
  message.type.lossTestResult.numPacketSent = p_result->numPacketSent;
  message.type.lossTestResult.numPacketReceived = p_result->numPacketReceived;
  message.type.lossTestResult.success = p_result->success;

  return backpack_encode_collector_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  encode_collector_loss_progress_msg()
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
bool encode_collector_loss_progress_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  collectorPkg_collectorMsg message = collectorPkg_collectorMsg_init_zero;
  message.which_type = collectorPkg_collectorMsg_lossTestProgress_tag;

  loss_test_progress_t *p_progress = (loss_test_progress_t *)p_data;
  message.type.lossTestProgress.packetNum = p_progress->packetNum;
  message.type.lossTestProgress.success = p_progress->success;

  return backpack_encode_collector_msg(p_buf, buf_size, p_written_len, &message);
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
bool collector_process_dev_msg(void *p_msg)
{
  if (!p_msg)
  {
    return false;
  }

  collectorPkg_collectorMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.collectorMsg);
  bool ret = false;

  switch (msg->which_type)
  {
  case collectorPkg_collectorMsg_reqList_tag:
    clt_host_com_list_dev();
    break;
  case collectorPkg_collectorMsg_startLossTest_tag:
  {
    collectorPkg_startLossTestMsg *cfg = &msg->type.startLossTest;
    Sc_startTestPacketLoss(cfg->packetNum, cfg->packetSize, cfg->period);
  }
    break;
  case collectorPkg_collectorMsg_reqStopLossTest_tag:
    Sc_stopTestPacketLoss();
    break;
  case collectorPkg_collectorMsg_identify_tag:
    uwb_process_indetify_msg(&msg->type.identify);
    break;
  case collectorPkg_collectorMsg_blink_tag:
    Collector_sendAllNodeRequest(Smsgs_cmdIds_toggleLedReq);
    break;
  }

  return ret;
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
  Toan Huynh, 05/11/2022
*********************************************************************************/
static bool uwb_process_indetify_msg(void *p_msg)
{
  collectorPkg_identifyMsg *msg = (collectorPkg_identifyMsg *)p_msg;

  ApiMac_sAddr_t pDstAddr = {
      .addrMode = ApiMac_addrType_short,
      .addr.shortAddr = msg->address};

  Collector_sendNodeRequest(Smsgs_cmdIds_toggleLedReq, &pDstAddr);
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF collector_parser.c
//###########################################################################################################
