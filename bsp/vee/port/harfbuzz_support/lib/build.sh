#!/bin/bash -xe
#
# Copyright 2024-2026 MicroEJ Corp. This file has been modified and/or created by MicroEJ Corp.
#
#    This is part of HarfBuzz, a text shaping library.
#
# Permission is hereby granted, without written agreement and without
# license or royalty fees, to use, copy, modify, and distribute this
# software and its documentation for any purpose, provided that the
# above copyright notice and the following two paragraphs appear in
# all copies of this software.
#
# IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
# ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
# IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
# THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
# BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
# ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

INSTALL_DIR="$PWD"
CROSS_FILE="$PWD/arm-armv5.txt"
FREETYPE_SUPPORT_DIR="../../freetype_support"

cd ../../thirdparty/harfbuzz
FREETYPE_DIR="../freetype"

meson setup --wipe --cross-file "$CROSS_FILE" \
    -Db_staticpic=false -Db_coverage=false -Dbuildtype=release \
    -Dcpp_args="-DHB_TINY -DHB_CUSTOM_MALLOC -static-libgcc -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fno-builtin -specs=nano.specs -fpermissive -fno-pie" \
    -Dwerror=false -Dtests=disabled -Dutilities=disabled -Ddefault_library=static -Dfreetype=enabled -Dfreetype_includes="$FREETYPE_DIR/include" \
    build

meson compile -C build lib

arm-none-eabi-ar x "$(arm-none-eabi-gcc -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -print-file-name=libgcc.a)" _popcountsi2.o

cp build/src/libharfbuzz.a _popcountsi2.o "$INSTALL_DIR/"
