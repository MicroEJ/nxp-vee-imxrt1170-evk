/*
 * C
 *
 * Copyright 2025 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef LLSERIAL_CONNECTION_CONFIG_H_
#define LLSERIAL_CONNECTION_CONFIG_H_

#define ENABLE_LOG_ERROR                0
#define ENABLE_LOG_INFO                 0

/**
 * @brief Size of the internal RX BUFFER
 *
 * The buffer size is set to handle at most the default size of LPUART ring buffer (LPUART_RING_BUFFER_SIZE)
 */
#define RX_BUF_SIZE                     128U

/**
 * @brief Buffer size for both the arrays backing the send ping-pong buffer
 */
#define TX_BUF_SIZE                     64U

#endif /* LLSERIAL_CONNECTION_CONFIG_H_ */
