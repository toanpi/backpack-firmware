/************************************************************************************************************
Module:       app_main

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/15/2022    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2022 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include "compiler.h"
#include "port_mcu.h"
#include "deca_device_api.h"
#include "deca_spi.h"
#include "deca_regs.h"
#include "lib.h"
#include "system_dbg.h"
#include "dw_main.h"
#include "instance_config.h"
#include "app_state.h"
#include "host_msg.h"
#include "host_com_msg.h"
#include "timestamp.h"
#include "instance_utilities.h"
#include "instance.h"
#include "uwb_dev_driver.h"
#include "app_main.h"
#include "network.h"
#include "distance.h"
#include "uwb_event.h"
#include "tdma_handler.h"
#include "net_node.h"
#include "twr_send.h"
#include "twr_main.h"
#include "network_packet.h"
#include "app_monitor.h"


#if CONFIG_ENABLE_LOCATION_ENGINE
#include "location_engine.h"
#endif


//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################



//###########################################################################################################
//      MACROS
//###########################################################################################################
#define FORCE_UPDATE_TIMEOUT(timeout) 			(*p_timeout = (timeout))

#define UPDATE_TIMEOUT(timeout)                    \
	{                                                \
		if (*p_timeout == 0 || *p_timeout > (timeout)) \
		{                                              \
			*p_timeout = (timeout);                      \
		}                                              \
	}

#define GET_TIMEOUT() 								(*p_timeout)

#define APP_EXEC(process_handler, passed_handler, failed_handler)                                   \
	{                                                                                                 \
		uint32 __timeout = 0;                                                                           \
		bool __ret = app_state_exec(inst, process_handler, passed_handler, failed_handler, &__timeout); \
		if (__ret)                                                                                      \
		{                                                                                               \
			UPDATE_TIMEOUT(__timeout);                                                                    \
			done = INST_DONE_WAIT_FOR_NEXT_EVENT;                                                         \
		}                                                                                               \
	}


#define APP_CHECK_TIMEOUT(_start_time, _duration, _timeout_experied)             \
	uint32 _remaining_time = app_state_wait_timeout(inst, _start_time, _duration); \
	if (_remaining_time)                                                           \
	{                                                                              \
		done = INST_DONE_WAIT_FOR_NEXT_EVENT_TO;                                     \
		UPDATE_TIMEOUT(_remaining_time);                                             \
		_timeout_experied = FALSE;                                                   \
	}                                                                              \
	else                                                                           \
	{                                                                              \
		_timeout_experied = TRUE;                                                    \
	}

#define APP_WAIT_TIMEOUT(_start_time, _duration)              \
	bool timeout_experied = false;                              \
	APP_CHECK_TIMEOUT(_start_time, _duration, timeout_experied) \
	if (timeout_experied == FALSE)                              \
	{                                                           \
		break;                                                    \
	}

//###########################################################################################################
//      MODULE TYPES
//###########################################################################################################



//###########################################################################################################
//      CONSTANTS
//###########################################################################################################



//###########################################################################################################
//      MODULE LEVEL VARIABLES
//###########################################################################################################
static node_joined_cb joined_cb_func = NULL;



//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################
static int app_event_handler(instance_data_t *inst, struct TDMAHandler *tdma_handler, int message, uint32 *p_timeout);
static uwb_msg_info_t app_process_rx_msg(event_data_t *dw_event);
static bool app_handler_rx_packet(instance_data_t *inst, struct TDMAHandler *tdma_handler, event_data_t *dw_event);
static bool app_handle_blink_response(instance_data_t *inst, event_data_t *dw_event);
static bool app_process_rx_timeout(instance_data_t *inst, struct TDMAHandler *tdma_handler);
static bool app_check_event_timeout(instance_data_t *inst, struct TDMAHandler *tdma_handler, uint32 * p_timeout);
static bool app_maintain_receiver(instance_data_t *inst, uint32 *p_timeout);
static void app_join_network_callback(void);
static void app_update_error(instance_data_t *inst, app_uwb_err err);
static bool app_process_host_msg(instance_data_t *inst, uwb_msg_info_t *p_msg);


//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
	app_state_machine_run()
Input Parameters:
	---
Output Parameters:
	---
Description:
	---
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
uint32 app_state_machine_run(instance_data_t *inst,  struct TDMAHandler *tdma_handler)
{
	int done = INST_NOT_DONE_YET;
	int message = instance_peekevent(); // get any of the received events from ISR
	uint32 timeout = 0;

	while (done == INST_NOT_DONE_YET)
	{
		done = app_event_handler(inst, tdma_handler, message, &timeout); // run the communications application
		message = 0;
	}

	if (timeout > 1000000)
	{
		sys_printf("ERROR TIMEOUT %u\n", timeout);
	}

	// only check timeouts if we aren't in the middle of ranging messages
	if (inst->canPrintLCD == TRUE && !inst->isInDeepSleep)
	{
		// check timeouts.
		if (tdma_handler->check_timeouts(tdma_handler))
		{
			println("NO NEIGHBOR TIMEOUT\r\n");
			// handle case where we timeout and no longer have any neighbors
			inst->mode = DISCOVERY;
			tdma_handler->enter_discovery_mode(tdma_handler);
			inst_processtxrxtimeout(inst);
		}
		else if (inst->mode == DISCOVERY)
		{
			if (tdma_handler->discovery_mode_expires == TRUE &&
					tdma_handler->discovery_mode_duration > MAX_WAIT_TIMEOUT_MS)
			{
				// Error when in discovery mode
				println("DISCOVERY DURATION ERROR");
				inst->mode = DISCOVERY;
				tdma_handler->enter_discovery_mode(tdma_handler);
				inst_processtxrxtimeout(inst);
			}
		}
	}

	return timeout;
}

/********************************************************************************
Function:
	app_join_network_cb_register()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Register a callback function to be called when the node joins the network.
Notes:
	---
Author, Date:
	Toan Huynh, 08/01/2022
*********************************************************************************/
void app_join_network_cb_register(node_joined_cb cb)
{
	joined_cb_func = cb;
}

/********************************************************************************
Function:
	inst_processtxrxtimeout()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Process the TX/RX timeout.
Notes:
	---
Author, Date:
	Toan Huynh, 07/15/2022
*********************************************************************************/
void inst_processtxrxtimeout(instance_data_t *inst)
{
	struct TDMAHandler *tdma_handler = (struct TDMAHandler *)get_tdma_handler();

	if (inst->mode == DISCOVERY)
	{
		inst->wait4ack = 0;
		inst->uwbToRangeWith = 255;

		if (tdma_handler->discovery_mode == WAIT_RNG_INIT || tdma_handler->discovery_mode == WAIT_INF_INIT)
		{
			uint32 time_now = timestamp_get_ms();
			tdma_handler->set_discovery_mode(tdma_handler, WAIT_INF_REG, time_now);
		}

		if (tdma_handler->check_blink(tdma_handler) == TRUE)
		{
			inst->testAppState = TA_TX_SELECT;
		}
		else
		{
			inst->testAppState = TA_RXE_WAIT;
		}
	}
	else if (inst->mode == ANCHOR) // we did not receive the final/ACK - wait for next poll
	{
		inst->wait4ack = 0;
		inst->uwbToRangeWith = 255;
		inst->testAppState = TA_RXE_WAIT; // wait for next frame
	}
	else // if(inst->mode == TAG)
	{
		inst->testAppState = TA_TX_SELECT;
	}

	inst->canPrintLCD = TRUE;
	inst->previousState = TA_INIT;

	dwt_setrxtimeout(0); // units are 1.0256us
	// timeout - disable the radio (if using SW timeout the rx will not be off)
	uwb_dev_off_transceiver();
}

//###########################################################################################################
//      PRIVATE FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  app_run()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  the main instance state machine (all the instance modes Tag or Anchor use the same state machine)
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
static int app_event_handler(instance_data_t *inst, struct TDMAHandler *tdma_handler, int message, uint32 *p_timeout)
{
	int done = INST_NOT_DONE_YET;
	bool need_sleep = false;

	if (p_timeout == NULL)
	{
		return done;
	}

	*p_timeout = 0;

	/* Don't process slot transition if in sleep */
	if (inst->testAppState != TA_SLEEP_DONE)
	{
		if (tdma_handler->slot_transition(tdma_handler, p_timeout, &need_sleep))
		{
			updateTestWithTimeout(inst, GET_TIMEOUT());

			done = INST_DONE_WAIT_FOR_NEXT_EVENT;
			message = 0;
		}
	}

	tdma_handler->check_discovery_mode_expiration(tdma_handler, p_timeout, &need_sleep);

	if (message == DWT_SIG_TX_DONE && inst->testAppState != TA_TX_WAIT_CONF)
	{
		// It is possible to get an interrupt which takes the UWB out of TX_WAIT_CONF
		// before we have to process it a DWT_SIG_TX_DONE event.
		// Clear the event in this case
		instance_getevent(11);
		done = INST_DONE_WAIT_FOR_NEXT_EVENT;
	}

	switch (inst->testAppState)
	{
	case TA_INIT:
	{
		switch (inst->mode)
		{
		case DISCOVERY:
		{
			net_discovery_init(inst, tdma_handler);
			inst->testAppState = TA_RXE_WAIT;
		}
		break;
		default:
			break;
		}
		break;
	} // end case TA_INIT
	case TA_TX_SELECT:
	{
		uint32 tx_delay = 0;
		// select a TX action, return TRUE if we should move on to another state
		if (tdma_handler->tx_select(tdma_handler, &tx_delay) == TRUE)
		{
			uwb_dev_off_transceiver();
		}
		else
		{
			UPDATE_TIMEOUT(tx_delay);
			done = INST_DONE_WAIT_FOR_NEXT_EVENT;
			updateTestWithTimeout(inst, GET_TIMEOUT());
		}

		break;
	}
	case TA_TXBLINK_WAIT_SEND:
	{
		APP_EXEC(net_node_blink_send, app_state_tx_confirm, app_state_rx_wait);
		break;
	}
	case TA_TXRANGINGINIT_WAIT_SEND:
	{
		APP_EXEC(net_node_ranging_init_send, app_state_tx_confirm, app_state_rx_wait);
		break;
	}
	case TA_TXINF_WAIT_SEND:
	{
		APP_EXEC(net_node_inf_send, app_state_tx_confirm, app_state_rx_wait);
		break;
	}
	case TA_TXPOLL_WAIT_SEND:
	{
		APP_EXEC(twr_poll_send, app_state_tx_confirm, app_state_rx_wait);
		break;
	}
	case TA_TXREPORT_WAIT_SEND:
	{
		APP_EXEC(twr_report_send, app_state_tx_confirm, app_state_rx_wait);
		break;
	}
	case TA_TX_HOST_MSG_WAIT_SEND:
	{
		/* Turn off the transceiver first then starting it for new packet */
		uwb_dev_off_transceiver();
		APP_EXEC(inst_send_host_msg, app_state_tx_confirm, app_state_rx_wait);
		break;
	}
	case TA_SLEEP_DONE:
	{
		/* Clear the event from the queue */
		instance_getevent(10);

		/* Wait here until timeout is reached */
		APP_WAIT_TIMEOUT(inst->timeofSleep, inst->sleepTime_ms);

		/* Sleep done, go to next state */
		app_state_rx_wait(inst);

		done = INST_NOT_DONE_YET;

		/* Node wakeup */
		net_node_exit_sleep(inst);

		sys_printf("[%u] EXIT SLEEP\r\n", timestamp_get_ms());

		break;
	}
	case TA_TX_WAIT_CONF:
	{
		event_data_t *dw_event = instance_getevent(11); // get and clear this event

		// NOTE: Can get the ACK before the TX confirm event for the frame requesting the ACK
		// this happens because if polling the ISR the RX event will be processed 1st and then the TX event
		// thus the reception of the ACK will be processed before the TX confirmation of the frame that requested it.
		if (dw_event->type != DWT_SIG_TX_DONE) // wait for TX done confirmation
		{
			if (dw_event->type == DWT_SIG_RX_TIMEOUT) // got RX timeout - i.e. did not get the response (e.g. ACK)
			{
				inst->gotTO = TRUE;
			}
			else
			{
				// sometimes the DW1000 tx callback (TXFRS) fails to trigger and the the SYS_STATE register
				// reads IDLE for for PMSC, RX, and TX so we need another way to timeout since RX FWTO won't be triggered.
				APP_CHECK_TIMEOUT(inst->timeofTx, inst->txDoneTimeoutDuration, inst->gotTO);
			}

			if (inst->gotTO == FALSE)
			{
				break;
			}
		}

		done = INST_NOT_DONE_YET;

		if (inst->gotTO) // timeout
		{
			sys_printf("[TX] WAIT CONFIRM TIMEOUT\r\n");
			app_update_error(inst, APP_UWB_WAIT_CONFIRM_TIMEOUT);

			inst_processtxrxtimeout(inst);
			inst->gotTO = FALSE;
			inst->wait4ack = 0; // clear this

			break;
		}
		else
		{
			if (inst->previousState == TA_TXINF_WAIT_SEND)
			{
				// get the message FCODE
				uint8 fcode;
				memcpy(&fcode, &inst->inf_msg.messageData[FCODE], sizeof(uint8));

				// exit discovery mode if we successfully send INF_SUG
				if (fcode == RTLS_DEMO_MSG_INF_SUG)
				{
					net_node_joined_network(inst, tdma_handler, NULL);
					
					app_join_network_callback();
				}

				// if we successfully send out INF_INIT, INF_SUG, or INF_UPDATE, switch to INF_REG
				if (fcode == RTLS_DEMO_MSG_INF_INIT ||
						fcode == RTLS_DEMO_MSG_INF_SUG ||
						fcode == RTLS_DEMO_MSG_INF_UPDATE)
				{
					fcode = RTLS_DEMO_MSG_INF_REG;
					memcpy(&inst->inf_msg.messageData[FCODE], &fcode, sizeof(uint8));
				}

				/* If need send message to host, do it */
				if (inst->operationMode == CONFIG_OPERATION_TAG && inst->mode != DISCOVERY)
				{
					done = INST_NOT_DONE_YET;
					inst->testAppState = TA_TX_HOST_MSG_WAIT_SEND;
					/* Break here to start sending the message to host */
					break;
				}
			}

			// After sending, tag expects response/report, anchor waits to receive a final/new poll
			inst->testAppState = TA_RXE_WAIT; 

			// fall into the next case (turn on the RX)
			message = 0;
		}
	} // end case TA_TX_WAIT_CONF
	case TA_RXE_WAIT:
	{
		/* Safe check if device still is in deep sleep */
		if (inst->isInDeepSleep)
		{
			/* Something go wrong if reaching here, force device exit from sleep mode */
			instance_exit_sleep(inst);
			sys_printf("[ERR] EXIT DEEP SLEEP\r\n");
		}

		if (inst->wait4ack == 0) // if this is set the RX will turn on automatically after TX
		{
			// turn RX on
			instance_rxon(inst, 0, 0); // turn RX on, without delay
		}
		else
		{
			inst->wait4ack = 0; // clear the flag, the next time we want to turn the RX on it might not be auto
		}

		// we are going to use anchor/tag timeout
		done = INST_DONE_WAIT_FOR_NEXT_EVENT; // using RX FWTO

		inst->testAppState = TA_RX_WAIT_DATA; // let this state handle it
		inst->rxCheckOnTime = timestamp_get_ms();

		// end case TA_RXE_WAIT, don't break, but fall through into the TA_RX_WAIT_DATA state to process it immediately.
		// if (message == 0)
		// {
		// 	break;
		// }
	}
	case TA_RX_WAIT_DATA:
	{
		// Wait RX data
		switch (message)
		{
		case DWT_SIG_RX_BLINK:
		{
			event_data_t *dw_event = instance_getevent(12); // get and clear this event

			if (app_handle_blink_response(inst, dw_event) == FALSE)
			{
				/* If not initiating ranging - continue to receive */
				inst->testAppState = TA_RXE_WAIT;
				done = INST_NOT_DONE_YET;
			}

			break;
		}
		case DWT_SIG_RX_OKAY:
		{
			/* Get and clear this event */
			event_data_t *dw_event = instance_getevent(15);
			app_handler_rx_packet(inst, tdma_handler, dw_event);
			break;
		}
		case DWT_SIG_RX_TIMEOUT:
		{
			app_process_rx_timeout(inst, tdma_handler);
			break;
		}
		case DWT_SIG_TX_AA_DONE: // ignore this event - just process the rx frame that was received before the ACK response
		case 0:
		default:
		{
			uint32 rx_timeout = GET_TIMEOUT();

			/* Default is waitting for an event */
			done = INST_DONE_WAIT_FOR_NEXT_EVENT;

			uint32_t sleep_timeout = UINT32_MAX;

			// if tag is scheduled to go to sleep before next sending poll
			if (app_state_process_sleep(inst, tdma_handler, need_sleep, &sleep_timeout))
			{
				/* Device is entered sleep mode successfully */
				/* Set sleep time timeout */
				FORCE_UPDATE_TIMEOUT(inst->sleepTime_ms);
				done = INST_DONE_WAIT_FOR_NEXT_EVENT_TO;
				break;
			}
			else if(sleep_timeout != UINT32_MAX)
			{
				UPDATE_TIMEOUT(sleep_timeout);
			}

			/* Check if we have event timeout */
			if (app_check_event_timeout(inst, tdma_handler, &rx_timeout))
			{
				done = INST_NOT_DONE_YET;
			}
			else
			{
				UPDATE_TIMEOUT(rx_timeout);
			}

			rx_timeout = GET_TIMEOUT();
			/* Check if RX is on every so often. Turn it on if it isn't. */
			app_maintain_receiver(inst, &rx_timeout);

			/* Don't updating timeout if timeout is zero -> Can casue missing slot transition */
			if (rx_timeout < GET_TIMEOUT())
			{
				UPDATE_TIMEOUT(rx_timeout);
			}

			/* Check if it's time to blink */
			if (tdma_handler->check_blink(tdma_handler) == TRUE)
			{
				inst->testAppState = TA_TX_SELECT;
				done = INST_NOT_DONE_YET;
			}

			if (need_blink(tdma_handler))
			{
				uint32 blink_timeout = next_blink_time(tdma_handler);
				UPDATE_TIMEOUT(blink_timeout);
			}

			// sys_printf("IDLE TIMEOUT %u\r\n", GET_TIMEOUT());

			break;
		}
		} // end of switch on message

		break;
	} // end case TA_RX_WAIT_DATA
	default:
	{
		break;
	}
	} // end switch on testAppState

	return done;
} // end testapprun()


/********************************************************************************
Function:
	app_process_rx_msg()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Processes the received message.
Notes:
	---
Author, Date:
	Toan Huynh, 07/20/2022
*********************************************************************************/
static uwb_msg_info_t app_process_rx_msg(event_data_t *dw_event)
{
	uwb_msg_info_t msg_info = {.dw_event = dw_event};

	if (!dw_event)
	{
		return msg_info;
	}

	// 16 or 64 bit addresses
	switch (dw_event->msgu.frame[1])
	{
	case 0xCC:
		memcpy(&msg_info.srcAddr[0], &(dw_event->msgu.rxmsg_ll.sourceAddr[0]), ADDR_BYTE_SIZE_L);
		msg_info.fn_code = dw_event->msgu.rxmsg_ll.messageData[FCODE];
		msg_info.p_data = &dw_event->msgu.rxmsg_ll.messageData[0];
		break;
	case 0xC8:
		memcpy(&msg_info.srcAddr[0], &(dw_event->msgu.rxmsg_sl.sourceAddr[0]), ADDR_BYTE_SIZE_L);
		msg_info.fn_code = dw_event->msgu.rxmsg_sl.messageData[FCODE];
		msg_info.p_data = &dw_event->msgu.rxmsg_sl.messageData[0];
		break;
	case 0x8C:
		memcpy(&msg_info.srcAddr[0], &(dw_event->msgu.rxmsg_ls.sourceAddr[0]), ADDR_BYTE_SIZE_S);
		msg_info.fn_code = dw_event->msgu.rxmsg_ls.messageData[FCODE];
		msg_info.p_data = &dw_event->msgu.rxmsg_ls.messageData[0];
		break;
	case 0x88:
		memcpy(&msg_info.srcAddr[0], &(dw_event->msgu.rxmsg_ss.sourceAddr[0]), ADDR_BYTE_SIZE_S);
		msg_info.fn_code = dw_event->msgu.rxmsg_ss.messageData[FCODE];
		msg_info.p_data = &dw_event->msgu.rxmsg_ss.messageData[0];
		break;
	}

	return msg_info;
}

/********************************************************************************
Function:
	app_handler_rx_packet()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Handles the received packet.
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
static bool app_handler_rx_packet(instance_data_t *inst, struct TDMAHandler *tdma_handler, event_data_t *dw_event)
{
	/*
	 * If we have received a DWT_SIG_RX_OKAY event
	 * this means that the message is IEEE data type
	 * need to check frame control to know which addressing mode is used
	 */
	bool ret = TRUE;
	uwb_msg_info_t msg = app_process_rx_msg(dw_event);

	// sys_printf("[%u][RX][0x%X] %s\r\n", timestamp_get_ms(), *(uint16_t *)&srcAddr, get_msg_fcode_string(fn_code));

	switch (msg.fn_code)
	{
	case RTLS_DEMO_MSG_RNG_INIT:
	{
		/* Create the network */
		network_create(tdma_handler, inst);
	
		app_join_network_callback();
	
		inst->testAppState = TA_TX_SELECT;
		break;
	}
	case RTLS_DEMO_MSG_SYNC:
	{
		net_node_frame_sync(inst, tdma_handler, &msg);
		break;
	}
	case RTLS_DEMO_MSG_INF_UPDATE: // fall through
	case RTLS_DEMO_MSG_INF_SUG:		 // fall through
	case RTLS_DEMO_MSG_INF_REG:
	{
		net_packet_process_inf(inst, tdma_handler, &msg);
		// INF is last message in slot, we can print after processing
		inst->canPrintLCD = TRUE;
		// wait for next RX
		inst->testAppState = TA_RXE_WAIT;
		break;
	}
	case RTLS_DEMO_MSG_INF_INIT:
	{
		net_node_joined_network(inst, tdma_handler, &msg);

		app_join_network_callback();
		
		// stay in RX wait for next frame...
		inst->testAppState = TA_RXE_WAIT;
		break;
	}
	case RTLS_DEMO_MSG_TAG_POLL:
	{
		if (dw_event->typePend == DWT_SIG_TX_PENDING)
		{
			inst->testAppState = TA_TX_WAIT_CONF; // wait confirmation
			inst->previousState = TA_TXRESPONSE_WAIT_SEND;
		}
		else
		{
			// stay in RX wait for next frame...
			inst->testAppState = TA_RXE_WAIT;
		}
		break;
	}
	case RTLS_DEMO_MSG_ANCH_RESP:
	{
		/* Store the anchor position */
		instance_store_neigbor_position(inst, &msg);

		if (dw_event->typePend == DWT_SIG_TX_PENDING)
		{
			inst->testAppState = TA_TX_WAIT_CONF; // wait confirmation
			inst->previousState = TA_TXFINAL_WAIT_SEND;
		}
		else
		{
			// Stay in RX wait for next frame...
			inst->testAppState = TA_RXE_WAIT;
		}

		break;
	}
	case RTLS_DEMO_MSG_TAG_FINAL:
	{
		twr_report_distance(inst, tdma_handler, &msg);
		inst->testAppState = TA_TXREPORT_WAIT_SEND;
		break;
	}
	case RTLS_DEMO_MSG_RNG_REPORT:
	{
		bool my_report = FALSE;

		twr_process_report_msg(inst, tdma_handler, &msg, &my_report);

		if (my_report)
		{
			/* Update error code */
			app_update_error(inst, APP_UWB_SUCCESS);
	
			if (is_last_node_to_range(tdma_handler)) // Last node to range
			{
				sys_printf("[%u] RUN LOC ENGINE: ", timestamp_get_ms());
				for (uint8 i = 0; i < NUM_NODE_TO_RANGE; i++)
				{
					sys_printf("%u ", tdma_handler->lastNodeRanged[i]);
				}
				sys_printf("\r\n");

#if CONFIG_ENABLE_LOCATION_ENGINE
				/* Perform location engine */
				loc_engine_run();
#endif
				/* Clear the ranging info */
				instance_clear_ranged_node_info();
			}

			tdma_handler->nthOldest = 1;

			if (inst->uwbToRangeWith < inst->uwbListLen)
			{
				tdma_handler->uwbListTDMAInfo[inst->uwbToRangeWith].lastRange = timestamp_get_ms();
			}

			tdma_handler->firstPollComplete = TRUE;
			inst->uwbToRangeWith = 255;
			app_state_change(inst, TA_TX_SELECT);
		}
		else
		{
			inst->uwbToRangeWith = 255;
			app_state_change(inst, TA_RXE_WAIT);
		}

		inst->canPrintLCD = FALSE;

		break;
	} // RTLS_DEMO_MSG_RNG_REPORT
	case RTLS_HOST_MSG:
	{
		// Process host message
		app_process_host_msg(inst, &msg);
		// wait for next RX
		inst->testAppState = TA_RXE_WAIT;
		break;
	}
	default:
	{
		/* Wait for next frame */
		inst->testAppState = TA_RXE_WAIT;
		ret = FALSE;
		break;
	}
	} // end switch (fcode)

	return ret;
}

/********************************************************************************
Function:
	app_handle_blink_response()
Input Parameters:
	---
Output Parameters:
	---
Description:
	If node is in blink mode, this function is called to handle the response from the node blinked
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
static bool app_handle_blink_response(instance_data_t *inst, event_data_t *dw_event)
{
	bool ret = FALSE;

	if (inst->mode == DISCOVERY)
	{
		/* Remember who to send the reply to, set destination address */
		memcpy(&inst->rng_initmsg.destAddr[0], &(dw_event->msgu.rxblinkmsg.tagID[0]), BLINK_FRAME_SOURCE_ADDRESS);

		inst->testAppState = TA_TXRANGINGINIT_WAIT_SEND;
		ret = TRUE;
	}

	return ret;
}

/********************************************************************************
Function:
	app_process_rx_timeout()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Handle the RX timeout event
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
static bool app_process_rx_timeout(instance_data_t *inst, struct TDMAHandler *tdma_handler)
{
	bool ret = TRUE;

	if (tdma_handler->discovery_mode == WAIT_RNG_INIT || tdma_handler->discovery_mode == WAIT_INF_INIT)
	{
		uint32 time_now = timestamp_get_ms();
		tdma_handler->set_discovery_mode(tdma_handler, WAIT_INF_REG, time_now);
	}

	app_update_error(inst, APP_UWB_RX_TIMEOUT);

	/* Get and clear this event */
	instance_getevent(17);
	inst_processtxrxtimeout(inst);

	sys_printf("[%u] RX TIMEOUT ERROR\r\n", timestamp_get_ms());

	return ret;
}

/********************************************************************************
Input:
	---
Output:
	---
Description:
	Process host message
Author, Date:
	Toan Huynh, 11/16/2023
*********************************************************************************/
static bool app_process_host_msg(instance_data_t *inst, uwb_msg_info_t *p_msg)
{
	uint32_t src_addr = *(uint32_t *)p_msg->srcAddr;
	uint8_t *p_data = &p_msg->p_data[HOST_MSG_PAYLOAD_IDEX];
	uint32_t data_len = p_msg->dw_event->rxLength;

	/* Process the message */
	/* Forward broadcast message to host if need */
	host_msg_parse_msgs(inst, src_addr, p_data, data_len);

	if (host_msg_find_msg(src_addr))
	{
		// Node waked up in it's alloced slot, send messages to it if having
		host_msg_forward_msg_to_node(src_addr);
	}

	return true;
}

/********************************************************************************
Function:
	app_check_event_timeout()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Process the event timeout event if happened
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
static bool app_check_event_timeout(instance_data_t *inst, struct TDMAHandler *tdma_handler, uint32 *p_timeout)
{
	if (!p_timeout)
	{
		return FALSE;
	}

	bool ret = FALSE;

	if (inst->mode == TAG)
	{
		// get the message FCODE
		uint8 fcode;
		memcpy(&fcode, &inst->msg.messageData[FCODE], sizeof(uint8));

		if (fcode == RTLS_DEMO_MSG_TAG_POLL)
		{
			*p_timeout = app_state_wait_timeout(inst, inst->timeofTxPoll, inst->durationPollTimeout_ms);

			if (*p_timeout == 0)
			{
				/* Received poll timeout */
				sys_printf("[ERR] POLL TIMEOUT 0x%x\r\n", (uint16_t)instance_get_uwbaddr(inst->uwbToRangeWith));

				app_update_error(inst, APP_UWB_POLL_TIMEOUT);

				inst_processtxrxtimeout(inst);
				ret = TRUE;
			}
		}
		else if (fcode == RTLS_DEMO_MSG_TAG_FINAL && inst->previousState == TA_TXFINAL_WAIT_SEND)
		{
			*p_timeout = app_state_wait_timeout(inst, inst->timeofTxFinal, inst->durationFinalTimeout_ms);

			if (*p_timeout == 0)
			{
				/* Received final sending timeout */
				sys_printf("[ERR] FINAL TIMEOUT 0x%x\r\n", (uint16_t)instance_get_uwbaddr(inst->uwbToRangeWith));
				
				app_update_error(inst, APP_UWB_FINAL_TIMEOUT);

				inst_processtxrxtimeout(inst);
				ret = TRUE;
			}
		}
	}

	return ret;
}

/********************************************************************************
Function:
	app_maintain_receiver()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Maintain the receiver
Notes:
	---
Author, Date:
	Toan Huynh, 07/21/2022
*********************************************************************************/
static bool app_maintain_receiver(instance_data_t *inst, uint32 *p_timeout)
{
	bool ret = TRUE;

	uint32 remain_time = app_state_wait_timeout(inst, inst->rxCheckOnTime, RX_CHECK_ON_PERIOD_MS);

	if (remain_time == 0)
	{
		inst->rxCheckOnTime = timestamp_get_ms();
		// read SYS_STATE, getting second byte
		uint8 regval = dwt_read8bitoffsetreg(SYS_STATE_ID, 1);
		// get the first 5 bytes
		regval &= 0x1F;
		if (regval == 0)
		{ // RX IDLE
			uwb_dev_off_transceiver();
			instance_rxon(inst, 0, 0);
		}
	}
	else
	{
		if (p_timeout)
		{
			*p_timeout = remain_time;
		}
	}

	return ret;
}

/********************************************************************************
Function:
	app_join_network_callback()
Input Parameters:
	---
Output Parameters:
	---
Description:
	Run the callback function when join the network
Notes:
	---
Author, Date:
	Toan Huynh, 08/01/2022
*********************************************************************************/
static void app_join_network_callback(void)
{
	if(joined_cb_func)
	{
		joined_cb_func();
	}
}

/********************************************************************************
Input:
	---
Output:
	---
Description:
	---
Author, Date:
	Toan Huynh, 08/24/2023
*********************************************************************************/
static void app_update_error(instance_data_t *inst, app_uwb_err err)
{
	if(inst->uwbToRangeWith < ARRAY_SIZE(inst->error))
	{
		inst->error[inst->uwbToRangeWith] = err;
	}
}

//###########################################################################################################
//      INTERRUPTS
//###########################################################################################################



//###########################################################################################################
//      TEST HARNESSES
//###########################################################################################################



//###########################################################################################################
//      END OF app_main.c
//###########################################################################################################
