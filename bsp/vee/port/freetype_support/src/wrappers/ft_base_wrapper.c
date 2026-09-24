/*
 * Copyright 2022-2026 MicroEJ Corp. This file has been modified and/or created by MicroEJ Corp.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 */

/**
* @file
* @brief MicroEJ Freetype wrapper on freetype c files
* @author MicroEJ Developer Team
* @version 5.0.1
*/

#include "vg_configuration.h"
#if defined (VG_FEATURE_FONT)

#include "../../thirdparty/freetype/src/base/ftbase.c"
#include "../../thirdparty/freetype/src/base/ftinit.c"
#include "../../thirdparty/freetype/src/base/ftmm.c"
#include "../../thirdparty/freetype/src/autofit/autofit.c"
#include "../../thirdparty/freetype/src/pshinter/pshinter.c"
#include "../../thirdparty/freetype/src/sfnt/sfnt.c"

// do not use the default memory management
#define FT_New_Memory FT_New_Memory_Unused
#define FT_Done_Memory FT_Done_Memory_Unused
#include "../../thirdparty/freetype/src/base/ftsystem.c"
#undef FT_New_Memory
#undef FT_Done_Memory

// use a custom memory management
#include "../ftmemory/ftmemory.c"

#endif // defined (VG_FEATURE_FONT)
