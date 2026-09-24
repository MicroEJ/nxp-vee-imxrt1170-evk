/**
 * Copyright 2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 *
 * @file veeport_configuration.h
 *
 * @brief Configures the C modules integrated for this port of MICROEJ VEE.
 *
 * This header file contains all the custom C modules configurations of the VEE Port.
 * Use this file to override the C module configurations in the VEE Port.
 * Please refer to the C module configuration files to learn about the available configurations.
 * This file should also be used to define VEE Port specific configurations.
 *
 * ***** HOW TO INTEGRATE THIS FILE WITH C MODULES *****
 *
 * This section explains how to integrate this file with C module configuration files.
 *
 * The configuration file of each C module (i.e. '[C MODULE NAME]_configuration.h') should include this
 * file as followed:
 *
 *  // Include VEE Port User configuration file
 *  #if defined __has_include
 *  	#if __has_include("veeport_configuration.h")
 *  		#include "veeport_configuration.h"
 *  	#endif // __has_include("veeport_configuration.h")
 *  #else
 *  // Ensure 'veeport_configuration.h' exists in your project for custom configurations.
 *  	#include "veeport_configuration.h"
 *  #endif // defined __has_include
 *
 * Then, in the '[C MODULE NAME]_configuration.h' file, default values for each parameter should be provided
 * as followed:
 *
 * #ifndef MY_CONFIGURATION
 * #define MY_CONFIGURATION (MY_CONFIGURATION_DEFAULT_VALUE)
 * #endif
 *
 */

#ifndef VEEPORT_CONFIGURATION_H
#define VEEPORT_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "display_configuration.h"
#include "fsl_debug_console.h"

// ############################################################################
//                        Essentials Configuration
// ############################################################################

/*
 * @brief Routes the VEE Port character output to the SDK debug console.
 * The C library is not retargeted on this BSP (SDK_DEBUGCONSOLE=1), so the default
 * implementation based on putchar() would discard the output.
 */
#define VEEPORT_PUTCHAR(...) ((void)PUTCHAR(__VA_ARGS__))

/*
 * @brief Routes the VEE Port logger output to the SDK debug console.
 * The trailing line return is appended here: log messages must not end with '\n'.
 */
#define VEEPORT_PRINTF(...) do { (void)PRINTF(__VA_ARGS__); (void)PRINTF("\n"); } while (0)

// ############################################################################
//                        Core Engine Configuration
// ############################################################################

// ############################################################################
//                        KF Configuration
// ############################################################################

// ############################################################################
//                        Event Queue Configuration
// ############################################################################

// ############################################################################
//                        FS Configuration
// ############################################################################

// ############################################################################
//                        NET Configuration
// ############################################################################

// ############################################################################
//                        UI Configuration
// ############################################################################

// -----------------------------------------------------------------------------
// MicroUI's Features Implementation
// -----------------------------------------------------------------------------

/**
 * @brief Standard "printf" indirection.
 */
#ifndef UI_DEBUG_PRINT
#define UI_DEBUG_PRINT (void)PRINTF
#endif

/*
 * @brief Defines the number of rectangles that collections can contain.
 */
#ifndef UI_RECT_COLLECTION_MAX_LENGTH
#define UI_RECT_COLLECTION_MAX_LENGTH (8u)
#endif

/**
 * @brief Uncomment this define to use the allocator "BESTFIT".
 *
 * When unset, the Graphics Engine uses its internal allocator (a best fit allocator that does not provide any functions
 * to analyse its content).
 *
 * The default graphics engine's allocator can be replaced by a 3rd-party allocator by implementing the functions
 * LLUI_DISPLAY_IMPL_imageHeap*().
 */
//#define UI_FEATURE_ALLOCATOR UI_FEATURE_ALLOCATOR_BESTFIT

/**
 * @brief When defined, the logger is enabled. The call to LLUI_INPUT_dump()
 * has no effect when the logger is disabled.
 *
 * By default the logger is not enabled.
 */
#define UI_FEATURE_EVENT_DECODER (0)

#if (1 == UI_FEATURE_EVENT_DECODER)

// header file created by MicroEJ Platform builder.
#include "microui_constants.h"

/**
 * @brief When defined, the MicroUI event decoder is able to decode the *input*
 * "Command" events. The define's value is the MicroUI Event Generator
 * "Command" fixed in the microui.xml file and used to build the MicroEJ Platform.
 * Most of time the MicroUI Event Generator "Command" is "MICROUI_EVENTGEN_COMMANDS":
 *
 *   #define UI_EVENTDECODER_EVENTGEN_COMMAND MICROUI_EVENTGEN_COMMANDS
 *
 * When not defined, the MicroUI event decoder does not try to decode the MicroUI
 * events "Command".
 */
#ifndef UI_EVENTDECODER_EVENTGEN_COMMAND
#define UI_EVENTDECODER_EVENTGEN_COMMAND MICROUI_EVENTGEN_COMMANDS
#endif

/**
 * @brief When defined, the MicroUI event decoder is able to decode the *input*
 * "Buttons" events. The define's value is the MicroUI Event Generator
 * "Buttons" fixed in the microui.xml file and used to build the MicroEJ Platform.
 * Most of time the MicroUI Event Generator "Buttons" is "MICROUI_EVENTGEN_BUTTONS":
 *
 *   #define UI_EVENTDECODER_EVENTGEN_BUTTONS MICROUI_EVENTGEN_BUTTONS
 *
 * When not defined, the MicroUI event decoder does not try to decode the MicroUI
 * events "Buttons".
 */
#ifndef UI_EVENTDECODER_EVENTGEN_BUTTONS
#define UI_EVENTDECODER_EVENTGEN_BUTTONS MICROUI_EVENTGEN_BUTTONS
#endif

/**
 * @brief When defined, the MicroUI event decoder is able to decode the *input*
 * "Touch" events. The define's value is the MicroUI Event Generator
 * "Touch" fixed in the microui.xml file and used to build the MicroEJ Platform.
 * Most of time the MicroUI Event Generator "Touch" is "MICROUI_EVENTGEN_TOUCH":
 *
 *   #define UI_EVENTDECODER_EVENTGEN_TOUCH MICROUI_EVENTGEN_TOUCH
 *
 * When not defined, the MicroUI event decoder does not try to decode the MicroUI
 * events "Touch".
 */
#ifndef UI_EVENTDECODER_EVENTGEN_TOUCH
#define UI_EVENTDECODER_EVENTGEN_TOUCH MICROUI_EVENTGEN_TOUCH
#endif

#endif // UI_FEATURE_EVENT_DECODER

/**
 * @brief Defines the display buffer refresh strategy (BRS) to use: one of the strategies above, the Graphics Engine's
 * default refresh strategy or a BSP's custom refresh strategy.
 *
 * When not set, the BSP has to implement the LLUI_DISPLAY_impl.h's API to define its custom
 * display buffer refresh strategy. If no strategy is set, the default Graphics Engine's strategy is
 * used that consists to only called LLUI_DISPLAY_IMPL_flush()(always full screen, no restore).
 */
#ifndef UI_FEATURE_BRS
#define UI_FEATURE_BRS (UI_FEATURE_BRS_PREDRAW)
#endif

/**
 * @brief Defines the available number of drawing buffers; in other words, the number of buffers the
 * Graphics Engine can use to draw into. According to the display BRS and the LCD connection, a drawing
 * buffer can be alternatively a back buffer (the buffer currently used by the Graphics Engine), a
 * front buffer (the buffer currently used by the LCD driver to map the LCD device), a transmission
 * buffer (the buffer currently used by the LCD driver to transmit the data to the LCD device) or a free
 * buffer (a buffer currently unused).
 *
 * Warning: This counter only defines the buffers the Graphics Engine can use and not the buffer reserved
 * to the LCD device (mapped or or not on the MCU address space).
 */
#ifndef UI_FEATURE_BRS_DRAWING_BUFFER_COUNT
#define UI_FEATURE_BRS_DRAWING_BUFFER_COUNT (FRAME_BUFFER_COUNT)
#endif

/**
 * @brief Defines the number of rectangles the strategy uses when calling LLUI_DISPLAY_IMPL_flush().
 * When set, the strategy sends only one rectangle that includes all dirty regions (a rectangle that
 * encapsulates all rectangles). When not set, the strategy sends all rectangles.
 *
 * When the implementation of LLUI_DISPLAY_IMPL_flush() only consists in swapping the back buffers, the
 * rectangles list is useless.
 *
 * When the implementation of LLUI_DISPLAY_IMPL_flush() consists in transmitting an unique portion of the
 * back buffer (for instance: the back buffer is transmitted to the LCD through a DSI bus), the single
 * rectangle mode is useful.
 *
 * When the implementation of LLUI_DISPLAY_IMPL_flush() consists in transmitting several portions of the back
 * buffer (for instance: the back buffer is transmitted to the LCD through a SPI bus), the rectangle list
 * is useful.
 *
 * By default, the rectangle list is given as-is (the option is not enabled).
 * @see the strategies' comments to have more information on the use of this option.
 */
//#define UI_FEATURE_BRS_FLUSH_SINGLE_RECTANGLE

/**
 * @brief Defines the number of supported destination formats. When not set or smaller than
 * "2", the file ui_drawing.c considers only one destination format is available: the same format as
 * the buffer of the display.
 * @see ui_drawing.c for more information.
 */
#ifndef UI_GC_SUPPORTED_FORMATS
#define UI_GC_SUPPORTED_FORMATS (2u)
#endif

/**
 * @brief When defined, in addition to the standard image formats (ARGB8888, A8, etc), the VEE Port can
 * support one or several custom formats.
 * @see ui_image_drawing.c for more information.
 */
//#define UI_FEATURE_IMAGE_CUSTOM_FORMATS

/**
 * @brief When defined, in addition to the graphics engine's internal font format, the VEE Port can
 * support one or several custom formats.
 * @see ui_font_drawing.c for more information.
 */
#define UI_FEATURE_FONT_CUSTOM_FORMATS (1)

// MicroUI with VGLITE

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
#define VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES (1)
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

// ############################################################################
//                        VG Configuration
// ############################################################################

/*
 * @brief Set this define to embed the implementation of the MicroVG's
 * Path (dynamic path creation and path rendering).
 *
 * This implementation holds the path's commands and paramters defined by the
 * application. The define value specifies how this data is stored (one or two arrays).
 *
 * When not set, a stub implementation is used. No error is thrown at runtime when
 * the application uses a path: the dynamic path are not created and the path
 * rendering is not performed.
 */
#ifndef VG_FEATURE_PATH
#define VG_FEATURE_PATH VG_FEATURE_PATH_SINGLE_ARRAY
#endif

/*
 * @brief Set this define to specify the implementation of the MicroVG's
 * LinearGradient (dynamic gradient creation and drawings with gradient).
 *
 * @see VG_FEATURE_GRADIENT_FULL
 * @see VG_FEATURE_GRADIENT_FIRST_COLOR
 *
 * When not set, a stub implementation is used. No error is thrown at runtime when
 * the application uses a gradient: the dynamic gradient are not created and the
 * gradient rendering is not performed.
 */
#ifndef VG_FEATURE_GRADIENT
#define VG_FEATURE_GRADIENT VG_FEATURE_GRADIENT_FULL
#endif

/*
 * @brief Set this define to specify the implementation of the MicroVG'
 * VectorFont (dynamic font loading and text rendering).
 *
 * @see VG_FEATURE_FONT_FREETYPE_VECTOR
 * @see VG_FEATURE_FONT_FREETYPE_BITMAP
 *
 * When not set, a stub implementation is used. The application cannot load a font
 * (and by consequence cannot draw a text).
 */
#ifndef VG_FEATURE_FONT
#define VG_FEATURE_FONT VG_FEATURE_FONT_FREETYPE_VECTOR
#endif

/*
 * @brief Uncomment this define to enable the support of TTF font files.
 *
 */
#define VG_FEATURE_FREETYPE_TTF (1)

/*
 * @brief Uncomment this define to enable the support of OTF font files.
 *
 */
#define VG_FEATURE_FREETYPE_OTF (1)

/*
 * @brief Uncomment this define to enable the support of colored emoji.
 *
 */
#define VG_FEATURE_FREETYPE_COLORED_EMOJI (1)

/*
 * @brief Uncomment this define to enable the support of complex layout.
 *
 * When set, the complex layout feature is disabled by default (the Freetype layout
 * manager is used). See functions MICROVG_HELPER_set_complex_layout() and
 * MICROVG_HELPER_has_complex_layouter().
 *
 * Note: the complex layout feature is managed by the Harfbuzz engine.
 *       Harfbuzz is used beside Freetype, thus the FT_CONFIG_OPTION_USE_HARFBUZZ define
 *       is not needed. This implementation has been chosen to ease the replacement of
 *       harfbuzz by an other complex layouter.
 */
#define VG_FEATURE_FONT_COMPLEX_LAYOUT (1)

/*
 * @brief Uncomment this define to allow to load external font files. When a font
 * file is not available in the application classpath, the implementation tries to
 * load it from an external resource system.
 *
 * The Platform must embbed the module "External Resource" and the BSP must implement
 * "LLEXT_RES_impl.h" header file.
 *
 * When not set, only the resources compiled with the application are used.
 */
//#define VG_FEATURE_FONT_EXTERNAL (1)

/*
 * @brief Configure this define to set the freetype heap size
 *
 * The freetype heap size depends on the font used by the application
 * @see MICROVG_MONITOR_HEAP in vg_helper.h to monitor the heap usage evolution.
 */
#ifndef VG_FEATURE_FREETYPE_HEAP_SIZE
#define VG_FEATURE_FREETYPE_HEAP_SIZE (80 * 1024)
#endif

/*
 * @brief Configure this define to set the complex layouter heap size
 *
 *@see VG_FEATURE_FONT_COMPLEX_LAYOUT
 *
 * The complex layouter heap size depends on the font used by the application
 * @see MICROVG_MONITOR_HEAP in vg_helper.h to monitor the heap usage evolution.
 */
#ifdef VG_FEATURE_FONT_COMPLEX_LAYOUT
#define VG_FEATURE_FONT_COMPLEX_LAYOUT_HEAP_SIZE (80 * 1024)
#endif

/*
 * @brief Set this define to enable the support of MicroVG BufferedVectorImage.
 * This feature requires the available number of supported GraphicsContext formats
 * is higher than 1.
 *
 * Comment the define VG_FEATURE_BUFFERED_VECTOR_IMAGE to remove the support of
 * BufferedVectorImage (even if the available number of supported GraphicsContext
 * formats is higher than 1).
 */
#if defined(UI_GC_SUPPORTED_FORMATS) && (UI_GC_SUPPORTED_FORMATS > 1)

/*
 * @brief Uncomment the define VG_FEATURE_BUFFERED_VECTOR_IMAGE to enable the support
 * of BufferedVectorImage.
 */
#define VG_FEATURE_BUFFERED_VECTOR_IMAGE (1)

/*
 * @brief The drawing functions to target the BufferedVectorImage have by default the
 * identifier 1.
 */
#ifndef UI_DRAWING_IDENTIFIER_BVI_FORMAT
#define UI_DRAWING_IDENTIFIER_BVI_FORMAT 1
#endif

#elif defined(VG_FEATURE_BUFFERED_VECTOR_IMAGE)
#error "The BufferedVectorImage feature requires the support of several Graphics Context formats".
#endif // if defined(UI_GC_SUPPORTED_FORMATS) && (UI_GC_SUPPORTED_FORMATS > 1)

#ifndef FT_CONFIG_MODULES_H
#define FT_CONFIG_MODULES_H <freetype/config/ftmodule.h>
#endif

/*
 * @brief FreeType is built as a library. The build system already defines this symbol on the command line
 * (see flags.cmake); the guard keeps this file usable when it does not.
 */
#ifndef FT2_BUILD_LIBRARY
#define FT2_BUILD_LIBRARY
#endif

// ----------------------------------------------------------------------------
// End
// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // VEEPORT_CONFIGURATION_H
