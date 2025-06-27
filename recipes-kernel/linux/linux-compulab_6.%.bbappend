FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/files:"

SRC_URI:append = " file://sbc-mcm-imx93.dtsi"
SRC_URI:append = " file://spdif.cfg"

do_configure:append() {
    cp ${WORKDIR}/sbc-mcm-imx93.dtsi ${S}/arch/${ARCH}/boot/dts/compulab/
    echo '#include "sbc-mcm-imx93.dtsi"' >> ${S}/arch/${ARCH}/boot/dts/compulab/sbc-mcm-imx93.dts
}
