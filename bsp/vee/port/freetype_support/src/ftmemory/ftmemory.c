/****************************************************************************
 *
 * ftmemory.c
 *
 *   ANSI-specific FreeType low-level memory interface (body).
 *   This file is a sub part of ftsystem.c
 *
 * Copyright 2021-2026 MicroEJ Corp. This file has been modified and/or created by MicroEJ Corp.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

  /**************************************************************************
   *
   * This file contains the default interface used by FreeType to access
   * low-level, i.e. memory management, i/o access as well as thread
   * synchronisation.  It can be replaced by user-specific routines if
   * necessary.
   *
   */


#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include <freetype/ftsystem.h>
#include <freetype/fttypes.h>

#include <BESTFIT_ALLOCATOR.h>

#include "vg_helper.h"
#include "vg_configuration.h"

  /**************************************************************************
   *
   *                      MEMORY MANAGEMENT INTERFACE
   *
   */

  /**************************************************************************
   *
   * It is not necessary to do any error checking for the
   * allocation-related functions.  This will be done by the higher level
   * routines like ft_mem_alloc() or ft_mem_realloc().
   *
   */

BESTFIT_ALLOCATOR freetypeAllocatorInstance;

static uint8_t _ft_heap[VG_FEATURE_FREETYPE_HEAP_SIZE];

#define FT_HEAP_START   ( &_ft_heap[0] )
#define FT_HEAP_END     ( &_ft_heap[VG_FEATURE_FREETYPE_HEAP_SIZE] )

#ifdef MICROVG_MONITOR_HEAP
static uint32_t current_heap_size = 0;
#endif

  /**************************************************************************
   *
   * @Function:
   *   ft_alloc
   *
   * @Description:
   *   The memory allocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   size ::
   *     The requested size in bytes.
   *
   * @Return:
   *   The address of newly allocated block.
   */
  FT_CALLBACK_DEF( void* )
  ft_alloc_mej( FT_Memory  memory,
            long       size )
  {
    FT_UNUSED( memory );

    void* ptr = BESTFIT_ALLOCATOR_allocate(&freetypeAllocatorInstance, size);

	#ifdef MICROVG_MONITOR_HEAP
    if(NULL != ptr) {
		current_heap_size += size;
		MEJ_LOG_INFO_MICROVG("FT Heap - alloc -> size= %d", current_heap_size);
    }
	#endif

    return ptr;
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_realloc
   *
   * @Description:
   *   The memory reallocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   cur_size ::
   *     The current size of the allocated memory block.
   *
   *   new_size ::
   *     The newly requested size in bytes.
   *
   *   block ::
   *     The current address of the block in memory.
   *
   * @Return:
   *   The address of the reallocated memory block.
   */
  FT_CALLBACK_DEF( void* )
  ft_realloc_mej( FT_Memory  memory,
              long       cur_size,
              long       new_size,
              void*      block )
  {
    FT_UNUSED( memory );

    void* _pNewBock = block;

    if ( new_size > cur_size ) {
        _pNewBock = BESTFIT_ALLOCATOR_allocate(&freetypeAllocatorInstance, new_size);

        if (_pNewBock != NULL) {
            memcpy(_pNewBock, block, cur_size);
        }

        BESTFIT_ALLOCATOR_free(&freetypeAllocatorInstance, block);
    }

	#ifdef MICROVG_MONITOR_HEAP
	if (NULL != _pNewBock) {
		current_heap_size -= cur_size;
		current_heap_size += new_size;

		MEJ_LOG_INFO_MICROVG("FT Heap - realloc -> size= %d", current_heap_size);
	}
	#endif

    return _pNewBock;
}

  /**************************************************************************
   *
   * @Function:
   *   ft_free
   *
   * @Description:
   *   The memory release function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   block ::
   *     The address of block in memory to be freed.
   */
  FT_CALLBACK_DEF( void )
  ft_free_mej( FT_Memory  memory,
           void*      block )
  {
    FT_UNUSED( memory );

	#ifdef MICROVG_MONITOR_HEAP
    uint32_t * ptr  = (uint32_t*) block;
	current_heap_size -= *(ptr -1) & 0x7FFFFFFF;
	MEJ_LOG_INFO_MICROVG("FT Heap - free -> size= %d", current_heap_size);
	#endif

    BESTFIT_ALLOCATOR_free(&freetypeAllocatorInstance, block);
  }

  /* documentation is in ftobjs.h */

  FT_BASE_DEF( FT_Memory )
  FT_New_Memory( void )
  {
    static struct FT_MemoryRec_  memory;

    memory.user    = 0;
    memory.alloc   = ft_alloc_mej;
    memory.realloc = ft_realloc_mej;
    memory.free    = ft_free_mej;

    BESTFIT_ALLOCATOR_new(&freetypeAllocatorInstance);
    BESTFIT_ALLOCATOR_initialize(&freetypeAllocatorInstance, (uint32_t) FT_HEAP_START, (uint32_t) FT_HEAP_END);

    return &memory;
  }

  /* documentation is in ftobjs.h */

  FT_BASE_DEF( void )
  FT_Done_Memory( FT_Memory  memory )
  {
  }

/* END */
