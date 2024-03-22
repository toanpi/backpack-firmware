/************************************************************************************************************
Module:       host_msg

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/13/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include "host_msg.h"
#include "uwb_transfer.h"
#include "host_com_msg.h"
#include "instance.h"
#include "packet.h"
#include "backpack_parser.h"
#include "host_connection.h"
#include "instance_utilities.h"


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define HOST_MSG_BUFFER_SIZE  256
#define MAX_MSG_NODE          2



//###########################################################################################################
//      MACROS
//###########################################################################################################



//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################
typedef struct 
{
  uint8_t buffer[HOST_MSG_BUFFER_SIZE];
  uint32_t length;
  uint64_t last_time;
  uint32_t addr;
} msg_slot_t;

typedef struct 
{
  msg_slot_t slots[MAX_MSG_NODE];
  uint64_t last_time;
} msg_storage_t;



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static transfer_control_host_msg host_msg_handler = 
{
  .location = true,
  .ranging = false,
  // .ranging = true,
};

static msg_storage_t msg_storage = {0};
static uint32_t g_src_addr = BROADCAST_ADDRESS;

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static msg_slot_t * alloc_slot(msg_storage_t *p_msg_storage);
static bool free_slot(msg_slot_t * slot);
static msg_slot_t *find_slot(uint32_t addr, msg_storage_t *p_msg_storage);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/20/2023
*********************************************************************************/
bool host_msg_config_set(transfer_control_host_msg *cfg)
{
  if(cfg)
  {
    host_msg_handler = *cfg;
    return true;
  }
  return false;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/26/2023
*********************************************************************************/
bool host_msg_config_ranging_transfer(bool enable_rang_info)
{
  host_msg_handler.ranging = enable_rang_info;
  return true;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/23/2023
*********************************************************************************/
transfer_control_host_msg * host_msg_config_get(void)
{
  return &host_msg_handler;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  Store message in storage then deliver it to node when node waked up and connected.
Author, Date:
  Toan Huynh, 11/14/2023
*********************************************************************************/
bool host_msg_process_host_msg(uint32_t node_addr,uint8_t  *p_data, uint32_t len)
{
  /* Get slot from storage */
  msg_slot_t *slot = find_slot(node_addr, &msg_storage);

  if(!slot)
  {
    slot = alloc_slot(&msg_storage);
  }

  /* Store message while waiting for destination node to get message later */
  /* NOTE: Only one message per slot for one node address for now --> Improve if need */
  if (slot && p_data && len < HOST_MSG_BUFFER_SIZE)
  {
    memcpy(slot->buffer, p_data, len);
    slot->length = len;
    slot->addr = node_addr;

    return true;
  }

  return false;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  Send host message to node if found in storage
Author, Date:
  Toan Huynh, 11/14/2023
*********************************************************************************/
bool host_msg_forward_msg_to_node(uint32_t node_addr)
{
  msg_slot_t *slot = find_slot(node_addr, &msg_storage);

  if(!slot || slot->length == 0)
  {
    return false;
  }

  instance_data_t *inst = instance_get_local_structure_ptr(0);
  
  bool ret = inst_net_send_msg(inst, slot->buffer, slot->length, node_addr);

  /* Clear buffer for next message */
  free_slot(slot);

  return ret;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/17/2023
*********************************************************************************/
bool host_msg_find_msg(uint32_t node_addr)
{
  msg_slot_t *slot = find_slot(node_addr, &msg_storage);
  return slot && slot->length > 0;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/14/2023
*********************************************************************************/
static bool send_data(uint8_t *p_data, uint32_t len)
{
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  uint32_t des_addr = g_src_addr; // destination address

  return inst_net_send_msg(inst, p_data, len, des_addr);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/17/2023
*********************************************************************************/
bool host_msg_parse_msgs(instance_data_t *inst, uint32_t src_addr, uint8_t *data, uint32_t len)
{
  if (!data || len == 0)
  {
    return false;
  }

  uint32_t pos = 0;
  hc_package_t package = {0};
  uint32_t inst_addr = (uint32_t) instance_get_addr();

  // Message can be split into multiple packages
  // Format: [header][data][footer][header][data][footer]...
  do
  {
    uint8_t *p_data = &data[pos];
    package = packet_parse_data(p_data, len - pos);

    if(package.p_data == NULL || package.total_len == 0)
    {
      break;
    }

    uint32_t des_addr = package.p_header->desAddr;

    // Process message if it is for this instance by checking destination address
    if (des_addr == inst_addr)
    {
      g_src_addr = src_addr;

      // Process message by backpack parser
      host_connection_process(p_data, package.total_len, backpack_process_msg, send_data);

      // Restore default
      g_src_addr = BROADCAST_ADDRESS;
    }
    else if (des_addr == HOST_ADDR)
    {
      /* Forward broadcast message to host */
      /* TODO: Only anchor can forward this message for now */
      // TODO: Check if this device is linked to host before sending
      if (inst->operationMode == CONFIG_OPERATION_ANCHOR)
      {
        host_com_send_signal(p_data, package.package_len);
      }
    }

    pos += package.package_len;

  } while (pos < len && package.data_len && package.p_data);

  return true;
}

/********************************************************************************
Function:
  inst_send_host_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Send a message to the host.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/12/2022
*********************************************************************************/
bool inst_send_host_msg(instance_data_t *inst, uint32 *p_timeout)
{
  bool ret = false;
  uint8_t buffer[256] = {0};
  uint32 buffer_len = sizeof(buffer);

  uint32 packet_len = host_com_encode_net_info(buffer, buffer_len, (uint8_t *)inst, sizeof(*inst));

  if (packet_len > 0 && host_msg_handler.location)
  {
    /* send the packet to the host */
    host_com_write(buffer, packet_len);

    ret = inst_net_broadcast_msg(inst, buffer, packet_len);

    if (p_timeout)
    {
      *p_timeout = uwb_get_tx_timeout_ms(5000, packet_len);
    }
  }

  return ret;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/14/2023
*********************************************************************************/
bool inst_net_broadcast_msg(instance_data_t *inst, uint8_t *p_data, uint32_t data_len)
{
  return inst_net_send_msg(inst, p_data, data_len, BROADCAST_ADDRESS);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  Broadcast the message to the network 
Author, Date:
  Toan Huynh, 11/14/2023
*********************************************************************************/
bool inst_net_send_msg(instance_data_t *inst, uint8_t *p_data, uint32_t data_len, uint16_t des_addr)
{
  if(!inst || !p_data || data_len == 0)
  {
    return false;
  }

  // configure destination address
  memcpy(&inst->host_msg.destAddr[0], &des_addr, 2);

  /* Payload data */
  memcpy(&inst->host_msg.messageData[HOST_MSG_PAYLOAD_IDEX], p_data, data_len);

  /* Message length */
  uint16_t psdu_len = 1 + data_len + FRAME_CRTL_AND_ADDRESS_S + FRAME_CRC;

  inst->host_msg.seqNum = inst->frameSN++;

  /* Send the message to the network */
  bool ret = uwb_send_msg((uint8 *)&inst->host_msg, psdu_len, DWT_START_TX_IMMEDIATE, 0);

  // reconfigure destination default address as broadcasr address 
  uint16 broadcast_address = BROADCAST_ADDRESS;
  memcpy(&inst->host_msg.destAddr[0], &broadcast_address, 2);

  return ret;
}

/********************************************************************************
Function:
	send_device_net_info()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 05/01/2022
*********************************************************************************/
void send_device_raning_info(void)
{
  if(host_msg_handler.ranging)
  {
    host_com_send_ranging_info();
  }
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 08/24/2023
*********************************************************************************/
void send_device_net_info(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	host_com_send_net_info((uint8_t *)inst, sizeof(*inst));
}

/********************************************************************************
Function:
	send_device_state_info()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 05/05/2022
*********************************************************************************/
void send_device_state_info(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	host_com_send_dev_info((uint8_t *)inst, sizeof(*inst));
}

/********************************************************************************
Function:
	send_dev_config()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Send device configuration to host
Notes:
	---
Author, Date:
	Toan Huynh, 05/01/2022
*********************************************************************************/
void send_dev_config(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
	host_com_send_dev_config((uint8_t *)inst, sizeof(*inst));
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/13/2023
*********************************************************************************/
void host_msg_send_neighbor_list(void)
{
	host_com_send_neighbor_list();
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/18/2023
*********************************************************************************/
void host_msg_send_host_setup(void)
{
	host_com_send_host_setup();
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/18/2023
*********************************************************************************/
void host_msg_send_test_msg_res(uint32_t arg)
{
	host_com_send_test_msg_res(arg);
}

/********************************************************************************
Function:
  send_dev_calib_info()
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
void send_dev_calib_info(void)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);
  host_com_send_dev_calib((uint8_t *)inst, sizeof(*inst));
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  Function to update the array in most recently used of storage slot
  Get free or last used slot
Author, Date:
  Toan Huynh, 11/15/2023
*********************************************************************************/
static msg_slot_t * alloc_slot(msg_storage_t *p_msg_storage)
{
  uint64_t earliest_time = UINT64_MAX;
  msg_slot_t *earliest_slot = NULL;

  // Find free or ealiest used slot
  for (uint32_t i = 0; i < MAX_MSG_NODE; i++)
  {
    msg_slot_t *slot = &p_msg_storage->slots[i];

    if (slot->last_time < earliest_time)
    {
      earliest_time = slot->last_time;
      earliest_slot = slot;
    }
  }

  // Store last time for usuage
  earliest_slot->last_time = ++p_msg_storage->last_time;

  return earliest_slot;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/16/2023
*********************************************************************************/
static bool free_slot(msg_slot_t * slot)
{
  slot->last_time = 0;
  slot->length = 0;
  slot->addr = UINT32_MAX; // Invalid address

  return true;
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/16/2023
*********************************************************************************/
static msg_slot_t *find_slot(uint32_t addr, msg_storage_t *p_msg_storage)
{
  for(uint32_t i = 0; i < MAX_MSG_NODE; i++)
  {
    msg_slot_t *slot = &p_msg_storage->slots[i];

    if(slot->addr == addr)
    {
      return slot;
    }
  }

  return NULL;
}
//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF host_msg.c
//###########################################################################################################
