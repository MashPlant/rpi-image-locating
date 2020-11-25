#!/bin/sh

set -u
set -e

# Add a console on tty1
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    grep -qE '^tty1::' ${TARGET_DIR}/etc/inittab || \
	sed -i '/GENERIC_SERIAL/a\
tty1::respawn:/sbin/getty -L  tty1 0 vt100 # HDMI console' ${TARGET_DIR}/etc/inittab
fi

UNNEEDED_FILES="20-acpi-vendor.hwdb \
    20-bluetooth-vendor-product.hwdb \
    20-OUI.hwdb \
    20-pci-vendor-model.hwdb \
    20-usb-vendor-model.hwdb \
    60-keyboard.hwdb \
    60-sensor.hwdb \
    70-mouse.hwdb"

rm -f ${TARGET_DIR}/etc/udev/hwdb.bin
if [ -d ${TARGET_DIR}/etc/udev/hwdb.d ]; then
    cd ${TARGET_DIR}/etc/udev/hwdb.d
    for f in $UNNEEDED_FILES; do
        if [ -f $f ]; then
            rm $f
        fi
    done
fi
