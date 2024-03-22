/************************************************************************************************************
Module:       uwb_event

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
#include "instance.h"
#include "system_dbg.h"



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
extern instance_data_t instance_data[NUM_INST];
static event_data_t dw_event_g;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################
/********************************************************************************
Function:
  instance_peekevent()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Peek the event from the event queue.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
int instance_peekevent(void)
{
	int instance = 0;
	return instance_data[instance].dwevent[instance_data[instance].dweventPeek].type; // return the type of event that is in front of the queue
}

/********************************************************************************
Function:
  instance_putevent()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Put the event into the event queue.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_putevent(event_data_t newevent)
{
	int instance = 0;
	uint8 etype = newevent.type;

	newevent.type = 0;

	// copy event
	instance_data[instance].dwevent[instance_data[instance].dweventIdxIn] = newevent;

	// set type - this makes it a new event (making sure the event data is copied before event is set as new)
	// to make sure that the get event function does not get an incomplete event
	instance_data[instance].dwevent[instance_data[instance].dweventIdxIn].type = etype;

	updateTestWithEvent(&instance_data[instance].dwevent[instance_data[instance].dweventIdxIn]);

	instance_data[instance].dweventIdxIn++;

	if (MAX_EVENT_NUMBER == instance_data[instance].dweventIdxIn)
	{
		instance_data[instance].dweventIdxIn = 0;
	}


	/* Notify app state machine */
	dw_app_signal();
}

/********************************************************************************
Function:
  instance_getevent()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Get the event from the event queue.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
event_data_t *instance_getevent(int x)
{
	int instance = 0;
	int indexOut = instance_data[instance].dweventIdxOut;

	if (instance_data[instance].dwevent[indexOut].type == 0) // exit with "no event"
	{
		dw_event_g.type = 0;
		dw_event_g.typeSave = 0;
		return &dw_event_g;
	}

	// copy the event
	dw_event_g.typeSave = instance_data[instance].dwevent[indexOut].typeSave;
	dw_event_g.typePend = instance_data[instance].dwevent[indexOut].typePend;
	dw_event_g.rxLength = instance_data[instance].dwevent[indexOut].rxLength;
	dw_event_g.timeStamp = instance_data[instance].dwevent[indexOut].timeStamp;
	dw_event_g.timeStamp32l = instance_data[instance].dwevent[indexOut].timeStamp32l;
	dw_event_g.timeStamp32h = instance_data[instance].dwevent[indexOut].timeStamp32h;

	memcpy(&dw_event_g.msgu, &instance_data[instance].dwevent[indexOut].msgu, sizeof(instance_data[instance].dwevent[indexOut].msgu));

	dw_event_g.type = instance_data[instance].dwevent[indexOut].type;

	instance_data[instance].dwevent[indexOut].type = 0; // clear the event

	instance_data[instance].dweventIdxOut++;
	if (MAX_EVENT_NUMBER == instance_data[instance].dweventIdxOut) // wrap the counter
	{
		instance_data[instance].dweventIdxOut = 0;
	}

	instance_data[instance].dweventPeek = instance_data[instance].dweventIdxOut; // set the new peek value

	return &dw_event_g;
}

/********************************************************************************
Function:
  instance_clearevents()
Input Parameters:
  ---
Output Parameters:
  ---
Description:
  Clear the event queue.
Notes:
  ---
Author, Date:
  Toan Huynh, 07/15/2022
*********************************************************************************/
void instance_clearevents(void)
{
	int i = 0;
	int instance = 0;

	for (i = 0; i < MAX_EVENT_NUMBER; i++)
	{
		memset(&instance_data[instance].dwevent[i], 0, sizeof(event_data_t));
	}

	instance_data[instance].dweventIdxIn = 0;
	instance_data[instance].dweventIdxOut = 0;
	instance_data[instance].dweventPeek = 0;
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
//      END OF uwb_event.c
//###########################################################################################################
