/*
 *  HID driver for acer devices
 *
 *  Copyright (c) 2015 Simon Wörner
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"

/* Acer keyboards e.g. in Acer SW5-012 use synaptics touchpad USB ID
 * (06cb:2968 and 06CB:2991) and have the following issue:
 * - The report descriptor specifies an excessively large number of usages
 *   and logical max (2^16), which is more than HID_MAX_USAGES. This prevents
 *   proper parsing of the report descriptor.
 *
 * The byte replace in the descriptor below fixes the size.
 */

#define ACER_KBD_RDESC_ORIG_SIZE	188
#define ACER_KBD_RDESC_CHECK_POS	(150 * sizeof(__u8))
#define ACER_KBD_RDESC_CHECK_DATA	0x2AFFFF150026FFFF
#define ACER_KBD_RDESC_FIX_POS1		152
#define ACER_KBD_RDESC_FIX_POS2		157

static __u8 *acer_kbd_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	/* check for invalid descriptor */
	if (*rsize == ACER_KBD_RDESC_ORIG_SIZE) {
		__u64 check = be64_to_cpu(*(__be64 *)(rdesc + ACER_KBD_RDESC_CHECK_POS));

		/* check for invalid max usages and logical 0xFFFF (2^16) */
		if (check == ACER_KBD_RDESC_CHECK_DATA) {
			hid_info(hdev, "fixing up acer keyboard report descriptor\n");

			/* fix max values with 0xFF00 (2^8) */
			rdesc[ACER_KBD_RDESC_FIX_POS1] = 0x00;
			rdesc[ACER_KBD_RDESC_FIX_POS2] = 0x00;
		}
	}

	return rdesc;
}

static const struct hid_device_id acer_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_ACER_SYNAPTICS,
		USB_VENDOR_ID_ACER_SYNAPTICS_TP_2968) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_ACER_SYNAPTICS,
		USB_VENDOR_ID_ACER_SYNAPTICS_TP_2991) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_ACER_SYNAPTICS,
		USB_VENDOR_ID_ACER_SYNAPTICS_TP_74D9) },
	{ }
};
MODULE_DEVICE_TABLE(hid, acer_devices);

static struct hid_driver acer_driver = {
	.name = "acer",
	.id_table = acer_devices,
	.report_fixup = acer_kbd_report_fixup,
};
module_hid_driver(acer_driver);

MODULE_AUTHOR("Simon Wörner");
MODULE_LICENSE("GPL");
