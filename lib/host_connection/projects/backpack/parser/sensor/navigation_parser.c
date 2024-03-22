/************************************************************************************************************
Module:       navigation_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
09/26/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include <math.h>
#include <pb_encode.h>
#include "backpack.pb.h"
#include "sensor_parser.h"
#include "backpack_parser.h"
#include "controller.h"
#include "system_utils.h"
#include "backpack_host_com.h"



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define H_PI                  3.14159265358979f



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
// ---------------------- VARIABLES FOR HUMAN DETECTION  ------------------------ //
extern volatile  float Seeking_HumanDetectionScore;
extern volatile  bool Seeking_IsOnboardHumanDetection;
extern volatile  bool Seeking_IsOnboardBlobDetection;
extern volatile  uint8_t Seeking_BlobResult[2];
extern volatile  float Seeking_Alpha, Seeking_Gama, Seeking_Beta;
extern volatile  float Seeking_Step1, Seeking_Step2;
extern volatile  float Seeking_MinTemp, Seeking_MaxTemp, Seeking_MaxMedian;
extern volatile  uint32_t Seeking_NumberPixel, Seeking_NumberPixelInRange, Seeking_IRCapture;
extern volatile  uint32_t HumanDetection_NumberPixel;
extern volatile  bool Seeking_IsPhaseI;
extern volatile float Seeking_XMin;
extern volatile float Seeking_XMax;
extern volatile float Seeking_YMin;
extern volatile float Seeking_YMax;
extern volatile float Seeking_LevyWalk_LengthMax;
extern volatile float Seeking_LevyWalk_LengthMin;
extern volatile bool  Seeking_FirstLoop;
extern volatile bool  Seeking_IsPhaseIII;
extern volatile float Seeking_AdditionalTargetPhase3;
// ------------------------ VARIABLES FOR AUTOMATIC NAVIGATION ---------------- //
// Note: Due to the inherit from LevyWalk, all variables are named started with "LevyWalk"
extern volatile uint32_t LevyWalk_ControlRate;
extern volatile bool     LevyWalk_TimeForExecute;
extern volatile uint32_t LevyWalk_ControlCount;
extern volatile uint32_t LevyWalk_Turn;
extern volatile uint32_t LevyWalk_TurnPrevious;
extern volatile float LevyWalk_Target[16];
extern volatile float LevyWalk_Target_Current[2];
extern volatile uint32_t LevyWalk_Tagert_Order;
extern volatile float LevyWalk_RobotPos[4];
extern volatile float LevyWalk_AngleThreshold;
extern volatile float LevyWalk_DistanceThreshold;
extern volatile uint32_t LevyWalk_ReactionTime;
extern volatile uint32_t LevyWalk_ThresholdOfLongStimulus;
extern volatile uint32_t LevyWalk_ThresholdOfLongStimulusForward;
extern volatile uint32_t LevyWalk_ThresholdOfLongStimulusFreeWalk;
extern volatile uint32_t LevyWalk_DurationToCheckSpeed;
extern volatile uint32_t LevyWalk_DurationToCheckSpeed_FreeWalk;
extern volatile uint32_t LevyWalk_DurationToCheckSpeed_Stimulation;
extern volatile float    LevyWalk_ThresholdOfForwardVelocity;
extern volatile float    LevyWalk_ThresholdOfAngularVelocity;
extern volatile float    LevyWalk_TimeStoreToCalculateSpeed;
extern volatile uint32_t LevyWalk_ThresholdOfTimeForwardWhenSpeedIsSmall;
extern volatile uint32_t LevyWalk_ThresholdOfTimeFreeWalkWhenSpeedIsSmall;
extern volatile float    LevyWalk_SpeedArray_X[300];
extern volatile float    LevyWalk_SpeedArray_Y[300];
extern volatile float    LevyWalk_SpeedArray_XHead[300];
extern volatile float    LevyWalk_SpeedArray_YHead[300];
extern volatile uint32_t LevyWalk_SpeedArray_T[300];
extern volatile uint32_t LevyWalk_SpeedCount;
extern volatile bool     LevyWalk_SpeedFlag_Linear;
extern volatile bool     LevyWalk_SpeedFlag_Angular;
extern volatile float    LevyWalk_ForwardVelocity;
extern volatile float    LevyWalk_AngularVelocity;
extern volatile float    LevyWalk_ForwardVelocity_Vicon;
extern volatile float    LevyWalk_AngularVelocity_Vicon;
extern volatile float    LevyWalk_ForwardVelocity_IMU;
extern volatile float    LevyWalk_AngularVelocity_IMU;
extern volatile bool     LevyWalk_UseIMU_LinearSpeed;
extern volatile bool     LevyWalk_UseIMU_AngularSpeed;
extern volatile uint32_t LevyWalk_AngularSpeedMethod;
extern volatile bool     LevyWalk_UseEscapeMode;
extern volatile bool  LevyWalk_IsReached;
extern volatile bool  LevyWalk_IsTheRoachConfined;
// Variable for handler the reduction of stimulation
extern volatile bool     LevyWalk_EnableTheFreeWalkingAfterStimulation;
extern volatile uint32_t LevyWalk_ThresholdForFreeWalkAfterSteering;
extern volatile uint32_t LevyWalk_ThresholdofFinalPartForLongStimulus;
// Variable to reset navigation
extern volatile bool LevyWalk_IsLongStimulusHandleStarted; // The variable to indicate that long stimulus is being handled
extern volatile bool LevyWalk_IsLongStimulusBeingChecked;  // The variable to indicate that long stimulus is being monitored
extern volatile bool LevyWalk_IsNavigated;                 // Enable navigation process
extern volatile bool LevyWalk_IsForwardTriggeredAlready;   // The variable to indicate that forward stimulation has been already started
extern volatile bool LevyWalk_ShouldCheckTheForwardSpeed;
extern volatile uint32_t LevyWalk_TimeafterForwardStimulus;
extern volatile uint32_t LevyWalk_TimeofStimulusForSteering;
extern volatile uint32_t LevyWalk_TimeafterLongStimulus;
extern volatile uint32_t LevyWalk_TimeafterAngleSmallerThanThreshold;
extern volatile uint32_t LevyWalk_TimeCounterForFreeWalkAfterSteering;
extern  volatile bool LevyWalk_StartCoutingTimeForFreeWalkAfterSteering;
// Variable to count the number of zero velocity
extern  volatile uint32_t LevyWalk_NumberAngularSpeed;
extern  volatile uint32_t LevyWalk_NumberLinearSpeed;

extern volatile bool     StimulationStartStop;
extern volatile bool     StimulationTrigger;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool stop_nav(void);
static bool update_nav_param(navigationPkg_paramMsg *cfg);
static bool update_target(navigationPkg_target *cfg);
static bool stop_nav(void);
static bool load_state(void);
static bool update_human_seeking_param(navigationPkg_seeking *msg);
static bool auto_navigation(navigationPkg_autoNav *cfg);
extern void levywalk_signal(void);
extern float LevyWalk_DistanceCalculation(float *A, float *B);
extern bool LevyWalk_CompareFloat(float f1, float f2);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  bp_process_navigation_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Process incoming navigation message
Notes:
  ---
Author, Date:
  Toan Huynh, 09/26/2022
*********************************************************************************/
bool bp_process_navigation_msg(void *p_msg)
{
  if (!p_msg)
  {
    return false;
  }

  navigationPkg_navigationMsg *msg = (navigationPkg_navigationMsg *)p_msg;
  bool ret = false;

  switch (msg->which_type)
  {
  case navigationPkg_navigationMsg_paramCfg_tag:
    ret = update_nav_param(&msg->type.paramCfg);
    break;
  case navigationPkg_navigationMsg_targetSet_tag:
    ret = update_target(&msg->type.targetSet);
    break;
  case navigationPkg_navigationMsg_stopNav_tag:
    ret = stop_nav();
    break;
  case navigationPkg_navigationMsg_seekingCfg_tag:
    ret = update_human_seeking_param(&msg->type.seekingCfg);
    break;
  case navigationPkg_navigationMsg_autoNavCfg_tag:
    ret = auto_navigation(&msg->type.autoNavCfg);
    break;
  case navigationPkg_navigationMsg_loadState_tag:
    ret = load_state();
    break;
  }

  return ret;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  update_nav_param()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Update navigation params
Notes:
  ---
Author, Date:
  Toan Huynh, 09/26/2022
*********************************************************************************/
static bool update_nav_param(navigationPkg_paramMsg *cfg)
{
  // CASE 50: This case is used to update navigational parameters
  LevyWalk_AngleThreshold = cfg->angle;
  LevyWalk_DistanceThreshold = cfg->distance;
  LevyWalk_ThresholdOfTimeForwardWhenSpeedIsSmall = cfg->forwardTime;
  LevyWalk_ThresholdOfTimeFreeWalkWhenSpeedIsSmall = cfg->freeTime;
  LevyWalk_ThresholdOfLongStimulus = cfg->longStimulus;
  LevyWalk_ThresholdOfLongStimulusForward = cfg->longStimulusForwardTime;
  LevyWalk_ThresholdOfLongStimulusFreeWalk = cfg->longStimulusFreeTime;
  LevyWalk_ControlRate = cfg->controlRate;
  LevyWalk_DurationToCheckSpeed = cfg->speedDuration;
  LevyWalk_DurationToCheckSpeed_FreeWalk = LevyWalk_DurationToCheckSpeed;
  LevyWalk_DurationToCheckSpeed_Stimulation = LevyWalk_DurationToCheckSpeed;
  LevyWalk_ThresholdOfForwardVelocity = cfg->linearSpeedThreshold;
  LevyWalk_TimeStoreToCalculateSpeed = cfg->speedStore;
  LevyWalk_EnableTheFreeWalkingAfterStimulation = cfg->allowFreeWalkAfterSteering;
  LevyWalk_ThresholdForFreeWalkAfterSteering = cfg->freeTimeAfterSteering;
  LevyWalk_ThresholdofFinalPartForLongStimulus = cfg->longStimulusFreeTime2;
  LevyWalk_ThresholdOfAngularVelocity = cfg->angularSpeedThreshold;
  LevyWalk_UseEscapeMode = cfg->useEscapeMode;
  LevyWalk_ReactionTime = cfg->reactionTime;
  LevyWalk_AngularSpeedMethod = cfg->angularSpeedMethod;

  LevyWalk_UseIMU_LinearSpeed = cfg->linearSpeed == navigationPkg_linearSpeedType_LINEAR_IMU;
  LevyWalk_UseIMU_AngularSpeed = cfg->angularSpeed == navigationPkg_angleSpeedType_ANGLE_IMU;

  // This line is input to re-use the structure of long-stimulus checking to check the angular speed
  // Once the stimulation is on, the speed will be checked at every LevyWalk_DurationToCheckSpeed_Stimulation
  // To do this, we take advantage of the old program in which the navigational program takes action when the stimulus lasts longer than
  // LevyWalk_ThresholdOfLongStimulus
  LevyWalk_ThresholdOfLongStimulus = LevyWalk_DurationToCheckSpeed_Stimulation + LevyWalk_ReactionTime;
  // Reset counter of speed = 0
  LevyWalk_NumberAngularSpeed = 0;
  LevyWalk_NumberLinearSpeed = 0;

  // This part is to only allow Flash Saving Work at 0x40 (Demonstration purpose - Oct 2020)
  // Flash_ShouldWeSave = false;
  // Flash_ShouldWeSave_Previous = false;
  // Flash_IsFlashBeingWritten = false;
  // Flash_IsItTimeToSaveFlash = false;
  LevyWalk_Tagert_Order = 1;
  Seeking_NumberPixelInRange = 0;
  Seeking_BlobResult[0] = 0;
  Seeking_BlobResult[1] = 0;
  Seeking_IsPhaseI = true;
  Seeking_FirstLoop = true;
  Seeking_IsPhaseIII = false;

  return true;
}

/********************************************************************************
Function:
  update_target()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Update navigation target
Notes:
  ---
Author, Date:
  Toan Huynh, 09/26/2022
*********************************************************************************/
static bool update_target(navigationPkg_target *cfg)
{
  // CASE 51: This case is used to update targets
  for (int i = 0; i < 16; i += 4)
  {
    LevyWalk_Target[i] = cfg->target[0].x * 10.0;
    LevyWalk_Target[i + 1] = cfg->target[0].y * 10.0;
    LevyWalk_Target[i + 2] = cfg->target[1].x * 10.0;
    LevyWalk_Target[i + 3] = cfg->target[1].y * 10.0;
  }

  LevyWalk_Target_Current[0] = LevyWalk_Target[0];
  LevyWalk_Target_Current[1] = LevyWalk_Target[1];
  LevyWalk_Tagert_Order = 1;
  // Reset counter of speed = 0
  LevyWalk_NumberAngularSpeed = 0;
  LevyWalk_NumberLinearSpeed = 0;

  // This part is to only allow Flash Saving Work at 0x40 (Demonstration purpose - Oct 2020)
  // Flash_ShouldWeSave = false;
  // Flash_ShouldWeSave_Previous = false;
  // Flash_IsFlashBeingWritten = false;
  // Flash_IsItTimeToSaveFlash = false;
  LevyWalk_Tagert_Order = 1;
  Seeking_NumberPixelInRange = 0;
  Seeking_BlobResult[0] = 0;
  Seeking_BlobResult[1] = 0;
  Seeking_IsPhaseI = true;
  Seeking_FirstLoop = true;
  Seeking_IsPhaseIII = false;

  return true;
}

/********************************************************************************
Function:
  auto_navigation()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 09/26/2022
*********************************************************************************/
static bool auto_navigation(navigationPkg_autoNav *cfg)
{
  // ----------------------------------- AUTOMATIC NAVIGATION --------------------------------------- //
  // CASE 40: This case is used to update the cockroach's position recorded from Vicon and allow the automatic navigation to happen
  //          The sampling rate to the navigation is depended on the setup on the GUI
  // Note: - In this program, there is only one cockroach being navigated at a time.
  //       - The cockroach position starts from byte 203 (C)
  // Enale or Disable Flash
  // Flash_IsFlashWrittingStarted = pbufferreceive[190];
  LevyWalk_IsTheRoachConfined = cfg->isTheRoachConfined;
  // if (Flash_IsFlashWrittingStarted == true)
  // {
  //   if (Flash_ShouldWeSave_Previous == false)
  //   {
  //     Flash_ShouldWeSave = true;
  //     Flash_ShouldWeSave_Previous = true;
  //     Flash_IsFlashBeingWritten = false;
  //     Flash_IsItTimeToSaveFlash = false;
  //     Flash_Counter = 0;
  //   }
  // }
  // else
  // {
  //   Flash_ShouldWeSave = false;
  //   Flash_ShouldWeSave_Previous = false;
  //   Flash_IsFlashBeingWritten = false;
  //   Flash_IsItTimeToSaveFlash = false;
  // }
  // Input the roach's position into three arrays of speed for the forward speed calculation
  LevyWalk_RobotPos[0] = cfg->robotHeadPos.x;
  LevyWalk_RobotPos[1] = cfg->robotHeadPos.y;
  LevyWalk_RobotPos[2] = cfg->robotTailPos.x;
  LevyWalk_RobotPos[3] = cfg->robotTailPos.y;

  float RobotHead[2] = {0.0, 0.0};
  float RobotTail[2] = {0.0, 0.0};
  float MarkerTest   = 0.0;

  RobotHead[0] = LevyWalk_RobotPos[0]; // X Position
  RobotHead[1] = LevyWalk_RobotPos[1]; // Y Position
  RobotTail[0] = LevyWalk_RobotPos[2]; // X Position
  RobotTail[1] = LevyWalk_RobotPos[3]; // Y Position

  MarkerTest = LevyWalk_DistanceCalculation(RobotHead, RobotTail);
  if (LevyWalk_CompareFloat(MarkerTest, 0.0) == false)
  {
    if (LevyWalk_SpeedCount > 299)
    {
      LevyWalk_SpeedCount = 0;
    }
    LevyWalk_SpeedArray_XHead[LevyWalk_SpeedCount] = cfg->robotHeadPos.x;
    LevyWalk_SpeedArray_YHead[LevyWalk_SpeedCount] = cfg->robotHeadPos.y;
    LevyWalk_SpeedArray_X[LevyWalk_SpeedCount] = cfg->robotTailPos.x;
    LevyWalk_SpeedArray_Y[LevyWalk_SpeedCount] = cfg->robotTailPos.y;
    LevyWalk_SpeedArray_T[LevyWalk_SpeedCount] = cfg->frameNumber;
    LevyWalk_SpeedCount++;
  }

  // FrameNumberSendBack = cfg->frameNumber;

  // Call semaphore of LevyWalk every pre-defined interval
  if (LevyWalk_TimeForExecute)
  {
    levywalk_signal();
  }

  return true;
}

/********************************************************************************
Function:
  stop_nav()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Stop navigation
Notes:
  ---
Author, Date:
  Toan Huynh, 09/26/2022
*********************************************************************************/
static bool stop_nav(void)
{
  LevyWalk_Tagert_Order = 1;
  StimulationTrigger = true;
  StimulationStartStop = false;
  // Reset all the navigational variable to prepare for the next navigation
  LevyWalk_TurnPrevious = 0;
  LevyWalk_Turn = 0;                            // Stop the navigation
  LevyWalk_IsLongStimulusHandleStarted = false; // The variable to indicate that long stimulus is being handled
  LevyWalk_IsLongStimulusBeingChecked = false;  // The variable to indicate that long stimulus is being monitored
  LevyWalk_IsNavigated = true;                  // Enable navigation process
  LevyWalk_IsForwardTriggeredAlready = false;   // The variable to indicate that forward stimulation has been already started
  LevyWalk_ShouldCheckTheForwardSpeed = false;
  LevyWalk_TimeafterForwardStimulus = 0;
  LevyWalk_TimeofStimulusForSteering = 0;
  LevyWalk_TimeafterLongStimulus = 0;
  LevyWalk_TimeafterAngleSmallerThanThreshold = 0;
  LevyWalk_TimeCounterForFreeWalkAfterSteering = 0;
  LevyWalk_StartCoutingTimeForFreeWalkAfterSteering = false;

  // Reset counter of speed = 0
  LevyWalk_NumberAngularSpeed = 0;
  LevyWalk_NumberLinearSpeed = 0;

  // This part is to only allow Flash Saving Work at 0x40 (Demonstration purpose - Oct 2020)
  // Flash_ShouldWeSave = false;
  // Flash_ShouldWeSave_Previous = false;
  // Flash_IsFlashBeingWritten = false;
  // Flash_IsItTimeToSaveFlash = false;
  Seeking_NumberPixelInRange = 0;
  Seeking_BlobResult[0] = 0;
  Seeking_BlobResult[1] = 0;
  Seeking_IsPhaseI = true;
  Seeking_FirstLoop = true;
  Seeking_IsPhaseIII = false;

  return true;
}

/********************************************************************************
Function:
  update_human_seeking_param()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Update human seeking data
Notes:
  ---
Author, Date:
  Toan Huynh, 09/26/2022
*********************************************************************************/
static bool update_human_seeking_param(navigationPkg_seeking *msg)
{
  // CASE 52: This case is used to update variable relating to Human Seeking
  Seeking_Step1 = msg->step1 * 10;
  Seeking_Step2 = msg->step2 * 10;
  Seeking_MinTemp = msg->minTemp;
  Seeking_MaxTemp = msg->maxTemp;
  Seeking_NumberPixel = msg->numberPixelToStart;
  Seeking_MaxMedian = msg->maxMedian;
  HumanDetection_NumberPixel = msg->numberPixelToStart;
  Seeking_XMin = msg->xmin;
  Seeking_XMax = msg->xmax;
  Seeking_YMin = msg->ymin;
  Seeking_YMax = msg->ymax;
  Seeking_LevyWalk_LengthMin = msg->levyMin;
  Seeking_LevyWalk_LengthMax = msg->levyMax;
  Seeking_AdditionalTargetPhase3 = msg->humanAdditionalStep;

  // Reset counter of speed = 0
  LevyWalk_NumberAngularSpeed = 0;
  LevyWalk_NumberLinearSpeed = 0;

  // This part is to only allow Flash Saving Work at 0x40 (Demonstration purpose - Oct 2020)
  // Flash_ShouldWeSave = false;
  // Flash_ShouldWeSave_Previous = false;
  // Flash_IsFlashBeingWritten = false;
  // Flash_IsItTimeToSaveFlash = false;
  LevyWalk_Tagert_Order = 1;
  Seeking_NumberPixelInRange = 0;
  Seeking_BlobResult[0] = 0;
  Seeking_BlobResult[1] = 0;
  Seeking_IsPhaseI = true;
  Seeking_FirstLoop = true;
  Seeking_IsPhaseIII = false;

  return true;
}

/********************************************************************************
Function:
  load_state()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Load current state
Notes:
  ---
Author, Date:
  Toan Huynh, 09/27/2022
*********************************************************************************/
static bool load_state(void)
{
  backpack_send_nav_state_data();
  return true;
}

/********************************************************************************
Function:
  sti_encode_state_info_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode state info message
Notes:
  ---
Author, Date:
  Toan Huynh, 05/04/2022
*********************************************************************************/
bool nav_encode_state_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  sensorPkg_sensorMsg message = sensorPkg_sensorMsg_init_zero;
  message.which_type = sensorPkg_sensorMsg_navigationMsg_tag;

  navigationPkg_navigationMsg *p_msg = &message.type.navigationMsg;
  p_msg->which_type = navigationPkg_navigationMsg_levyStatus_tag;
  navigationPkg_levyStatus *ret = &p_msg->type.levyStatus;

  ret->levyWalkTurn = LevyWalk_Turn;
  // Angular Speed and Linear Speed for Navigation
  ret->levyWalkForwardVelocityVicon = LevyWalk_ForwardVelocity_Vicon;
  ret->levyWalkAngularVelocityVicon = fabs(LevyWalk_AngularVelocity_Vicon);

  ret->has_levyWalkTargetCurrent = true;
  ret->levyWalkTargetCurrent.x = LevyWalk_Target_Current[0];
  ret->levyWalkTargetCurrent.y = LevyWalk_Target_Current[1];
  ret->levyWalkTargetCurrent.z = 0;

  // Blob Detection Angular Variables
  ret->seekingAlpha = Seeking_Alpha * 180.0 / H_PI;
  ret->seekingGama = Seeking_Gama;
  ret->seekingBeta = Seeking_Beta * 180.0 / H_PI;
  ret->seekingNumberPixelInRange = (float)Seeking_NumberPixelInRange;

  ret->levyWalkTarget_count = 2;

  ret->levyWalkTarget[0].x = LevyWalk_Target[0];
  ret->levyWalkTarget[0].y = LevyWalk_Target[1];
  ret->levyWalkTarget[0].z = 0;
  ret->levyWalkTarget[1].x = LevyWalk_Target[2];
  ret->levyWalkTarget[1].y = LevyWalk_Target[3];
  ret->levyWalkTarget[1].z = 0;

  return backpack_encode_sensor_msg(p_buf, buf_size, p_written_len, &message);
}
//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF navigation_parser.c
//###########################################################################################################
