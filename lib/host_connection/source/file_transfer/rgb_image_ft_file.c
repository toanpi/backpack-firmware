/************************************************************************************************************
Module:       hwlog_ft_file

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/16/2023    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2023 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "file_transfer.h"
#include "file-transfer.pb.h"
#include "rgb_cam_control.h"


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define WIDTH     80
#define HEIGHT    60


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

static unsigned int buffer_image[HEIGHT][WIDTH];

//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static bool open(void)
{
  return true;
}

static bool close(void)
{
  memset(buffer_image, 0, sizeof(buffer_image));
  return true;

}
static bool delete(void)
{
  return true;
}

static bool read(uint8_t *p_buf, uint32_t buf_len, uint32_t offset)
{
  return rgb_cam_read(p_buf, buf_len, offset);
}

static uint32_t get_file_size(void)
{
  uint32_t height = 0;
  uint32_t width = 0;

  rgb_image_size(&width, &height);

  return width * height;
}

static bool image_size(uint32_t *p_width, uint32_t* p_height)
{
  return rgb_image_size(p_width, p_height);
}

//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
static ft_send_info_t file_info =
{
  .session_id = 0,
  .type = fileTransferPkg_fileType_RGB_IMAGE,
  .del_when_done = true,
  .open = open,
  .close = close,
  .delete = delete,
  .file_size = get_file_size,
  .read = read,
  .image_size = image_size
};

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
bool rgb_image_ft_file_init(void)
{
  return file_transfer_register(&file_info, fileTransferPkg_fileType_RGB_IMAGE);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 05/19/2023
*********************************************************************************/
ft_send_info_t *rgb_image_file_get_info(void)
{
  return &file_info;
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
//      END OF hwlog_ft_file.c
//###########################################################################################################