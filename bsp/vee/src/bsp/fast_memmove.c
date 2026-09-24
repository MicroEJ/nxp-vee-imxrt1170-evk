/*
 * Copyright 2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 *
 * Build: 7E4D1F7C
 */

/*
 * Fast, overlap-safe memmove() overriding the C library implementation.
 *
 * Rationale: the C library memmove() caps System.arraycopy() throughput on this target. newlib-nano
 * copies byte-by-byte; even the full newlib word-wide path is defeated by a one-byte misalignment
 * and is slower than the MCUXpresso SDK memcpy(). The SDK provides an optimized word-wide memcpy()
 * (fsl_memcpy.S) but does NOT override memmove(). This file fills that gap: forward-safe copies are
 * delegated to the fast memcpy(), and the overlapping backward case is handled here so the full
 * memmove() contract (unlike memcpy(), overlap is allowed) is preserved.
 *
 * The linker uses this strong definition instead of the C library one; "-z muldefs" is set in the
 * link flags so no multiple-definition error occurs even if the library member is also seen.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/** Number of bytes in a 32-bit word, the granularity of the fast copy path. */
#define BYTES_PER_WORD (4U)

/** Mask selecting the sub-word part of an address (non-zero when not word-aligned). */
#define WORD_ALIGN_MASK (BYTES_PER_WORD - 1U)

void *memmove(void *dst, const void *src, size_t n) {
	/* Nothing to do for a self-copy or an empty range. */
	if ((dst == src) || (0U == n)) {
		return dst;
	}

	/*
	 * A forward copy is safe whenever it never overwrites a source byte that is still to be read:
	 * that holds when the destination starts below the source, or when the two ranges do not
	 * overlap at all. Both cases are delegated to the fast (forward-only) memcpy().
	 */
	uintptr_t dst_addr = (uintptr_t)dst;
	uintptr_t src_addr = (uintptr_t)src;
	if ((dst_addr < src_addr) || (dst_addr >= (src_addr + n))) {
		return memcpy(dst, src, n);
	}

	/* Ranges overlap with the destination above the source: copy back to front. */
	uint8_t *d = (uint8_t *)dst + n;
	const uint8_t *s = (const uint8_t *)src + n;

	/* Align the destination tail down to a word boundary, one byte at a time. */
	while ((n > 0U) && (0U != ((uintptr_t)d & WORD_ALIGN_MASK))) {
		d--;
		s--;
		*d = *s;
		n--;
	}

	/* If the source is word-aligned too, copy a full word per iteration. */
	if (0U == ((uintptr_t)s & WORD_ALIGN_MASK)) {
		while (n >= BYTES_PER_WORD) {
			d -= BYTES_PER_WORD;
			s -= BYTES_PER_WORD;
			*(uint32_t *)d = *(const uint32_t *)s;
			n -= BYTES_PER_WORD;
		}
	}

	/* Copy any remaining bytes. */
	while (n > 0U) {
		d--;
		s--;
		*d = *s;
		n--;
	}

	return dst;
}
