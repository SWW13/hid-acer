# hid-acer kernel module

Acer keyboards e.g. in Acer SW5-012 use synaptics touchpad USB ID
(06cb:2968, 06CB:2991 & 06CB:74D9) and have the following issue:
 - The report descriptor specifies an excessively large number of usages
   and logical max (2^16), which is more than HID_MAX_USAGES. This prevents
   proper parsing of the report descriptor.

# Upstream fix
see http://www.spinics.net/lists/linux-input/msg40080.html
> Applied to for-4.3/upstream

# Known broken keyboards
* `06CB:2968`
  * Acer SW5-012
* `06CB:2991`
  * Acer SW5-171
* `06CB:74d9`
  * Acer SW5-171 Firmware revision 2.1

# Build / Install
* Install linux-headers (for current version) and build utils.  
  (see e.g. http://www.cyberciti.biz/tips/build-linux-kernel-module-against-installed-kernel-source-tree.html)

```
git clone https://github.com/SWW13/hid-acer.git
cd hid-acer
make
sudo make install
```

# Uninstall
```
sudo make uninstall
```
