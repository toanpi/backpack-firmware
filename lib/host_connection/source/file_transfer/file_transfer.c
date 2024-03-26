/************************************************************************************************************
Module:       file_transfer

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
06/16/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <string.h>
#include "pb_encode.h"
#include "proto_utilities.h"
#include "file-transfer.pb.h"
#include "backpack_parser.h"
#include "host_connection.h"
#include "file_transfer.h"
// #include "system_utils.h"
#include <port_mcu.h>




//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define FT_CHUNK_SIZE_BYTES          (80)
#define MAX_FILE                     (10)


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

static ft_send_info_t *file_man[MAX_FILE] = {0};

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool file_transfer_process_msg(void *p_msg);
static bool sendGetFileRes(uint32_t session_id,
                           uint8_t file_type,
                           fileTransferPkg_fileError error,
                           ft_send_cfg_t *handler);

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  file_transfer_init()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Initializes the file transfer module.
Notes:
  ---
Author, Date:
  Toan Huynh, 06/20/2022
*********************************************************************************/
void file_transfer_init(void)
{
  /* Initialize the file transfer module */
  backpack_decode_register(backpackPkg_backpackMsg_fileMsg_tag, file_transfer_process_msg);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 05/16/2023
*********************************************************************************/
bool file_transfer_register(ft_send_info_t *p_file, uint32_t fileType)
{
  fileTransferPkg_fileType file = (fileTransferPkg_fileType)fileType;

  if (file < MAX_FILE)
  {
    file_man[file] = p_file;
    return true;
  }

  return false;
}

/********************************************************************************
Function:
  file_transfer_send_hwlog_file()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Transmits the hwlog file to the host.
Notes:
  ---
Author, Date:
  Toan Huynh, 06/16/2022
*********************************************************************************/
bool file_transfer_send_file(ft_send_info_t *p_file_info, ft_send_cfg_t *handler)
{
  /* Open the hwlog file */
  if (!p_file_info || !handler->send || !p_file_info->open())
  {
    return sendGetFileRes(p_file_info->session_id,
                          p_file_info->type,
                          fileTransferPkg_fileError_OPEN_FILE_ERROR,
                          handler);
  }

  /* Read the hwlog file in chunk */
  uint32_t remain_length = p_file_info->file_size();
  uint32_t cur_file_size = remain_length;	
  uint32_t chunk_size = handler->chunk_size;
  uint8_t buf[chunk_size];
  uint32_t offset = 0;
  bool ret = true;

  file_transfer_data_t file_info = {
      .file_type = p_file_info->type,
      .seq = 0,
      .is_last_packet = false,
      .data_len = 0,
      .p_data = NULL,
      .session_id = p_file_info->session_id,
      .width = 0,
      .height = 0
  };

  db_printf("[File Transfer] Start session %d", p_file_info->session_id);

  while (ret && remain_length > 0)
  {
    uint32_t read_length = MIN(remain_length, chunk_size);

    memset(buf, 0, sizeof(buf));

    if (!p_file_info->read(buf, read_length, offset))
    {
      ret = false;
      break;
    }

    remain_length -= read_length;
    offset += read_length;

    /* Send the chunk to the host */
    file_info.seq++;
    file_info.is_last_packet = (remain_length == 0);
    file_info.data_len = read_length;
    file_info.p_data = buf;

    if (file_info.is_last_packet && p_file_info->image_size)
    {
      p_file_info->image_size(&file_info.width, &file_info.height);
    }

    ret = handler->send((uint8_t *)&file_info, sizeof(file_info), file_transfer_encode_file);
    
    db_printf(".");

    if(handler->send_delay)
    {
      handler->send_delay();
    }

    // if(!ret)
    // {
      // db_printf("\n");
      // db_printf("Failed to send data seq %d - size %d\n", file_info.seq, file_info.data_len)
    // }
    // else
    // {
    //   db_printf("Send seq %d - size %d - last %s\n", file_info.seq, file_info.data_len, file_info.is_last_packet? "true" : "false");
    // }
  }

  if (cur_file_size == 0)
  {
    ret = sendGetFileRes(p_file_info->session_id,
                         p_file_info->type,
                         fileTransferPkg_fileError_FILE_NOT_FOUND, handler);
  }

  /* Delete the hwlog file if necessary */
  if (p_file_info->del_when_done)
  {
    p_file_info->delete();
  }

  /* Close the hwlog file */
  p_file_info->close();

  /* Send the response to the host */
  if (ret == false)
  {
    sendGetFileRes(p_file_info->session_id,
                   p_file_info->type,
                   fileTransferPkg_fileError_READ_FILE_ERROR,
                   handler);
  }

  db_printf(" %s\n", ret ? "Done" : "Error");

  return ret;
}

/********************************************************************************
Function:
  file_transfer_encode_hwlog_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encodes the hwlog via file transfer protocol.
Notes:
  ---
Author, Date:
  Toan Huynh, 06/16/2022
*********************************************************************************/
bool file_transfer_encode_file(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t data_len, uint32_t *p_written_len)
{
  fileTransferPkg_fileMsg pkg = fileTransferPkg_fileMsg_init_zero;
  pkg.which_type = fileTransferPkg_fileMsg_fileData_tag;
  fileTransferPkg_fileDataMsg *msg = &pkg.type.fileData;
  file_transfer_data_t *p_file_info = (file_transfer_data_t *)p_data;

  msg->fileType = (fileTransferPkg_fileType)p_file_info->file_type;
  msg->seq = p_file_info->seq;
  msg->isLast = p_file_info->is_last_packet;
  msg->length = p_file_info->data_len;
  msg->sessionId = p_file_info->session_id;

  if(p_file_info->p_data && p_file_info->data_len > 0)
  {
    pb_buffer_t pb_data = {
        .buffer = p_file_info->p_data,
        .size = p_file_info->data_len,
    };

    msg->data.arg = &pb_data;
    msg->data.funcs.encode = proto_write_bytes;
  }

  msg->error = p_file_info->error;
  msg->width = p_file_info->width;
  msg->height = p_file_info->height;

  return backpack_encode_file_transfer_msg(p_buf, buf_size, p_written_len, &pkg);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 05/18/2023
*********************************************************************************/
bool file_transfer_get_file_req(uint8_t *p_buf, uint16_t buf_size, uint8_t *p_data, uint32_t data_len, uint32_t *p_written_len)
{
  fileTransferPkg_fileMsg pkg = fileTransferPkg_fileMsg_init_zero;
  pkg.which_type = fileTransferPkg_fileMsg_getFile_tag;

  fileTransferPkg_getFileMsg *msg = &pkg.type.getFile;
  
  ft_send_info_t *p_file_info = (ft_send_info_t *)p_data;

  msg->fileType = p_file_info->type;
  msg->deleteWhenDone = false;
  msg->sessionId = 0;

  return backpack_encode_file_transfer_msg(p_buf, buf_size, p_written_len, &pkg);
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  sendGetFileRes()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/21/2022
*********************************************************************************/
static bool sendGetFileRes(uint32_t session_id,
                           uint8_t file_type,
                           fileTransferPkg_fileError error,
                           ft_send_cfg_t *handler)
{
  file_transfer_data_t file_info = {
      .file_type = file_type,
      .seq = 0,
      .is_last_packet = true,
      .data_len = 0,
      .p_data = NULL,
      .session_id = session_id,
      .error = error,
  };

  return handler->send((uint8_t *)&file_info, sizeof(file_info), file_transfer_encode_file);
  // return host_connection_send(&file_info, sizeof(file_info), file_transfer_encode_file) == HOST_CONNECTION_SUCCESS;
}

/********************************************************************************
Function:
  file_transfer_process_get_file()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Processes the get file message.
Notes:
  ---
Author, Date:
  Toan Huynh, 06/18/2022
*********************************************************************************/
static bool file_transfer_process_get_file(fileTransferPkg_getFileMsg *p_msg)
{
  if (p_msg->fileType >= MAX_FILE)
  {
    return false;
  }

  file_man[p_msg->fileType]->del_when_done = p_msg->deleteWhenDone;
  file_man[p_msg->fileType]->session_id = p_msg->sessionId;

  ft_send_cfg_t cfg = {
      .send = NULL // TODO
  };

  return file_transfer_send_file(file_man[p_msg->fileType], &cfg);
}
/********************************************************************************
Function:
  collector_process_dev_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Processes the file transfer message.
Notes:
  ---
Author, Date:
  Toan Huynh, 05/03/2022
*********************************************************************************/
static bool file_transfer_process_msg(void *p_msg)
{
  if (!p_msg)
  {
    return false;
  }

  fileTransferPkg_fileMsg *msg = &(((backpackPkg_backpackMsg *)p_msg)->type.fileMsg);
  bool ret = false;

  switch (msg->which_type)
  {
  case fileTransferPkg_fileMsg_getFile_tag:
    file_transfer_process_get_file(&msg->type.getFile);
    break;
  case fileTransferPkg_fileMsg_stopTransfer_tag:
    break;
  case fileTransferPkg_fileMsg_deleteFile_tag:
    break;
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
//      END OF file_transfer.c
//###########################################################################################################
