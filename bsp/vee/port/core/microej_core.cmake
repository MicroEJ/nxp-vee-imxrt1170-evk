# CMake
#
# Copyright 2026 MicroEJ Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found with this software.
#
# Build: 7E4D1F7C

include_guard()
message("microej/core component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/microej_time_freertos.c
	${CMAKE_CURRENT_LIST_DIR}/src/LLMJVM_FreeRTOS.c
	${CMAKE_CURRENT_LIST_DIR}/src/interrupts.c
	${CMAKE_CURRENT_LIST_DIR}/src/LLDEVICE_impl.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE    ${CMAKE_CURRENT_LIST_DIR}/inc)
