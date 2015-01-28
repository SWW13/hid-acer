#!/bin/bash

#install synaptics driver

rmmod ./hid-acer.ko; rmmod usbhid;
insmod ./hid-acer.ko ; modprobe usbhid;
