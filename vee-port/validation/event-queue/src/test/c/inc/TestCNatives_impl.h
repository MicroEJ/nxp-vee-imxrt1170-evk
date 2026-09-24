/*
 * C
 *
 * Copyright 2023-2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 * Build: 7E4D1F7C
 */
#ifndef TEST_C_NATIVES
#define TEST_C_NATIVES

#include <stdint.h>

/**
 * Offers an event to the EventQueue through the C API.
 *
 * @param type: the type of the event.
 * @param data: the data of the event.
 * @return 0 -> success, -1 -> failed: illegal arguments, -2 -> failed: the FIFO is full
 */
int32_t Java_ej_event_utils_TestCNatives_offerEventCApi(uint32_t type, int32_t data);


/**
* Offers an event to the EventQueue through the C API.
*
* @param type
*            the type of the event.
* @param data
*            the data of the event.
* @param data_length
*            the length of the data.
* @return 0 -> success, -1 -> failed: illegal arguments, -2 -> failed: the FIFO is full
*/
int32_t Java_ej_event_utils_TestCNatives_offerExtendedEventCApi(int32_t type, int8_t* data, uint32_t data_length);

/**
* Offers extended data composed of boolean values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedBoolean(int32_t type);
/**
* Offers extended data composed of byte values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedByte(int32_t type);

/**
* Offers extended data composed of char values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedChar(int32_t type);

/**
* Offers extended data composed of double values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedDouble(int32_t type);

/**
* Offers extended data composed of float values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedFloat(int32_t type);

/**
* Offers extended data composed of integer values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment int32_to account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedInt(int32_t type);

/**
* Offers extended data composed of long values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedLong(int32_t type);

/**
* Offers extended data composed of short values. These values correspond to those expected by the testsuite. The
* extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedShort(int32_t type);

/**
* Offers extended data composed of unsigned byte values. These values correspond to those expected by the
* testsuite. The extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedUnsignedByte(int32_t type);

/**
* Offers extended data composed of unsigned short values. These values correspond to those expected by the
* testsuite. The extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedUnsignedShort(int32_t type);

/**
* Offers extended data composed of multiple types of values. These values correspond to those expected by the
* testsuite. The extended data array is taking C structure alignment into account.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_testAlignment(int32_t type);

/**
 * Offers 2 extended events. The first one containing more than 6 bytes.
 *
 * @param type
 *            the type of the event.
 */
void Java_ej_event_utils_TestCNatives_testNotConsumeAllEventBytes(int32_t type);

/**
* Offers an event to the EventQueue asynchronously, from an RTOS task distinct from the MicroEJ
* Core Engine task. Unlike Java_ej_event_utils_TestCNatives_offerEventCApi, the offer does not run
* atomically with the Java threads: it can interleave with the suspend/resume of the Event Queue
* receiving thread. This is required to detect lost wake-ups when an offer races the receiving
* thread resume.
*
* @param type
*            the type of the event.
* @param data
*            the data of the event.
*/
void Java_ej_event_utils_TestCNatives_offerEventFromTask(int32_t type, int32_t data);

/**
* Offers an extended event composed of the byte values expected by the testsuite, asynchronously,
* from an RTOS task distinct from the MicroEJ Core Engine task. Unlike
* Java_ej_event_utils_TestCNatives_offerExtendedByte, the offer does not run atomically with the
* Java threads: it can interleave with the suspend/resume of the Event Queue receiving thread.
* This is required to detect lost wake-ups when an offer races the receiving thread resume.
*
* @param type
*            the type of the event.
*/
void Java_ej_event_utils_TestCNatives_offerExtendedByteFromTask(int32_t type);

#endif // TEST_C_NATIVES
