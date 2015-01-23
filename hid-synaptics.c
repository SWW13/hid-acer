/*
 *  HID driver for synaptics devices
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
#include <linux/usb.h>

/* Synaptics keyboards (USB ID 06cb:2968) e.g. in Acer SW5-012
 * have the following issue:
 * - The report descriptor specifies an excessively large number of consumer
 *   usages (2^15), which is more than HID_MAX_USAGES. This prevents proper
 *   parsing of the report descriptor.
 *
 * The replacement descriptor below fixes the number of consumer usages.
 */

static __u8 synaptics_kbd_rdesc_fixed[] = {
	/* Application Collection */
	0x06, 0x85, 0xFF,	/* (GLOBAL) USAGE_PAGE (Vendor-defined)       */
	0x09, 0x95,		/* (LOCAL)  USAGE (0xFF850095)                */
	0xA1, 0x01,		/* (MAIN)   COLLECTION (Application)          */
	0x85, 0x5A,		/*   (GLOBAL) REPORT_ID (0x5A (90) 'Z')       */
	0x09, 0x01,		/*   (LOCAL)  USAGE (0xFF850001)              */
	0x26, 0xFF, 0x00,	/*   (GLOBAL) LOGICAL_MAXIMUM 0x00FF (255)    */
	0x75, 0x08,		/*   (GLOBAL) REPORT_SIZE 0x08 (8)            */
	0x95, 0x10,		/*   (GLOBAL) REPORT_COUNT 0x10 (16)          */
	0xB1, 0x00,		/*   (MAIN)   FEATURE 0x00                    */
	0xC0,			/* (MAIN)   END_COLLECTION (Application)      */

	/* Application Collection */
	0x05, 0x01,		/* (GLOBAL) USAGE_PAGE (Desktop)              */
	0x09, 0x06,		/* (LOCAL)  USAGE 0x06 (Keyboard)             */
	0xA1, 0x01,		/* (MAIN)   COLLECTION (Application)          */
	0x85, 0x01,		/*   (GLOBAL) REPORT_ID 0x01 (1)              */
	0x75, 0x01,		/*   (GLOBAL) REPORT_SIZE 0x01 (1)            */
	0x95, 0x08,		/*   (GLOBAL) REPORT_COUNT 0x08 (8)           */
	0x05, 0x07,		/*   (GLOBAL) USAGE_PAGE 0x07 (Keyboard)      */
	0x19, 0xE0,		/*   (LOCAL)  USAGE_MINIMUM 0xE0              */
	0x29, 0xE7,		/*   (LOCAL)  USAGE_MAXIMUM 0xE7              */
	0x25, 0x01,		/*   (GLOBAL) LOGICAL_MAXIMUM 0x01 (1)        */
	0x81, 0x02,		/*   (MAIN)   INPUT 0x02                      */
	0x95, 0x01,		/*   (GLOBAL) REPORT_COUNT 0x01 (1)           */
	0x75, 0x08,		/*   (GLOBAL) REPORT_SIZE 0x08 (8)            */
	0x81, 0x03,		/*   (MAIN)   INPUT 0x03                      */
	0x95, 0x05,		/*   (GLOBAL) REPORT_COUNT 0x05 (5)           */
	0x75, 0x01,		/*   (GLOBAL) REPORT_SIZE 0x01 (1)            */
	0x05, 0x08,		/*   (GLOBAL) USAGE_PAGE 0x08 (LED Indicator) */
	0x19, 0x01,		/*   (LOCAL)  USAGE_MINIMUM 0x01              */
	0x29, 0x05,		/*   (LOCAL)  USAGE_MAXIMUM 0x05              */
	0x91, 0x02,		/*   (MAIN)   OUTPUT 0x02                     */
	0x95, 0x01,		/*   (GLOBAL) REPORT_COUNT 0x01 (1)           */
	0x75, 0x03,		/*   (GLOBAL) REPORT_SIZE 0x03 (3)            */
	0x91, 0x03,		/*   (MAIN)   OUTPUT 0x03                     */
	0x95, 0x06,		/*   (GLOBAL) REPORT_COUNT 0x06 (6)           */
	0x75, 0x08,		/*   (GLOBAL) REPORT_SIZE 0x08 (8)            */
	0x26, 0xFF, 0x00,	/*   (GLOBAL) LOGICAL_MAXIMUM 0x00FF (255)    */
	0x05, 0x07,		/*   (GLOBAL) USAGE_PAGE 0x0007 (Keyboard)    */
	0x19, 0x00,		/*   (LOCAL)  USAGE_MINIMUM 0x00              */
	0x2A, 0xFF, 0x00,	/*   (LOCAL)  USAGE_MAXIMUM 0xFF              */
	0x81, 0x00,		/*   (MAIN)   INPUT 0x00                      */
	0xC0,			/* (MAIN)   END_COLLECTION (Application)      */

	/* Application Collection */
	0x05, 0x0C,		/* (GLOBAL) USAGE_PAGE (Consumer)             */
	0x09, 0x01,		/* (LOCAL)  USAGE 0x01 (Consumer Control)     */
	0xA1, 0x01,		/* (MAIN)   COLLECTION (Application)          */
	0x85, 0x02,		/*   (GLOBAL) REPORT_ID 0x02 (2)              */
	0x19, 0x00,		/*   (LOCAL)  USAGE_MINIMUM 0x00              */
	0x2A, 0x3C, 0x02,	/*   (LOCAL)  USAGE_MAXIMUM 0x023C            */
	0x26, 0x3C, 0x02,	/*   (GLOBAL) LOGICAL_MAXIMUM 0x023C (572)    */
	0x75, 0x10,		/*   (GLOBAL) REPORT_SIZE 0x10 (16)           */
	0x95, 0x01,		/*   (GLOBAL) REPORT_COUNT 0x01 (1)           */
	0x81, 0x00,		/*   (MAIN)   INPUT 0x00                      */
	0xC0,			/* (MAIN)   END_COLLECTION (Application)      */

	/* Application Collection */
	0x05, 0x01,		/* (GLOBAL) USAGE_PAGE (Desktop)              */
	0x09, 0x0C,		/* (LOCAL)  USAGE (Wireless Radio Controls)   */
	0xA1, 0x01,		/* (MAIN)   COLLECTION (Application)          */
	0x85, 0x03,		/*   (GLOBAL) REPORT_ID 0x03 (3)              */
	0x25, 0x01,		/*   (GLOBAL) LOGICAL_MAXIMUM 0x01 (1)        */
	0x09, 0xC6,		/*   (LOCAL)  USAGE 0xC6                      */
	0x75, 0x01,		/*   (GLOBAL) REPORT_SIZE 0x01 (1)            */
	0x81, 0x06,		/*   (MAIN)   INPUT 0x06                      */
	0x75, 0x07,		/*   (GLOBAL) REPORT_SIZE 0x07 (7)            */
	0x81, 0x03,		/*   (MAIN)   INPUT 0x03                      */
	0xC0,			/* (MAIN)   END_COLLECTION (Application)      */

	/* Application Collection */
	0x05, 0x88,		/* (GLOBAL) USAGE_PAGE (0x88)                 */
	0x09, 0x01,		/* (LOCAL)  USAGE (0x01)                      */
	0xA1, 0x01,		/* (MAIN)   COLLECTION (Application)          */
	0x85, 0x04,		/*   (GLOBAL) REPORT_ID 0x04 (4)              */
	0x19, 0x00,		/*   (LOCAL)  USAGE_MINIMUM 0x00              */
	0x2A, 0xFF, 0x00,	/*   (LOCAL)  USAGE_MAXIMUM 0x00FF            */
	0x26, 0xFF, 0x00,	/*   (GLOBAL) LOGICAL_MAXIMUM 0x00FF          */
	0x75, 0x08,		/*   (GLOBAL) REPORT_SIZE 0x08 (8)            */
	0x95, 0x02,		/*   (GLOBAL) REPORT_COUNT 0x02 (2)           */
	0x81, 0x02,		/*   (MAIN)   INPUT 0x02                      */
	0xC0,			/* (MAIN)   END_COLLECTION (Application)      */

	/* Application Collection */
	0x05, 0x01,		/* (GLOBAL) USAGE_PAGE (Desktop)              */
	0x09, 0x80,		/* (LOCAL)  USAGE (System Control)            */
	0xA1, 0x01,		/* (MAIN)   COLLECTION (Application)          */
	0x85, 0x05,		/*   (GLOBAL) REPORT_ID 0x05 (5)              */
	0x19, 0x81,		/*   (LOCAL)  USAGE_MINIMUM 0x81              */
	0x29, 0x83,		/*   (LOCAL)  USAGE_MAXIMUM 0x83              */
	0x25, 0x01,		/*   (GLOBAL) LOGICAL_MAXIMUM 0x01 (1)        */
	0x95, 0x08,		/*   (GLOBAL) REPORT_COUNT 0x08 (8)           */
	0x75, 0x01,		/*   (GLOBAL) REPORT_SIZE 0x01 (1)            */
	0x81, 0x02,		/*   (MAIN)   INPUT 0x02                      */
	0xC0,			/* (MAIN)   END_COLLECTION (Application)      */
};

static __u8 *synaptics_kbd_report_fixup(struct hid_device *hdev, __u8 *rdesc,
unsigned int *rsize)
{
	struct usb_interface *intf = to_usb_interface(hdev->dev.parent);

	if (intf->cur_altsetting->desc.bInterfaceNumber == 0) {
		rdesc = synaptics_kbd_rdesc_fixed;
		*rsize = sizeof(synaptics_kbd_rdesc_fixed);
	}
	return rdesc;
}

static int synaptics_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		goto err_free;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		goto err_free;
	}

	if (ret < 0)
		goto err_stop;

	return 0;
err_stop:
	hid_hw_stop(hdev);
err_free:
	return ret;
}

static void synaptics_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
	kfree(hid_get_drvdata(hdev));
}

#define USB_VENDOR_ID_SYNAPTICS         		0x06cb
#define USB_VENDOR_ID_SYNAPTICS_KEYBOARD		0x2968

static const struct hid_device_id synaptics_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_SYNAPTICS,
		USB_VENDOR_ID_SYNAPTICS_KEYBOARD) },
	{ }
};
MODULE_DEVICE_TABLE(hid, synaptics_devices);

static struct hid_driver synaptics_driver = {
	.name = "synaptics",
	.id_table = synaptics_devices,
	.report_fixup = synaptics_kbd_report_fixup,
	.probe = synaptics_probe,
	.remove = synaptics_remove,
};
module_hid_driver(synaptics_driver);

MODULE_LICENSE("GPL");
