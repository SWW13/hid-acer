#!/bin/bash

#install synaptics driver

rmmod ./hid-synaptics.ko; rmmod usbhid;
insmod ./hid-synaptics.ko ; modprobe usbhid;
