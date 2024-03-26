/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.7 */

#ifndef PB_FILETRANSFERPKG_FILE_TRANSFER_PB_H_INCLUDED
#define PB_FILETRANSFERPKG_FILE_TRANSFER_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Enum definitions */
typedef enum _fileTransferPkg_fileType {
    fileTransferPkg_fileType_HW_LOG = 0,
    fileTransferPkg_fileType_IR_IMAGE = 1,
    fileTransferPkg_fileType_RGB_IMAGE = 2
} fileTransferPkg_fileType;

typedef enum _fileTransferPkg_fileError {
    fileTransferPkg_fileError_OK = 0,
    fileTransferPkg_fileError_OPEN_FILE_ERROR = 1,
    fileTransferPkg_fileError_WRITE_FILE_ERROR = 2,
    fileTransferPkg_fileError_CLOSE_FILE_ERROR = 3,
    fileTransferPkg_fileError_FILE_NOT_FOUND = 4,
    fileTransferPkg_fileError_READ_FILE_ERROR = 5
} fileTransferPkg_fileError;

/* Struct definitions */
/* ******************************************************************************
  FILE TRANSFER MESSENGER
****************************************************************************** */
typedef struct _fileTransferPkg_fileDataMsg {
    fileTransferPkg_fileType fileType;
    uint32_t sessionId;
    uint32_t seq;
    bool isLast;
    uint32_t length;
    pb_callback_t data;
    fileTransferPkg_fileError error;
    uint32_t width;
    uint32_t height;
} fileTransferPkg_fileDataMsg;

typedef struct _fileTransferPkg_getFileMsg {
    fileTransferPkg_fileType fileType;
    uint32_t sessionId;
    bool deleteWhenDone;
} fileTransferPkg_getFileMsg;

typedef struct _fileTransferPkg_fileInfo {
    fileTransferPkg_fileType fileType;
    uint32_t fileSize;
} fileTransferPkg_fileInfo;

typedef struct _fileTransferPkg_listFilesMsg {
    pb_size_t files_count;
    fileTransferPkg_fileInfo files[2];
} fileTransferPkg_listFilesMsg;

typedef struct _fileTransferPkg_delFilesMsg {
    pb_size_t files_count;
    fileTransferPkg_fileType files[2];
} fileTransferPkg_delFilesMsg;

typedef struct _fileTransferPkg_fileMsg {
    pb_size_t which_type;
    union {
        fileTransferPkg_fileDataMsg fileData;
        fileTransferPkg_getFileMsg getFile;
        fileTransferPkg_listFilesMsg filesInfo;
        bool stopTransfer;
        fileTransferPkg_delFilesMsg deleteFile;
        bool listFiles;
    } type;
} fileTransferPkg_fileMsg;


#ifdef __cplusplus
extern "C" {
#endif

/* Helper constants for enums */
#define _fileTransferPkg_fileType_MIN fileTransferPkg_fileType_HW_LOG
#define _fileTransferPkg_fileType_MAX fileTransferPkg_fileType_RGB_IMAGE
#define _fileTransferPkg_fileType_ARRAYSIZE ((fileTransferPkg_fileType)(fileTransferPkg_fileType_RGB_IMAGE+1))

#define _fileTransferPkg_fileError_MIN fileTransferPkg_fileError_OK
#define _fileTransferPkg_fileError_MAX fileTransferPkg_fileError_READ_FILE_ERROR
#define _fileTransferPkg_fileError_ARRAYSIZE ((fileTransferPkg_fileError)(fileTransferPkg_fileError_READ_FILE_ERROR+1))

#define fileTransferPkg_fileDataMsg_fileType_ENUMTYPE fileTransferPkg_fileType
#define fileTransferPkg_fileDataMsg_error_ENUMTYPE fileTransferPkg_fileError

#define fileTransferPkg_getFileMsg_fileType_ENUMTYPE fileTransferPkg_fileType

#define fileTransferPkg_fileInfo_fileType_ENUMTYPE fileTransferPkg_fileType


#define fileTransferPkg_delFilesMsg_files_ENUMTYPE fileTransferPkg_fileType



/* Initializer values for message structs */
#define fileTransferPkg_fileDataMsg_init_default {_fileTransferPkg_fileType_MIN, 0, 0, 0, 0, {{NULL}, NULL}, _fileTransferPkg_fileError_MIN, 0, 0}
#define fileTransferPkg_getFileMsg_init_default  {_fileTransferPkg_fileType_MIN, 0, 0}
#define fileTransferPkg_fileInfo_init_default    {_fileTransferPkg_fileType_MIN, 0}
#define fileTransferPkg_listFilesMsg_init_default {0, {fileTransferPkg_fileInfo_init_default, fileTransferPkg_fileInfo_init_default}}
#define fileTransferPkg_delFilesMsg_init_default {0, {_fileTransferPkg_fileType_MIN, _fileTransferPkg_fileType_MIN}}
#define fileTransferPkg_fileMsg_init_default     {0, {fileTransferPkg_fileDataMsg_init_default}}
#define fileTransferPkg_fileDataMsg_init_zero    {_fileTransferPkg_fileType_MIN, 0, 0, 0, 0, {{NULL}, NULL}, _fileTransferPkg_fileError_MIN, 0, 0}
#define fileTransferPkg_getFileMsg_init_zero     {_fileTransferPkg_fileType_MIN, 0, 0}
#define fileTransferPkg_fileInfo_init_zero       {_fileTransferPkg_fileType_MIN, 0}
#define fileTransferPkg_listFilesMsg_init_zero   {0, {fileTransferPkg_fileInfo_init_zero, fileTransferPkg_fileInfo_init_zero}}
#define fileTransferPkg_delFilesMsg_init_zero    {0, {_fileTransferPkg_fileType_MIN, _fileTransferPkg_fileType_MIN}}
#define fileTransferPkg_fileMsg_init_zero        {0, {fileTransferPkg_fileDataMsg_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define fileTransferPkg_fileDataMsg_fileType_tag 1
#define fileTransferPkg_fileDataMsg_sessionId_tag 2
#define fileTransferPkg_fileDataMsg_seq_tag      3
#define fileTransferPkg_fileDataMsg_isLast_tag   4
#define fileTransferPkg_fileDataMsg_length_tag   5
#define fileTransferPkg_fileDataMsg_data_tag     6
#define fileTransferPkg_fileDataMsg_error_tag    7
#define fileTransferPkg_fileDataMsg_width_tag    8
#define fileTransferPkg_fileDataMsg_height_tag   9
#define fileTransferPkg_getFileMsg_fileType_tag  1
#define fileTransferPkg_getFileMsg_sessionId_tag 2
#define fileTransferPkg_getFileMsg_deleteWhenDone_tag 3
#define fileTransferPkg_fileInfo_fileType_tag    1
#define fileTransferPkg_fileInfo_fileSize_tag    2
#define fileTransferPkg_listFilesMsg_files_tag   1
#define fileTransferPkg_delFilesMsg_files_tag    1
#define fileTransferPkg_fileMsg_fileData_tag     1
#define fileTransferPkg_fileMsg_getFile_tag      2
#define fileTransferPkg_fileMsg_filesInfo_tag    3
#define fileTransferPkg_fileMsg_stopTransfer_tag 4
#define fileTransferPkg_fileMsg_deleteFile_tag   5
#define fileTransferPkg_fileMsg_listFiles_tag    6

/* Struct field encoding specification for nanopb */
#define fileTransferPkg_fileDataMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    fileType,          1) \
X(a, STATIC,   SINGULAR, UINT32,   sessionId,         2) \
X(a, STATIC,   SINGULAR, UINT32,   seq,               3) \
X(a, STATIC,   SINGULAR, BOOL,     isLast,            4) \
X(a, STATIC,   SINGULAR, UINT32,   length,            5) \
X(a, CALLBACK, SINGULAR, BYTES,    data,              6) \
X(a, STATIC,   SINGULAR, UENUM,    error,             7) \
X(a, STATIC,   SINGULAR, UINT32,   width,             8) \
X(a, STATIC,   SINGULAR, UINT32,   height,            9)
#define fileTransferPkg_fileDataMsg_CALLBACK pb_default_field_callback
#define fileTransferPkg_fileDataMsg_DEFAULT NULL

#define fileTransferPkg_getFileMsg_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    fileType,          1) \
X(a, STATIC,   SINGULAR, UINT32,   sessionId,         2) \
X(a, STATIC,   SINGULAR, BOOL,     deleteWhenDone,    3)
#define fileTransferPkg_getFileMsg_CALLBACK NULL
#define fileTransferPkg_getFileMsg_DEFAULT NULL

#define fileTransferPkg_fileInfo_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UENUM,    fileType,          1) \
X(a, STATIC,   SINGULAR, UINT32,   fileSize,          2)
#define fileTransferPkg_fileInfo_CALLBACK NULL
#define fileTransferPkg_fileInfo_DEFAULT NULL

#define fileTransferPkg_listFilesMsg_FIELDLIST(X, a) \
X(a, STATIC,   REPEATED, MESSAGE,  files,             1)
#define fileTransferPkg_listFilesMsg_CALLBACK NULL
#define fileTransferPkg_listFilesMsg_DEFAULT NULL
#define fileTransferPkg_listFilesMsg_files_MSGTYPE fileTransferPkg_fileInfo

#define fileTransferPkg_delFilesMsg_FIELDLIST(X, a) \
X(a, STATIC,   REPEATED, UENUM,    files,             1)
#define fileTransferPkg_delFilesMsg_CALLBACK NULL
#define fileTransferPkg_delFilesMsg_DEFAULT NULL

#define fileTransferPkg_fileMsg_FIELDLIST(X, a) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,fileData,type.fileData),   1) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,getFile,type.getFile),   2) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,filesInfo,type.filesInfo),   3) \
X(a, STATIC,   ONEOF,    BOOL,     (type,stopTransfer,type.stopTransfer),   4) \
X(a, STATIC,   ONEOF,    MESSAGE,  (type,deleteFile,type.deleteFile),   5) \
X(a, STATIC,   ONEOF,    BOOL,     (type,listFiles,type.listFiles),   6)
#define fileTransferPkg_fileMsg_CALLBACK NULL
#define fileTransferPkg_fileMsg_DEFAULT NULL
#define fileTransferPkg_fileMsg_type_fileData_MSGTYPE fileTransferPkg_fileDataMsg
#define fileTransferPkg_fileMsg_type_getFile_MSGTYPE fileTransferPkg_getFileMsg
#define fileTransferPkg_fileMsg_type_filesInfo_MSGTYPE fileTransferPkg_listFilesMsg
#define fileTransferPkg_fileMsg_type_deleteFile_MSGTYPE fileTransferPkg_delFilesMsg

extern const pb_msgdesc_t fileTransferPkg_fileDataMsg_msg;
extern const pb_msgdesc_t fileTransferPkg_getFileMsg_msg;
extern const pb_msgdesc_t fileTransferPkg_fileInfo_msg;
extern const pb_msgdesc_t fileTransferPkg_listFilesMsg_msg;
extern const pb_msgdesc_t fileTransferPkg_delFilesMsg_msg;
extern const pb_msgdesc_t fileTransferPkg_fileMsg_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define fileTransferPkg_fileDataMsg_fields &fileTransferPkg_fileDataMsg_msg
#define fileTransferPkg_getFileMsg_fields &fileTransferPkg_getFileMsg_msg
#define fileTransferPkg_fileInfo_fields &fileTransferPkg_fileInfo_msg
#define fileTransferPkg_listFilesMsg_fields &fileTransferPkg_listFilesMsg_msg
#define fileTransferPkg_delFilesMsg_fields &fileTransferPkg_delFilesMsg_msg
#define fileTransferPkg_fileMsg_fields &fileTransferPkg_fileMsg_msg

/* Maximum encoded size of messages (where known) */
/* fileTransferPkg_fileDataMsg_size depends on runtime parameters */
/* fileTransferPkg_fileMsg_size depends on runtime parameters */
#define fileTransferPkg_delFilesMsg_size         4
#define fileTransferPkg_fileInfo_size            8
#define fileTransferPkg_getFileMsg_size          10
#define fileTransferPkg_listFilesMsg_size        20

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif