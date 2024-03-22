/************************************************************************************************************
Module:       location_engine

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/12/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "location_engine.h"
#include "instance_config.h"
#include "instance.h"
#include "positioning_calculation.h"
#include "location_smoothing.h"
#include "timestamp.h"



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



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  loc_engine_run()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Perform the location engine
Notes:
  ---
Author, Date:
  Toan Huynh, 07/12/2022
*********************************************************************************/
bool loc_engine_run(void)
{
  instance_data_t *inst = instance_get_local_structure_ptr(0);
  ranged_node_info_t info[4] = {0};
  uint32 num_devs = 0;
  int use_4th_node = 1;

  get_last_ranged_node_info(inst, info, &num_devs);
  //for (uint8_t count = 0; count < 4; count ++){
  //  sys_printf("B 0x%x: RSL %.3f\r\n", (uint16_t)(info[count].addr), (double)(info[count].distance_rsl));
  //}
  for (uint8_t count = 0; count < 4; count++)
  {
    BeaconID[count] = info[count].addr;
    distanceArray[count] = info[count].distance_rsl;
    // distanceArray[count] = info[count].distance;
    anchorArray[count].x = info[count].pos.x;
    anchorArray[count].y = info[count].pos.y;
    anchorArray[count].z = info[count].pos.z;
  }
  // distance_conditioning(&distanceArray);
  /* Check repeat detail. Ideally, 3 unique results must be at slot 0, 1, 2
  So we test if 1 == 0 or 2 == 0 > swap 0 and 3
  if 1 == 2 > swap 2 and 3
  if 3 == 0/1/2 no need to swap but flag
  */
  if (BeaconID[0] == BeaconID[1] || BeaconID[0] == BeaconID[2])
  {
    // Flag
    use_4th_node = 0;
    // Swap 0 and 3 ID;
    uint64_t tempID;
    tempID = BeaconID[0];
    BeaconID[0] = BeaconID[3];
    BeaconID[3] = tempID;
    // Swap 0 and 3
    float temp;
    temp = distanceArray[0];
    distanceArray[0] = distanceArray[3];
    distanceArray[3] = temp;
    temp = anchorArray[0].x;
    anchorArray[0].x = anchorArray[3].x;
    anchorArray[3].x = temp;
    temp = anchorArray[0].y;
    anchorArray[0].y = anchorArray[3].y;
    anchorArray[3].y = temp;
    temp = anchorArray[0].z;
    anchorArray[0].z = anchorArray[3].z;
    anchorArray[3].z = temp;
  }
  else if (BeaconID[1] == BeaconID[2])
  {
    // Flag
    use_4th_node = 0;
    // Swap 1 and 3 ID;
    uint64_t tempID;
    tempID = BeaconID[1];
    BeaconID[1] = BeaconID[3];
    BeaconID[3] = tempID;
    // Swap 1 and 3
    float temp;
    temp = distanceArray[1];
    distanceArray[1] = distanceArray[3];
    distanceArray[3] = temp;
    temp = anchorArray[1].x;
    anchorArray[1].x = anchorArray[3].x;
    anchorArray[3].x = temp;
    temp = anchorArray[1].y;
    anchorArray[1].y = anchorArray[3].y;
    anchorArray[3].y = temp;
    temp = anchorArray[1].z;
    anchorArray[1].z = anchorArray[3].z;
    anchorArray[3].z = temp;
  }
  else if (BeaconID[3] == BeaconID[0] || BeaconID[3] == BeaconID[1] || BeaconID[3] == BeaconID[2])
  {
    use_4th_node = 0;
  }
  // Run the engine
  GetLocation(&UWBPosition, use_4th_node, &anchorArray, &distanceArray);
  // Store the position data to node information instance
  position_t tempPos;
  tempPos.x = UWBPosition.x;
  tempPos.y = UWBPosition.y;
  tempPos.z = UWBPosition.z;

  /* Smooth raw position data */
  if (inst->enSmoothingAlgo)
  {
    tempPos = uwb_smoothing(inst->uwbShortAdd, tempPos, timestamp_get_ms());
  }

  inst_config_position(inst, &tempPos, NULL);

  return true;
}
void distance_conditioning (float *distance_array)
{
  for (int i = 0; i < 4; i++)
  {
    if (distance_array[i] < 1.7)
      distance_array[i] = 1.7;    // Mininum distance selected
    else if (distance_array[i] < 2.0)
      distance_array[i] *= 1.1;   // Extend the distance
    else if (distance_array[i] < 3.0)
      distance_array[i] *= 1.0;   // Maintain
    else if (distance_array[i] < 4.0)
      distance_array[i] *= 0.91;  // Divide by 1.1
    else if (distance_array[i] < 4.5)
      distance_array[i] *= 0.87;  // Divide by 1.15
    else if (distance_array[i] < 4.7)
      distance_array[i] *= 0.83;  // Divide by 1.2
    else
      distance_array[i] = 4.7;    // Maximum value allowed    
  }
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
//      END OF location_engine.c
//###########################################################################################################
