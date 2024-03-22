/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.7 */

#ifndef PB_SENSORPKG_SENSOR_PB_H_INCLUDED
#define PB_SENSORPKG_SENSOR_PB_H_INCLUDED
#include <pb.h>
#include "stimulation.pb.h"
#include "navigation.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* ******************************************************************************
  BACKPACK CONFIG MESSENGER
****************************************************************************** */
typedef struct _sensorPkg_channelConfigMsg {
    bool enable;
    uint32_t duration;
    uint32_t ton1;
    uint32_t toff1;
    uint32_t ton2;
    uint32_t toff2;
    uint32_t cycle;
    uint32_t rest;
    uint32_t amp;
    uint32_t chId;
} sensorPkg_channelConfigMsg;

typedef struct _sensorPkg_channelsConfigMsg {
    bool stiStartStop;
    uint32_t gain;
    uint32_t command;
    pb_size_t config_count;
    sensorPkg_channelConfigMsg config[4];
} sensorPkg_channelsConfigMsg;

typedef struct _sensorPkg_position {
    float x;
    float y;
    float z;
} sensorPkg_position;

typedef struct _sensorPkg_vectorChannel {
    float x;
    float y;
    float z;
} sensorPkg_vectorChannel;

typedef struct _sensorPkg_beacon {
    uint32_t id;
    float distance;
} sensorPkg_beacon;

typedef struct _sensorPkg_quaternion {
    float q0; /* /< scalar component */
    float q1; /* /< x vector component */
    float q2; /* /< y vector component */
    float q3; /* /< z vector component */
} sensorPkg_quaternion;

typedef struct _sensorPkg_fusionMsg {
    /* Angular Velocity */
    pb_size_t fOmega_count;
    float fOmega[3];
    /* Deimention */
    float fPhiPl; /* Roll */
    float fThePl; /* Pitch */
    float fPsiPl; /* Yaw */
    /* Quaternion */
    bool has_fqPl;
    sensorPkg_quaternion fqPl;
    /* Accel */
    bool has_fAccGl;
    sensorPkg_vectorChannel fAccGl;
    /* IMU */
    float imuOmega;
    float imuAbsoluteLinearSpeed;
} sensorPkg_fusionMsg;

typedef struct _sensorPkg_positionMsg {
    bool has_imuPosition;
    sensorPkg_position imuPosition;
    pb_size_t beacons_count;
    sensorPkg_beacon beacons[4];
    bool has_uwbPosition;
    sensorPkg_position uwbPosition;
    float speedMagCms;
} sensorPkg_positionMsg;

typedef struct _sensorPkg_stimulationCommand {
    bool ch1En;
    bool ch2En;
    bool ch3En;
} sensorPkg_stimulationCommand;

typedef struct _sensorPkg_devInfo {
    float batteryMV;
} sensorPkg_devInfo;

typedef struct _sensorPkg_sensorMsg {
    pb_size_t which_type;
    union {
        sensorPkg_fusionMsg fusionMsg;
        sensorPkg_positionMsg positionMsg;
        sensorPkg_channelsConfigMsg channelCfg;
        sensorPkg_stimulationCommand stimulationCommand;
        stimulationPkg_stimulationMsg stimulationMsg;
        sensorPkg_devInfo devInfo;
        navigationPkg_navigationMsg navigationMsg;
    } type;
} sensorPkg_sensorMsg;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define sensorPkg_channelConfigMsg_init_default  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define sensorPkg_channelsConfigMsg_init_default {0, 0, 0, 0, {sensorPkg_channelConfigMsg_init_default, sensorPkg_channelConfigMsg_init_default, sensorPkg_channelConfigMsg_init_default, sensorPkg_channelConfigMsg_init_default}}
#define sensorPkg_position_init_default          {0, 0, 0}
#define sensorPkg_vectorChannel_init_default     {0, 0, 0}
#define sensorPkg_beacon_init_default            {0, 0}
#define sensorPkg_quaternion_init_default        {0, 0, 0, 0}
#define sensorPkg_fusionMsg_init_default         {0, {0, 0, 0}, 0, 0, 0, false, sensorPkg_quaternion_init_default, false, sensorPkg_vectorChannel_init_default, 0, 0}
#define sensorPkg_positionMsg_init_default       {false, sensorPkg_position_init_default, 0, {sensorPkg_beacon_init_default, sensorPkg_beacon_init_default, sensorPkg_beacon_init_default, sensorPkg_beacon_init_default}, false, sensorPkg_position_init_default, 0}
#define sensorPkg_stimulationCommand_init_default {0, 0, 0}
#define sensorPkg_devInfo_init_default           {0}
#define sensorPkg_sensorMsg_init_default         {0, {sensorPkg_fusionMsg_init_default}}
#define sensorPkg_channelConfigMsg_init_zero     {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define sensorPkg_channelsConfigMsg_init_zero    {0, 0, 0, 0, {sensorPkg_channelConfigMsg_init_zero, sensorPkg_channelConfigMsg_init_zero, sensorPkg_channelConfigMsg_init_zero, sensorPkg_channelConfigMsg_init_zero}}
#define sensorPkg_position_init_zero             {0, 0, 0}
#define sensorPkg_vectorChannel_init_zero        {0, 0, 0}
#define sensorPkg_beacon_init_zero               {0, 0}
#define sensorPkg_quaternion_init_zero           {0, 0, 0, 0}
#define sensorPkg_fusionMsg_init_zero            {0, {0, 0, 0}, 0, 0, 0, false, sensorPkg_quaternion_init_zero, false, sensorPkg_vectorChannel_init_zero, 0, 0}
#define sensorPkg_positionMsg_init_zero          {false, sensorPkg_position_init_zero, 0, {sensorPkg_beacon_init_zero, sensorPkg_beacon_init_zero, sensorPkg_beacon_init_zero, sensorPkg_beacon_init_zero}, false, sensorPkg_position_init_zero, 0}
#define sensorPkg_stimulationCommand_init_zero   {0, 0, 0}
#define sensorPkg_devInfo_init_zero              {0}
#define sensorPkg_sensorMsg_init_zero            {0, {sensorPkg_fusionMsg_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define sensorPkg_channelConfigMsg_enable_tag    1
#define sensorPkg_channelConfigMsg_duration_tag  2
#define sensorPkg_channelConfigMsg_ton1_tag      3
#define sensorPkg_channelConfigMsg_toff1_tag     4
#define sensorPkg_channelConfigMsg_ton2_tag      5
#define sensorPkg_channelConfigMsg_toff2_tag     6
#define sensorPkg_channelConfigMsg_cycle_tag     7
#define sensorPkg_channelConfigMsg_rest_tag      8
#define sensorPkg_channelConfigMsg_amp_tag       9
#define sensorPkg_channelConfigMsg_chId_tag      10
#define sensorPkg_channelsConfigMsg_stiStartStop_tag 1
#define sensorPkg_channelsConfigMsg_gain_tag     2
#define sensorPkg_channelsConfigMsg_command_tag  3
#define sensorPkg_channelsConfigMsg_config_tag   4
#define sensorPkg_position_x_tag                 1
#define sensorPkg_position_y_tag                 2
#define sensorPkg_position_z_tag                 3
#define sensorPkg_vectorChannel_x_tag            1
#define sensorPkg_vectorChannel_y_tag            2
#define sensorPkg_vectorChannel_z_tag            3
#define sensorPkg_beacon_id_tag                  1
#define sensorPkg_beacon_distance_tag            2
#define sensorPkg_quaternion_q0_tag              1
#define sensorPkg_quaternion_q1_tag              2
#define sensorPkg_quaternion_q2_tag              3
#define sensorPkg_quaternion_q3_tag              4
#define sensorPkg_fusionMsg_fOmega_tag           1
#define sensorPkg_fusionMsg_fPhiPl_tag           2
#define sensorPkg_fusionMsg_fThePl_tag           3
#define sensorPkg_fusionMsg_fPsiPl_tag           4
#define sensorPkg_fusionMsg_fqPl_tag             5
#define sensorPkg_fusionMsg_fAccGl_tag           6
#define sensorPkg_fusionMsg_imuOmega_tag         7
#define sensorPkg_fusionMsg_imuAbsoluteLinearSpeed_tag 8
#define sensorPkg_positionMsg_imuPosition_tag    1
#define sensorPkg_positionMsg_beacons_tag        2
#define sensorPkg_positionMsg_uwbPosition_tag    3
#define sensorPkg_positionMsg_speedMagCms_tag    4
#define sensorPkg_stimulationCommand_ch1En_tag   1
#define sensorPkg_stimulationCommand_ch2En_tag   2
#define sensorPkg_stimulationCommand_ch3En_tag   3
#define sensorPkg_devInfo_batteryMV_tag          1
#define sensorPkg_sensorMsg_fusionMsg_tag        1
#define sensorPkg_sensorMsg_positionMsg_tag      2
#define sensorPkg_sensorMsg_channelCfg_tag       3
#define sensorPkg_sensorMsg_stimulationCommand_tag 4
#define sensorPkg_sensorMsg_stimulationMsg_tag   5
#define sensorPkg_sensorMsg_devInfo_tag          6
#define sensorPkg_sensorMsg_navigationMsg_tag    7

/* Struct field encoding specification for nanopb */
#define sensorPkg_channelConfigMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, BOOL,     enable,            1) \
X(a, STATIC,   SINGULAR, UINT32,   duration,          2) \
X(a, STATIC,   SINGULAR, UINT32,   ton1,              3) \
X(a, STATIC,   SINGULAR, UINT32,   toff1,             4) \
X(a, STATIC,   SINGULAR, UINT32,   ton2,              5) \
X(a, STATIC,   SINGULAR, UINT32,   toff2,             6) \
X(a, STATIC,   SINGULAR, UINT32,   cycle,             7) \
X(a, STATIC,   SINGULAR, UINT32,   rest,              8) \
X(a, STATIC,   SINGULAR, UINT32,   amp,               9) \
X(a, STATIC,   SINGULAR, UINT32,   chId,             10)
#define sensorPkg_channelConfigMsg_CALLBACK NULL
#define sensorPkg_channelConfigMsg_DEFAULT NULL

#define sensorPkg_channelsConfigMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, BOOL,     stiStartStop,      1) \
X(a, STATIC,   SINGULAR, UINT32,   gain,              2) \
X(a, STATIC,   SINGULAR, UINT32,   command,           3) \
X(a, STATIC,   REPEATED, MESSAGE,  config,            4)
#define sensorPkg_channelsConfigMsg_CALLBACK NULL
#define sensorPkg_channelsConfigMsg_DEFAULT NULL
#define sensorPkg_channelsConfigMsg_config_MSGTYPE sensorPkg_channelConfigMsg

#define sensorPkg_position_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    x,                 1) \
X(a, STATIC,   SINGULAR, FLOAT,    y,                 2) \
X(a, STATIC,   SINGULAR, FLOAT,    z,                 3)
#define sensorPkg_position_CALLBACK NULL
#define sensorPkg_position_DEFAULT NULL

#define sensorPkg_vectorChannel_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    x,                 1) \
X(a, STATIC,   SINGULAR, FLOAT,    y,                 2) \
X(a, STATIC,   SINGULAR, FLOAT,    z,                 3)
#define sensorPkg_vectorChannel_CALLBACK NULL
#define sensorPkg_vectorChannel_DEFAULT NULL

#define sensorPkg_beacon_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   id,                1) \
X(a, STATIC,   SINGULAR, FLOAT,    distance,          2)
#define sensorPkg_beacon_CALLBACK NULL
#define sensorPkg_beacon_DEFAULT NULL

#define sensorPkg_quaternion_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    q0,                1) \
X(a, STATIC,   SINGULAR, FLOAT,    q1,                2) \
X(a, STATIC,   SINGULAR, FLOAT,    q2,                3) \
X(a, STATIC,   SINGULAR, FLOAT,    q3,                4)
#define sensorPkg_quaternion_CALLBACK NULL
#define sensorPkg_quaternion_DEFAULT NULL

#define sensorPkg_fusionMsg_FIELDLIST(X, a) \
X(a, STATIC,   REPEATED, FLOAT,    fOmega,            1) \
X(a, STATIC,   SINGULAR, FLOAT,    fPhiPl,            2) \
X(a, STATIC,   SINGULAR, FLOAT,    fThePl,            3) \
X(a, STATIC,   SINGULAR, FLOAT,    fPsiPl,            4) \
X(a, STATIC,   OPTIONAL, MESSAGE,  fqPl,              5) \
X(a, STATIC,   OPTIONAL, MESSAGE,  fAccGl,            6) \
X(a, STATIC,   SINGULAR, FLOAT,    imuOmega,          7) \
X(a, STATIC,   SINGULAR, FLOAT,    imuAbsoluteLinearSpeed,   8)
#define sensorPkg_fusionMsg_CALLBACK NULL
#define sensorPkg_fusionMsg_DEFAULT NULL
#define sensorPkg_fusionMsg_fqPl_MSGTYPE sensorPkg_quaternion
#define sensorPkg_fusionMsg_fAccGl_MSGTYPE sensorPkg_vectorChannel

#define sensorPkg_positionMsg_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  imuPosition,       1) \
X(a, STATIC,   REPEATED, MESSAGE,  beacons,           2) \
X(a, STATIC,   OPTIONAL, MESSAGE,  uwbPosition,       3) \
X(a, STATIC,   SINGULAR, FLOAT,    speedMagCms,       4)
#define sensorPkg_positionMsg_CALLBACK NULL
#define sensorPkg_positionMsg_DEFAULT NULL
#define sensorPkg_positionMsg_imuPosition_MSGTYPE sensorPkg_position
#define sensorPkg_positionMsg_beacons_MSGTYPE sensorPkg_beacon
#define sensorPkg_positionMsg_uwbPosition_MSGTYPE sensorPkg_position

#define sensorPkg_stimulationCommand_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, BOOL,     ch1En,             1) \
X(a, STATIC,   SINGULAR, BOOL,     ch2En,             2) \
X(a, STATIC,   SINGULAR, BOOL,     ch3En,             3)
#define sensorPkg_stimulationCommand_CALLBACK NULL
#define sensorPkg_stimulationCommand_DEFAULT NULL

#define sensorPkg_devInfo_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    batteryMV,         1)
#define sensorPkg_devInfo_CALLBACK NULL
#define sensorPkg_devInfo_DEFAULT NULL

#define sensorPkg_sensorMsg_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,fusionMsg,type.fusionMsg),   1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,positionMsg,type.positionMsg),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,channelCfg,type.channelCfg),   3) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,stimulationCommand,type.stimulationCommand),   4) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,stimulationMsg,type.stimulationMsg),   5) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,devInfo,type.devInfo),   6) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,navigationMsg,type.navigationMsg),   7)
#define sensorPkg_sensorMsg_CALLBACK NULL
#define sensorPkg_sensorMsg_DEFAULT NULL
#define sensorPkg_sensorMsg_type_fusionMsg_MSGTYPE sensorPkg_fusionMsg
#define sensorPkg_sensorMsg_type_positionMsg_MSGTYPE sensorPkg_positionMsg
#define sensorPkg_sensorMsg_type_channelCfg_MSGTYPE sensorPkg_channelsConfigMsg
#define sensorPkg_sensorMsg_type_stimulationCommand_MSGTYPE sensorPkg_stimulationCommand
#define sensorPkg_sensorMsg_type_stimulationMsg_MSGTYPE stimulationPkg_stimulationMsg
#define sensorPkg_sensorMsg_type_devInfo_MSGTYPE sensorPkg_devInfo
#define sensorPkg_sensorMsg_type_navigationMsg_MSGTYPE navigationPkg_navigationMsg

extern const pb_msgdesc_t sensorPkg_channelConfigMsg_msg;
extern const pb_msgdesc_t sensorPkg_channelsConfigMsg_msg;
extern const pb_msgdesc_t sensorPkg_position_msg;
extern const pb_msgdesc_t sensorPkg_vectorChannel_msg;
extern const pb_msgdesc_t sensorPkg_beacon_msg;
extern const pb_msgdesc_t sensorPkg_quaternion_msg;
extern const pb_msgdesc_t sensorPkg_fusionMsg_msg;
extern const pb_msgdesc_t sensorPkg_positionMsg_msg;
extern const pb_msgdesc_t sensorPkg_stimulationCommand_msg;
extern const pb_msgdesc_t sensorPkg_devInfo_msg;
extern const pb_msgdesc_t sensorPkg_sensorMsg_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define sensorPkg_channelConfigMsg_fields &sensorPkg_channelConfigMsg_msg
#define sensorPkg_channelsConfigMsg_fields &sensorPkg_channelsConfigMsg_msg
#define sensorPkg_position_fields &sensorPkg_position_msg
#define sensorPkg_vectorChannel_fields &sensorPkg_vectorChannel_msg
#define sensorPkg_beacon_fields &sensorPkg_beacon_msg
#define sensorPkg_quaternion_fields &sensorPkg_quaternion_msg
#define sensorPkg_fusionMsg_fields &sensorPkg_fusionMsg_msg
#define sensorPkg_positionMsg_fields &sensorPkg_positionMsg_msg
#define sensorPkg_stimulationCommand_fields &sensorPkg_stimulationCommand_msg
#define sensorPkg_devInfo_fields &sensorPkg_devInfo_msg
#define sensorPkg_sensorMsg_fields &sensorPkg_sensorMsg_msg

/* Maximum encoded size of messages (where known) */
#define sensorPkg_beacon_size                    11
#define sensorPkg_channelConfigMsg_size          56
#define sensorPkg_channelsConfigMsg_size         246
#define sensorPkg_devInfo_size                   5
#define sensorPkg_fusionMsg_size                 79
#define sensorPkg_positionMsg_size               91
#define sensorPkg_position_size                  15
#define sensorPkg_quaternion_size                20
#define sensorPkg_sensorMsg_size                 249
#define sensorPkg_stimulationCommand_size        6
#define sensorPkg_vectorChannel_size             15

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
