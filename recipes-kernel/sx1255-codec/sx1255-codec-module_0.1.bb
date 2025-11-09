SUMMARY = "Codec Kernel module for the Semtech SX1255"
DESCRIPTION = "Codec Kernel module for the Semtech SX1255"
LICENSE = "GPL-3.0-or-later"
LIC_FILES_CHKSUM = "file://sx1255-codec.c;beginline=6;endline=9;md5=18b25acdb3b8dd4c6320ddff8a7b6c56"

# Inherit the 'module' class to build a kernel module
inherit module

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Point to the source files within the recipe's directory
SRC_URI = "file://sx1255-codec.c \
           file://Makefile \
          "

S = "${WORKDIR}"

# This ensures the module gets auto-loaded if the device tree node is present
KERNEL_MODULE_AUTOLOAD += "sx1255-codec"

do_install() {
    echo "KERNEL_BUILD=${KERNEL_BUILD}" >&2
    echo "B=${B}" >&2
    echo "WORKDIR=${WORKDIR}" >&2

    install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 ${B}/sx1255-codec.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/extra/
}

EXTRA_OEMAKE = "V=1"
