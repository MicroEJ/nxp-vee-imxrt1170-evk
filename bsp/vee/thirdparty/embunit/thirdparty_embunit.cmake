# CMake
#
# Copyright 2026 MicroEJ Corp. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found with this software.

include_guard()
message("microej/thirdparty/embUnit component is included.")

target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/embUnit/AssertImpl.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/RepeatedTest.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/stdImpl.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/TestCaller.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/TestCase.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/TestResult.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/TestRunner.c
	${CMAKE_CURRENT_LIST_DIR}/embUnit/TestSuite.c
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE    ${CMAKE_CURRENT_LIST_DIR}/embUnit)
