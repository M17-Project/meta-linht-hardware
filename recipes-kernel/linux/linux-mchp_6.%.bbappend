FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}/files:"

SRC_URI:append = " file://sam9x60_spi_i2s.dtsi"
SRC_URI:append = " file://spdif.cfg"

do_configure:append() {
    cp ${WORKDIR}/sam9x60_spi_i2s.dtsi ${S}/arch/${ARCH}/boot/dts/microchip/
    echo '#include "sam9x60_spi_i2s.dtsi"' >> ${S}/arch/${ARCH}/boot/dts/microchip/at91-sam9x60_curiosity.dts
}
