/*
 * C
 *
 * Copyright 2025 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file LLSERIAL_CONNECTION_impl.c
 * @brief This module implements the LLAPI for serial ports on NXP's MCUXpresso enabled boards.
 *
 * LLSERIAL is implemented on top of NXP's `fsl_adapter_uart.h`. This API provides sufficient abstraction to cover all
 * UART based communication without the overhead and extra buffering from `fsl_component_serial_manager.h`.
 *
 * The implementation follows the following principles:
 *   - Suspend the calling thread to let the Core Engine run other threads during potentially long IO operations.
 *   - The Core Engine may move managed memory when not executing native code, thus a native memory buffer is required.
 *   - RX and TX operations are split over multiple copy operations. Transfering the data to and from managed memory in
 *   multiple increments minimizes the buffer size requirement and to use static memory.
 *   - Use DMA transfer to interract with IO data registers.
 *
 * Send operation uses ping-pong buffering. The CPU writes into one active buffer while the DMA reads from the inactive
 * buffer. The buffers are swapped when operation is finished.
 *
 * Receive operation copies data from a circular buffer managed by `fsl_adapter_uart`. Data is continuously received by
 * the DMA into this buffer. Notifications on key events (transfer half-complete, complete, and idle line) signal
 * available data in a timely maner. The size of the ring buffer must be set to in accordance with the latency to wake
 * up the managed receive thread.
 *
 * A handle to a serial connection (`connectionId` in the Java implementation) is composed of two elements:
 *   - An index in the array of the managed UART ports,
 *   - An id from a singleton counter, incremented after every open operation.
 * This provides some added security against reusing a closed handle from the Java implementation.
 *
 * @note
 * Although the implementation targets the abstract fsl_adapter_uart interface, some of the initialization is specific
 * to LPUART peripherals.
 */

// MicroEJ
#include "LLSERIAL_CONNECTION_config.h"
#include "LLSERIAL_CONNECTION_impl.h"
#include "sni.h"
#include "sni_extras.h"
#include "ping_pong_buffer.h"

// NXP
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_adapter_uart.h"

// Standards
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#if ENABLE_LOG_ERROR
#define LOG_ERROR(fmt, ...)             PRINTF("ERROR %s:%d " fmt "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#if ENABLE_LOG_INFO
#define LOG_INFO(fmt, ...)              PRINTF("INFO %s:%d " fmt "\n", __FILE__, __LINE__, ## __VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

/** LLAPI doesn't define error codes for exception handling. Define a unspecified error code to avoid using magic
 * numbers */
#define UNDEFINED_EXCEPTION_ERROR       (-1)

#define LPUART_MAX_SUPPORTED_BAUDRATE   (20 * 1000 * 1000) // 20 Mbps, according to datasheet for IMXRT1170BCEC.

#define CONN_TO_CONNECTION_ID(conn)     ((((conn) - &conn_pool[0]) << 16) | (conn)->id)

enum llserial_conn_state {
	LLSERIAL_CONN_STATE_CLOSED,
	LLSERIAL_CONN_STATE_OPEN,
	LLSERIAL_CONN_STATE_CONFIGURED
};

/**
 * @brief State of a serial port
 */
/** Id part of the connection handle. This represents the id of the connection the owns this port */
struct llserial_conn {
	uint16_t id;
	/** String to the device's name (e.g. "lpuart2") */
	const char *device;

	/* State of the port, open meaning that the port is busy, has already been associated with a handle and a Java
	 * object. */
	enum llserial_conn_state state;

	/** Id of a suspended thread waiting to receive data. SNI_ERROR if no suspended thread. */
	volatile int32_t read_thread_id;
	/** Id of a suspended thread waiting to send data. SNI_ERROR if no suspended thread. */
	volatile int32_t write_thread_id;

	/** ping-pong buffer used to send data */
	pp_t write_pp;
	/** Number of bytes remaining to write in the ping pong buffer to complete the send operation. A positive value
	 * signals that an operation is ongoing. A negative value can be set if an error is reported from the transfer
	 * callback. */
	volatile int32_t write_rem;
	/** number of bytes remaining to send out of the ping pong buffer to complete the send operation. */
	volatile int32_t send_rem;

	/** Intermediate buffer between managed memory and the ring buffer in fsl_adapter_uart. */
	uint8_t read_buf[RX_BUF_SIZE];

	/** UART Handle. Using an indirect reference to uart_handle_mem let us record the initialization status of the
	 * handle, i.e. a non-NULL handle signals that the handle may be deinitialized. */
	hal_uart_handle_t uart_handle;
	/** backing array for the UART handle */
	UART_HANDLE_DEFINE(uart_handle_mem);
	/** UART configuration */
	hal_uart_config_t *uart_config;

	/** DMA handle. The same separation principle explained in uart_handle applies here. */
	hal_uart_dma_handle_t dma_handle;
	/** backing array for the DMA handle */
	UART_DMA_HANDLE_DEFINE(dma_handle_mem);
	/** DMA configuration */
	hal_uart_dma_config_t *dma_config;
};

/**
 * @brief Static initializer for `hal_uart_config_t`
 * @param uart_inst Instance number among LPUART peripherals ("LPUART2" -> 2)
 */
#define HAL_UART_CONFIG_DEFAULT_INIT(uart_inst) {  \
			.baudRate_Bps = 115200,                \
			.parityMode = kHAL_UartParityDisabled, \
			.stopBitCount = kHAL_UartOneStopBit,   \
			.enableRx = 1,                         \
			.enableTx = 1,                         \
			.enableRxRTS = 0,                      \
			.enableTxCTS = 0,                      \
			.instance = uart_inst,                 \
}

/**
 * @brief Static initializer for `hal_uart_dma_config_t`
 * @param uart_inst Instance number among LPUART peripherals ("LPUART2" -> 2)
 * @param dma_inst Instance number among (e)DMA peripherals. (Is it consistent with DMAMUX index?)
 * @param rx_chan Channel number in the DMA instance assigned to RX
 * @param tx_chan Channel number in the DMA instance assigned to TX
 */
#define HAL_UART_DMA_CONFIG_DEFAULT_INIT(uart_inst, dma_inst, rx_chan, tx_chan) { \
			.uart_instance = uart_inst,                                           \
			.dma_instance = dma_inst,                                             \
			.rx_channel = rx_chan,                                                \
			.tx_channel = tx_chan,                                                \
			.dma_mux_configure = &(dma_mux_configure_t) {                         \
				.dma_dmamux_configure = {                                         \
					.dma_mux_instance = dma_inst,                                 \
					.rx_request = kDmaRequestMuxLPUART ## uart_inst ## Rx,        \
					.tx_request = kDmaRequestMuxLPUART ## uart_inst ## Tx,        \
				} },                                                              \
}

/**
 * @brief Default static initializer for `struct llserial_conn`
 * @param inst Instance number among LPUART peripherals ("LPUART2" -> 2)
 * @param config_idx Index of the various configurations to load. Index must be consistent accross {`uart_configs`,
 *`dma_configs` and `pp_send_arrays`}.
 */
#define CONN_DEFAUT_INIT(inst, config_idx)                                                  \
		{                                                                                   \
			.id = 0xFFFF,                                                                   \
			.device = "lpuart" #inst,                                                       \
			.state = LLSERIAL_CONN_STATE_CLOSED,                                            \
			.read_thread_id = SNI_ERROR,                                                    \
			.write_thread_id = SNI_ERROR,                                                   \
			.write_pp = PP_CONTIGUOUS_INITIALIZER(pp_send_arrays[config_idx], TX_BUF_SIZE), \
			.uart_config = &uart_configs[config_idx],                                       \
			.dma_config = &dma_configs[config_idx],                                         \
		}

/**
 * @brief Array containing the configuration for every serial port. The last used configuration is reused when a port is
 * reopened
 */
static hal_uart_config_t uart_configs[] = {
	HAL_UART_CONFIG_DEFAULT_INIT(2),
#if (defined(ENABLE_LPUART7) && ENABLE_LPUART7)
	HAL_UART_CONFIG_DEFAULT_INIT(7),
#endif
};

/**
 * @brief Array containing the configuration of the DMA for every serial port.
 *
 */
static hal_uart_dma_config_t dma_configs[] = {
	HAL_UART_DMA_CONFIG_DEFAULT_INIT(2, 0, 0, 1),
#if (defined(ENABLE_LPUART7) && ENABLE_LPUART7)
	HAL_UART_DMA_CONFIG_DEFAULT_INIT(7, 0, 2, 3),
#endif
};

/**
 * @brief Backing arrays for the send ping pong buffer of each port.
 *
 * Notice that the array size is twice the TX_BUF_SIZE (one half goes to the active buffer and the other half to the
 * inactive buffer).
 */
AT_NONCACHEABLE_SECTION(
	static uint8_t pp_send_arrays[ARRAY_SIZE(uart_configs)][2 * TX_BUF_SIZE]);

/**
 * @brief Array containing the state of every managed serial port.
 *
 */
static struct llserial_conn conn_pool[] = {
	CONN_DEFAUT_INIT(2, 0),
#if (defined(ENABLE_LPUART7) && ENABLE_LPUART7)
	CONN_DEFAUT_INIT(7, 1),
#endif
	/* configure other instances by expanding this list */
};

/**
 * @brief Singleton counter used to assign unique ids to serial connection handles.
 */
static volatile uint16_t id_cnt = 0;

/**
 * @brief Map a parity setting from LLAPI to fsl_adapter_uart's domain.
 *
 * @param llserial_parity Parity setting in the LLAPI domain.
 * @return One of hal_uart_parity_mode_t or -1 on error
 */
static int llserial_map_parity(int llserial_parity) {
	int ret;

	switch (llserial_parity) {
	case LLSERIAL_CONNECTION_PARITY_NONE:
		ret = kHAL_UartParityDisabled;
		break;
	case LLSERIAL_CONNECTION_PARITY_ODD:
		ret = kHAL_UartParityOdd;
		break;
	case LLSERIAL_CONNECTION_PARITY_EVEN:
		ret = kHAL_UartParityEven;
		break;
	default:
		/* this should never happen as the Java implementation is responsible for filtering illegal arguments */
		assert(0);
		ret = -1;
		break;
	}

	return ret;
}

/**
 * @brief Map a number of stop bit setting from LLAPI's to fsl_adapter_uart's domain
 *
 * @param llserial_stopbits Stop bit setting in the LLAPI domain
 * @return One of hal_uart_stop_bit_count_t or -1 on error
 */
static int llserial_map_stopbits(int llserial_stopbits) {
	int ret;

	switch (llserial_stopbits) {
	case LLSERIAL_CONNECTION_STOPBITS_1:
		ret = kHAL_UartOneStopBit;
		break;
	case LLSERIAL_CONNECTION_STOPBITS_2:
		ret = kHAL_UartTwoStopBit;
		break;
	default:
		/* this should never happen as the Java implementation is responsible for filtering illegal arguments */
		assert(0);
	/* intentional fallthrough, no break */
	case LLSERIAL_CONNECTION_STOPBITS_1_5:
		/* unsupported stop bit configuration */
		ret = -1;
		break;
	}

	return ret;
}

static struct llserial_conn *throwIfNotOpen(struct llserial_conn *conn) {
	struct llserial_conn *result = NULL;
	if (conn) {
		if (conn->state >= LLSERIAL_CONN_STATE_OPEN) {
			result = conn;
		} else {
			LOG_ERROR("connection_id: %d is not open", CONN_TO_CONNECTION_ID(conn));
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_CONNECTION_CLOSED);
		}
	} else {
		/* Not being able to get a conn structure from a connection id result from a mishandling of the LLAPI from the
		 * Java implementation. Throw a runtime exception that will crash the VM */
		THROW_RUNTIME_EXCEPTION(UNDEFINED_EXCEPTION_ERROR, LLSERIAL_CONNECTION_CONNECTION_CLOSED);
	}

	return result;
}

static struct llserial_conn *throwIfNotConfigured(struct llserial_conn *conn) {
	struct llserial_conn *result = NULL;

	if (throwIfNotOpen(conn) != NULL) {
		if (conn->state == LLSERIAL_CONN_STATE_CONFIGURED) {
			result = conn;
		} else {
			LOG_ERROR("connection_id: %d is not configured", CONN_TO_CONNECTION_ID(conn));
			/* Using an unconfigured connection results from mishandling the LLAPI from the Java implementation. Java
			 * constructors MUST configure the serial port. Throw a runtime exception that will crash the VM */
			THROW_RUNTIME_EXCEPTION(UNDEFINED_EXCEPTION_ERROR, LLSERIAL_CONNECTION_INTERNAL_ERROR);
		}
	} else {
		// exception thrown by throwIfNotOpen
	}

	return result;
}

/**
 * @brief Get a llserial_conn pointer from a handle
 *
 * @param handle
 * @return A pointer to `struct llserial_conn` or NULL if the handle doesn't exist.
 */
static struct llserial_conn *llserial_conn_get(int32_t connection_id) {
	struct llserial_conn *ptr = NULL;
	uint16_t id = connection_id & 0xFFFF;
	int index = connection_id >> 16;

	if (index >= 0 && index < ARRAY_SIZE(conn_pool)) {
		if ((conn_pool[index].id == id)) {
			ptr = &conn_pool[index];
		} else {
			LOG_ERROR("connection_id: %d unexpected id %d, expected %d", connection_id, id, conn_pool[index].id);
		}
	} else {
		LOG_ERROR("connection_id: %d has illegal pool index:%d", connection_id, index);
	}

	return ptr;
}

/**
 * @brief Return the connection index mapping to a port's name
 *
 * @param port_name
 * @return The index of the matching connection in the pool, or -1 if no match
 */
static int find_index_by_port_name(const char *port_name) {
	int idx = -1;

	for (int i = 0; i < ARRAY_SIZE(conn_pool); i++) {
		if (strcmp(conn_pool[i].device, port_name) == 0) {
			idx = i;
			break;
		}
	}

	return idx;
}

/**
 * @brief Try starting a DMA transfer to send data in the write ping pong buffer
 *
 * @param conn Connection context from which buffered data is to be sent.
 *
 * @return int The status of the operation.
 * @retval 0 If transfer was successfully started
 * @retval -1 If no buffer is ready to be consumed
 * @retval >0 One of hal_uart_dma_status_t if starting the DMA transfer failed
 */
static int try_sending(struct llserial_conn *conn) {
	assert(conn);

	int ret = -1;
	uint32_t len;
	uint8_t *buf = pp_consumer_try_acquire(&conn->write_pp, &len);

	if (buf) {
		hal_uart_dma_status_t rc = HAL_UartDMATransferSend(conn->uart_handle, buf, len);
		if (rc == kStatus_HAL_UartDmaSuccess) {
			// success
			ret = 0;
		} else {
			// cancel acquisition if DMA transfer failed to start
			pp_consumer_cancel(&conn->write_pp);
			ret = rc;
		}
	}

	return ret;
}

/**
 * @brief a `hal_uart_dma_transfer_callback_t` called from fsl_adapter_uart to signal events
 *
 * @param handle Handle on which the event is notified
 * @param msg Notification
 * @param callbackParam a pointer to `struct llserial_conn` configured while opening the port.
 */
static void transfer_callback(hal_uart_dma_handle_t handle, hal_dma_callback_msg_t *msg, void *callbackParam) {
	assert(msg);

	struct llserial_conn *conn = (struct llserial_conn *)callbackParam;

	switch (msg->status) {
	case kStatus_HAL_UartDmaRxIdle:
	case kStatus_HAL_UartDmaIdleline:
	{
		if (conn->read_thread_id != SNI_ERROR) {
			int32_t rc = SNI_resumeJavaThreadWithArg(conn->read_thread_id, (void *)msg->dataSize);
			assert(rc == SNI_OK);
			UNUSED(rc);
		}

		break;
	}
	case kStatus_HAL_UartDmaTxIdle:
	{
		int rc;
		conn->send_rem -= msg->dataSize;
		pp_consumer_release(&conn->write_pp);

		rc = try_sending(conn);
		if (rc > 0) {
			/* try_sending returned a positive value which means that there was an IO error. The error is signaled to
			 * the managed thread by passing a negative value in conn->write_rem. */
			conn->write_rem = -1;
		} else if (rc < 0) {
			/* There was no data ready to send, this can happen if the java tx thread is lagging but it's NOT an
			 * error. The native function will try to start the DMA anyway. */
		} else {
			/* buffer acquired and transfer started */
		}

		/* Is there never a case receiving TxIdle with unset thread_id? write_thread_id is unset
		 * - after an error is reported from the transfer callback
		 * - when the javathread is resumed after the connection is closed
		 * - after a successful transfer
		 * we should always have a thread to notify */
		assert(conn->write_thread_id != SNI_ERROR);
		rc = SNI_resumeJavaThread(conn->write_thread_id);
		// Failing to resume the thread would always be a programming error.
		assert(rc == SNI_OK);

		break;
	}
	default:
	{
		/* fsl_adapter_uart has mixed use for `hal_uart_dma_status_t`, where sometimes it's return type and sometimes a
		 * callback type. For example, a callback message with `kStatus_HAL_UartDmaRxBusy` wouldn't make sense here: the
		 * DMA returns busy when we request the transfer, it doesn't become busy later. Above are all the cases used in
		 * callbacks by lpuart, use an assert() if we catch the default case in case the implementation changes or if
		 * it's different for peripherals other than LPUART. */
		assert(0);
		break;
	}
	}
}

static void llserial_conn_getdescription(int32_t connection_id, char *buffer, uint32_t bufferLength) {
	struct llserial_conn *conn = llserial_conn_get(connection_id);
	if (conn != NULL) {
		snprintf(buffer, bufferLength, "handle: %" PRId32 ", %s, state: %d", connection_id, conn->device, conn->state);
	} else {
		snprintf(buffer, bufferLength, "handle: %" PRId32 " invalid", connection_id);
	}
}

/**
 * @brief Deinitialize safely the resources of a port.
 *
 * @param conn Port to be released.
 */
static void deinitialize_conn(struct llserial_conn *conn) {
	if (conn->dma_handle) {
		hal_uart_dma_status_t rc = HAL_UartDMADeinit(conn->uart_handle);
		assert(rc == kStatus_HAL_UartDmaSuccess);
		UNUSED(rc);
		conn->dma_handle = NULL;
	}

	if (conn->uart_handle) {
		hal_uart_status_t rc = HAL_UartDeinit(conn->uart_handle);
		assert(rc == kStatus_HAL_UartSuccess);
		UNUSED(rc);
		conn->uart_handle = NULL;
	}

	conn->state = LLSERIAL_CONN_STATE_OPEN;

	if (conn->read_thread_id != SNI_ERROR) {
		/* negative number to indicate closed connection */
		SNI_resumeJavaThreadWithArg(conn->read_thread_id, (void *)-1);
	}

	if (conn->write_thread_id != SNI_ERROR) {
		SNI_resumeJavaThread(conn->write_thread_id);
	}
}

/**
 * @brief Initialize a connection
 *
 * Setup a connection using the last known setting used by the port.
 *
 * @param conn
 * @throw NativeIOException on failure
 */
static void initialize_conn(struct llserial_conn *conn) {
	assert(conn);
	assert(conn->uart_handle == NULL);
	assert(conn->dma_handle == NULL);

	{
		/* initialize root clock frequency at runtime */
		clock_root_t clock_root = kCLOCK_Root_Lpuart1 - 1 + conn->uart_config->instance;
		conn->uart_config->srcClock_Hz = CLOCK_GetRootClockFreq(clock_root);
		assert(conn->uart_config->srcClock_Hz > 0);
		if (conn->uart_config->srcClock_Hz <= 0) {
			LOG_ERROR("failed to get %s source clock frequency", conn->device);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_INTERNAL_ERROR);
			goto end;
		}
	}

	{
		hal_uart_status_t rc = HAL_UartInit(conn->uart_handle_mem, conn->uart_config);
		if (rc != kStatus_HAL_UartSuccess) {
			LOG_ERROR("HAL_UartInit() failed with error %d", rc);
			if (rc == kStatus_HAL_UartBaudrateNotSupport) {
				THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_UNSUPPORTED_CONFIG, LLSERIAL_CONNECTION_CONFIGURATION_ERROR);
			} else {
				/* some other internal error happened */
				THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_INTERNAL_ERROR);
			}

			/* nothing was initialized yet */
			goto end;
		}
		conn->uart_handle = conn->uart_handle_mem;
	}

	{
		hal_uart_dma_status_t rc = HAL_UartDMAInit(conn->uart_handle_mem, conn->dma_handle_mem, conn->dma_config);
		if (rc != kStatus_HAL_UartDmaSuccess) {
			LOG_ERROR("HAL_UartDMAInit() failed with error %d", rc);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_INTERNAL_ERROR);
			goto cleanup;
		}
		conn->dma_handle = conn->dma_handle_mem;
	}

	{
		hal_uart_dma_status_t rc = HAL_UartDMATransferInstallCallback(conn->uart_handle, transfer_callback, conn);
		if (rc != kStatus_HAL_UartDmaSuccess) {
			LOG_ERROR("HAL_UartDMATransferInstallCallback() failed with error %d", rc);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_INTERNAL_ERROR);
			goto cleanup;
		}
	}

	conn->state = LLSERIAL_CONN_STATE_CONFIGURED;
	goto end;

cleanup:
	deinitialize_conn(conn);

end:
	return;
}

/**
 * @brief Open a connection by its index in `conn_pool`
 *
 * @param idx Index of the connection to be opened
 * @throw NativeIOException on failure
 * @return int32_t A positive handle on success, -1 on failure
 */
static int32_t open_conn_by_index(int idx) {
	int32_t connection_id = -1;

	if (idx >= 0) {
		struct llserial_conn *conn = &conn_pool[idx];
		if (conn->state == LLSERIAL_CONN_STATE_CLOSED) {
			// allocate a new handle number for this serial connection
			conn->id = ++id_cnt;
			connection_id = (idx << 16) | (conn->id);

			conn->state = LLSERIAL_CONN_STATE_OPEN;

			SNI_registerResource((void *)connection_id, (SNI_closeFunction)LLSERIAL_CONNECTION_IMPL_close,
			                     (SNI_getDescriptionFunction)llserial_conn_getdescription);
		} else {
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_PORT_ALREADY_USED);
		}
	} else {
		THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_INVALID_PORT);
	}

	return connection_id;
}

/**
 * @brief Start receiving up to len bytes on conn and store it in its internal buffer
 *
 * @param conn
 * @param len
 * @return 0 on success, -1 on failure
 */
static int start_recv_to_rx_buffer(struct llserial_conn *conn, uint32_t len) {
	int ret = 0;
	uint32_t read_len = MIN(RX_BUF_SIZE, len);

	hal_uart_dma_status_t status = HAL_UartDMATransferReceive(conn->uart_handle, conn->read_buf, read_len,
	                                                          false);
	if (status != kStatus_HAL_UartDmaSuccess) {
		LOG_ERROR("HAL_UartDMATransferReceive() returned %d", __func__, status);
		ret = -1;
	}

	return ret;
}

static int32_t LLSERIAL_CONNECTION_IMPL_read_cb(int32_t connection_id, uint8_t *b, int32_t off, int32_t len) {
	struct llserial_conn *conn;
	int32_t returned_size = SNI_IGNORED_RETURNED_VALUE;
	int rc;

	rc = SNI_getCallbackArgs((void **)&conn, (void **)&returned_size);
	assert(rc == SNI_OK);
	UNUSED(rc);
	assert(conn != NULL);

	if (returned_size > 0) {
		memcpy(&b[off], conn->read_buf, returned_size);
	} else {
		/* if the returned size is not more than zero, this indicates the connection has been closed */
		LOG_ERROR(
			"connection id %" PRId32 " was closed while thread %" PRId32 " was suspended waiting to receive bytes",
			connection_id,
			conn->read_thread_id);
		THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_CONNECTION_CLOSED);
	}

	/* Reset the suspended thread id */
	conn->read_thread_id = SNI_ERROR;

	return returned_size;
}

static int32_t LLSERIAL_CONNECTION_IMPL_read_unguarded(int32_t connection_id, uint8_t *b, int32_t off, int32_t len) {
	struct llserial_conn *conn = NULL;

	conn = throwIfNotConfigured(llserial_conn_get(connection_id));

	if (conn) {
		/* At this stage, there cannot be a suspended thread. This implementation is not thread-safe. If the port is
		 * read by multiple threads, only the last read request will be honored. In single thread use case, we normally
		 * cannot have a pending read request at this point. */
		assert(conn->read_thread_id == SNI_ERROR);
		conn->read_thread_id = SNI_getCurrentJavaThreadID();
		assert(conn->read_thread_id != SNI_ERROR);

		if (start_recv_to_rx_buffer(conn, len) < 0) {
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_RX_ERROR);
		} else {
			LOG_INFO("suspending thread %d", conn->read_thread_id);
			int32_t rc = SNI_suspendCurrentJavaThreadWithCallback(0, (SNI_callback)LLSERIAL_CONNECTION_IMPL_read_cb,
			                                                      conn);
			assert(rc == SNI_OK);
			UNUSED(rc);
		}
	}

	return SNI_IGNORED_RETURNED_VALUE;
}

/**
 * @brief Put as many bytes as possible in the send ping-pong buffer.
 *
 * @param pp ping-pong buffer to be filled
 * @param src Data to copy
 * @param len Number of bytes to copy
 * @return size_t Number of bytes copied into the ping-pong buffer
 */
static size_t try_fillup_pp(pp_t *pp, uint8_t *src, uint32_t len) {
	size_t copied_len = 0;
	while (copied_len < len) {
		uint32_t iteration_len;
		uint32_t capacity;
		uint8_t *buf = pp_producer_try_acquire(pp, &capacity);
		if (buf == NULL) {
			break;
		}

		iteration_len = MIN(capacity, len - copied_len);

		memcpy(buf, &src[copied_len], iteration_len);
		copied_len += iteration_len;

		pp_producer_commit(pp, iteration_len);
	}

	return copied_len;
}

/**
 * @brief Callback for the write API when the writing thread is resumed
 *
 * @param connection_id
 * @param b
 * @param off
 * @param len
 */
static void LLSERIAL_CONNECTION_IMPL_write_cb(int32_t connection_id, uint8_t *b, int32_t off, int32_t len) {
	struct llserial_conn *conn = NULL;
	bool ok = true;

	conn = throwIfNotConfigured(llserial_conn_get(connection_id));
	if (conn == NULL) {
		LOG_ERROR("%s(%d) called but port is closed", __func__, connection_id);
		ok = false;
	}

	if (ok) {
		if (conn->write_rem < 0) {
			LOG_ERROR("%s(%" PRId32 ") non blocking write reported error: %d", __func__, connection_id,
			          conn->write_rem);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_TX_ERROR);
			ok = false;
		} else if (conn->write_rem > 0) {
			size_t filled_len = try_fillup_pp(&conn->write_pp, &b[off + len - conn->write_rem], conn->write_rem);
			conn->write_rem -= filled_len;
		} else {
			// no more data to push in buffer
		}
	}

	if (ok) {
		if (conn->send_rem > 0) {
			int32_t status;
			// return value is unchecked, it's possible the UART is already sending something
			(void)try_sending(conn);
			status = SNI_suspendCurrentJavaThreadWithCallback(0, (SNI_callback)LLSERIAL_CONNECTION_IMPL_write_cb, NULL);
			/* If we fail to suspend the current thread, an exception has been thrown, which is a programming error */
			assert(status == SNI_OK);
			UNUSED(status);
		} else {
			/* clear the thread id once the transmission is over */
			conn->write_thread_id = SNI_ERROR;
		}
	}

	if (!ok && conn) {
		// on error, reset value to allow further transfer
		conn->write_rem = 0;
		conn->write_thread_id = SNI_ERROR;
	}
}

/**
 * @brief Same as LLSERIAL_CONNECTION_IMPL_write without checks for illegal arguments
 */
static void LLSERIAL_CONNECTION_IMPL_write_unguarded(int32_t connection_id, uint8_t *b, int32_t off, int32_t len) {
	bool ok = true;
	struct llserial_conn *conn = NULL;

	assert(b);
	assert(off >= 0);
	assert(len >= 0);

	conn = throwIfNotConfigured(llserial_conn_get(connection_id));
	if (conn == NULL) {
		LOG_ERROR("%s(%" PRId32 ") called with unknown connection id", __func__, connection_id);
		ok = false;
	}

	if (ok) {
		if (conn->write_rem > 0) {
			LOG_ERROR("%s(%" PRId32 ") called while a write operation is already in progress", __func__, connection_id);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_TX_ERROR);
			ok = false;
		}
	}

	if (ok) {
		assert(conn->write_thread_id == SNI_ERROR);
		conn->write_thread_id = SNI_getCurrentJavaThreadID();
		assert(conn->write_thread_id != SNI_ERROR);

		conn->send_rem = conn->write_rem = len;

		size_t copied_bytes = try_fillup_pp(&conn->write_pp, &b[off], len);
		if (copied_bytes == 0) {
			// this function must succeed to copy some bytes because this the ping bong buffer should be in the reset
			// state
			LOG_ERROR("%s(%" PRId32 ") failed to fillup ping pong buffers", __func__, connection_id);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_TX_ERROR);
			ok = false;
		} else {
			conn->write_rem -= copied_bytes;
		}
	}

	if (ok) {
		int rc = try_sending(conn);
		if (rc != 0) {
			LOG_ERROR("%s(%" PRId32 ") failed to send from ping pong buffers: %d", __func__, connection_id, rc);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_TX_ERROR);
			ok = false;
		}
	}

	if (ok) {
		int32_t status = SNI_suspendCurrentJavaThreadWithCallback(0, (SNI_callback)LLSERIAL_CONNECTION_IMPL_write_cb,
		                                                          NULL);
		assert(status == SNI_OK);
		UNUSED(status);
	}

	if (!ok && conn) {
		// cleanup allocated resources on error
		HAL_UartDMAAbortSend(conn->uart_handle);
		pp_reset(&conn->write_pp);
		conn->write_rem = 0;
		conn->write_thread_id = SNI_ERROR;
	}
}

/* API */

void LLSERIAL_CONNECTION_IMPL_init(void) {
	LOG_INFO("Initialize serial native backend");
	// no resources to initialize
}

int32_t LLSERIAL_CONNECTION_IMPL_open(uint8_t *port_name) {
	int32_t connection_id = open_conn_by_index(find_index_by_port_name((char *)port_name));
	LOG_INFO("Open port %s -> connection_id: %" PRId32, port_name, connection_id);
	return connection_id;
}

void LLSERIAL_CONNECTION_IMPL_configure(int32_t connection_id, int32_t baudrate, int32_t databits, int32_t parity,
                                        int32_t stopbits) {
	int mapped_stopbits;
	int mapped_parity;
	struct llserial_conn *conn;

	assert(baudrate >= 0);
	assert(databits >= LLSERIAL_CONNECTION_DATABITS_5 && databits <= LLSERIAL_CONNECTION_DATABITS_9);
	assert(parity >= LLSERIAL_CONNECTION_PARITY_NONE && parity <= LLSERIAL_CONNECTION_PARITY_EVEN);
	assert(stopbits >= LLSERIAL_CONNECTION_STOPBITS_1 && stopbits <= LLSERIAL_CONNECTION_STOPBITS_2);

	LOG_INFO(
		"Configure connection_id: %" PRId32 " %" PRId32 "-%" PRId32 "%c%s",
		connection_id, baudrate, databits,
		parity == LLSERIAL_CONNECTION_PARITY_NONE ? 'N' : parity == LLSERIAL_CONNECTION_PARITY_EVEN ? 'E' : 'O',
		stopbits == LLSERIAL_CONNECTION_STOPBITS_1 ? "1" : stopbits == LLSERIAL_CONNECTION_STOPBITS_2 ? "2" : "1.5");

	if (baudrate > LPUART_MAX_SUPPORTED_BAUDRATE) {
		THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_UNSUPPORTED_CONFIG, LLSERIAL_CONNECTION_INVALID_BAUDRATE);
		goto end;
	}

	if (databits != LLSERIAL_CONNECTION_DATABITS_8) {
		// only 8 bit uart is supported
		LOG_ERROR("only 8-bit word length is supported");
		THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_UNSUPPORTED_CONFIG, LLSERIAL_CONNECTION_INVALID_DATABITS);
		goto end;
	}

	mapped_parity = llserial_map_parity(parity);
	if (mapped_parity < 0) {
		THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_UNSUPPORTED_CONFIG, LLSERIAL_CONNECTION_INVALID_PARITY);
		goto end;
	}

	mapped_stopbits = llserial_map_stopbits(stopbits);
	if (mapped_stopbits < 0) {
		THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_UNSUPPORTED_CONFIG, LLSERIAL_CONNECTION_INVALID_STOPBITS);
		goto end;
	}

	conn = throwIfNotOpen(llserial_conn_get(connection_id));
	if (conn != NULL) {
		hal_uart_config_t *config = conn->uart_config;
		config->baudRate_Bps = baudrate;
		config->parityMode = mapped_parity;
		config->stopBitCount = mapped_stopbits;

		deinitialize_conn(conn);
		// initialize_conn throws an IOException if configuration fails
		initialize_conn(conn);
	}

end:
	return;
}

int32_t LLSERIAL_CONNECTION_IMPL_available(int32_t connection_id) {
	uint32_t available = 0;
	struct llserial_conn *conn;

	conn = throwIfNotConfigured(llserial_conn_get(connection_id));

	if (conn != NULL) {
		hal_uart_dma_status_t status = HAL_UartDMAGetReceiveCount(conn->uart_handle, &available);
		if (status != kStatus_HAL_UartDmaSuccess) {
			LOG_ERROR("HAL_UartDMAGetReceiveCount() failed with: %d", status);
			THROW_IO_EXCEPTION(LLSERIAL_IOEXCEPTION_GENERIC, LLSERIAL_CONNECTION_INTERNAL_ERROR);
		}
	}

	LOG_INFO("connection_id: %" PRId32 " has %" PRIu32 " available bytes", connection_id, available);

	// There will never be a DMA transfer for more than INT32_MAX bytes (2 GB).
	assert(available <= INT32_MAX);
	return (int32_t)available;
}

void LLSERIAL_CONNECTION_IMPL_flush(int32_t connection_id) {
	LOG_INFO("flush connection_id: %" PRId32 " (no-op)", connection_id);

	throwIfNotConfigured(llserial_conn_get(connection_id));
	// Serial port is flushed immediately when write() is called, nothing to do
}

int32_t LLSERIAL_CONNECTION_IMPL_read(int32_t connection_id, uint8_t *b, int32_t off, int32_t len) {
	int32_t ret;

	LOG_INFO("read %" PRId32 " bytes on connection_id: %" PRId32 "", len, connection_id);

	if ((b != NULL) && (off >= 0) && (len > 0)) {
		ret = LLSERIAL_CONNECTION_IMPL_read_unguarded(connection_id, b, off, len);
	} else {
		THROW_RUNTIME_EXCEPTION(connection_id, "illegal arguments");
		ret = SNI_IGNORED_RETURNED_VALUE;
	}

	return ret;
}

void LLSERIAL_CONNECTION_IMPL_write(int32_t connection_id, uint8_t *b, int32_t off, int32_t len) {
	LOG_INFO("write %" PRId32 " bytes on connection_id: %" PRId32, len, connection_id);

	if ((b != NULL) && (off >= 0) && (len > 0)) {
		LLSERIAL_CONNECTION_IMPL_write_unguarded(connection_id, b, off, len);
	} else {
		THROW_RUNTIME_EXCEPTION(connection_id, "illegal arguments");
	}
}

void LLSERIAL_CONNECTION_IMPL_close(int32_t connection_id) {
	struct llserial_conn *conn;

	LOG_INFO("close connection_id: %" PRId32, connection_id);

	conn = llserial_conn_get(connection_id);
	/* unconditionnaly unregister the resource */
	SNI_unregisterResource((void *)connection_id, (SNI_closeFunction)LLSERIAL_CONNECTION_IMPL_close);

	if (conn != NULL) {
		deinitialize_conn(conn);
		conn->state = LLSERIAL_CONN_STATE_CLOSED;
	} else {
		/* Not being able to recover a `struct conn*` signals that something irrecoverable went wrong in our internal
		 * bookkeeping */
		THROW_RUNTIME_EXCEPTION(UNDEFINED_EXCEPTION_ERROR, LLSERIAL_CONNECTION_INTERNAL_ERROR);
	}
}

int64_t LLSERIAL_CONNECTION_IMPL_getCloseFunction(int32_t connection_id) {
	/* we manage a single type of serial port, the close function is always the same */
	UNUSED(connection_id);

	/* casting to `intptr_t` guarantees to get an integer type large enough for a data pointer, which is actually not
	 * strictly the same as a function pointer. The compiler will complain if `jlong` cannot hold it */
	return (intptr_t)LLSERIAL_CONNECTION_IMPL_close;
}
