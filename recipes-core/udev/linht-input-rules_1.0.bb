SUMMARY = "Stable /dev/input symlink for the LinHT keypad"
DESCRIPTION = "udev rule that gives the matrix-keypad input device a fixed \
name, so applications do not depend on /dev/input/eventN allocation order."
LICENSE = "CLOSED"

inherit allarch

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI = "file://70-linht-input.rules"

S = "${WORKDIR}"

RDEPENDS:${PN} = "udev"

do_install() {
    install -Dm 0644 ${WORKDIR}/70-linht-input.rules \
        ${D}${nonarch_base_libdir}/udev/rules.d/70-linht-input.rules
}

FILES:${PN} = "${nonarch_base_libdir}/udev/rules.d/70-linht-input.rules"
