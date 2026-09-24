/*
 * C
 *
 * Copyright 2019-2025 MicroEJ Corp.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VGLite. Provides a set of defines to configure the
 * implementation.
 *
 * Refer to the VEE Porting Guide > Graphics User Interface > C Module documentation to have more information about the
 * feature of this C module, how to use it and how to configure it.
 *
 * @author MicroEJ Developer Team
 */

#if !defined UI_VGLITE_CONFIGURATION_H
# define UI_VGLITE_CONFIGURATION_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "ui_configuration.h"

// -----------------------------------------------------------------------------
// Configuration Sanity Check
// -----------------------------------------------------------------------------

/*
 * @brief This workaround should not be defined in order to avoid issues related to the UI testsuite.
 */
#if (defined(VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1))
#error "This define must not be set."
#endif

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

/*
 * @brief Width of the Tesselation window
 *
 * @Warning: this impacts the VGLite allocation size
 */
#ifndef VGLITE_TESSELATION_WIDTH
#define VGLITE_TESSELATION_WIDTH    256
#endif

/*
 * @brief Height of the Tesselation window
 *
 * @Warning: this impacts the VGLite allocation size
 */
#ifndef VGLITE_TESSELATION_HEIGHT
#define VGLITE_TESSELATION_HEIGHT   256
#endif

/*
 * @brief Set this define to 1 to use the GPU to draw simple aliased drawings (line, rectangle etc.).
 *
 * By default this option is disabled and the software algorithms are used instead (because the software algorithms are
 * often faster than the GPU to draw these simple shapes).
 */
#ifndef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
#define VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS (0)
#endif

/*
 * @brief Set this define to 1 to use the GPU to draw a simple "draw image": when the image to render has the same pixel
 * definition than the destination buffer and no alpha blending is required.
 *
 * By default this option is disabled and the software algorithms are used instead (because the software algorithms are
 * often faster than the GPU for these use cases).
 */
#ifndef VGLITE_USE_GPU_FOR_RGB565_IMAGES
#define VGLITE_USE_GPU_FOR_RGB565_IMAGES (0)
#endif

/*
 * @brief Set this define to 1 to use the GPU to draw transparent images (with or without transformation like rotation
 * or scale).
 *
 * By default this option is disabled and the software algorithms are used instead because the GCNanoLite-V does not
 * support MSAA.
 */
#ifndef VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES
#define VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES (0)
#endif

/*
 * @brief Set this define to 1 to enable the use of the GPU (toggle) at runtime. When the option is disabled, the calls
 * to UI_VGLITE_xxx_hardware_rendering have no effect.
 *
 * By default this option is disabled (GPU is always used).
 */
#ifndef VGLITE_OPTION_TOGGLE_GPU
#define VGLITE_OPTION_TOGGLE_GPU (0)
#endif

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined UI_VGLITE_CONFIGURATION_H
