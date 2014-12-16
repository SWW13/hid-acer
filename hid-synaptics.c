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
#include <linux/slab.h>
#include <linux/usb.h>

#define DEBUG_SYNAPTICS

static int synaptics_raw_event(struct hid_device *hdev, struct hid_report *report,
		__u8 *rawdata, int size)
{
	// nothing done so far

    return 0;
}

static int synaptics_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;

#ifdef DEBUG_SYNAPTICS
	printk(KERN_ALERT "synaptics_probe\n");
#endif

	hdev->quirks |= id->driver_data;

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
	{ HID_USB_DEVICE(USB_VENDOR_ID_SYNAPTICS, USB_VENDOR_ID_SYNAPTICS_KEYBOARD),
		 .driver_data = 0x0},
};
MODULE_DEVICE_TABLE(hid, synaptics_devices);

static struct hid_driver synaptics_driver = {
	.name = "synaptics",
	.id_table = synaptics_devices,
	.probe = synaptics_probe,
	.remove = synaptics_remove,
	.raw_event = synaptics_raw_event
};

static int __init synaptics_init(void)
{
#ifdef DEBUG_SYNAPTICS
	printk(KERN_ALERT "synaptics_init\n");
#endif
	return hid_register_driver(&synaptics_driver);
}

static void __exit synaptics_exit(void)
{
#ifdef DEBUG_SYNAPTICS
	printk(KERN_ALERT "synaptics_exit\n");
#endif
	hid_unregister_driver(&synaptics_driver);
}

module_init(synaptics_init);
module_exit(synaptics_exit);
MODULE_LICENSE("GPL");
