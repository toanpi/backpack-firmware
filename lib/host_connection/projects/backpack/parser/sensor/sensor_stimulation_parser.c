/************************************************************************************************************
Module:       sensor_stimulation_parser

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/24/2022    TH       Began Coding    (TH = Toan Huynh)

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
#include "backpack.pb.h"
#include "sensor_parser.h"
#include "backpack_parser.h"
#include "controller.h"
#include "system_utils.h"
#include "stimulation_exp.h"
#include "Stimulation_exp_variables.h"
#include "voltage_range_forward.h"
#include "positioning_calculation.h"




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
extern volatile float movinggain;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static void reset_state(void);
static void auto_stimulation_config(stimulationPkg_autoStimulationMsg *p_cfg);
static void start_auto_sti(void);
static void sti_turn_right(stimulationPkg_directionControlMsg *cfg);
static void sti_turn_left(stimulationPkg_directionControlMsg *cfg);
static void sti_move_forward(stimulationPkg_directionControlMsg *cfg);
static uint16_t cal_amp(stimulationPkg_channelConfigMsg *ch_cfg);
static uint32_t cal_auto_rest(uint32_t res_min);
static bool sti_vol_range_forward(stimulationPkg_volRangeForwardMsg *p_cfg);



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  bp_process_stimulation_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 05/24/2022
*********************************************************************************/
bool bp_process_stimulation_msg(void *p_msg)
{
  if (!p_msg)
  {
    return false;
  }

  stimulationPkg_stimulationMsg *msg = (stimulationPkg_stimulationMsg *)p_msg;
  bool ret = false;

  switch (msg->which_type)
  {
  case stimulationPkg_stimulationMsg_channelsConfig_tag:
    // channel_config(&msg->type.channelsConfig);
    break;
  case stimulationPkg_stimulationMsg_autoStiCfg_tag:
    auto_stimulation_config(&msg->type.autoStiCfg);
    break;
  case stimulationPkg_stimulationMsg_resetState_tag:
    reset_state();
    break;
  case stimulationPkg_stimulationMsg_autoSti_tag:
    start_auto_sti();
    break;
  case stimulationPkg_stimulationMsg_turnRight_tag:
    sti_turn_right(&msg->type.turnRight);
    break;
  case stimulationPkg_stimulationMsg_turnLeft_tag:
    sti_turn_left(&msg->type.turnLeft);
    break;
  case stimulationPkg_stimulationMsg_moveForward_tag:
    sti_move_forward(&msg->type.moveForward);
    break;
  case stimulationPkg_stimulationMsg_stopSti_tag:
    stop_sti();
    break;
  case stimulationPkg_stimulationMsg_volRangeForwardMsg_tag:
    sti_vol_range_forward(&msg->type.volRangeForwardMsg);
    break;
  }

  return ret;
}
/********************************************************************************
Function:
  stimulation_init_param()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Initialize stimulation parameters
Notes:
  ---
Author, Date:
  Toan Huynh, 07/28/2022
*********************************************************************************/
bool stimulation_init_param(void)
{
  bool ret = false;

  /* Turn left setup */
  stimulationPkg_directionControlMsg turnLeft =
  {
    .has_burst = true,
    .burst = {.enable = false}, // Duoc uses continous stimulation to induce turning
    .gain = 0.5,
    .topChannel = 4,
    .bottomChannel = 1, // 2, Duoc changes to cerci to induce turning
    .has_config = true,
    .config = 
    {
      .enable = 1, .duration = 3000, .ton1 = 12, .toff1 = 0, // Duoc changes duration to 3000 to induce turning
      .ton2 = 12, .toff2 = 0, .cycle = 3, .rest = 100, .amp = 2.99
    },
    .start = false
  };
  sti_turn_left(&turnLeft);

  /* Turn right setup */
  stimulationPkg_directionControlMsg turnRight =
  {
    .has_burst = true,
    .burst = {.enable = false}, // Duoc uses continous stimulation to induce turning
    .gain = 0.5,
    .topChannel = 3, 
    .bottomChannel = 1, // 2, Duoc changes to cerci to induce turning
    .has_config = true,
    .config = 
    {
      .enable = 1, .duration = 3000, .ton1 = 12, .toff1 = 0, // Duoc changes duration to 3000 to induce turning
      .ton2 = 12, .toff2 = 0, .cycle = 3, .rest = 100, .amp = 2.99
    },
    .start = false
  };
  sti_turn_right(&turnRight);

  /* Move forward setup */
  stimulationPkg_directionControlMsg moveForward =
  {
    .has_burst = true,
    .burst = {.enable = true},
    .gain = 0.5,
    .topChannel = 1, 
    .bottomChannel = 2,
    .has_config = true,
    .config = 
    {
      .enable = 1, .duration = 500, .ton1 = 12, .toff1 = 0,
      .ton2 = 12, .toff2 = 0, .cycle = 12, .rest = 100, .amp = 2.99
    },
    .start = false
  };
  sti_move_forward(&moveForward);

  /* Voltage range forward setup */
  stimulationPkg_volRangeForwardMsg volRangeForwardMsg =
  {
    .startIdx = 0,
    .range_count = 4,
    .range =
        {
          {.durationSecond = 300, .minVol = 1.5, .maxVol = 2.5},
          {.durationSecond = 300, .minVol = 2, .maxVol = 3},
          {.durationSecond = 300, .minVol = 2.5, .maxVol = 3.5},
          {.durationSecond = 300, .minVol = 3, .maxVol = 4}
        },
    .needReset = true
  };
  sti_vol_range_forward(&volRangeForwardMsg);

  /* Auto stimulation setup */
  stimulationPkg_autoStimulationMsg autoStiCfg =
  {
    .interval = 5000,
    .minInterval  = 1500,
    .maxInterval  = 3000,
    .restInterval = 10,
    .restDuration = 1,
    .leftMinAmp    = 2, // 1 - 4, Duoc changes to 2-5 to induce turning
    .leftMaxAmp    = 5,
    .rightMinAmp   = 2, // 1 - 4, Duoc changes to 2-5 to induce turning
    .rightMaxAmp   = 5,
    .forwardMinAmp = 2,
    .forwardMaxAmp = 5,
    .linearSpeedThreshold = 200,
    .omegaThreshold01 = 10,
    .omegaThreshold02 = 50,
    .amplitudeStepUpRatio = 1.2,
    .amplitudeStepDownRatio = 0.8,
    .amplitudeReactivation  = 8,
    .forwardProbability = 0.6, // Duoc reuce probability of accel from 0.7 to 0.6 expecting to reduce the wall-following
    .intervalIsRandomized = true,
    .algoType = stimulationPkg_stiAlgoType_RANDOMLY_ADJUST // stimulationPkg_stiAlgoType_SPEED_BASED_REGULATOR
  };
  auto_stimulation_config(&autoStiCfg);

  /* Start auto stimulation */
  start_auto_sti();

  ret = true;

  return ret;
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################

static uint32_t cal_burst_cycles(stimulationPkg_channelConfigMsg *ch_cfg)
{
  return ch_cfg->cycle;
}

static uint32_t cal_burst_period(stimulationPkg_channelConfigMsg *ch_cfg)
{
  uint32_t period = ch_cfg->ton1 + ch_cfg->ton2 + ch_cfg->toff1 + ch_cfg->toff2;
  return period * ch_cfg->cycle + ch_cfg->rest;
}

static uint32_t cal_burst_duration(stimulationPkg_channelConfigMsg *ch_cfg)
{
  return ch_cfg->duration;
}

static uint16_t cal_amp(stimulationPkg_channelConfigMsg *ch_cfg)
{
  float amp_voltage = ch_cfg->amp;

  if (amp_voltage < 0.0)
  {
    amp_voltage = 0.0;
  }
  else if (amp_voltage > 12.0)
  {
    amp_voltage = 12.0;
  }

  uint16_t amp = (uint16_t)(amp_voltage * 136.5333);

  return amp;
}

static uint32_t cal_auto_rest(uint32_t res_min)
{
  uint32_t res_ms = res_min * 60 * 1000;
  return res_ms;
}

/********************************************************************************
Function:
  reset_state()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Reset the state of the stimulation
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
static void reset_state(void)
{
  // Reset all variables
  // IsIMUBeingCalibrated = false; // TH: No defined yet
  AutoStimulation_IsStarted = false;
  AutoStimulation_Counter = 0;
  AutoStimulation_RestInterval_Count = 0;
  AutoStimulation_RestDuration_Count = 0;
  AutoStimulation_IsResting = false;
  if (StimulationStartStop == true)
  {
    StimulationStartStop = false;
    StimulationTrigger = true;
  }
  Burst_IsBurstModeUsed = false;
  Burst_IsBurstModeStarted = false;
  Burst_TimerForDuration = 0;
  Burst_TimerForPeriod = 0;
  Burst_Duration = 0;
  Burst_Period = 0;
  IsStimulationStarted = false;
  Stimulation_Counter = 0;

  Regulator_AccelerateThenSteer = false;
  Regulator_ReactivationIsTriggered = false;
  Regulator_IsStimulationStarted = false;
  Regulator_TypeOfStimulation = 'N';
  Regulator_TypeOfSteer = 'N';

  AutoStimulate_Type = 0;
  AutoStimulate_Amplitude_L = 0.0;
  AutoStimulate_Amplitude_R = 0.0;
  AutoStimulate_Amplitude_F = 0.0;

  AutoStimulate_Switch = false;

  Controller_DoAction(stimualtion_send_state_info);
  // sys_printf("Sty Type %d\n", AutoStimulate_Type);
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
  Toan Huynh, 06/01/2022
*********************************************************************************/
void stop_sti(void)
{
  // Disable all channels before setting the new parameter
  for (uint8_t i = 0; i < NO_CHANNEL_STI; i++)
  {
    ChannelActive[i] = false;
  }

  // Reset Variable of Burst Mode and Stimulation
  Burst_IsBurstModeUsed = false;
  Burst_IsBurstModeStarted = false;
  Burst_TimerForDuration = 0;
  Burst_TimerForPeriod = 0;
  Burst_Duration = 0;
  Burst_Period = 0;
  IsStimulationStarted = false;
  Stimulation_Counter = 0;

  StimulationTrigger = true;
}

/********************************************************************************
Function:
  channel_config()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Configure the stimulation channels
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
void channel_config(stimulationPkg_directionControlMsg *cfg)
{
  if (cfg->bottomChannel == 0 || cfg->topChannel == 0)
  {
    return;
  }

  uint32_t period = 0;

  // Disable all channels before setting the new parameter
  for (uint8_t i = 0; i < NO_CHANNEL_STI; i++)
  {
    ChannelActive[i] = false;
  }

  period = cfg->config.ton1 + cfg->config.ton2 + cfg->config.toff1 + cfg->config.toff2;

  // Top Channel
  uint8_t idx = cfg->topChannel - 1;
  ChannelActive[idx] = cfg->config.enable;
  StimulateDuration[idx] = cfg->config.duration;
  ActiveTime1[idx] = cfg->config.ton1;
  ZeroTime1[idx] = 0;
  ZeroTime2[idx] = period - cfg->config.ton1;
  ChannelAmplitude[idx] = cal_amp(&cfg->config);
  StimulatePeriod[idx] = period;

  if (ChannelActive[idx] == true)
  {
    Stimulation_Duration = StimulateDuration[idx];
  }

  // Bottom Channel
  idx = cfg->bottomChannel - 1;
  ChannelActive[idx] = cfg->config.enable;
  StimulateDuration[idx] = cfg->config.duration;
  ActiveTime1[idx] = cfg->config.ton2;
  ZeroTime1[idx] = cfg->config.toff1 + cfg->config.ton1;
  ZeroTime2[idx] = cfg->config.toff2;
  ChannelAmplitude[idx] =  cal_amp(&cfg->config);
  StimulatePeriod[idx] = period;

  if (ChannelActive[idx] == true)
  {
    Stimulation_Duration = StimulateDuration[idx];
  }

  Burst_IsBurstModeUsed = cfg->burst.enable;

  StimulationStartStop = cfg->start;

  if (StimulationStartStop == true)
  {
    IsStimulationStarted = true;
    Stimulation_Counter = 0;
    if (Burst_IsBurstModeUsed == false)
    {
      // Reset Variable of Burst Mode
      Burst_IsBurstModeUsed = false;
      Burst_IsBurstModeStarted = false;
      Burst_TimerForDuration = 0;
      Burst_TimerForPeriod = 0;
      Burst_Duration = 0;
      Burst_Period = 0;
    }
    else
    {
      // Assign data for Burst Period and Burst Duration
      // Note: Bust Period = Period_Continous*NoCycle;
      Burst_Period = cal_burst_period(&cfg->config);
      Burst_Duration = cal_burst_duration(&cfg->config);
      Burst_Cycle = cal_burst_cycles(&cfg->config);

      // Adjust the duration of each burst
      for (uint8_t jj = 0; jj < 4; jj++)
      {
        StimulateDuration[jj] = StimulatePeriod[jj] * ((uint32_t)Burst_Cycle);
      }
      // Start the timer
      Burst_TimerForDuration = 0;
      Burst_TimerForPeriod = 0;
      Burst_IsBurstModeUsed = true;
      Burst_IsBurstModeStarted = true;
    }
  }
  else
  {
    // Reset Variable of Burst Mode and Stimulation
    Burst_IsBurstModeUsed = false;
    Burst_IsBurstModeStarted = false;
    Burst_TimerForDuration = 0;
    Burst_TimerForPeriod = 0;
    Burst_Duration = 0;
    Burst_Period = 0;
    IsStimulationStarted = false;
    Stimulation_Counter = 0;
  }
  StimulationTrigger = true;
}

/********************************************************************************
Function:
  auto_stimulation_config()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Configure the auto stimulation
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
static void auto_stimulation_config(stimulationPkg_autoStimulationMsg *p_cfg)
{
  AutoStimulation_Interval = p_cfg->interval;
  AutoStimulate_Forward_Probability = p_cfg->forwardProbability;

  AutoStimulation_RestInterval = cal_auto_rest(p_cfg->restInterval);
  AutoStimulation_RestDuration = cal_auto_rest(p_cfg->restDuration);
  AutoStimulate_LeftMinAmplitude = p_cfg->leftMinAmp;
  AutoStimulate_LeftMaxAmplitude = p_cfg->leftMaxAmp;
  AutoStimulate_RightMinAmplitude = p_cfg->rightMinAmp;
  AutoStimulate_RightMaxAmplitude = p_cfg->rightMaxAmp;
  AutoStimulate_ForwardMinAmplitude = p_cfg->forwardMinAmp;
  AutoStimulate_ForwardMaxAmplitude = p_cfg->forwardMaxAmp;

  switch (p_cfg->algoType)
  {
  case stimulationPkg_stiAlgoType_FIXED_VOLTAGE:
    Regulator_IsVoltageRegulatorUsed = false;
    AutoStimulate_ModifyAmplitude = false;
    break;
  case stimulationPkg_stiAlgoType_RANDOMLY_ADJUST:
    Regulator_IsVoltageRegulatorUsed = false;
    AutoStimulate_ModifyAmplitude = true;
    break;
  case stimulationPkg_stiAlgoType_SPEED_BASED_REGULATOR:
    Regulator_IsVoltageRegulatorUsed = true;
    AutoStimulate_ModifyAmplitude = false;
    break;
  }
  
  AutoStimulation_Interval_Min = p_cfg->minInterval;
  AutoStimulation_Interval_Max = p_cfg->maxInterval;
  AutoStimulation_Interval_IsRandomized = p_cfg->intervalIsRandomized;

  Regulator_Amplitude_Left_Current = AutoStimulate_LeftMinAmplitude;
  Regulator_Amplitude_Right_Current = AutoStimulate_RightMinAmplitude;
  Regulator_Amplitude_Forward_Current = AutoStimulate_ForwardMinAmplitude;
  Regulator_LinearSpeedThreshold = p_cfg->linearSpeedThreshold;
  Regulator_OmegaThreshold_01 = p_cfg->omegaThreshold01;
  Regulator_OmegaThreshold_02 = p_cfg->omegaThreshold02;
  Regulator_AmplitudeStepUpRatio = p_cfg->amplitudeStepUpRatio;
  Regulator_AmplitudeStepDownRatio = p_cfg->amplitudeStepDownRatio;
  Regulator_AmplitudeReactivation = p_cfg->amplitudeReactivation;
}
/********************************************************************************
Function:
  start_auto_sti()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Start the auto stimulation
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
static void start_auto_sti(void)
{
  if (AutoStimulate_Switch == false)
  {
    AutoStimulate_Switch = true;
    AutoStimulate_Type = 0;
    AutoStimulation_IsStarted = true;
    AutoStimulation_Counter = 0;
    AutoStimulation_RestInterval_Count = 0;
    AutoStimulation_RestDuration_Count = 0;
    AutoStimulation_IsResting = false;
    Regulator_AccelerateThenSteer = false;
    Regulator_ReactivationIsTriggered = false;
    Regulator_IsStimulationStarted = false;
    Regulator_TypeOfStimulation = 'N';
    Regulator_TypeOfSteer = 'N';
    AutoStimulate_Amplitude_L = 0.0;
    AutoStimulate_Amplitude_R = 0.0;
    AutoStimulate_Amplitude_F = 0.0;
    AutoStimulate_SelectStimulation();
    Controller_DoAction(stimualtion_send_state_info);
    // sys_printf("Sty Type %d\n", AutoStimulate_Type);
  }
}

/********************************************************************************
Function:
  sti_turn_right()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Turn right
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
static void sti_turn_right(stimulationPkg_directionControlMsg *cfg)
{
  movinggain = cfg->gain; // Huu Duoc uses this to update the IMU gain

  if (cfg->start)
  {
    // Move to common control and exit
    channel_config(cfg);
    return;
  }

  if (cfg->bottomChannel == 0 || cfg->topChannel == 0)
  {
    return;
  }

  uint32_t period = 0;

  // CASE 42: This case is used to prepare right stimulation parameter for the cockroach

  // Disable all channels before setting the new parameter
  for (uint8_t i = 0; i < NO_CHANNEL_STI; i++)
  {
    AutoStimulate_RightChannelActive[i] = false;
  }

  period = cfg->config.ton1 + cfg->config.ton2 + cfg->config.toff1 + cfg->config.toff2;

  // Top Channel
  uint8_t idx = cfg->topChannel - 1;
  AutoStimulate_RightChannelActive[idx] = cfg->config.enable;
  AutoStimulate_RightDuration[idx] = cfg->config.duration;
  AutoStimulate_RightActiveTime1[idx] = cfg->config.ton1;
  AutoStimulate_RightZeroTime1[idx] = 0;
  AutoStimulate_RightZeroTime2[idx] = period - cfg->config.ton1;
  AutoStimulate_RightChannelAmplitude[idx] = cal_amp(&cfg->config);
  AutoStimulate_RightPeriod[idx] = period;

  // Bottom Channel
  idx = cfg->bottomChannel - 1;
  AutoStimulate_RightChannelActive[idx] = cfg->config.enable;
  AutoStimulate_RightDuration[idx] = cfg->config.duration;
  AutoStimulate_RightActiveTime1[idx] = cfg->config.ton2;
  AutoStimulate_RightZeroTime1[idx] = cfg->config.toff1 + cfg->config.ton1;
  AutoStimulate_RightZeroTime2[idx] = cfg->config.toff2;
  AutoStimulate_RightChannelAmplitude[idx] = cal_amp(&cfg->config);
  AutoStimulate_RightPeriod[idx] = period;

  AutoStimulate_RightBurst_IsBurstModeUsed = cfg->burst.enable;

  // Assign data for Burst Period and Burst Duration
  if (AutoStimulate_RightBurst_IsBurstModeUsed == true)
  {
    // Note: Bust Period = Period_Continous*NoCycle;
    AutoStimulate_RightBurst_Period = cal_burst_period(&cfg->config);
    AutoStimulate_RightBurst_Duration = cal_burst_duration(&cfg->config);
    AutoStimulate_RightBurst_Cycle = cal_burst_cycles(&cfg->config);

    // Adjust the duration of each burst
    for (uint8_t jj = 0; jj < 4; jj++)
    {
      AutoStimulate_RightDuration[jj] = AutoStimulate_RightPeriod[jj] * ((uint32_t)AutoStimulate_RightBurst_Cycle);
    }
  }
}

/********************************************************************************
Function:
  sti_turn_left()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Turn left
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
static void sti_turn_left(stimulationPkg_directionControlMsg *cfg)
{
  movinggain = cfg->gain; // Huu Duoc uses this to update the IMU gain

  if (cfg->start)
  {
    // Move to common control and exit
    channel_config(cfg);
    return;
  }

  // CASE 41: This case is used to prepare left stimulation parameter for the cockroach
  if (cfg->bottomChannel == 0 || cfg->topChannel == 0)
  {
    return;
  }

  uint32_t period = 0;

  // Disable all channels before setting the new parameter
  for (uint8_t i = 0; i < NO_CHANNEL_STI; i++)
  {
    AutoStimulate_LeftChannelActive[i] = false;
  }

  period = cfg->config.ton1 + cfg->config.ton2 + cfg->config.toff1 + cfg->config.toff2;

  // Top Channel
  uint8_t idx = cfg->topChannel - 1;
  AutoStimulate_LeftChannelActive[idx] = cfg->config.enable;
  AutoStimulate_LeftDuration[idx] = cfg->config.duration;
  AutoStimulate_LeftActiveTime1[idx] = cfg->config.ton1;
  AutoStimulate_LeftZeroTime1[idx] = 0;
  AutoStimulate_LeftZeroTime2[idx] = period - cfg->config.ton1;
  AutoStimulate_LeftChannelAmplitude[idx] = cal_amp(&cfg->config);
  AutoStimulate_LeftPeriod[idx] = period;

  // Bottom Channel
  idx = cfg->bottomChannel - 1;
  AutoStimulate_LeftChannelActive[idx] = cfg->config.enable;
  AutoStimulate_LeftDuration[idx] = cfg->config.duration;
  AutoStimulate_LeftActiveTime1[idx] = cfg->config.ton2;
  AutoStimulate_LeftZeroTime1[idx] = cfg->config.toff1 + cfg->config.ton1;
  AutoStimulate_LeftZeroTime2[idx] = cfg->config.toff2;
  AutoStimulate_LeftChannelAmplitude[idx] = cal_amp(&cfg->config);
  AutoStimulate_LeftPeriod[idx] = period;

  AutoStimulate_LeftBurst_IsBurstModeUsed = cfg->burst.enable;

  // Assign data for Burst Period and Burst Duration
  if (AutoStimulate_LeftBurst_IsBurstModeUsed == true)
  {
    // Note: Bust Period = Period_Continous*NoCycle;
    AutoStimulate_LeftBurst_Period = cal_burst_period(&cfg->config);
    AutoStimulate_LeftBurst_Duration = cal_burst_duration(&cfg->config);
    AutoStimulate_LeftBurst_Cycle = cal_burst_cycles(&cfg->config);

    // Adjust the duration of each burst
    for (uint8_t jj = 0; jj < 4; jj++)
    {
      AutoStimulate_LeftDuration[jj] = AutoStimulate_LeftPeriod[jj] * ((uint32_t)AutoStimulate_LeftBurst_Cycle);
    }
  }
}
/********************************************************************************
Function:
  sti_move_forward()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Move forward
Notes:
  ---
Author, Date:
  Toan Huynh, 05/25/2022
*********************************************************************************/
static void sti_move_forward(stimulationPkg_directionControlMsg *cfg)
{
  movinggain = cfg->gain; // Huu Duoc uses this to update the IMU gain

  if (cfg->start)
  {
    // Move to common control and exit
    channel_config(cfg);
    return;
  }

  if (cfg->bottomChannel == 0 || cfg->topChannel == 0)
  {
    return;
  }

  uint32_t period = 0;

  // Disable all channels before setting the new parameter
  for (uint8_t i = 0; i < NO_CHANNEL_STI; i++)
  {
    AutoStimulate_ForwardChannelActive[i] = false;
  }

  period = cfg->config.ton1 + cfg->config.ton2 + cfg->config.toff1 + cfg->config.toff2;

  // Top Channel
  uint8_t idx = cfg->topChannel - 1;
  AutoStimulate_ForwardChannelActive[idx] = cfg->config.enable;
  AutoStimulate_ForwardDuration[idx] = cfg->config.duration;
  AutoStimulate_ForwardActiveTime1[idx] = cfg->config.ton1;
  AutoStimulate_ForwardZeroTime1[idx] = 0;
  AutoStimulate_ForwardZeroTime2[idx] = period - cfg->config.ton1;
  AutoStimulate_ForwardChannelAmplitude[idx] = cal_amp(&cfg->config);
  AutoStimulate_ForwardPeriod[idx] = period;

  // Bottom Channel
  idx = cfg->bottomChannel - 1;
  AutoStimulate_ForwardChannelActive[idx] = cfg->config.enable;
  AutoStimulate_ForwardDuration[idx] = cfg->config.duration;
  AutoStimulate_ForwardActiveTime1[idx] = cfg->config.ton2;
  AutoStimulate_ForwardZeroTime1[idx] = cfg->config.toff1 + cfg->config.ton1;
  AutoStimulate_ForwardZeroTime2[idx] = cfg->config.toff2;
  AutoStimulate_ForwardChannelAmplitude[idx] = cal_amp(&cfg->config);
  AutoStimulate_ForwardPeriod[idx] = period;

  AutoStimulate_ForwardBurst_IsBurstModeUsed = cfg->burst.enable;

  // Assign data for Burst Period and Burst Duration
  if (AutoStimulate_ForwardBurst_IsBurstModeUsed == true)
  {
    // Note: Bust Period = Period_Continous*NoCycle;
    AutoStimulate_ForwardBurst_Period = cal_burst_period(&cfg->config);
    AutoStimulate_ForwardBurst_Duration = cal_burst_duration(&cfg->config);
    AutoStimulate_ForwardBurst_Cycle = cal_burst_cycles(&cfg->config);

    // Adjust the duration of each burst
    for (uint8_t jj = 0; jj < 4; jj++)
    {
      AutoStimulate_ForwardDuration[jj] = AutoStimulate_ForwardPeriod[jj] * ((uint32_t)AutoStimulate_ForwardBurst_Cycle);
    }
  }
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
bool sti_encode_state_info_msg(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t len, uint32_t *p_written_len)
{
  static uint8_t last_send_sti_type = 0xFF;

  /* Only send the state info message if the state type  has changed */
  if (last_send_sti_type == AutoStimulate_Type)
  {
    return false;
  }

  sensorPkg_sensorMsg message = sensorPkg_sensorMsg_init_zero;
  message.which_type = sensorPkg_sensorMsg_stimulationMsg_tag;
  stimulationPkg_stimulationMsg *p_msg = &message.type.stimulationMsg;

  p_msg->which_type = stimulationPkg_stimulationMsg_stiInfo_tag;

  // Encode the data
  // p_msg->type.stiInfo.batteryVoltage = BatteryVoltage;
  p_msg->type.stiInfo.leftTurnVoltage = AutoStimulate_Amplitude_L;
  p_msg->type.stiInfo.rightTurnVoltage = AutoStimulate_Amplitude_R;
  p_msg->type.stiInfo.forwardVoltage = AutoStimulate_Amplitude_F;
  p_msg->type.stiInfo.stiType = AutoStimulate_Type;
  last_send_sti_type = AutoStimulate_Type;

  return backpack_encode_sensor_msg(p_buf, buf_size, p_written_len, &message);
}

/********************************************************************************
Function:
  sti_vol_range_forward()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/28/2022
*********************************************************************************/
static bool sti_vol_range_forward(stimulationPkg_volRangeForwardMsg *p_cfg)
{
  bool ret = false;
  uint32_t start_index = p_cfg->startIdx;

  if (p_cfg->needReset)
  {
    voltage_range_forward_reset();
  }

  for (size_t i = 0; i < p_cfg->range_count; i++)
  {
    voltage_range_forward_t range = {
        .duration_ms = p_cfg->range[i].durationSecond * 1000,
        .min_voltage = p_cfg->range[i].minVol,
        .max_voltage = p_cfg->range[i].maxVol,
    };

    ret = voltage_range_forward_set(start_index++, &range);
    if (ret == false)
    {
      /* Failed to set the voltage range */
      break;
    }


  }

  return ret;
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF sensor_stimulation_parser.c
//###########################################################################################################
