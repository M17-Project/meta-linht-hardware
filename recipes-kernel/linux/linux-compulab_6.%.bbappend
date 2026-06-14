FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://sbc-mcm-imx93.dts"
SRC_URI += "file://sbc-mcm-imx93.dtsi"
SRC_URI += "file://custom_defconfig.cfg"
SRC_URI += "file://0001-ASoC-tlv320aic31xx-add-divider-table-entries-for-12.patch"

do_configure:append() {
    cp ${WORKDIR}/sbc-mcm-imx93.dts ${S}/arch/${ARCH}/boot/dts/compulab/
    cp ${WORKDIR}/sbc-mcm-imx93.dtsi ${S}/arch/${ARCH}/boot/dts/compulab/

    cat ${WORKDIR}/custom_defconfig.cfg >>  ${B}/.config
}
