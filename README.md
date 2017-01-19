# How to connect the OpenBCI via wired USB and change the sample rate

## Table of Contents:

I. [Introduction](#intro)
  1. [Why do this](#why)
  2. [Caveats](#caveats)
  3. [What do I need](#requirements)
II. [Connecting and testing the USB Breakout Board](#usb)
  1. [Test Echo IO on USB Breakout](#testusbbreakout)
  2. [Solder headers onto OBCI and connect the USB Breakout Board](#connect)
III. [Update the OBCI Firmware](#firmware)
  1. [Prepare to update the OBCI Firmware](#prep)
  2. [Set Tx/Rx Pins within the chipkit code](#pins)
  3. [Download the new firmware to the board](#download)
  4. [Optional: - change the baud and sample rate](#baud)
  5. [Upload the new firmware](#upload)
  6. [Set the COM Port settings](#com)
  7. [Test the system](#testeeg)
  8. [Connect the USB Isolator before connecting to a human](#isolator)
IV. [Concluding Remarks](#conclusion)
  1. [Firmware Modifications Made](#changelog)
  2. [Thanks!!!](#thanks)
  3. [Contribute!](#contribute)


## <a name="intro"></a> I. Introduction

This tutorial shows how a third party USB breakout chip can be configured with OpenBCI (OBCI) to allow for a wired USB connection. I've only verified this on Windows, but there's no reason why it shouldn't work on other OSes.  For extra related information beyond this tutorial, see [this thread](http://openbci.com/forum/index.php?p=/discussion/712/prospects-for-higher-sample-rates/p1) on the forum.

### <a name="why"></a> 1. Why do this?

Reasons one might carry out the modifications herein include:
* Increasing the sample rate of OBCI - The default rates of 250 Hz (8-channel) and 125 Hz (16-channel) can be increased, but need a faster data connection than the 115200 baud throughput that bottlenecks the default setup. So far, the maximum speed I can get with this is 1kHz on 8-channels at 460800 baud. I haven't tested the code for the Daisy mode since I do not have one.
* More reliable packet delivery for timing-sensitive applications

### <a name="caveats"></a> 2. Caveats

The modified system won't run on the default version of OBCI's software ([the Processing code](https://github.com/OpenBCI/OpenBCI_Processing)).  The change in baud rate and possibly other changes break it.  This might be easy to fix, but I haven't investigated it.  It works just fine with the [OBCI Node JS Server](https://github.com/OpenBCI/OpenBCI_NodeJS). In that repos there are also instrucitons on how to get the system to interface with Python.

### <a name="requirements"></a> 3. What do I need?

 1. OpenBCI V3 32-bit system. Simple modifications should also work for the 8-bit board.
 2. UART-capable USB breakout board. E.g. [Adafruit FT232H](https://www.adafruit.com/product/2264)
 3. USB isolator capable of "full speed" (12.0 Mbps). E.g. [Adafruit 100mA USB Isolator](https://www.adafruit.com/product/2107)
 4. USB cables to connect the PC-->USB isolator --> USB breakout board. For the models listed, it's 1x Micro USB cable and 1x Mini USB Cable.
 6. Female/Male jumper wires.  Shorter is better, to avoid signal degradation.  E.g. [Adafruit's](https://www.adafruit.com/products/825)
 7. Female/Female jumper wires.  Used to do a basic I/O echo test on the USB breakout board.  E.g. [Adafruit's](https://www.adafruit.com/products/794)
 8. Access to a soldering iron, if using the FT232H USB breakout, in order to solder on the pins.  Your OBCI must also have its J3 and J4 pinblock headers soldered on, if they haven't already been soldered on.

### <a name="connect"></a> II. Connecting and testing the USB breakout board

### <a name="testusbbreakout"></a> 1. Test echo IO on the USB breakout board
Connect the USB breakout board to your computer with a USB cable.  If using the FT232H, follow [Adafruit's tutorial](https://learn.adafruit.com/adafruit-ft232h-breakout) up through the [serial UART page](https://learn.adafruit.com/adafruit-ft232h-breakout/serial-uart) to verify that the drivers are working on your system and that a basic IO test succeeeds.

### <a name="connect"></a> 2. Solder headers onto OBCI and connect the USB breakout board
Solder headers into the J3 and J4 pin blocks on OBCI (Pins GND, VOD, D12, D13, D11, D10, RST, D17).

Use 3 female/male jumper wires to connect pins GND, TX (D0 on the FT232H), RX (D1 on the FT232H), to GND, D12, and D11 respectively, on OBCI.

## <a name="firmware"></a> III. Update the OBCI firmware

### <a name="prep"></a> 1. Prepare to update the OBCI firmware
We will be replacing OBCI's default firmware with a custom version.  To prepare for the update, follow the procedure [here](http://docs.openbci.com/Hardware/05-Cyton_Board_Programming_Tutorial) up to the point of putting the board into bootloader mode.  We won't do that until we are ready to upload the new code.

### <a name="pins"></a> 2. Set TX/RX pins within the chipkit code
We need to change a few lines of code in the OBCI board definition file to enable pins D11 as TX and D12 as RX.  Find the file whose path should resemble: Documents/Arduino/hardware/chipkit-core/pic32/variants/openbci/Board_Defs.h

In this file make changes to the corresponding code so that it reads:

\#define       _SER1_TX_OUT         PPS_OUT_U2TX     // RPB14R = U2TX = 2  
\#define       _SER1_TX_PIN         11  //7                // RB14 CVREF/AN10/C3INB/RPB14/VBUSON/SCK1/CTED5/RB14  
\#define       _SER1_RX_IN          PPS_IN_U2RX      // U2RXR = RPA1 = 0  
\#define       _SER1_RX_PIN         12  //10               // RA1  PGEC3/VREF-/CVREF-/AN1/RPA1/CTED2/PMD6/RA1  


Save the changes.

### <a name="download"></a> 3. Download the new firmware to the board
The custom firmware and associated libraries are in [this repository](https://github.com/WinslowStrong/OpenBCI_Wired_USB).  Place the entire folder into /Arduino/libraries/ (replacing the original OpenBCI_32bit_Library folder; symlinks are useful here). In examples/ there are a bunch of example .ino files that provide examples about how to use the firmware and can be stored anywhere, but each individual example need to be in the same folder as `SD_Card_Stuff.ino`.

### <a name="baud"></a> 4. Optional - change the baud and sample rates
The default OBCI firmware is configured to use a USB serial connection at 115200 baud and run at a 250 Hz sample rate for 8 channels, and 125 Hz alternating sample rate in "daisy mode" (i.e. with 16 channels).

The custom firmware for this tutorial sets a default baud rate of 460800 and a sample rate of 500 Hz for its 8-channel system (and 250 Hz for 16-channel alternating). You can change the baud and sample rate parameters by opening the OBCI_USB_Serial.ino file in the Arduino editor. 
* baud - Change the line `#define OPENBCI_BAUD_RATE_USB  460800` in OpenBCI_32bit_Library_Definitions.h by replacing 460800 with the baud rate you want.  I'm not sure that any rates other than factor-of-two multiples/divisors can work.
* Change the sample rate via the variable passed into the `board.beginDebug()` constructor in your .ino.  The possible values are:  ADS_RATE_250Hz, ADS_RATE_500Hz, ADS_RATE_1kHz, ADS_RATE_2kHz, ADS_RATE_4kHz, ADS_RATE_8kHz, and ADS_RATE_16kHz. 


##### Limitations
* I haven't been able to get this setup to work above 460800 baud (communication becomes totally garbled at 921600 baud).  
* On my system, 460800 baud can handle a max sample rate of 1 kHz with no dropped channels over 15 minutes.  The baud and max sample rate track each other down from here by factors of 2.
* Winslow Strong for a [previous version of this hack](https://github.com/WinslowStrong/OpenBCI_Wired_USB) tried a different USB breakout board, [Numato's FT2232H](http://numato.com/ft2232h-breakout-module/), in an attempt to overcome the occasional lost packets at 460800 baud and 500 Hz sample rate using the 16 channel OBCI.  The thought was that its larger send/receive buffers (4k vs 1k) might solve the problem.  It does reduce dropped packets, but it caused another issue on his system: approximately 1/3 of sent commands don't get through successfully.


### <a name="upload"></a> 5. Upload the new firmware
Resume the [uploading tutorial](http://docs.openbci.com/Hardware/05-Cyton_Board_Programming_Tutorial) by putting the OBCI into bootloader mode.  Follow the rest of the tutorial to upload the custom firmware that you downloaded above. In case it doesn't run for you for some reason or you later want to revert, you can follow the same procedure to upload the default firmware, available [here](https://github.com/OpenBCI/OpenBCI_32bit).  

### <a name="com"></a> 6. Set the COM port settings
In Linux (Ubuntu 16.04 LTS), everything worked out of the box. In Windows (you can probably [make similar changes on Mac OS](http://docs.openbci.com/Tutorials/10-Mac_FTDI_Driver_Fix)), go into device manager-->Ports, and find the COM port of the USB breakout board (NOT the USB dongle!).  Right click on it, select "Properties" and select the "Port Settings" tab.  Make these changes:
* "bits per second" is the baud rate. Set it to whatever you initialized Serial1 to in the new firmware (460800 if you made no change).
* Within "Advanced," set the port latency to 8 ms.  1 ms is too fast for the FT232H, and can lead to dropped packets. Also, 16 ms is a bit too long and causes the buffer on the FT232H to fill too quickly
* Also within "Advanced," set the Receive and Transmit USB Transfer rate to be 512 bytes. If set to 4kB, then the computer waits for 4 kB of data to be accumulated before shuttling over the USB connection. However, this causes the connection to be very stuttery and cauess packet drop

On my system, these changes need to be "OKed" individually, or else the baud rate change doesn't save.  It might tell you that you have to restart your system, but you don't.  Just unplug the USB connection and plug it back in.


### <a name="testeeg"></a> 7. Test the system
As mentioned up front, the default OBCI Processing software won't run as-is with this custom OBCI firmware. Instead, OpenBCI provides a brilliant [OBCI Node JS Server](https://github.com/OpenBCI/OpenBCI_NodeJS) which works amazingly with the system. When initializing, use these settings:
* COM port of the *Serial USB* (not the OBCI dongle!): "-p=COM*#*" where "*#*" is the number of the COM port of the Serial USB breakout board. On Mac/Ubuntu this will look like "/dev/ttyUSB#" 
*  baud rate: "-b=*rate*"  where "*rate*" is the baud rate you are using (230400 unless you changed it).
* Iff you are using the 16 channel OBCI, then supply "-d" to force daisy mode.

### <a name="isolator"></a> 8. Connect the USB isolator before connecting to a human
Use your mini USB cable to connect the USB isolator to your computer.  The USB breakout board then connects to the isolator.  This is a safety precaution so that a human connected to OBCI is electronically isolated from the mains power supply running through the computer and its USB port.  

## <a name="conclusion"></a> III. Concluding remarks

### <a name="changelog"></a> 1. Firmware modifications

The modifications made to the firmware are:
* In the OBCI board definition chipkit core file, enabled pins 11 (D11 on the board) and 12 (D12) as UART1 TX and RX, repsectively.
* Replaced all "Serial0" --> "Serial1" and vice-versa in the .ino and the OpenBCI_32_Daisy library files.  Serial0 is still the RFduino, while Serial1 is UART1, using OBCI pins D11 and D12 to send and recieve data.
* Changed `sendChannelData()` and its cousins to write start byte `0xA0` first instead of `A`. I assume this was originally because the RFDuino does some packet changing, which no longer takes place since we go through the wired USB connection.
* Changed all calls of `board.begin()` to be `board.beginDebug(uint8_t srate)` because for some reason both serial ports need to be open.
* Have board check on Serial 1 for input instead of Serial 0


### <a name="thanks"></a> 2. Thanks!

Most of the inspiration/foundations for this were made possible by Winslow Strong (@Winslow_Strong on the OBCI forums), who figured out how to get the wired USB connection to work with v1 of the firmware (this is an update to work with firmware v2). Huge thanks to William Croft (@wjcroft) for troubleshooting and making clarifications about the hardware. Many thanks goes to AJ (@pushtheworld) as well for providing feedback on the firmware v2 and cluing me in towards the NodeJS architecture.

### <a name="contribute"></a> 3. Contribute!

If you do some modifications based on this work that you think could be useful for others, please share it.  Posting your code to github and/or updating this document would be very appreciated!