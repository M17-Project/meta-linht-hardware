FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Faster boot: no autoboot delay (a key held during power-on still stops
# autoboot), eMMC first with the backup SD card as fallback, no USB scan
SRC_URI += "file://0001-compulab-imx93-boot-from-eMMC-first-skip-USB.patch"
SRC_URI += "file://bootdelay.cfg"

# No U-Boot video/splash: the CompuLab default initializes an LCDIF/LVDS
# panel that LinHT does not have
SRC_URI += "file://no-video.cfg"
