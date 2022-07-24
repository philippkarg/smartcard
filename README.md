# ATmega644 Smart-Card running AES decryption

- [ATmega644 Smart-Card running AES decryption](#atmega644-smart-card-running-aes-decryption)
  - [Introduction](#introduction)
  - [Project Overview](#project-overview)
  - [Build Configurations](#build-configurations)
    - [Prerequisites](#prerequisites)
    - [Building the Project](#building-the-project)
    - [Debug Mode](#debug-mode)
    - [Countermeasures](#countermeasures)

## Introduction

This repository contains an 128-bit AES decryption algorithm, that runs on a Smart-Card with an AVR ATmega644 microcontroller on it. The code was implemented in the scope of a "Smart-Card Laboratory" at the Technical University of Munich, with the purpose to decrypt chunks of a video stream while communicating with a Smart-Card reader or Terminal. The communication between the Smart-Card an the Terminal occurs over the Smart-Card's ISO7816 I/O contact and follows the *T=0* protocol specified in ISO7816. This markdown page provides some information on build configurations & gives an overview of the code.

## Project Overview

The code for the clone consists of these main classes:

- The `Communication` class manages the *T=0* protocol to communicate with the Terminal.
- The `AES` class contains all the functionality required for the 128-bit AES decryption running on the processor.
- The `AESMath` class contains some math helper functions for the decryption.
- The `Hiding` class implements countermeasures Shuffling & Dummy-Ops.
- The `Masking` class implements the Masking countermeasure.
- The `RNG` class implements a small, lightweight Random-Number-Generator.
- The `Logger` class can be used to log message to a serial console over USART & USB. Note that this functionality is only available in *debug mode*.

## Build Configurations

### Prerequisites

- This project uses CMake and requires Version *3.5* or higher.

- If you want to flash the executable onto an ATmega644 yourself, make sure to install `avrdude` (e.g. on Debian: `sudo apt-get install avrdude`)


### Building the Project

The repository is built using CMake. To build the default `.hex` file that can be flashed on the ATmega644 using an AVR programmer, do the following:

- Create a build directory: `$ mkdir build/`

- Change into the directory: `$ cd build/`

- Run CMake: `$ cmake ..`

- If you only want to compile the project, run: ` $ make`

- If you also want to flash it onto an ATmega644, run: ` $ make flash`


There are a couple of CMake options that can be turned on/off.
To see a complete list of them, run: `$ cmake -L ..` from you build folder.

### Debug Mode

The Smart-Card used in this laboratory came with an USART to USB converter chip. This makes it possible to log messages to a serial console on a PC over the ATmega644's USART peripheral. If you are compiling this code for your own Smart-Card, you might run into issues with USART.
To enable logging of debug messages, the project can be compiled in *debug mode*. Note that this greatly increases the executable size & that some UART message might interfere with the Terminal communication. If these message are unwanted, the project can also be compiled in *release mode*.

- Run `$ cmake -DDebug=ON ..` to build the project in *debug mode*.
- Run `$ cmake -DDebug=OFF ..` to build the project in *release mode*.
- The default value is `OFF`.


### Countermeasures

The AES implementation contains a number of DPA countermeasures, including Masking, Shuffling the S-Box access & inserting Dummy NOPs. You can enable/disable these countermeasures with CMake flags:

- **Masking**: When masking is enabled, the current 16-byte AES state & all round-keys, will be masked with 6 randomly generated masks. These masks will be applied once before the first round, re-applied after every inverse MixColumns operation & removed after the last round. To enable/disable masking, do the following:
	- Run `$ cmake -DMasking=ON ..` to enable masking.
	- Run `$ cmake -DMasking=OFF ..` to disable masking.
	- The default value is `OFF`.
- **Shuffling**: Another countermeasure that was added is the shuffling of S-Box accesses. When reading values from the S-Box look-up table, these accesses are not performed in a specific order, but a random. The goal of this is to randomize the chips power consumption during decryption. To enable/disable Shuffling, do the following:
	- Run ` $ cmake -DShuffling=ON` to enable Shuffling.
	- Run `$ cmake -DShuffling=OFF` to disable Shuffling.
	- The default value is `OFF`.
- **Dummy-Ops**: The last countermeasure that was implemented are Dummy NOPs. These are inserted before every AES operation (AddRoundKey, InvMixCol, InvShiftRows, InvSubBytes) to also randomize the power consumption during decryption. To enable/disable Dummy-Ops do the following:
	- Run `$ cmake -DDummyOps=ON` to enable Dummy-Ops.
	- Run `$ cmake -DDummyOps=OFF` to disable Dummy-Ops.
	- The default value is `OFF`.
