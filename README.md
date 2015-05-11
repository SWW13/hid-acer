# hid-acer kernel module

Acer keyboards e.g. in Acer SW5-012 use synaptics touchpad USB ID
(06cb:2968 and 06CB:2991) and have the following issue:
 - The report descriptor specifies an excessively large number of usages
   and logical max (2^16), which is more than HID_MAX_USAGES. This prevents
   proper parsing of the report descriptor.

# Known broken keyboards
* `06CB:2968`
  * Acer SW5-012
  * Acer SW5-171
* `06CB:2991`
  * Acer SW5-171

# Build / Install
* Install linux-headers (for current version) and build utils.  
  (see e.g. http://www.cyberciti.biz/tips/build-linux-kernel-module-against-installed-kernel-source-tree.html)

```
git clone https://github.com/SWW13/hid-synaptics.git
cd hid-synaptics
make
sudo make install
```

# Uninstall
```
sudo make uninstall
```
