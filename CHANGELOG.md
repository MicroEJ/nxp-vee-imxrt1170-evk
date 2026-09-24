# Changelog
All notable changes to this project will be documented in this file.

## [4.0.0] - 2026-09-24

### Added

- Add a `NOTICE.txt` file listing Component Specific Licenses
- Add Serial Foundation Library support.
- Add an overlap-safe, word-wide `memmove()` implementation to the GCC BSP to speed up `System.arraycopy()` on byte arrays.
- Add ABLA Essentials 1.0.1.

### Changed

- Project moved under MicroEJ BSD License.
- Module moved under `com.microej.veeport.nxp.mimxrt1170` package.
- MicroUI Pack is now under MICROEJ COMMERCIAL SOFTWARE COMPONENT LICENSE AGREEMENT.
- MicroVG Pack is now under MICROEJ COMMERCIAL SOFTWARE COMPONENT LICENSE AGREEMENT.
- Update MicroEJ SDK to 1.8.0.
- Update Architecture to 8.6.0.
- Update MicroUI Pack to 14.5.2.
- Update MicroVG Pack to 1.8.1.
- Update Event Queue to 3.0.6.
- Update FS pack to 6.0.5.
- Update Freetype thirdparty library to 2.14.1.
- Update Harfbuzz thirdparty library to 12.3.0.
- validation: Update core testsuite to 3.7.0. The `ArrayCopyPerformance` test now also measures two overlapping copies, and its default buffer size is 64 KB.
- validation: Set the four `ArrayCopyPerformance` minimum throughput thresholds to the MIMXRT1170-EVKB baseline minus 10%.
- Switch the GCC BSP C library from newlib-nano to the full newlib to improve `System.arraycopy()` throughput.

### Fixed

- core: Fix implementation of LLMJVM_IMPL_ackWakeup().

## [3.1.0] - 2025-09-12

### Added

- Add MicroAI support.
- Add Event Queue support.

### Changed

- Update MicroEJ SDK to 1.3.1 version.
- Update LLKERNEL Abstraction Layer to 3.0.1 version.
- Updated VG pack to v1.7.1.
- Update Net pack to v12.0.0.

### Fixed

- ui: Draw transparent images using GPU.
- ssl: Remove ciphersuites limitation.

## [3.0.0] - 2025-03-07

### Changed

- Update folder architecture to new VEE Port template.
- Update to gradle build system for SDK6 compatibility and remove ivy scripts.
- validation: Align properties with new PQT.
- Add default values for VEE Port properties that didn't appear before.
- Rename nxpvee_ui in npavee.
- vee-port: extensions: Update frontpanel widget and UI pack version.
- vee/scripts: Use standard ARM toolchain instead of NXP one.
- core: Update architecture to 8.3.0.
- net: Update to pack 11.2.0.
- validation: Update security testsuite to 1.8.0 and net testsuite to 4.1.2.
- ui: Update to pack 14.3.2.
- Put VMCOREMicroJvm in ITCM for better performances.
- Put images_heap in cacheable RAM for better UI performances.

### Fixed

- net: Workaround to get a unique MAC address.
- net: lwip_util: Fix connectivity issues.

## [2.2.0] - 2024-09-27

### Added

- Add AI Foundation Library with sample application.
- Add MIMXRT1170-EVKB support (as default).
- Add multi-Sandbox capability support.

### Changed

- Updated MEJ architecture to v8.1.1.
- Updated PQT Core to 3.2.0.
- Updated UI pack to v14.0.2.
- Updated VG pack to v1.6.0.
- Updated MCUXpresso SDK to v2.15.100.
- Updated BSP build system to CMakePresets.

## [2.1.1] - 2024-02-16

### Changed

- Fix MicroEJ SDK 6 builds on Linux.

## [2.1.0] - 2024-01-19

### Added

- Add Changelog.

### Changed

- Update MicroEJ UI pack to 13.7.2.

## [2.0.0] - 2024-01-03

### Added

- Add VSCode support.
- Add MicroEJ Device support.
- Add support for Multi-Sandbox Firmware.
- Add MicroEJ security-mbedtls CCO.
- Add MicroEJ CPU Load.
- Add SystemView.

### Changed

- Update MicroEJ Architecture to flopi7G26#8.0.0.
- Update MicroEJ UI pack to 13.7.0.
- Update MicroEJ VG pack to 1.4.1.
- Update MicroEJ FS pack to 6.0.4.
- Update MicroEJ NET pack to 11.0.2.
- Update AnimatedMascot application.

### Removed

- Remove MCUXPresso IDE support.

## [1.0.0] - 2023-09-11

### Added

- Add MicroEJ CORE support.
- Add MicroEJ UI support.
- Add MicroEJ VG support.
- Add MicroEJ FS support.
- Add MicroEJ NET support.
- Add Demo applications (AnimatedMascot & SimpleGFX).
- Initial release of the VEE Port.

---

_Markdown_  

_Copyright 2026 MicroEJ Corp. All rights reserved._
_Use of this source code is governed by a BSD-style license that can be found with this software._

_Build: 7E4D1F7C_
