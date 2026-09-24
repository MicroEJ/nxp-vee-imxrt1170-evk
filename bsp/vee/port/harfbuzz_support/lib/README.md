
# Overview

MicroEJ C Module: `@MMM_MODULE_NAME@`, a patched version of Harfbuzz 10.0.1.

This module provides the source code of the text-shaping library [HarfBuzz](https://github.com/harfbuzz/harfbuzz), as well as support files to integrate it in the BSP.

# Usage

Add the following line to your `module.ivy`:

    @MMM_DEPENDENCY_DECLARATION@

## Building HarfBuzz

To integrate HarfBuzz into your project, it must first be built.
The directory `thirdparty/harfbuzz` contains the source code of HarfBuzz with the build files `meson.build` and `meson_options.txt` patched for compatibility with build scripts provided by MicroEJ.
The applied patch can be consulted in the file `harfbuzz_support/lib/meson.patch`.

This module provides build scripts to build the library.
As HarfBuzz depends on the Meson build system, you must first [install Meson and its dependencies](https://mesonbuild.com/Getting-meson.html).
Make sure the `meson` command is available in your shell.

You will also need FreeType to be available to your BSP.
You will typically do so by adding the [Abstraction Layer for FreeType](https://forge.microej.com/ui/repos/tree/General/microej-developer-repository-release/com/microej/clibrary/thirdparty/freetype) to your VEE Port.
If you choose to use the Abstraction Layer for FreeType, the header files of FreeType should be located in the directory `thirdparty/freetype/include`, sibling of the directory `thirdparty/harfbuzz`. This is where the build scripts will search first.

If your FreeType headers are located at a different path, you should modify the option `-Dfreetype_includes` in the `build.*` scripts to point to that location, or remove the option altogether to let Meson find the headers from the system locations (typically if FreeType is installed on an OS with a package manager).

Then, in a terminal, go to `harfbuzz_support/lib` and run either `build.bat` or `build.sh` depending on the shell used.
The file `libharfbuzz.a` will be created in `harfbuzz_support/lib`; you can then link your project against this file.

Before building HarfBuzz, you may want to edit `harfbuzz_support/lib/arm-armv5.txt` or the `build.*` scripts to better suit your needs.

### Building HarfBuzz for IAR

IAR cannot build HarfBuzz.
Instead, you should use the GNU toolchain (`arm-none-eabi-gcc`) to build it.

In addition to `libharfbuzz.a`, the file `_popcountsi2.o` will be added in `harfbuzz_support/lib`.
This file contains the compiled code extracted from the GCC library needed to run HarfBuzz.
You should link your project against it as well as against `libharfbuzz.a`.

An IAR project file `lib_harfbuzz.ewp` is provided for convenience.
This project merely launches `build.bat` when built.

## Adding support files

By default, HarfBuzz uses `malloc` to allocate memory.
This feature has been disabled in favor of a custom allocator.
You must compile `hb-alloc.c` in your project to use this allocator.

If you wish to use the default `malloc` allocator instead, you must build HarfBuzz without the preprocessor macro `HB_CUSTOM_MALLOC` (see the `build.*` scripts).
If you do so, you must not compile `hb-alloc.c` in your project.

# Requirements

* [Meson](https://mesonbuild.com/)
* [FreeType](https://freetype.org/) (also available [from the Developer Repository](https://forge.microej.com/ui/repos/tree/General/microej-developer-repository-release/com/microej/clibrary/thirdparty/freetype))
* [The GNU ARM toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

# Dependencies

_All dependencies are retrieved transitively by MicroEJ Module Manager_.

# Source

Fork of Harfbuzz 10.0.1.

# Restrictions

None.

---
_Copyright 2022-2025 MicroEJ Corp. This file has been modified and/or created by MicroEJ Corp._

_ This is part of HarfBuzz, a text shaping library._

_Permission is hereby granted, without written agreement and without_\
_license or royalty fees, to use, copy, modify, and distribute this_\
_software and its documentation for any purpose, provided that the_\
_above copyright notice and the following two paragraphs appear in_\
_all copies of this software._

_IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR_\
_DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES_\
_ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN_\
_IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH_\
_DAMAGE._

_THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,_\
_BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND_\
_FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS_\
_ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO_\
_PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS._
