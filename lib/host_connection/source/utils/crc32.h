/*************************************************************************************************/
/*!
 *  \file   crc32.h
 *
 *  \brief  CRC-32 utilities.
 *
 *          $Date: 2016-12-28 16:12:14 -0600 (Wed, 28 Dec 2016) $
 *          $Revision: 10805 $
 *
 *  Copyright (c) 2010-2017 ARM Ltd., all rights reserved.
 *  ARM Ltd. confidential and proprietary.
 *
 *  IMPORTANT.  Your use of this file is governed by a Software License Agreement
 *  ("Agreement") that must be accepted in order to download or otherwise receive a
 *  copy of this file.  You may not use or copy this file for any purpose other than
 *  as described in the Agreement.  If you do not agree to all of the terms of the
 *  Agreement do not use this file and delete all copies in your possession or control;
 *  if you do not have a copy of the Agreement, you must contact ARM Ltd. prior
 *  to any use, copying or further distribution of this software.
 */
/*************************************************************************************************/

#ifndef CRC32_H
#define CRC32_H

#ifdef __cplusplus
extern "C" {
#endif

//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################
typedef enum
{
  CRC32,
  CRC32C,
  NUM_CRC_TYPE
} crc_t;

//############################################################################################################
// DEFINED CONSTANTS
//############################################################################################################

// crc32 used for normal files and normal tranmission
#define CRC_START_NORMAL                (0)

/*************************************************************************************************/
/*!
 *  \fn     crc32_calc
 *
 *  \brief  Calculate the CRC-32 of the given buffer.
 *
 *  \param  crcInit  Initial value of the CRC.
 *  \param  len      Length of the buffer.
 *  \param  pBuf     Buffer to compute the CRC.
 *
 *  \return None.
 *
 *  This routine was originally generated with crcmod.py using the following parameters:
 *    - polynomial 0x104C11DB7
 *    - bit reverse algorithm
 */
/*************************************************************************************************/
uint32_t crc32_calc(uint32_t crcInit, uint32_t len, uint8_t *pBuf);
  
/*************************************************************************************************/
/*!
 *  \fn     crc32c_calc
 *
 *  \brief  Calculate the CRC-32C of the given buffer.
 *
 *  \param  crcInit  Initial value of the CRC.
 *  \param  len      Length of the buffer.
 *  \param  pBuf     Buffer to compute the CRC.
 *
 *  \return None.
 *
 *  Implement CRC32-C along with CRC32 to fix the issue that has the same CRC32 with 2 different
 *  files. We will calculate CRC32-C for each file data (without file header) and CRC32 for
 *  the whole file (used in file transfered) to fix this issue.
 */
/*************************************************************************************************/
uint32_t crc32c_calc(uint32_t crcInit, uint32_t len, uint8_t *pBuf);

/*************************************************************************************************/
/*!
 *  \fn     crc32c_chunk_calc
 *
 *  \brief  Calculate the CRC-32C of the given buffer. This can be used to calculate
 the crc32 of a buffer in chunks by passing using the calculated
 crc of the previous chunk as the inital crc-seed in the current
 chunk.
 *
 *  \param  crcInit     Initial value of the CRC.
 *  \param  len         Length of the buffer.
 *  \param  pBuf        Buffer to compute the CRC.
 *  \param  total_len   Pointer to total data length (Content of this pointer
 *                      will be changed).
 *
 *  \return crc32 value.
 *
 */
/*************************************************************************************************/
uint32_t crc32c_chunk_calc(uint32_t crc_init, uint32_t len, uint8_t *p_buff,
                           uint32_t *total_len);

/*!
 *  \fn     crc32_chunk_calc
 *          
 *  \brief  Calculate the CRC-32 of the given buffer. This can be used to calculate
            the crc32 of a buffer in chunks by passing using the calculated
            crc of the previous chunk as the inital crc-seed in the current
            chunk.
 *
 *  \param  crcInit     Initial value of the CRC.
 *  \param  len         Length of the buffer.
 *  \param  pBuf        Buffer to compute the CRC.
 *  \param  total_len   Pointer to total data length (Content of this pointer 
 *                      will be changed).
 *
 *  \return crc32 value.
 *
 *  This routine was originally generated with crcmod.py using the following parameters:
 *    - polynomial 0x104C11DB7
 *    - bit reverse algorithm
 */
/*************************************************************************************************/
uint32_t crc32_chunk_calc(uint32_t crc_init, uint32_t len, uint8_t *p_buff,
                          uint32_t *total_len);
#ifdef __cplusplus
};
#endif

#endif /* CRC32_H */
