@REM Batch
@REM
@REM Copyright 2024-2026 MicroEJ Corp. This file has been modified and/or created by MicroEJ Corp.
@REM
@REM    This is part of HarfBuzz, a text shaping library.
@REM
@REM Permission is hereby granted, without written agreement and without
@REM license or royalty fees, to use, copy, modify, and distribute this
@REM software and its documentation for any purpose, provided that the
@REM above copyright notice and the following two paragraphs appear in
@REM all copies of this software.
@REM
@REM IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
@REM DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
@REM ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
@REM IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
@REM DAMAGE.
@REM
@REM THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
@REM BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
@REM FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
@REM ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
@REM PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

@SET INSTALL_DIR=%CD%
@SET CROSS_FILE=%CD%\arm-armv5.txt
@SET FREETYPE_SUPPORT_DIR=..\..\freetype_support

@CD ..\..\..\thirdparty\harfbuzz
@SET FREETYPE_DIR=..\freetype

meson setup --wipe --cross-file %CROSS_FILE% ^
    -Db_staticpic=false -Db_coverage=false -Dbuildtype=release ^
    -Dcpp_args="-DHB_TINY -DHB_CUSTOM_MALLOC -static-libgcc -mcpu=cortex-m33 -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -D__NEWLIB__ -Wall -fmessage-length=0 -ffunction-sections -fdata-sections -fno-builtin -specs=nano.specs -fpermissive -fno-pie" ^
    -Dwerror=false -Dtests=disabled -Dutilities=disabled -Ddefault_library=static -Dfreetype=enabled -Dfreetype_includes=%FREETYPE_DIR%\include ^
    build

@IF %ERRORLEVEL% NEQ 0 EXIT

meson compile -C build lib

FOR /F "delims=" %%I IN ('arm-none-eabi-gcc.exe -mcpu^=cortex-m33 -mfpu^=fpv5-sp-d16 -mfloat-abi^=hard -mthumb -print-file-name^="libgcc.a"') DO arm-none-eabi-ar x "%%I" _popcountsi2.o

COPY build\src\libharfbuzz.a %INSTALL_DIR%\
COPY _popcountsi2.o %INSTALL_DIR%\
