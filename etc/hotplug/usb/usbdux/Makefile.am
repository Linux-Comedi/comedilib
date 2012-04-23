EXTRA_DIST = usbdux usbdux_firmware.hex usbdux_firmware.bin

if INSTALL_UDEVHOTPLUG
udevfirmware_DATA = usbdux_firmware.bin
endif

if INSTALL_ETCHOTPLUG
usbfirmware_DATA =  usbdux_firmware.hex
usbhotplug_SCRIPTS = usbdux
endif
