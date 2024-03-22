/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.7 */

#ifndef PB_ALGOPKG_ALGORITHM_PB_H_INCLUDED
#define PB_ALGOPKG_ALGORITHM_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* ******************************************************************************
  MESSENGER
****************************************************************************** */
typedef struct _algoPkg_humanDetectionMsg {
    uint32_t hdResult;
    uint32_t hdProcessTimeUs;
    uint32_t captureIntervalMs;
    uint32_t camOnTimeMs;
    uint32_t camCircleTimeMs;
} algoPkg_humanDetectionMsg;

typedef struct _algoPkg_blobDetectionMsg {
    uint32_t resultX;
    uint32_t resultY;
} algoPkg_blobDetectionMsg;

typedef struct _algoPkg_hdConfigMsg {
    bool enable;
} algoPkg_hdConfigMsg;

typedef struct _algoPkg_algoMsg {
    pb_size_t which_type;
    union {
        algoPkg_humanDetectionMsg humanDetection;
        algoPkg_blobDetectionMsg blobDetection;
        algoPkg_hdConfigMsg hdConfig;
    } type;
} algoPkg_algoMsg;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define algoPkg_humanDetectionMsg_init_default   {0, 0, 0, 0, 0}
#define algoPkg_blobDetectionMsg_init_default    {0, 0}
#define algoPkg_hdConfigMsg_init_default         {0}
#define algoPkg_algoMsg_init_default             {0, {algoPkg_humanDetectionMsg_init_default}}
#define algoPkg_humanDetectionMsg_init_zero      {0, 0, 0, 0, 0}
#define algoPkg_blobDetectionMsg_init_zero       {0, 0}
#define algoPkg_hdConfigMsg_init_zero            {0}
#define algoPkg_algoMsg_init_zero                {0, {algoPkg_humanDetectionMsg_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define algoPkg_humanDetectionMsg_hdResult_tag   1
#define algoPkg_humanDetectionMsg_hdProcessTimeUs_tag 2
#define algoPkg_humanDetectionMsg_captureIntervalMs_tag 3
#define algoPkg_humanDetectionMsg_camOnTimeMs_tag 4
#define algoPkg_humanDetectionMsg_camCircleTimeMs_tag 5
#define algoPkg_blobDetectionMsg_resultX_tag     1
#define algoPkg_blobDetectionMsg_resultY_tag     2
#define algoPkg_hdConfigMsg_enable_tag           1
#define algoPkg_algoMsg_humanDetection_tag       1
#define algoPkg_algoMsg_blobDetection_tag        2
#define algoPkg_algoMsg_hdConfig_tag             3

/* Struct field encoding specification for nanopb */
#define algoPkg_humanDetectionMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   hdResult,          1) \
X(a, STATIC,   SINGULAR, UINT32,   hdProcessTimeUs,   2) \
X(a, STATIC,   SINGULAR, UINT32,   captureIntervalMs,   3) \
X(a, STATIC,   SINGULAR, UINT32,   camOnTimeMs,       4) \
X(a, STATIC,   SINGULAR, UINT32,   camCircleTimeMs,   5)
#define algoPkg_humanDetectionMsg_CALLBACK NULL
#define algoPkg_humanDetectionMsg_DEFAULT NULL

#define algoPkg_blobDetectionMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT32,   resultX,           1) \
X(a, STATIC,   SINGULAR, UINT32,   resultY,           2)
#define algoPkg_blobDetectionMsg_CALLBACK NULL
#define algoPkg_blobDetectionMsg_DEFAULT NULL

#define algoPkg_hdConfigMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, BOOL,     enable,            1)
#define algoPkg_hdConfigMsg_CALLBACK NULL
#define algoPkg_hdConfigMsg_DEFAULT NULL

#define algoPkg_algoMsg_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,humanDetection,type.humanDetection),   1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,blobDetection,type.blobDetection),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,hdConfig,type.hdConfig),   3)
#define algoPkg_algoMsg_CALLBACK NULL
#define algoPkg_algoMsg_DEFAULT NULL
#define algoPkg_algoMsg_type_humanDetection_MSGTYPE algoPkg_humanDetectionMsg
#define algoPkg_algoMsg_type_blobDetection_MSGTYPE algoPkg_blobDetectionMsg
#define algoPkg_algoMsg_type_hdConfig_MSGTYPE algoPkg_hdConfigMsg

extern const pb_msgdesc_t algoPkg_humanDetectionMsg_msg;
extern const pb_msgdesc_t algoPkg_blobDetectionMsg_msg;
extern const pb_msgdesc_t algoPkg_hdConfigMsg_msg;
extern const pb_msgdesc_t algoPkg_algoMsg_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define algoPkg_humanDetectionMsg_fields &algoPkg_humanDetectionMsg_msg
#define algoPkg_blobDetectionMsg_fields &algoPkg_blobDetectionMsg_msg
#define algoPkg_hdConfigMsg_fields &algoPkg_hdConfigMsg_msg
#define algoPkg_algoMsg_fields &algoPkg_algoMsg_msg

/* Maximum encoded size of messages (where known) */
#define algoPkg_algoMsg_size                     32
#define algoPkg_blobDetectionMsg_size            12
#define algoPkg_hdConfigMsg_size                 2
#define algoPkg_humanDetectionMsg_size           30

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
