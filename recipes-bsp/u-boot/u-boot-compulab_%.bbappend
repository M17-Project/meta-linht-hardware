FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Faster boot: no autoboot delay (a key held during power-on still stops
# autoboot), eMMC first with the backup SD card as fallback, no USB scan
SRC_URI += "file://0001-compulab-imx93-boot-from-eMMC-first-skip-USB.patch"
SRC_URI += "file://bootdelay.cfg"

# No U-Boot video/splash: the CompuLab default initializes an LCDIF/LVDS
# panel that LinHT does not have
SRC_URI += "file://no-video.cfg"

# Boot splash on the ST7735R SPI display, image converted from a PNG
SRC_URI += "file://0002-mcm-imx93-LinHT-boot-splash-on-the-ST7735R-SPI-display.patch"
SRC_URI += "file://linht-splash.cfg"
SRC_URI += "file://linht-splash.png"
SRC_URI += "file://png2rgb565.py"

do_configure:prepend() {
    python3 ${WORKDIR}/png2rgb565.py ${WORKDIR}/linht-splash.png \
        ${S}/board/compulab/mcm-imx93/linht_splash_img.h 160 128
}
