/************************************************************************************************************
Module:       dev_storage

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
03/25/2024    TH       Began Coding    (TH = Toan Huynh)

COPYRIGHT © 2024 TOAN HUYNH. ALL RIGHTS RESERVED.

THIS SOURCE IS TOAN HUYNH PROPRIETARY AND CONFIDENTIAL! NO PART OF THIS
SOURCE MAY BE DISCLOSED IN ANY MANNER TO A THIRD PARTY WITHOUT PRIOR WRITTEN
CONSENT OF TOAN HUYNH.
************************************************************************************************************/

//###########################################################################################################
//      #INCLUDES
//###########################################################################################################
#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include <dev_cfg.h>



//###########################################################################################################
//      TESTING #DEFINES
//###########################################################################################################



//###########################################################################################################
//      CONSTANT #DEFINES
//###########################################################################################################
#define NVS_PARTITION		storage_partition
#define NVS_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(NVS_PARTITION)

/* 1000 msec = 1 sec */
#define SLEEP_TIME      100
/* maximum reboot counts, make high enough to trigger sector change (buffer */
/* rotation). */
#define MAX_REBOOT 400

#define ADDRESS_ID 1
#define KEY_ID 2
#define RBT_CNT_ID 3
#define STRING_ID 4
#define LONG_ID 5



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

static struct nvs_fs fs;


//###########################################################################################################
//      PRIVATE FUNCTION PROTOTYPES
//###########################################################################################################



//###########################################################################################################
//      PUBLIC FUNCTIONS
//###########################################################################################################


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
//      END OF dev_storage.c
//###########################################################################################################
bool cfgReadU8list(ConfigField field, uint8_t *list, uint8_t lenth)
{
	int rc = nvs_read(&fs, field, list, lenth);
	
	if (rc) {
		printk("No found id %d\n", field);
		return false;
	}

	return true;
}

bool cfgSet(ConfigField field, uint8_t *p_data, uint8_t size)
{
	int rc = nvs_write(&fs, field, p_data, size);
	if (rc) {
		printk("Write failed\n");
		return false;
	}
	return true;

}

bool cfgStore(bool need_clear)
{
	int rc = 0;

	if (need_clear) {
		rc = nvs_clear(&fs);
		if (rc) {
			printk("Clear failed\n");
			return false;
		}
	}
	return true;
}

bool cfgReset(void)
{
	int rc = nvs_clear(&fs);
	if (rc) {
		printk("Clear failed\n");
		return false;
	}
	return true;
}


void cfgInit(void)
{
	int rc;
	struct flash_pages_info info;

	/* define the nvs file system by settings with:
	 *	sector_size equal to the pagesize,
	 *	3 sectors
	 *	starting at NVS_PARTITION_OFFSET
	 */

	fs.flash_device = NVS_PARTITION_DEVICE;
	if (!device_is_ready(fs.flash_device)) {
		__ASSERT(true, "Flash device %s is not ready\n", fs.flash_device->name);
		return;
	}

	fs.offset = NVS_PARTITION_OFFSET;
	rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);
	if (rc) {
		__ASSERT(true, "Unable to get page info\n");
		return;
	}

	fs.sector_size = info.size;
	fs.sector_count = 3U;

	rc = nvs_mount(&fs);
	if (rc) {
		__ASSERT(true, "Flash Init failed");
		return;
	}

}