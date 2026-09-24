/*
 * Copyright 2022-2026 MicroEJ Corp. This file has been modified and/or created by MicroEJ Corp.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/**
 * @file
 * @brief MicroEJ Harfbuzz memory allocation functions.
 * @author MicroEJ Developer Team
 * @version 4.0.0
 */

#include <stdlib.h>
#include <string.h>

#include "BESTFIT_ALLOCATOR.h"

#include "ui_util.h"
#include "vg_helper.h"
#include "vg_configuration.h"

#if defined VG_FEATURE_FONT_COMPLEX_LAYOUT && (VG_FEATURE_FONT_COMPLEX_LAYOUT == 1)

#define HB_HEAP_SIZE VG_FEATURE_FONT_COMPLEX_LAYOUT_HEAP_SIZE

static BESTFIT_ALLOCATOR _allocator;

#define HB_HEAP_START ((uintptr_t)(void *)_hb_heap)
#define HB_HEAP_END (HB_HEAP_START + (size_t)HB_HEAP_SIZE)

#ifdef __cplusplus
extern "C"
{
#endif

static void _initialize(void) {
	static bool initialized = false;

	if (!initialized) {
		static uint8_t _hb_heap[HB_HEAP_SIZE];

		BESTFIT_ALLOCATOR_new(&_allocator);
		BESTFIT_ALLOCATOR_initialize(&_allocator, (uintptr_t)HB_HEAP_START, (uintptr_t)HB_HEAP_END);
		initialized = true;
	}
}

static inline size_t get_block_stored_size(void *block) {
	return (*(((uint32_t *)block) - 1u)) & (~(((uint32_t)1u) << 31));
}

static inline size_t get_block_content_size(void *block) {
	return get_block_stored_size(block) - 8u;
}

#ifdef MICROVG_MONITOR_HEAP
static uint32_t current_heap_size = 0u;
#endif

void *hb_malloc_impl(size_t size) {
	_initialize();

	void *ptr = BESTFIT_ALLOCATOR_allocate(&_allocator, size);

#ifdef MICROVG_MONITOR_HEAP
	if (NULL != ptr) {
		current_heap_size += get_block_stored_size(ptr);
		MEJ_LOG_INFO_MICROVG("HB Heap - alloc -> size= %d", current_heap_size);
	}
#endif
	return ptr;
}

void *hb_calloc_impl(size_t nmemb, size_t size) {
	size_t bytes = nmemb * size;
	void *ptr = hb_malloc_impl(bytes);
	(void)memset(ptr, 0, bytes);

	return ptr;
}

void hb_free_impl(void *block) {
	if (block != NULL) {
#ifdef MICROVG_MONITOR_HEAP
		current_heap_size -= get_block_stored_size(block);
		MEJ_LOG_INFO_MICROVG("HB Heap - free -> size= %d", current_heap_size);
#endif

		BESTFIT_ALLOCATOR_free(&_allocator, block);
	}
}

void *hb_realloc_impl(void *block, size_t size) {
	void *new_block = hb_malloc_impl(size);

	if (block != NULL) {
		if (new_block != NULL) {
			const size_t previous_size = get_block_content_size(block);
			(void)memcpy(new_block, block, MIN(size, previous_size));
		}

		hb_free_impl(block);
	}

	return new_block;
}

#endif // VG_FEATURE_FONT_COMPLEX_LAYOUT

#ifdef __cplusplus
}
#endif
