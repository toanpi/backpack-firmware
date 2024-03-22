/************************************************************************************************************
Module:       location_smoothing

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
12/10/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

// ###########################################################################################################
//       #INCLUDES
// ###########################################################################################################
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "instance.h"
#include "timestamp.h"
#include "instance_utilities.h"

// ###########################################################################################################
//       TESTING #DEFINES
// ###########################################################################################################

// ###########################################################################################################
//       CONSTANT #DEFINES
// ###########################################################################################################
#define MAX_DEVS                    1

// ###########################################################################################################
//       MACROS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE TYPES
// ###########################################################################################################
typedef struct
{
  uint32 address;
  position_t position;
  uint32 timestamp;
  bool is_user_configured;  // Show that the data is configed at the first time
} coordinator_t;

// ###########################################################################################################
//       CONSTANTS
// ###########################################################################################################

// ###########################################################################################################
//       MODULE LEVEL VARIABLES
// ###########################################################################################################
static uint32 Speed_Max_CM_S = 3; // cm/s
static float E1 = 0.05;
static coordinator_t coordinate_man[MAX_DEVS] = {0};

// ###########################################################################################################
//       PRIVATE FUNCTION PROTOTYPES
// ###########################################################################################################
static coordinator_t *get_dev_info(uint32 addr);
static coordinator_t *get_free_slot(uint32 addr);
static coordinator_t *setup_start_point(uint32 address, position_t coordinate, uint32 timestamp);
static void set_start_point(uint32 address, position_t position, uint32 timestamp, bool is_user_configured);

// ###########################################################################################################
//       PUBLIC FUNCTIONS
// ###########################################################################################################
/********************************************************************************
Function:
  location_smoothing_init()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Init location smoothing
Notes:
  ---
Author, Date:
  Toan Huynh, 12/10/2022
*********************************************************************************/
void location_smoothing_init(void)
{
  memset(coordinate_man, 0xFF, sizeof(coordinate_man));
}

/*******************************************************************************
Function:
  uwb_smoothing()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Smoothing uwb coordinate data
Notes:
  ---
Author, Date:
  Toan Huynh, 07/22/2022
*******************************************************************************/
position_t uwb_smoothing(uint32 address, position_t coordinate, uint32 timestamp)
{
  coordinator_t *pre_point = setup_start_point(address, coordinate, timestamp);

  /* No previous point found, wait for next */
  if (!pre_point)
  {
    /* Return setup start point */
    coordinator_t *cur_point = get_dev_info(address);
    return cur_point ? cur_point->position : coordinate;
  }

  position_t last_coordinate = pre_point->position;
  float x = coordinate.x;
  float y = coordinate.y;

  uint32 duration_s = 2;
  uint32 cur_timestamp = timestamp;

  duration_s = get_dt32(pre_point->timestamp, cur_timestamp) / 1000;

  float e1 = E1;
  float max_length = ((float)Speed_Max_CM_S * duration_s) / 100;

  float x_distance = x - last_coordinate.x;
  float y_distance = y - last_coordinate.y;

  float distance = sqrt(x_distance * x_distance + y_distance * y_distance);
  float angle = atan2(y_distance, x_distance);

  float x_speed_limit = x;
  float y_speed_limit = y;

  if (distance > max_length)
  {
    x_speed_limit = max_length * cos(angle) + last_coordinate.x;
    y_speed_limit = max_length * sin(angle) + last_coordinate.y;
  }

  float x_new = e1 * x + (1 - e1) * x_speed_limit;
  float y_new = e1 * y + (1 - e1) * y_speed_limit;

  position_t new_coordinate = {
    x : x_new,
    y : y_new,
    z : coordinate.z,
  };

  /* Store new coordinate, it's used as previous point for next event */
  set_start_point(address, new_coordinate, cur_timestamp, false);

  return new_coordinate;
}

/*******************************************************************************
Function:
  uwb_insidesquare()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Bring UWB data inside each square
Notes:
  ---
Author, Date:
  Davis Nguyen, 09/207/2022
*******************************************************************************/
position_t uwb_insidesquare(position_t *coordinate)
{
  float x = floor(100 * coordinate->x);
  float y = floor(100 * coordinate->y);

  float x_new = 10 * (floor(x / 10)) + 5;
  float y_new = 10 * (floor(y / 10)) + 5;

  position_t new_coordinate = {
    x : x_new / 100,
    y : y_new / 100,
    z : coordinate->z,
  };

  return new_coordinate;
}

/*******************************************************************************
Function:
  smooth_config_coefficient()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Configure coefficient
Notes:
  ---
Author, Date:
  Toan Huynh, 07/22/2022
*******************************************************************************/
void smooth_config_coefficient(uint32 max_speed_cm_s, float e1)
{
  Speed_Max_CM_S = max_speed_cm_s;
  E1 = e1;
}

/********************************************************************************
Function:
  smooth_get_coefficient()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Ger current coefficient
Notes:
  ---
Author, Date:
  Toan Huynh, 12/12/2022
*********************************************************************************/
void smooth_get_coefficient(uint32 *max_speed_cm_s, float *e1)
{
  if (max_speed_cm_s)
  {
    *max_speed_cm_s = Speed_Max_CM_S;
  }

  if (e1)
  {
    *e1 = E1;
  }
}

/********************************************************************************
Function:
  user_set_start_point()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  User setup start point
Notes:
  ---
Author, Date:
  Toan Huynh, 12/12/2022
*********************************************************************************/
void user_set_smooth_start_point(uint32 address, position_t position)
{
  // No need timestamp 
  return set_start_point(address, position, 0, true);
}

// ###########################################################################################################
//       PRIVATE FUNCTIONS
// ###########################################################################################################

/*******************************************************************************
Function:
  set_start_point()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Set start point
Notes:
  ---
Author, Date:
  Toan Huynh, 07/22/2022
*******************************************************************************/
static void set_start_point(uint32 address, position_t position, uint32 timestamp, bool is_user_configured)
{
  coordinator_t *point = get_free_slot(address);

  if (point)
  {
    point->address = address;
    point->timestamp = timestamp;
    point->position.x = position.x;
    point->position.y = position.y;
    point->position.z = position.z;
    point->is_user_configured = is_user_configured;
  }
}

/*******************************************************************************
Function:
  setup_start_point()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Setup start point
Notes:
  ---
Author, Date:
  Toan Huynh, 07/24/2022
*******************************************************************************/
static coordinator_t *setup_start_point(uint32 address, position_t position, uint32 timestamp)
{
  coordinator_t *pre_point = NULL;
  bool is_first = false;
  coordinator_t *point = get_dev_info(address);

  if (!point || !is_valid_position(&point->position))
  {
    /* This is first point -> No start point found */
    set_start_point(address, position, timestamp, false);
    /* This is the first point */
    is_first = true;
  }

  /* Because that start point from user interface don't have timestamp */
  /* So we need to set timestamp here from the first coordinate came from node */
  if (point && point->is_user_configured)
  {
    point->timestamp = timestamp;
    point->is_user_configured = false;
    /* This is the first point */
    is_first = true;
  }

  if (!is_first)
  {
    pre_point = point;
  }

  return pre_point;
}

/********************************************************************************
Function:
  getDevInfo()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Return coordinator of given address
Notes:
  ---
Author, Date:
  Toan Huynh, 12/10/2022
*********************************************************************************/
static coordinator_t *get_dev_info(uint32 addr)
{
  coordinator_t *p = get_free_slot(addr);

  return (p && p->address == addr) ? p : NULL;
}

/********************************************************************************
Function:
  get_free_slot()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Return address slot if it's existing otherwise return free slot
Notes:
  ---
Author, Date:
  Toan Huynh, 12/10/2022
*********************************************************************************/
static coordinator_t *get_free_slot(uint32 addr)
{
  coordinator_t *free = NULL;

  for (uint32 i = 0; i < MAX_DEVS; i++)
  {
    if (coordinate_man[i].address == addr)
    {
      return &coordinate_man[i];
    }
    else if (!free && coordinate_man[i].address > 0xFFFF)
    {
      free = &coordinate_man[i];
    }
  }

  return free;
}

// ###########################################################################################################
//       PRIVATE FUNCTIONS
// ###########################################################################################################

// ###########################################################################################################
//       INTERRUPTS
// ###########################################################################################################

// ###########################################################################################################
//       TEST HARNESSES
// ###########################################################################################################

// ###########################################################################################################
//       END OF location_moothing.c
// ###########################################################################################################
