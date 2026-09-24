/* 
 * Copyright 2025-2026 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.nxp.example.serial;

import ej.serial.SerialConnection;
import ej.util.Device;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.logging.Logger;

public class Main {

	private static final Logger LOGGER = Logger.getLogger("[Main]");
	private static final char[] HEX_CHAR_ARRAY = "0123456789ABCDEF".toCharArray();

    private static final String PROPERTY_PORT = "com.nxp.example.serial.port";
    private static final String PROPERTY_BAUDRATE = "com.nxp.example.serial.baudrate";

    private static final String PORT = System.getProperty(PROPERTY_PORT);
    private static final int BAUDRATE = Integer.getInteger(PROPERTY_BAUDRATE, 115_200);
    private static final int DATABITS = SerialConnection.DATABITS_8;
    private static final int PARITY = SerialConnection.PARITY_NONE;
    private static final int STOPBITS = SerialConnection.STOPBITS_1;

    public static void main(String[] args) throws IOException {
		String id = bytesToHexString(Device.getId());
		String architecture = Device.getArchitecture();

		LOGGER.info("Serial Sample");
		LOGGER.info("NXP Platform Accelerator VM running on " + architecture +
				" device with ID 0x" + id);

        try (SerialConnection conn = new SerialConnection(PORT)) {
			conn.configure(BAUDRATE, DATABITS, PARITY, STOPBITS);

			OutputStream os = conn.getOutputStream();
			InputStream is = conn.getInputStream();

			PrintStream ps = new PrintStream(os, true);

			ps.print("Serial Sample\r\n"
					+ "NXP Platform Accelerator VM running on " + architecture + " device with ID 0x" + id + "\r\n"
					+ "This serial port echoes everything it receives\r\n");

			while (true) {
				int b = is.read();
				if (b < 0) {
					break;
				}
				os.write(b);
			}
        }
    }

	private static String bytesToHexString(byte[] bytes) {
		char[] hexChars = new char[bytes.length * 2];
		for (int i = 0; i < bytes.length; i++) {
			int b = bytes[i] & 0xFF;
			hexChars[i * 2] = HEX_CHAR_ARRAY[b >>> 4];
			hexChars[i * 2 + 1] = HEX_CHAR_ARRAY[b & 0x0F];
		}
		return new String(hexChars);
	}
}
