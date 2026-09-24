# Overview

This application contains a sample programs demonstrating how to use MicroEJ's Serial foundation library.

# Usage

The VEE port is configured so that LPUART2 is readily available:
* TX: connector `J9`, pin `4`
* RX: connector `J9`, pin `2`

(GND is available on `J9`, pins {`1`, `5`, `7`, `9`, `11`, `13`, `15`}, see schematics for more informations)

The default baudrate is 115200, it can be updated with `com.nxp.example.serial.baudrate` system property in [common.properties](configuration/common.properties) file.
The serial port is configured to run with 1 stop bit, 8-bit word, no parity.

The simulator emulates the serial port with a TCP socket listening on port `7777`.

## Echo terminal

This program sends back on its TX channel every byte received on its RX channel.

* Open a terminal connection to the serial port under test. For example, `putty` is suitable for connection
both on device and on simulator. `minicom`, `netcat` or others can also be used. Refer to your tool's manual to configure
the connection.
* Every keystroke is echoed back to the terminal.

## Run on simulator

In Visual Studio Code:
- Open the Gradle tool window by clicking on the elephant icon on the left side,
- Expand the tasks of the chosen demo project,
- Double-click on the `microej` > `runOnSimulator` task,
- The application starts, the traces are visible in the Run view.

Alternative ways to run in simulation are described in the [Run on Simulator](https://docs.microej.com/en/latest/SDK6UserGuide/runOnSimulator.html) documentation.

## Run on device

Make sure to properly set up the VEE Port environment before going further.
Refer to the [VEE Port README](../../README.md) for more information.

In Visual Studio Code:
- Open the Gradle tool window by clicking on the elephant icon on the left side,
- Expand the tasks of the chosen demo project,
- Double-click on the `microej` > `runOnDevice` task,
- The device is flashed. Use the appropriate tool to retrieve the execution traces.

Alternative ways to run on device are described in the [Run on Device](https://docs.microej.com/en/latest/SDK6UserGuide/runOnDevice.html) documentation.

# Dependencies

_All dependencies are retrieved transitively by Gradle._

# Source

N/A

# Restrictions

None.
 
---  
_Markdown_   
_Copyright 2025-2026 MicroEJ Corp. All rights reserved._  
_Use of this source code is governed by a BSD-style license that can be found with this software._
