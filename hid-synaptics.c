/*
 *  HID driver for synaptics devices
 *
 *  Copyright (c) 2014 Simon Wörner
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

#define DEBUG_SYNAPTICS

static __u8 synaptics_kbd_rdesc_fixed[] = {
	//--------------------------------------------------------------------------------
	// Decoded Application Collection
	//--------------------------------------------------------------------------------
	0x06, 0x85,0xFF,	// (GLOBAL) USAGE_PAGE         0xFF85 Vendor-defined
	0x09, 0x95,			// (LOCAL)  USAGE              0xFF850095
	0xA1, 0x01,			// (MAIN)   COLLECTION         0x01 Application (Usage=0xFF850095: Page=Vendor-defined, Usage=, Type=)
	0x85, 0x5A,			//   (GLOBAL) REPORT_ID          0x5A (90) 'Z'
	0x09, 0x01,			//   (LOCAL)  USAGE              0xFF850001
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x26, 0xFF,0x00,	//   (GLOBAL) LOGICAL_MAXIMUM    0x00FF (255)
	0x75, 0x08,			//   (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field
	0x95, 0x10,			//   (GLOBAL) REPORT_COUNT       0x10 (16) Number of fields
	0xB1, 0x00,			//   (MAIN)   FEATURE            0x00000000 (16 fields x 8 bits) 0=Data 0=Array 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0xC0,				// (MAIN)   END_COLLECTION     Application

	//--------------------------------------------------------------------------------
	// Decoded Application Collection
	//--------------------------------------------------------------------------------
	0x05, 0x01,			// (GLOBAL) USAGE_PAGE         0x0001 Generic Desktop Page
	0x09, 0x06,			// (LOCAL)  USAGE              0x00010006 Keyboard (CA=Application Collection)
	0xA1, 0x01,			// (MAIN)   COLLECTION         0x01 Application (Usage=0x00010006: Page=Generic Desktop Page, Usage=Keyboard, Type=CA)
	0x85, 0x01,			//   (GLOBAL) REPORT_ID          0x01 (1)
	0x75, 0x01,			//   (GLOBAL) REPORT_SIZE        0x01 (1) Number of bits per field
	0x95, 0x08,			//   (GLOBAL) REPORT_COUNT       0x08 (8) Number of fields
	0x05, 0x07,			//   (GLOBAL) USAGE_PAGE         0x0007 Keyboard/Keypad Page
	0x19, 0xE0,			//   (LOCAL)  USAGE_MINIMUM      0x000700E0 Keyboard Left Control (DV=Dynamic Value)
	0x29, 0xE7,			//   (LOCAL)  USAGE_MAXIMUM      0x000700E7 Keyboard Right GUI (DV=Dynamic Value)
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x25, 0x01,			//   (GLOBAL) LOGICAL_MAXIMUM    0x01 (1)
	0x81, 0x02,			//   (MAIN)   INPUT              0x00000002 (8 fields x 1 bit) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0x95, 0x01,			//   (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields
	0x75, 0x08,			//   (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field
	0x81, 0x03,			//   (MAIN)   INPUT              0x00000003 (1 field x 8 bits) 1=Constant 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0x95, 0x05,			//   (GLOBAL) REPORT_COUNT       0x05 (5) Number of fields
	0x75, 0x01,			//   (GLOBAL) REPORT_SIZE        0x01 (1) Number of bits per field
	0x05, 0x08,			//   (GLOBAL) USAGE_PAGE         0x0008 LED Indicator Page
	0x19, 0x01,			//   (LOCAL)  USAGE_MINIMUM      0x00080001 Num Lock (OOC=On/Off Control)
	0x29, 0x05,			//   (LOCAL)  USAGE_MAXIMUM      0x00080005 Kana (OOC=On/Off Control)
	0x91, 0x02,			//   (MAIN)   OUTPUT             0x00000002 (5 fields x 1 bit) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0x95, 0x01,			//   (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields
	0x75, 0x03,			//   (GLOBAL) REPORT_SIZE        0x03 (3) Number of bits per field
	0x91, 0x03,			//   (MAIN)   OUTPUT             0x00000003 (1 field x 3 bits) 1=Constant 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0x95, 0x06,			//   (GLOBAL) REPORT_COUNT       0x06 (6) Number of fields
	0x75, 0x08,			//   (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x26, 0xFF,0x00,	//   (GLOBAL) LOGICAL_MAXIMUM    0x00FF (255)
	0x05, 0x07,			//   (GLOBAL) USAGE_PAGE         0x0007 Keyboard/Keypad Page
	0x19, 0x00,			//   (LOCAL)  USAGE_MINIMUM      0x00070000 Keyboard No event indicated (Sel=Selector)
	0x2A, 0xFF,0x00,	//   (LOCAL)  USAGE_MAXIMUM      0x000700FF
	0x81, 0x00,			//   (MAIN)   INPUT              0x00000000 (6 fields x 8 bits) 0=Data 0=Array 0=Absolute 0=Ignored 0=Ignored 0=PrefState 0=NoNull
	0xC0,				// (MAIN)   END_COLLECTION     Application

	//--------------------------------------------------------------------------------
	// Decoded Application Collection
	//--------------------------------------------------------------------------------
	0x05, 0x0C,			// (GLOBAL) USAGE_PAGE         0x000C Consumer Device Page
	0x09, 0x01,			// (LOCAL)  USAGE              0x000C0001 Consumer Control (CA=Application Collection)
	0xA1, 0x01,			// (MAIN)   COLLECTION         0x01 Application (Usage=0x000C0001: Page=Consumer Device Page, Usage=Consumer Control, Type=CA)
	0x85, 0x02,			//   (GLOBAL) REPORT_ID          0x02 (2)
	0x19, 0x00,			//   (LOCAL)  USAGE_MINIMUM      0x000C0000 Unassigned
	0x2A, 0x3C,0x02,	//   (LOCAL)  USAGE_MAXIMUM      0x000C023C AC Format (Sel=Selector)
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x26, 0x3C,0x02,	//   (GLOBAL) LOGICAL_MAXIMUM    0x023C (572)
	0x75, 0x10,			//   (GLOBAL) REPORT_SIZE        0x10 (16) Number of bits per field
	0x95, 0x01,			//   (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields
	0x81, 0x00,			//   (MAIN)   INPUT              0x00000000 (1 field x 16 bits) 0=Data 0=Array 0=Absolute 0=Ignored 0=Ignored 0=PrefState 0=NoNull
	0xC0,				// (MAIN)   END_COLLECTION     Application

	//--------------------------------------------------------------------------------
	// Decoded Application Collection
	//--------------------------------------------------------------------------------
	0x05, 0x01,			// (GLOBAL) USAGE_PAGE         0x0001 Generic Desktop Page
	0x09, 0x0C,			// (LOCAL)  USAGE              0x0001000C Wireless Radio Controls (CA=Application Collection)
	0xA1, 0x01,			// (MAIN)   COLLECTION         0x01 Application (Usage=0x0001000C: Page=Generic Desktop Page, Usage=Wireless Radio Controls, Type=CA)
	0x85, 0x03,			//   (GLOBAL) REPORT_ID          0x03 (3)
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x25, 0x01,			//   (GLOBAL) LOGICAL_MAXIMUM    0x01 (1)
	0x09, 0xC6,			//   (LOCAL)  USAGE              0x000100C6 Wireless Radio Button (OOC=On/Off Control)
//	0x95, 0x01,			//   (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields <-- Redundant: REPORT_COUNT is already 1
	0x75, 0x01,			//   (GLOBAL) REPORT_SIZE        0x01 (1) Number of bits per field
	0x81, 0x06,			//   (MAIN)   INPUT              0x00000006 (1 field x 1 bit) 0=Data 1=Variable 1=Relative 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0x75, 0x07,			//   (GLOBAL) REPORT_SIZE        0x07 (7) Number of bits per field
	0x81, 0x03,			//   (MAIN)   INPUT              0x00000003 (1 field x 7 bits) 1=Constant 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0xC0,				// (MAIN)   END_COLLECTION     Application

	//--------------------------------------------------------------------------------
	// Decoded Application Collection
	//--------------------------------------------------------------------------------
	0x05, 0x88,			// (GLOBAL) USAGE_PAGE         0x0088
	0x09, 0x01,			// (LOCAL)  USAGE              0x00880001
	0xA1, 0x01,			// (MAIN)   COLLECTION         0x01 Application (Usage=0x00880001: Page=, Usage=, Type=) <-- Warning: USAGE type should be CA (Application)
	0x85, 0x04,			//   (GLOBAL) REPORT_ID          0x04 (4)
	0x19, 0x00,			//   (LOCAL)  USAGE_MINIMUM      0x00880000
	0x2A, 0xFF,0x00,	//   (LOCAL)  USAGE_MAXIMUM      0x008800FF <-- Fixed: was 0x0088FFFF
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x26, 0xFF,0x00,	//   (GLOBAL) LOGICAL_MAXIMUM    0x00FF (255) <-- Fixed: was 0xFFFF (65535)
	0x75, 0x08,			//   (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field
	0x95, 0x02,			//   (GLOBAL) REPORT_COUNT       0x02 (2) Number of fields
	0x81, 0x02,			//   (MAIN)   INPUT              0x00000002 (2 fields x 8 bits) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0xC0,				// (MAIN)   END_COLLECTION     Application

	//--------------------------------------------------------------------------------
	// Decoded Application Collection
	//--------------------------------------------------------------------------------
	0x05, 0x01,			// (GLOBAL) USAGE_PAGE         0x0001 Generic Desktop Page
	0x09, 0x80,			// (LOCAL)  USAGE              0x00010080 System Control (CA=Application Collection)
	0xA1, 0x01,			// (MAIN)   COLLECTION         0x01 Application (Usage=0x00010080: Page=Generic Desktop Page, Usage=System Control, Type=CA)
	0x85, 0x05,			//   (GLOBAL) REPORT_ID          0x05 (5)
	0x19, 0x81,			//   (LOCAL)  USAGE_MINIMUM      0x00010081 System Power Down (OSC=One Shot Control)
	0x29, 0x83,			//   (LOCAL)  USAGE_MAXIMUM      0x00010083 System Wake Up (OSC=One Shot Control)
//	0x15, 0x00,			//   (GLOBAL) LOGICAL_MINIMUM    0x00 (0) <-- Redundant: LOGICAL_MINIMUM is already 0
	0x25, 0x01,			//   (GLOBAL) LOGICAL_MAXIMUM    0x01 (1)
	0x95, 0x08,			//   (GLOBAL) REPORT_COUNT       0x08 (8) Number of fields
	0x75, 0x01,			//   (GLOBAL) REPORT_SIZE        0x01 (1) Number of bits per field
	0x81, 0x02,			//   (MAIN)   INPUT              0x00000002 (8 fields x 1 bit) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
	0xC0,				// (MAIN)   END_COLLECTION     Application
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

#ifdef DEBUG_SYNAPTICS
	printk(KERN_ALERT "synaptics_probe\n");
#endif

	//hdev->quirks |= id->driver_data;

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
	{ HID_USB_DEVICE(USB_VENDOR_ID_SYNAPTICS, USB_VENDOR_ID_SYNAPTICS_KEYBOARD) },
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
