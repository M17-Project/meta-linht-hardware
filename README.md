# meta-linht-hardware

A Yocto Project hardware layer for the [LinHT Handheld Transceiver](https://github.com/M17-Project/LinHT-hw), providing machine configuration and hardware support for the CompuLab MCM-iMX93 based platform.

## Overview

This layer provides hardware abstraction and device support for the LinHT project, developed by SP5WWP, OE3ANC and OK5VAS as part of the [M17 project ecosystem](https://m17project.org/). The layer targets the CompuLab MCM-iMX93 System-on-Module with custom hardware peripherals for radio frequency applications.

The current device tree targets **LinHT Rev C** hardware.

**Yocto release:** scarthgap

**Required layers:**
- CompuLab BSP layers for the MCM-iMX93 (meta-bsp-imx9, meta-compulab, meta-compulab-bsp) on top of the NXP i.MX BSP
- [meta-linht-software](https://github.com/M17-Project/meta-linht-software) (provides `linht-image`, which this layer extends)

The full build procedure is described on the M17 wiki: [LinHT Image Build](https://wiki.m17foundation.org/index.php?title=LinHT_Image_Build).

## Hardware

### Base platform
- **SoM**: [CompuLab MCM-iMX93](https://www.compulab.com/products/computer-on-modules/mcm-imx93-nxp-i-mx-93-som-smd-system-on-module/)
- **SoC**: NXP i.MX93 (Cortex-A55 dual-core + Cortex-M33)
- **Architecture**: ARMv8.2-A (Cortex-A55)

### Peripheral support

#### Audio subsystem
- **Codec**: Texas Instruments TLV320AIC3100 (I2C address 0x18 on LPI2C3)
  - Connected via SAI3 (I2S1)
  - MCLK from a dedicated 12 MHz oscillator (ECS-2520MV); the codec is I2S bit/frame clock master
  - Exposed as ALSA card `AUDIO`
  - Microphone gain and mute controlled via GPIO
  - Headphone jack detection via GPIO
- **RF I/Q Interface**: SX1255 transceiver
  - Connected via SAI1 (I2S0)
  - Bidirectional I/Q data streaming, exposed as ALSA card `SX1255`
  - SPI control interface via LPSPI3 (`spidev`)

#### Display
- **LCD Controller**: Sitronix ST7735R
  - 128x160 pixel TFT display
  - SPI interface via LPSPI6
  - 270° rotation configured
  - PWM backlight control via TPM3_CH3

#### User interface
- **Keypad**: 5x4 matrix (lines D1-D9), handled by the `matrix-keypad` kernel module
- **Buttons**: PTT (internal/external), side button
- **LEDs**: 2x status LEDs, flashlight
- **PWM**: Keypad backlight via TPM4_CH3

#### RF control
- RF switch control, attenuator (serial interface), PA enable and SX1255 reset via GPIO

#### GNSS
- GNSS receiver on LPUART3 (ENET1 pads)
- GPIOs for reset, wakeup and PPS

#### Communication
- **I2C**: LPI2C3 for audio codec communication
- **SPI**:
  - LPSPI3: SX1255 RF transceiver control
  - LPSPI6: ST7735R display interface
- **UART**: LPUART3 for GNSS
- **GPIO**: Extensive GPIO mapping for user controls and RF management

## Layer structure

```
meta-linht-hardware/
├── conf/
│   ├── layer.conf                        # Layer configuration
│   └── machine/
│       └── mcm-imx93.conf                # Machine definition
├── recipes-core/
│   └── images/
│       └── linht-image.bbappend          # Adds hardware kernel modules to linht-image
├── recipes-kernel/
│   ├── linux/
│   │   ├── linux-compulab_6.%.bbappend   # Kernel recipe append
│   │   └── files/
│   │       ├── custom_defconfig.cfg      # Kernel configuration
│   │       ├── sbc-mcm-imx93.dts         # Device tree source
│   │       └── sbc-mcm-imx93.dtsi        # Device tree include
│   └── matrix-keypad/
│       ├── matrix-keypad-module_0.1.bb   # Keypad kernel module
│       └── files/                        # Module source and Makefile
├── COPYING.GPL-v3                        # License file
└── README.md                             # This file
```

## Machine configuration

The [mcm-imx93.conf](conf/machine/mcm-imx93.conf) machine configuration provides:

- **Bootloader**: U-Boot CompuLab variant
- **Kernel**: Linux 6.x with CompuLab patches
- **Device Tree**: Custom `sbc-mcm-imx93.dtb` (model `LinHT`)
- **Serial Console**: 115200 baud on ttyLP0

## Kernel configuration

The layer appends custom configuration to the CompuLab kernel via [linux-compulab_6.%.bbappend](recipes-kernel/linux/linux-compulab_6.%.bbappend):

### DRM/Graphics support
- DRM core infrastructure with KMS helpers
- TinyDRM framework for simple displays
- ST7735R panel driver support
- MIPI DBI interface support

### Audio support
- TLV320AIC31xx codec driver (CONFIG_SND_SOC_TLV320AIC31XX)
- ALSA SoC simple sound card (CONFIG_SND_SOC_SIMPLE_CARD)

### Peripheral drivers
- i.MX SPI controller (CONFIG_SPI_IMX)
- i.MX TPM PWM driver (CONFIG_PWM_IMX_TPM)
- USB Ethernet gadget support
- Mailbox framework
- Backlight class device support

## Keypad driver

The [matrix-keypad-module](recipes-kernel/matrix-keypad/matrix-keypad-module_0.1.bb) recipe builds an out-of-tree kernel module for the LinHT (C62) keypad, based on the OpenRTX implementation. It binds to the `linht,matrix-keypad` device tree node, scans the key matrix, reads the PTT, external PTT and side buttons, and is loaded automatically at boot.

## Device tree configuration

The custom device tree ([sbc-mcm-imx93.dtsi](recipes-kernel/linux/files/sbc-mcm-imx93.dtsi)) configures:

### Pin multiplexing
- **SAI1**: I2S interface for SX1255 (TX_BCLK, TX_SYNC, TX_DATA, RX_DATA)
- **SAI3**: I2S interface for TLV320AIC3100 (clock slave, no MCLK output)
- **LPSPI3**: SPI for SX1255 control
- **LPSPI6**: SPI for ST7735R display
- **LPI2C3**: I2C for TLV320AIC3100 codec
- **LPUART3**: GNSS UART
- **TPM3/TPM4**: PWM channels for display and keypad backlight
- **GPIO**: User interface elements (keypad, buttons, LEDs), RF control, GNSS control, jack detection

### Audio configuration
```dts
sound0: SX1255 I/Q interface (simple-audio-card, "SX1255")
sound1: TLV320AIC3100 audio codec (simple-audio-card, "AUDIO")
```

The codec MCLK is described as a 12 MHz `fixed-clock` node. 12 MHz is in the TLV320AIC31xx driver's native divider table, so all standard sample rates work without kernel patches. On Rev C the SoM's `SAI3_MCLK` pin is no longer connected to the codec.

### Disabled CompuLab defaults
- **USDHC3**, **LPSPI1**, **LPUART5** and **LPI2C6** are disabled because their pins are reused by LinHT peripherals

## Integration instructions

### Adding to build configuration

1. Add layer to `bblayers.conf`:
```bash
BBLAYERS += "/path/to/meta-linht-hardware"
```

2. Set machine in `local.conf`:
```bash
MACHINE = "mcm-imx93"
```

## License

This layer is distributed under the GNU General Public License v3.0. See [COPYING.GPL-v3](COPYING.GPL-v3) for details.
