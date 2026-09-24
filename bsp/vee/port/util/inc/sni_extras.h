/*
 * C
 *
 * Copyright 2025 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file sni_extras.h
 * @author MicroEJ Developer Team
 * @brief Convenience macros and functions built on top of sni.h
 */

#include "sni.h"

#ifndef SNI_EXTRAS_H_
#define SNI_EXTRAS_H_

#define UNUSED(x)                       ((void)(x))

/**
 * @brief Asserted wrapper for SNI_throwNativeException()
 *
 * MISRA C:2012 17.7 demands that "the value returned by a function having non-void return type shall be used"; however,
 * an error return value from SNI_throw* can only come from:
 * - throwing an exception from a task that is not the VM, which is a programmer error,
 * - throwing an exception after a thread is suspended, which here is treated as a programmer error,
 * - a non-recoverable error from the virtual machine.
 *
 * MISRA C:2012 DIR 4.9: "A function should be used in preference to a function-like macro where they are
 * interchangeable" -> A macro is used so that `assert` returns a meaningful location.
 */
#define THROW_RUNTIME_EXCEPTION(errorCode, message)                        \
		do {                                                               \
			int32_t rc_ = SNI_throwNativeException((errorCode), (message)); \
			assert(rc_ == SNI_OK);                                          \
			UNUSED(rc_);                                                    \
		} while (0)

/**
 * @brief Asserted wrapper for SNI_throwNativeIOException()
 *
 * See @ref THROW_RUNTIME_EXCEPTION
 */
#define THROW_IO_EXCEPTION(errorCode, message)                               \
		do {                                                                 \
			int32_t rc_ = SNI_throwNativeIOException((errorCode), (message)); \
			assert(rc_ == SNI_OK);                                            \
			UNUSED(rc_);                                                      \
		} while (0);


#endif /*SNI_EXTRAS_H_*/
