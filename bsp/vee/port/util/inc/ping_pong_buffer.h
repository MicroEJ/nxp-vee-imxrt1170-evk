/*
 * C
 *
 * Copyright 2025 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file ping_pong_buffer.h
 * @author MicroEJ Dev Team
 * @brief A non blocking Ping Pong buffer implemented with FreeRTOS's atomic.
 * @date 2025-10-24
 *
 * A ping-pong buffer provides two interchangeable buffers used alternately as the "front" (read/consume) and "back"
 * (write/produce) buffer. The producer fills the back buffer while the consumer reads from the front buffer; once the
 * producer finishes a frame/segment the frame becomes available for the comsumer to acquire. This pattern enables
 * concurrent production and consumption with minimal contention.
 *
 * @warning This buffer is safe only for Single Producer Single Consummer usage!
 *
 * @note
 * Arguably, some functions could have returned an error if the internal state of the ping-pong buffers does not match
 * the expected state (for example, calling `pp_producer_commit()` without a previous successful call to
 * `pp_producer_try_acquire`). But given these errors would only arise from a misuse of the API and that operations here
 * are assumed to be performance oriented, these function returns void. It raises an assert internaly if the atomic
 * Compare-And-Swap (CAS) transition fails.
 *
 * @note
 * This buffer was develop with the goal of synchronising read/write between a CPU task and DMA. As DMA provides
 * interruptions on completion, further notification wasn't required. It could be improved by adding 'on_produced()' and
 * 'on_consumed' callback.
 */

#include "FreeRTOS.h"
#include "atomic.h"
#include <stdint.h>
#include <stddef.h>

#ifndef PING_PONG_BUFFER_H_
#define PING_PONG_BUFFER_H_

/**
 * @brief State of each buffer in the ping pong structure
 */
enum pp_state {
	PP_FREE, /**< Buffer ready to receive data */
	PP_FILLING, /**< Buffer acquired by producer */
	PP_READY, /**< Buffer ready to be consumed */
	PP_CONSUMING /**< Buffer being consumed */
};

/**
 * @brief Stateful wrapper around a raw memory buffer
 */
typedef struct {
	/** Pointer to an array of size `pp_t::capacity` bytes */
	uint8_t *data;
	/** Number of bytes available in `data`. This value is meaningful only when `state == PP_READY` */
	uint32_t len;
	/** One of pp_state. Used `uint32_t` for compatibility with FreeRTOS's `Atomic_CompareAndSwap_u32()` */
	uint32_t state;
} pp_buf_t;

/**
 * @brief Ping-pong buffer control structure.
 */
typedef struct {
	/** Holds two stateful buffers */
	pp_buf_t bufs[2];
	/** Capacity of each buffer */
	uint32_t capacity;
	/** Index of the producer. This index is read by `pp_producer_try_acquire()` and flipped by `pp_producer_commit()` */
	uint32_t prod_idx;
	/** Index of the consumer. This index is read by `pp_consumer_try_acquire()` and flipped by
	 * `pp_consumer_release()` */
	uint32_t cons_idx;
} pp_t;

/** Static initializer for two arrays of size `buffer_capacity` */
#define PP_INITIALIZER(buffer0, buffer1, buffer_capacity) { \
			.bufs[0].data = buffer0,                        \
			.bufs[1].data = buffer1,                        \
			.capacity = buffer_capacity,                    \
}

/** Static initializer for a contiguous array of size `2 * buffer_capacity` */
#define PP_CONTIGUOUS_INITIALIZER(double_buffer, buffer_capacity) \
		PP_INITIALIZER(&double_buffer[0], &double_buffer[buffer_capacity], buffer_capacity)

/**
 * @brief Resets the ping pong buffer to the free state
 *
 * @param pp buffer to reset
 */
void pp_reset(pp_t *pp);

/**
 * @brief Initialize ping pong buffer with the provided backing arrays
 *
 * Also reset the state of the buffer to empty.
 *
 * @param pp buffer to initialize
 * @param buf0 First backing array
 * @param buf1 Second backing array
 * @param capacity Capacity of each array
 */
void pp_init(pp_t *pp, uint8_t *buf0, uint8_t *buf1, uint32_t capacity);

/**
 * @brief Try to acquire a write buffer from the ping-pong buffer.
 *
 * This operation may fail if the next buffer to use as write buffer is not in the `PP_FREE` state. If the operation
 * succeed, the acquired buffer transitions to the `PP_FILLING` state.
 *
 * @param pp ping-pong buffer
 * @param[out] out_capacity Capacity of the obtained back buffer. Meaningful only if the function returned non `NULL`
 * @return Pointer to the write buffer
 * @retval NULL if no write buffer is available
 */
uint8_t *pp_producer_try_acquire(pp_t *pp, uint32_t *out_capacity);

/**
 * @brief Commit the previously acquired buffer back to the ping-pong buffer.
 *
 * The acquired buffer transitions from the `PP_FILLING` to the `PP_READY` state.
 *
 * @invariant pp_producer_try_acquire() must have succeeded before calling this function().
 *
 * @param pp ping-pong buffer
 * @param len number of bytes written in the acquired buffer
 */
void pp_producer_commit(pp_t *pp, uint32_t len);

/**
 * @brief Cancel a previous acquisition.
 *
 * Returns a previously acquired buffer to the `PP_FREE` state.
 *
 * @invariant pp_producer_try_acquire() must have succeeded before calling this function().
 *
 * @param pp ping-pong buffer
 */
void pp_producer_cancel(pp_t *pp);

/**
 * @brief Try to acquire a buffer ready for consuming
 *
 * This operation may fail if the next read buffer is not in the `PP_READY` state yet. If the operation succeed, the
 * buffer transitions to the `PP_CONSUMING` state.
 *
 * @param pp ping-pong buffer
 * @param out_len Number of bytes in the acquired buffer
 * @return Pointer to the read buffer
 * @retval NULL if no read buffer is available.
 */
uint8_t *pp_consumer_try_acquire(pp_t *pp, uint32_t *out_len);

/**
 * @brief Release the previously acquired consumer buffer.
 *
 * The consumed buffer transitions from the `PP_CONSUMING` to the `PP_FREE` state.
 *
 * @invariant pp_consumer_try_acquire() must have succeeded before calling this function().
 *
 * @param pp ping-pong buffer
 */
void pp_consumer_release(pp_t *pp);

/**
 * @brief Cancel a previous acquisition
 *
 * Return a previously acquired buffer to the `PP_READY` state.
 *
 * @invariant pp_consumer_try_acquire() must have succeeded before calling this function().
 *
 * @param pp ping-pong buffer
 */
void pp_consumer_cancel(pp_t *pp);

#endif /* PING_PONG_BUFFER_H_ */
