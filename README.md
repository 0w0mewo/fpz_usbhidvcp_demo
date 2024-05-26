## Buggy demo on emulating flipper zero as an USB HID+VCP composite device

### What it is

A simple demo to capture on-die temperature and report it through USB HID and print logs on USB VCP console.

### How to use

`ok`: start/stop.
`long press on back`: exit

Known issues:

- ~~The HID sensor is not recognisable by Windows if CDC also emulated. Therefore, To turn off logging to VCP in order to emulate temperature sensor properly, toggle `WITH_VCP` to `0` at `usb/config.h` and re-compile the FAP.~~ Resolved at commit 48253ecec05dcc5229e62b3004ab44321a444c34.
- Linux kernel fails to probe the HID sensor as a HID sensor but a generic HID device. However, VCP works perfectly.
