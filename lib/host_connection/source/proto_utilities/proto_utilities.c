/************************************************************************************************************
Module:       proto_utilities

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
04/15/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "proto_utilities.h"


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
  proto_encode_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode a message using protobuf
Notes:
  ---
Author, Date:
  Toan Huynh, 04/15/2022
*********************************************************************************/
uint32_t proto_encode_msg(uint8_t *p_buf, uint16_t buf_size, const pb_msgdesc_t *fields, const void *src_struct)
{
  if (p_buf == NULL || buf_size == 0 || fields == NULL || src_struct == NULL)
  {
    return 0;
  }

  pb_ostream_t stream = pb_ostream_from_buffer(p_buf, buf_size);

  if (pb_encode(&stream, fields, src_struct))
  {
  }

  return stream.bytes_written;
}

/********************************************************************************
Function:
  proto_encode_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Encode a message using protobuf
Notes:
  ---
Author, Date:
  Toan Huynh, 04/15/2022
*********************************************************************************/
uint32_t proto_encode_delimited_msg(uint8_t *p_buf, uint16_t buf_size, const pb_msgdesc_t *fields, const void *src_struct)
{
  if (p_buf == NULL || buf_size == 0 || fields == NULL || src_struct == NULL)
  {
    return 0;
  }

  pb_ostream_t stream = pb_ostream_from_buffer(p_buf, buf_size);

  if (pb_encode_delimited(&stream, fields, src_struct))
  {
  }

  return stream.bytes_written;
}

/********************************************************************************
Function:
  proto_decode_msg()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Decode a message using protobuf
Notes:
  ---
Author, Date:
  Toan Huynh, 04/28/2022
*********************************************************************************/
bool proto_decode_msg(uint8_t *buffer, uint32_t message_length, const pb_msgdesc_t *fields, void *dest_struct)
{
  /* Create a stream that reads from the buffer. */
  pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

  /* Now we are ready to decode the message. */
  bool status = pb_decode(&stream, fields, dest_struct);

  return status;
}


/********************************************************************************
Function:
  proto_write_string()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Write encoded string to stream
Notes:
  ---
Author, Date:
  Toan Huynh, 04/14/2022
*********************************************************************************/
bool proto_write_string(pb_ostream_t *stream, const pb_field_t *field, void *const *arg)
{
  return pb_encode_tag_for_field(stream, field) &&
         pb_encode_string(stream, *arg, strlen(*arg));
}

/********************************************************************************
Function:
  proto_write_bytes()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  ---
Notes:
  ---
Author, Date:
  Toan Huynh, 06/16/2022
*********************************************************************************/
bool proto_write_bytes(pb_ostream_t *ostream, const pb_field_t *field, void *const *arg)
{
  if ((NULL == arg) || (NULL == *arg))
  {
    return false;
  }

  const pb_buffer_t *msg = (const pb_buffer_t *)(*arg);

  return pb_encode_tag_for_field(ostream, field) &&
         pb_encode_string(ostream, msg->buffer, msg->size);
}

/********************************************************************************
Input:
  ---
Output:
  ---
Description:
  ---
Author, Date:
  Toan Huynh, 11/13/2023
*********************************************************************************/
bool proto_write_buffer(pb_ostream_t *ostream, const pb_field_t *field, void *const *arg)
{
  if ((NULL == arg) || (NULL == *arg))
  {
    return false;
  }

  const pb_buffer_t *msg = (const pb_buffer_t *)(*arg);

  return pb_encode_tag_for_field(ostream, field) &&
         pb_write(ostream, msg->buffer, msg->size);
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
//      END OF proto_utilities.c
//###########################################################################################################
