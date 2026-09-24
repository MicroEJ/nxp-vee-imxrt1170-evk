# VEE Port Release Notes for NXP i.MX RT1170 EVK

## Description

This is the release notes of the VEE Port for the NXP i.MX RT1170 Evaluation Kit (MIMXRT1170-EVK and MIMXRT1170-EVKB) with the RK055HDMIPI4MA0 display panel.

## Versions

### VEE Port

4.0.0

### Dependencies

This VEE Port contains the following dependencies:

| Dependency Name                          | Version |
| ---------------------------------------- | ------- |
| Architecture (flopi7G26, Cortex-M7 GCC)  | 8.6.0   |
| MicroUI Pack                             | 14.5.2  |
| MicroVG Pack                             | 1.8.1   |
| FS Pack                                  | 6.0.5   |
| NET Pack (NET, SSL, SECURITY)            | 12.0.0  |
| ECOM-WIFI Pack                           | 1.1.0   |
| Device Pack                              | 1.2.0   |
| Event Queue                              | 3.0.6   |
| Serial Pack                              | 3.0.1   |
| MicroAI                                  | 2.3.0   |
| GPIO Pack (NXP)                          | 1.0.0   |

Please refer to the VEE Port [module description file](./vee-port/build.gradle.kts)
and the [version catalogs file](./gradle/libs.versions.toml) for more details.

### Board Support Package

- BSP provider and name: NXP MCUXpresso SDK
- BSP version: 2.15.100

Please refer to the NXP MCUXpresso SDK git repository
available [here](https://github.com/nxp-mcuxpresso/mcux-sdk).

The BSP is fetched with `west` from the [manifest file](./west.yml).

### Third Party Software

The third party software used in the BSP, its location and its license are listed in the [NOTICE file](./NOTICE.txt).
The components of the MCUXpresso SDK are detailed in its software content register, `bsp/mcux-sdk/core/SW-Content-Register.txt`, once the BSP is fetched.

## Features

### Architecture

The VEE Port is built in mono-sandbox mode by default.
Multi-Sandbox can be enabled with the `com.microej.runtime.capability` property of the [VEE Port configuration file](./vee-port/configuration.properties).
The Multi-Sandbox Application download implementation is based on the Best Fit Allocator backend in RAM, so without reboot persistence.

### MicroUI Pack

The VEE Port features a graphical user interface based on MicroUI.
Drawings are accelerated by the GCNanoLite-V GPU through the VGLite library.

The display is the RK055HDMIPI4MA0 panel, a 720 x 1280 MIPI DSI display.
The frame buffers use the RGB565 pixel format, 16 bits per pixel.
Three frame buffers are allocated at fixed addresses in the non-cacheable external SDRAM, and the display uses the predraw buffer refresh strategy.

MicroUI uses a RAM buffer to store the dynamic images data in external SDRAM.

### MicroVG Pack

The VEE Port features vector graphics based on MicroVG, accelerated by the GCNanoLite-V GPU through the VGLite library.
Vector fonts are rendered with FreeType and shaped with HarfBuzz.

### Device Pack

The Device UID is based on the MCU silicon ID read from the OCOTP fuses.

### Network Pack

The VEE Port features a network interface with the 1 Gbit Ethernet port as the underlying hardware media.

### ECOM-WIFI Pack

A Wi-Fi interface is available through the Murata 1XK (NXP IW416) M.2 extension board and the NXP Wi-Fi driver.

### SSL Pack

The SSL Abstraction Layer is implemented on top of Mbed TLS.

### Security Pack

The Security Abstraction Layer is implemented on top of Mbed TLS.

### File System Pack

The VEE Port features a file system interface based on FatFs.
A microSD card is used for the storage, formatted with a FAT file system (FAT12, FAT16 or FAT32).

### Event Queue Pack

The Event Queue Abstraction Layer is implemented on top of a FreeRTOS queue through the OSAL APIs.

### MicroAI Pack

The MicroAI implementation is based on TensorFlow Lite for Microcontrollers, provided by the eIQ middleware of the MCUXpresso SDK.

### Serial Pack

The VEE Port features a serial Foundation Library on top of the LPUART peripherals, through the MCUXpresso SDK UART adapter with DMA transfers.

### GPIO Pack

The VEE Port features the NXP GPIO Foundation Library to drive the MCU pins from the Application.

### Trace

The Trace Foundation Library and the Core Engine monitoring are implemented on top of SEGGER SystemView.

## Known Issues/Limitations

Known issues:

- The RSA key size of the SECURITY Foundation Library is limited to 2048 bits. Larger keys cause errors.
- The secp256k1 elliptic curve is not supported by the [KeyPairGenerator](https://repository.microej.com/javadoc/microej_5.x/apis/java/security/KeyPairGenerator.html) and causes a crash when the `MBEDTLS_FREESCALE_CAAM_PKHA` flag is defined.

## VEE Port Memory Layout

### Memory Sections

Each memory section is described in the GCC linker file available
[here](./bsp/vee/scripts/armgcc/MIMXRT1176xxxxx_cm7_flexspi_nor_sdram.ld).

The memories used by the VEE Port are:

| Memory      | Linker Region | Address      | Size    |
| ----------- | ------------- | ------------ | ------- |
| FlexSPI NOR flash | `m_text` | `0x30002400` | 16 MB   |
| ITCM        | `m_qacode`    | `0x00000000` | 256 KB  |
| OCRAM       | `m_data2`     | `0x20000000` | 256 KB  |
| SDRAM, cacheable | `m_data` | `0x80000000` | 48 MB   |
| SDRAM, non-cacheable | `m_ncache` | `0x83000000` | 16 MB |
| SDRAM, C heap | `m_heap`    | end of SDRAM | 512 KB  |

### Memory Layout

| Section Content  | Section Source   | Section Destination  | Memory Type |
| ---------------- | ---------------- | -------------------- | ----------- |
| MicroEJ Application static fields | `.bss.microej.statics` | `.bss` | SDRAM |
| MicroEJ Application threads stack blocks | `.bss.microej.stacks` | `.qadata` | OCRAM |
| MicroEJ Core Engine internal structures | `.bss.microej.runtime` | `.qadata` | OCRAM |
| MicroEJ Application heap | `.bss.microej.heap` | `.bss` | SDRAM |
| MicroEJ Application Immortals heap | `.bss.microej.immortals` | `.bss` | SDRAM |
| MicroEJ Application resources | `.rodata.microej.resource.*` | `.text` | FlexSPI NOR flash |
| MicroEJ Application and Library code | `.rodata.microej.soar` | `.text` | FlexSPI NOR flash |
| MicroEJ Core Engine hot code | `.text.VMCOREMicroJvm*` | `.ram_function` | ITCM |
| MicroEJ Multi-Sandbox Feature code chunk (Multi-Sandbox only) | `.bss.microej.kernel` | `.bss` | SDRAM |
| MicroUI images heap | `.bss.microui.display.imagesHeap` | `.bss` | SDRAM |
| MicroUI frame buffers | fixed addresses from `0x83880000`, not linker-managed | - | SDRAM, non-cacheable |

The C heap is placed in the `m_heap` region, at the end of the SDRAM.
Its size is set by the `HEAP_SIZE` symbol of the linker file.

Information on MicroEJ memory sections can be found in the [MicroEJ documentation](https://docs.microej.com/en/latest/VEEPortingGuide/coreEngine.html#link).

---

_Markdown_  
_Copyright 2026 MicroEJ Corp. All rights reserved._  
_Use of this source code is governed by a BSD-style license that can be found with this software._  
_Build: 7E4D1F7C_
