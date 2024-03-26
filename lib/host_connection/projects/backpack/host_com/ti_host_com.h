/************************************************************************************************************
Module:       ti_host_com

Revision:     1.0

Description:  --- 

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
05/03/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH.  ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

#ifndef _TI_HOST_COM_H
#define _TI_HOST_COM_H

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include <stdbool.h>
#include <string.h>
#include <stdint.h>



//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################
#define UART_RECEIVE_LEN   (180)


//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef void (*post_event_f)(void);
typedef bool (*rx_process_f)(uint32_t des_addr, uint8_t *p_data, uint32_t data_len);

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################



//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
void host_com_init(uint32_t baudRate, post_event_f post_event_cb, rx_process_f rx_process_cb);
bool host_com_rx_process(void);
bool host_com_write(uint8_t *p_data, uint32_t data_len);
void ti_host_com_forward(uint8_t *p_data, uint32_t data_len, uint32_t src_addr);


//###########################################################################################################
// END OF ti_host_com.h
//###########################################################################################################
#endif