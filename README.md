##Emulator for the S6B0108 LCD display.

Implements basic functionality to write commands and data to the S6B0108 and to read from it. Also implements a few high level functions for an 2x20 ascii display (S6A0069), basic input (cursor keys) and delay functions.

Observe that this is a work in progress, and it's not assured that code that runs on the emulator work on the hardware. 

#### Installation

Build with make. The emulator runs under X11, so these are required libraries. I've used GCC 4.4.7 to compile these, on Debian and Red Hat distros.