FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://sbc-mcm-imx93.dtsi"
SRC_URI += "file://custom_defconfig.cfg"

do_configure:append() {
    cp ${WORKDIR}/sbc-mcm-imx93.dtsi ${S}/arch/${ARCH}/boot/dts/compulab/
    echo '#include "sbc-mcm-imx93.dtsi"' >> ${S}/arch/${ARCH}/boot/dts/compulab/sbc-mcm-imx93.dts

    cat ${WORKDIR}/custom_defconfig.cfg >>  ${B}/.config
}
