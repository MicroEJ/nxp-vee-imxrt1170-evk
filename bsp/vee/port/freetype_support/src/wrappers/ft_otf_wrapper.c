/*
 * C
 *
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

#if defined VG_FEATURE_FREETYPE_OTF && (VG_FEATURE_FREETYPE_OTF == 1)
#include "../../thirdparty/freetype/src/cff/cff.c"
#include "../../thirdparty/freetype/src/psaux/psaux.c"
#include "../../thirdparty/freetype/src/psnames/psnames.c"
#endif // VG_FEATURE_FREETYPE_OTF

#endif // defined (VG_FEATURE_FONT)
