# meta-linht-hardware

A Yocto Project hardware layer for the [LinHT Handheld Transceiver](https://github.com/M17-Project/LinHT-hw), providing machine configuration and hardware support for the CompuLab MCM-iMX93 based platform.

## Overview

This layer provides hardware abstraction and device support for the LinHT project, developed by SP5WWP, OE3ANC and OK5VAS as part of the [M17 project ecosystem](https://m17project.org/). The layer targets the CompuLab MCM-iMX93 System-on-Module with custom hardware peripherals for radio frequency applications.

## Hardware

### Base platform
- **SoM**: [CompuLab MCM-iMX93](https://www.compulab.com/products/computer-on-modules/mcm-imx93-nxp-i-mx-93-som-smd-system-on-module/)
- **SoC**: NXP i.MX93 (Cortex-A55 dual-core + Cortex-M33)
- **Architecture**: ARMv8.2-A (Cortex-A55)

### Peripheral support

#### Audio subsystem
- **Codec**: Wolfson WM8960 (I2C address 0x1a)
  - Connected via SAI3 (I2S1)
- **RF I/Q Interface**: SX1255 transceiver
  - Connected via SAI1 (I2S0)
  - Bidirectional I/Q data streaming
  - SPI control interface via LPSPI3

#### Display
- **LCD Controller**: Sitronix ST7735R
  - 128x160 pixel TFT display
  - SPI interface via LPSPI6
  - 270° rotation configured
  - PWM backlight control via TPM3_CH3

#### User interface
- **Keyboard**: 9-key matrix (D1-D9)
- **Buttons**: PTT (internal/external), side button
- **LEDs**: 2x status LEDs
- **PWM**: Keyboard backlight via TPM3_CH2

#### Communication
- **I2C**: LPI2C3 for audio codec communication
- **SPI**: 
  - LPSPI3: SX1255 RF transceiver control
  - LPSPI6: ST7735R display interface
- **GPIO**: Extensive GPIO mapping for user controls and RF management

## Layer structure

```
meta-linht-hardware/
├── conf/
│   ├── layer.conf                    # Layer configuration
│   └── machine/
│       └── mcm-imx93.conf           # Machine definition
├── recipes-kernel/
│   └── linux/
│       ├── linux-compulab_6.%.bbappend  # Kernel recipe append
│       └── files/
│           ├── custom_defconfig.cfg      # Kernel configuration
│           ├── sbc-mcm-imx93.dts        # Device tree source
│           └── sbc-mcm-imx93.dtsi       # Device tree include
├── COPYING.GPL-v3                       # License file
└── README.md                        # This file
```

## Machine configuration

The [mcm-imx93.conf](conf/machine/mcm-imx93.conf) machine configuration provides:

- **Bootloader**: U-Boot CompuLab variant
- **Kernel**: Linux 6.x with CompuLab patches
- **Device Tree**: Custom `sbc-mcm-imx93.dtb`
- **Serial Console**: 115200 baud on ttyLP0

## Kernel configuration

The layer appends custom configuration to the CompuLab kernel via [linux-compulab_6.%.bbappend](recipes-kernel/linux/linux-compulab_6.%.bbappend):

### DRM/Graphics support
- DRM core infrastructure with KMS helpers
- TinyDRM framework for simple displays
- ST7735R panel driver support
- MIPI DBI interface support

### Peripheral drivers
- i.MX SPI controller (CONFIG_SPI_IMX)
- i.MX TPM PWM driver (CONFIG_PWM_IMX_TPM)
- USB Ethernet gadget support
- Mailbox framework
- Backlight class device support

## Device tree configuration

The custom device tree ([sbc-mcm-imx93.dtsi](recipes-kernel/linux/files/sbc-mcm-imx93.dtsi)) configures:

### Pin multiplexing
- **SAI1**: I2S interface for SX1255 (TX_BCLK, TX_SYNC, TX_DATA, RX_DATA)
- **SAI3**: I2S interface for WM8960 with MCLK output
- **LPSPI3**: SPI for SX1255 control
- **LPSPI6**: SPI for ST7735R display
- **LPI2C3**: I2C for WM8960 codec
- **TPM3**: PWM channels for backlight control
- **GPIO**: User interface elements (keyboard, buttons, LEDs)

### Audio configuration
```dts
sound0: SX1255 I/Q interface (simple-audio-card)
sound1: WM8960 audio codec (fsl,imx-audio-wm8960)
```

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

