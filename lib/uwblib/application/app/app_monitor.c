/************************************************************************************************************
Module:       app_monitor

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
08/23/2023    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2023 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

// ###########################################################################################################
//       #INCLUDES
// ###########################################################################################################
#include "app_monitor.h"
#include "instance_utilities.h"

// ###########################################################################################################
//       TESTING #DEFINES
// ###########################################################################################################

// ###########################################################################################################
//       CONSTANT #DEFINES
// ###########################################################################################################
#define MAX_NUM_RANG_NODE (4)
#define MAX_CHANGE_CB (1)

// ###########################################################################################################
//       MACROS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE TYPES
// ###########################################################################################################

// ###########################################################################################################
//       CONSTANTS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE LEVEL VARIABLES
// ###########################################################################################################
static app_ranged_node_info_t ranged_info[MAX_NUM_RANG_NODE] = {0};
static uint32 num_ranged_devs = 0;

static app_monitor_change_cb changed_cbs[MAX_CHANGE_CB] = {0};
static uint32 reg_count = 0;

// ###########################################################################################################
//       PRIVATE FUNCTION PROTOTYPES
// ###########################################################################################################
static void changed_cb_call(void);

// ###########################################################################################################
//       PUBLIC FUNCTIONS
// ###########################################################################################################
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
bool app_monitor_register_change(app_monitor_change_cb func)
{
  if (reg_count < MAX_CHANGE_CB)
  {
    changed_cbs[reg_count++] = func;
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
  Toan Huynh, 08/23/2023
*********************************************************************************/
bool app_monitor_add_ranging_node(int node_index)
{
  if (node_index > 0 && node_index < UWB_LIST_SIZE && num_ranged_devs < MAX_NUM_RANG_NODE)
  {
    for (size_t i = 0; i < num_ranged_devs; i++)
    {
      if(ranged_info[i].index == node_index)
      {
        // Has already added --> Return here
        return true;
      }
    }

    ranged_info[num_ranged_devs++].index = node_index;
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
  Toan Huynh, 08/24/2023
*********************************************************************************/
bool app_monitor_ranging_start(void)
{
  num_ranged_devs = 0;
  memset(ranged_info, 0, sizeof(ranged_info));

  for (size_t i = 0; i < NUM_NODE_TO_RANGE; i++)
  {
    ranged_info[i].error = APP_UWB_UNKNOWN;
  }

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
  Toan Huynh, 08/24/2023
*********************************************************************************/
bool app_monitor_ranging_end(void)
{
  instance_data_t *inst = instance_get_local_structure_ptr(0);

  for (int i = 0; i < num_ranged_devs; i++)
  {
    uint8 uwb_idx = ranged_info[i].index;

    if (uwb_idx < UWB_LIST_SIZE && uwb_idx > 0)
    {
      ranged_info[i].addr = instance_get_uwbaddr(uwb_idx);

      const position_t *pos = get_node_position_by_idx(inst, uwb_idx);
      if(pos)
      {
        ranged_info[i].pos = *pos;
      }

      ranged_info[i].error = inst->error[uwb_idx];

      ranged_info[i].distance = 0;
      ranged_info[i].distance_rsl = 0;
  
      if(inst->error[uwb_idx] == APP_UWB_SUCCESS)
      {
        ranged_info[i].distance = instance_get_idist(uwb_idx);
        ranged_info[i].distance_rsl = instance_get_idistrsl(uwb_idx);
      }
    }
  }

  if(num_ranged_devs)
  {
    app_monitor_callback();
  }

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
  Toan Huynh, 08/24/2023
*********************************************************************************/
bool app_monitor_callback(void)
{
  // Changed callback
  changed_cb_call();

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
  Toan Huynh, 08/20/2023
*********************************************************************************/
const app_ranged_node_info_t *get_last_ranged_info(uint32_t *num_devs)
{
  if (num_devs)
  {
    *num_devs = num_ranged_devs;
  }

  return ranged_info;
}

// ###########################################################################################################
//       PRIVATE FUNCTIONS
// ###########################################################################################################
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
static void changed_cb_call(void)
{
  for (size_t i = 0; i < reg_count; i++)
  {
    changed_cbs[i]();
  }
}

// ###########################################################################################################
//       INTERRUPTS
// ###########################################################################################################

// ###########################################################################################################
//       TEST HARNESSES
// ###########################################################################################################

// ###########################################################################################################
//       END OF app_monitor.c
// ###########################################################################################################
