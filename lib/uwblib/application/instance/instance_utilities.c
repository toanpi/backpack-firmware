/************************************************************************************************************
Module:       instance_utilities

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/25/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "instance.h"
#include "instance_utilities.h"
#include "lib.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################
#define STRINGTIFY(name)               #name
#define ARRAY_STRING(name)             [name]=STRINGTIFY(name)


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



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  get_last_ranged_node_info()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/18/2022
*********************************************************************************/
void get_last_ranged_node_info(instance_data_t *inst, ranged_node_info_t info[4], uint32 * num_devs)
{
  const struct TDMAHandler *tdma_handler = get_tdma_handler();

  for (int i = 0; i < 4; i++)
  {
    if (tdma_handler->lastNodeRanged[i] > 0)
    {
      uint8 uwb_idx = tdma_handler->lastNodeRanged[i];
      info[i].addr = instance_get_uwbaddr(uwb_idx);
      
      const position_t * pos = get_node_position_by_idx(inst, uwb_idx);
      
      if(pos)
      {
        info[i].pos = *pos;
      }

      info[i].distance = instance_get_idist(uwb_idx);
      info[i].distance_rsl = instance_get_idistrsl(uwb_idx);
      if(num_devs)
      {
        (*num_devs)++;
      }
    }
  }
}

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
  Toan Huynh, 06/20/2022
*********************************************************************************/
bool instance_clear_ranged_node_info(void)
{
  const struct TDMAHandler *tdma_handler = get_tdma_handler();

  memset((void *)tdma_handler->lastNodeRanged, 0, sizeof(tdma_handler->lastNodeRanged));

  return true;
}

/********************************************************************************
Function:
  get_node_position_start_from_idx()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get node position from index
Notes:
  ---
Author, Date:
  Toan Huynh, 06/20/2022
*********************************************************************************/
const position_t *get_node_position_start_from_idx(instance_data_t *inst, const struct TDMAHandler *tdma_handler, uint8_t idx, uint32_t *node_idx)
{
  if (inst->uwbListLen == 0)
  {
    return NULL;
  }

  uint32_t i = idx;
  uint32_t stop_idx = idx;
  const position_t *pos = NULL;

  if (idx >= inst->uwbListLen || idx == 0)
  {
    /* Start from the second node because first node is this instance node */
    i = 1;
    stop_idx = 1;
  }

  /* Loop through the list */
  do
  {
    if (tdma_handler->uwbListTDMAInfo[i].connectionType == UWB_LIST_NEIGHBOR)
    {
      pos = get_node_position_by_idx(inst, i);
      if (pos)
      {
        if (node_idx)
        {
          *node_idx = i;
        }
        break;
      }
    }

    i++;

    if (i >= inst->uwbListLen)
    {
      i = 1; // Restart from the second node
    }
  } while (i != stop_idx);

  return pos;
}

/********************************************************************************
Function:
  get_inst_states_string()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get instance states string
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
const char *get_inst_states_string(enum inst_states state)
{
  const char *states[] = {
      STRINGTIFY(TA_INIT),
      STRINGTIFY(TA_TXPOLL_WAIT_SEND),
      STRINGTIFY(TA_TXFINAL_WAIT_SEND),
      STRINGTIFY(TA_TXRESPONSE_WAIT_SEND),
      STRINGTIFY(TA_TX_WAIT_CONF),
      STRINGTIFY(TA_RXE_WAIT),
      STRINGTIFY(TA_RX_WAIT_DATA),
      STRINGTIFY(TA_TXINF_WAIT_SEND),
      STRINGTIFY(TA_TXBLINK_WAIT_SEND),
      STRINGTIFY(TA_TXRANGINGINIT_WAIT_SEND),
      STRINGTIFY(TA_TX_SELECT),
      STRINGTIFY(TA_TXREPORT_WAIT_SEND),
      STRINGTIFY(TA_TXSUG_WAIT_SEND),
      STRINGTIFY(TA_SLEEP_DONE),
      STRINGTIFY(TA_TX_HOST_MSG_WAIT_SEND),
  };

  return states[state];
}
/********************************************************************************
Function:
  get_instanceModes_string()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get instance modes string
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
const char *get_instanceModes_string(enum instanceModes mode)
{
  const char *modes[] = {
      ARRAY_STRING(DISCOVERY),
      ARRAY_STRING(TAG),
      ARRAY_STRING(ANCHOR),
  };

  return mode <= NUM_MODES ? modes[mode] : "NONE";
}
/********************************************************************************
Function:
  get_discovery_modes_string()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get discovery modes string
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
const char *get_discovery_modes_string(enum discovery_modes mode)
{
  const char *dis_modes[] = {
      ARRAY_STRING(WAIT_INF_REG),
      ARRAY_STRING(COLLECT_INF_REG),
      ARRAY_STRING(WAIT_INF_INIT),
      ARRAY_STRING(WAIT_RNG_INIT),
      ARRAY_STRING(WAIT_SEND_SUG),
      ARRAY_STRING(SEND_SUG),
      ARRAY_STRING(EXIT),
      ARRAY_STRING(SLEEP_IN_DISCOVERY),
  };

  return dis_modes[mode];
}

/********************************************************************************
Function:
  get_msg_fcode_string()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get message fcode string
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
const char *get_msg_fcode_string(int fcode)
{
  const char *fcode_str[] = {
      ARRAY_STRING(RTLS_DEMO_MSG_RNG_INIT),
      ARRAY_STRING(RTLS_DEMO_MSG_TAG_POLL),
      ARRAY_STRING(RTLS_DEMO_MSG_ANCH_RESP),
      ARRAY_STRING(RTLS_DEMO_MSG_TAG_FINAL),
      ARRAY_STRING(RTLS_DEMO_MSG_INF_REG),
      ARRAY_STRING(RTLS_DEMO_MSG_INF_INIT),
      ARRAY_STRING(RTLS_DEMO_MSG_INF_SUG),
      ARRAY_STRING(RTLS_DEMO_MSG_INF_UPDATE),
      ARRAY_STRING(RTLS_DEMO_MSG_RNG_REPORT),
      ARRAY_STRING(RTLS_DEMO_MSG_SYNC),
      ARRAY_STRING(RTLS_HOST_MSG),
  };

  return fcode_str[fcode];
}

/********************************************************************************
Function:
  address64to16()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  NOTE could implement a hashing function to reduce chance of collisions among UWB addresses in network
  https://stackoverflow.com/questions/31710074/how-to-generate-smaller-unique-number-from-larger-11-bytes-unique-number-gene
  https://en.m.wikipedia.org/wiki/Pearson_hashing
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint16 address64to16(const uint8 *address)
{
	return address[0] + (address[1] << 8);
}

/********************************************************************************
Function:
  instance_getmessageduration_us()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Calculate message duration in microseconds
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 instance_getmessageduration_us(int data_length_bytes)
{
	instance_data_t *inst = instance_get_local_structure_ptr(0);

	// Compute the number of symbols for the given length.
	uint32 framelength_us = data_length_bytes * 8 + CEIL_DIV(data_length_bytes * 8, 330) * 48;
	// Convert from symbols to time and add PHY header length.
	if (inst->configData.dataRate == DWT_BR_110K)
	{
		framelength_us *= 820513;
		framelength_us += 17230800;
	}
	else if (inst->configData.dataRate == DWT_BR_850K)
	{
		framelength_us *= 102564;
		framelength_us += 2153900;
	}
	else
	{
		framelength_us *= 12821;
		framelength_us += 2153900;
	}
	// Last step: add preamble and SFD length and convert to microseconds.
	framelength_us += inst->storedPreLen;
	framelength_us = CEIL_DIV(framelength_us, 100000);

	return framelength_us;
}

/********************************************************************************
Function:
  instance_get_addr()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get own instance address
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint64 instance_get_addr(void)
{
	return instance_get_uwbaddr(0);
}

/********************************************************************************
Function:
  instance_get_uwbaddr()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get UWB address by device index
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
uint64 instance_get_uwbaddr(uint8 uwb_index)
{
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  uint64 x = 0;

  if (uwb_index < UWB_LIST_SIZE)
  {
    x |= (uint64)inst->uwbList[uwb_index][0];
    x |= (uint64)inst->uwbList[uwb_index][1] << 8;
#if (USING_64BIT_ADDR == 1)
    x |= (uint64)inst->uwbList[uwb_index][2] << 16;
    x |= (uint64)inst->uwbList[uwb_index][3] << 24;
    x |= (uint64)inst->uwbList[uwb_index][4] << 32;
    x |= (uint64)inst->uwbList[uwb_index][5] << 40;
    x |= (uint64)inst->uwbList[uwb_index][6] << 48;
    x |= (uint64)inst->uwbList[uwb_index][7] << 56;
#endif
  }

  return (x);
}


/********************************************************************************
Function:
	is_valid_position()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Checks if the position is valid.
Notes:
	---
Author, Date:
	Toan Huynh, 06/20/2022
*********************************************************************************/
bool is_valid_position(const position_t * position)
{
	position_t invalid_position;
	memset(&invalid_position, 0xFF, sizeof(position_t));

	if (memcmp(position, &invalid_position, sizeof(position_t)) == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}
/********************************************************************************
Function:
	get_node_position_by_idx()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Get a node position by index
Notes:
	---
Author, Date:
	Toan Huynh, 03/17/2022
*********************************************************************************/
const position_t *get_node_position_by_idx(instance_data_t *inst, uint8 idx)
{
	if (idx < inst->uwbListLen && is_valid_position(&inst->iPosition[idx]))
	{
		return (const position_t *)&inst->iPosition[idx];
	}
	else
	{
		return NULL;
	}
}

/********************************************************************************
Function:
	instance_set_position()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 03/17/2022
*********************************************************************************/
void instance_set_position(instance_data_t *isnt, float x, float y, float z)
{
	isnt->selfPosition.x = x;
	isnt->selfPosition.y = y;
	isnt->selfPosition.z = z;
}

/********************************************************************************
Function:
	is_tag()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Check if the node is a tag
Notes:
	---
Author, Date:
	Toan Huynh, 03/17/2022
*********************************************************************************/
bool is_tag(instance_data_t *inst, uint16 uwbIdx)
{
	if (uwbIdx < inst->uwbListLen)
	{
		uint16 shortAddr = *(uint16_t *)&inst->uwbList[uwbIdx];

		return (shortAddr <= MAX_TAG_ADDR_RANGE);
	}

	return false;
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
//      END OF instance_utilities.c
//###########################################################################################################
