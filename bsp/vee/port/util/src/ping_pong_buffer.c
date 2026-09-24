/*
 * C
 *
 * Copyright 2025 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "ping_pong_buffer.h"
#include <assert.h>

#define UNUSED(x)                       ((void)(x))

void pp_reset(pp_t *pp) {
	assert(pp != NULL);

	pp->bufs[0].state = pp->bufs[1].state = PP_FREE;
	pp->prod_idx = pp->cons_idx = 0;
}

void pp_init(pp_t *pp, uint8_t *buf0, uint8_t *buf1, uint32_t capacity) {
	assert(pp != NULL);
	assert(buf0 != NULL);
	assert(buf1 != NULL);

	pp->bufs[0].data = buf0;
	pp->bufs[0].len = 0;

	pp->bufs[1].data = buf1;
	pp->bufs[1].len = 0;

	pp->capacity = capacity;

	pp_reset(pp);
}

uint8_t *pp_producer_try_acquire(pp_t *pp, uint32_t *out_capacity) {
	assert(pp != NULL);

	uint8_t *ret = NULL;

	if (Atomic_CompareAndSwap_u32(&pp->bufs[pp->prod_idx].state, PP_FILLING,
	                              PP_FREE) == ATOMIC_COMPARE_AND_SWAP_SUCCESS) {
		*out_capacity = pp->capacity;
		ret = pp->bufs[pp->prod_idx].data;
	}

	return ret;
}

void pp_producer_commit(pp_t *pp, uint32_t len) {
	assert(pp != NULL);
	assert(len <= pp->capacity);

	pp->bufs[pp->prod_idx].len = len;

	/* Compare-And-Swap FILLING -> READY; if it fails, caller misused API or race occurred. */
	uint32_t rc = Atomic_CompareAndSwap_u32(&pp->bufs[pp->prod_idx].state, PP_READY, PP_FILLING);
	assert(rc == ATOMIC_COMPARE_AND_SWAP_SUCCESS);
	UNUSED(rc);
	pp->prod_idx ^= 1;
}

void pp_producer_cancel(pp_t *pp) {
	assert(pp != NULL);

	uint32_t rc = Atomic_CompareAndSwap_u32(&pp->bufs[pp->prod_idx].state, PP_FREE, PP_FILLING);
	assert(rc == ATOMIC_COMPARE_AND_SWAP_SUCCESS);
	UNUSED(rc);
}

uint8_t *pp_consumer_try_acquire(pp_t *pp, uint32_t *out_len) {
	assert(pp != NULL);
	assert(out_len != NULL);

	uint8_t *ret = NULL;
	if (Atomic_CompareAndSwap_u32(&pp->bufs[pp->cons_idx].state, PP_CONSUMING,
	                              PP_READY) == ATOMIC_COMPARE_AND_SWAP_SUCCESS) {
		*out_len = pp->bufs[pp->cons_idx].len;
		ret = pp->bufs[pp->cons_idx].data;
	}

	return ret;
}

void pp_consumer_release(pp_t *pp) {
	assert(pp != NULL);

	/* Compare-And-Swap CONSUMING -> FREE; if it fails, caller misused API or race occurred. */
	uint32_t rc = Atomic_CompareAndSwap_u32(&pp->bufs[pp->cons_idx].state, PP_FREE, PP_CONSUMING);
	assert(rc == ATOMIC_COMPARE_AND_SWAP_SUCCESS);
	UNUSED(rc);
	pp->cons_idx ^= 1;
}

void pp_consumer_cancel(pp_t *pp) {
	assert(pp != NULL);

	/* Compare-And-Swap CONSUMING -> READY; if it fails, caller misused API or race occurred. */
	uint32_t rc = Atomic_CompareAndSwap_u32(&pp->bufs[pp->cons_idx].state, PP_READY, PP_CONSUMING);
	assert(rc == ATOMIC_COMPARE_AND_SWAP_SUCCESS);
	UNUSED(rc);
}
