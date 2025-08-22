SUMMARY = "Kernel module for the LinHT (C62) keypad"
DESCRIPTION = "Based on the OpenRTX implementation of the C62 keypad"
LICENSE = "GPL-3.0-or-later"
LIC_FILES_CHKSUM = "file://matrix-keypad.c;beginline=2;endline=4;md5=25b314258cc162e136570a441862fd9b"

# Inherit the 'module' class to build a kernel module
inherit module

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Point to the source files within the recipe's directory
SRC_URI = "file://matrix-keypad.c \
           file://Makefile \
          "

S = "${WORKDIR}"

# This ensures the module gets auto-loaded if the device tree node is present
KERNEL_MODULE_AUTOLOAD += "matrix-keypad"

do_install() {
    echo "KERNEL_BUILD=${KERNEL_BUILD}" >&2
    echo "B=${B}" >&2
    echo "WORKDIR=${WORKDIR}" >&2

    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${B}/matrix-keypad.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
}

EXTRA_OEMAKE = "V=1"
