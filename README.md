# ztime
Firmware for pinetime smartwatch written in C and based on ZephyrOS

# Note: This software is not intended for use on the territory of the Russian Federation and the Republic of Belarus. Any problems, up to and including damage to the device on which this software is running, are the responsibility of the user of this device.

## Developer getting started guide

This document assumes that you run a GNU/Linux or Mac operating system.

### Set up the development environment

Follow Zephyr's [Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html)
up to step 3.2 "Get the Zephyr source code". Here you should run the commands
below instead of the ones in the guide:

```
$ git clone git clone git@github.com:AlexXZero/ztime.git
$ west init -l ztime/
$ west update
$ cd ztime
```

Then complete the remaining steps under section 3 and 4.

### Build and flash ztime

Run `west build -p auto -b pinetime_devkit0 .` to build everything with the defaults

Then connect your in-circuit programmer and run `west flash`. To install
without a programmer, see Firmware updates below.

## Firmware updates

### SMP over Bluetooth LE

ztime supports firmware image management over the Simple Management Protocol.

To make use of this feature, get the
[mcumgr](https://github.com/apache/mynewt-mcumgr#command-line-tool) command-line
tool. Then run the commands below to list, upload, test and confirm firmware
images over BLE:

```
# mcumgr --conntype="ble" --connstring ctlr_name=hci0,peer_name='ztime' image list
# mcumgr --conntype="ble" --connstring ctlr_name=hci0,peer_name='ztime' image upload ztime-mcuboot-app.bin
# mcumgr --conntype="ble" --connstring ctlr_name=hci0,peer_name='ztime' image test <hash of slot-1 image>
# mcumgr --conntype="ble" --connstring ctlr_name=hci0,peer_name='ztime' reset
# mcumgr --conntype="ble" --connstring ctlr_name=hci0,peer_name='ztime' image confirm
```

If you are unhappy with the new image, simply run the `reset` command again
instead of `image confirm` to revert to the old one. If the image has already
been confirmed but you still want to revert, simply run the commands above but
skip the upload step or perform a manual rollback (see below). [See this
document for more
information](https://docs.zephyrproject.org/latest/samples/subsys/mgmt/mcumgr/smp_svr/README.html).

### DFU over Bluetooth LE

To install ztime over the air from
[InfiniTime](https://github.com/JF002/Pinetime), run
`adafruit-nrfutil dfu genpkg --dev-type 0x0052 --application ztime-mcuboot-app.bin ztime-dfu-app.zip`
to create a (Nordic) DFU package and upload it using
[ota-dfu.py](https://github.com/JF002/Pinetime/tree/master/bootloader/ota-dfu-python)
or nRF Connect.

### Manual rollback

Version 5 of Lup Yuen's bootloader allows you to revert to the old firmware
image by holding the button during boot.
