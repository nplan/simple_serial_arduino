# Simple Serial

A simple library for communication over serial interfaces.
It can be used for **microcontroller to microcontroller** communication  or
**computer to microcontroller** communication.

Currently supported platforms:
* Arduino / C++
* Python

This is a repository for the Arduino / C++ version. Find python version at:
https://github.com/nplan/simple_serial_python

The library supports simple and efficient transfer of data types:
* Integer (signed, 32 bit)
* Floating point (32 bit)
* Strings

Custom data types can be easily added.

## Installation

### Add to Arduino IDE
Download the .zip file. Inside Arduino IDE, go to _Sketch > Include Library > Add .ZIP Library..._ and select the downloaded .zip file.
To include library in your sketch use `#include <SimpleSerial.h>`

### Add to your project
>This is the preferred way to install if you want to modify the library parameters

Download and extract the .zip file. Copy ***SimpleSerial.h*** and ***SimpleSerial.cpp*** to your project folder, next to the sketch (*.ino*) file.
To include library in your sketch use `#include "SimpleSerial.h"`

## Usage

Data is sent in packets. Each packet has an **id** in range *0 - 255*. The **id**
is used for identifying topic and data type of the packet.

```c++
#include <SimpleSerial.h>

s = SimpleSerial(&Serial, 115200);

void setup() {
  Serial.begin(115200);
}

void loop() {
  // This functions must be called inside main loop as frequently as possible.
  // Everything happens inside this functions.
  s.readLoop(millis());
  s.sendLoop(millis());
}

// Send some data
s.sendInt(1, 1234);  // Send packet with id 1 containing an integer
s.sendFloat(2, 2.345) // Send packet with id 2 containing a float
s.send(3, "this is a string") // Send packet containing a string

// Receive data
if (s.available()) {
  byte id;
  byte len;
  byte payload[MAX_LEN_PYLD];
  s.read(id, len, payload); // Return a packet from received buffer

  // Convert to desired type
  if (id == 123) {
    int val = s.bytes2Int(len, payload)
    // do sth with val
  }
  // Note: there is no way to know of what type the data inside a packet is.
  // You should use id to differentiate between packets.
}
```

The default payload length in bytes is set to 8. This can be changed by setting
the constant ```MAX_LEN_PYLD``` in ```SimpleSerial.h```.

## How it works?
Before being sent over serial port, data is framed into packets using using special **flag bytes**:
* *START* Byte - Signalling **start** of packet, *default = ASCII 2 STX*
* *END* Byte - Signalling **end** of packet, *default = ASCII 3 ETX*
* *ESC* Byte - Inserted prior to payload data byte if it happens to have the same byte value
as one of the flag bytes. *default = ASCII 1 SOH*

A complete packet frame:

|BYTE 0| BYTE 1 | BYTE 2 | BYTES 3 ... *N*-1 | BYTE *N*|
|------|--------|--------|-------------------|---------|
|START|LEN|ID|Payload Data Bytes|END|

Byte 1 *LEN* indicates the total length of packet in bytes.

When one of the ```SimpleSerial.send()``` functions is called, the payload is framed into a packet
and placed into **send queue**. Packets from this queue are sent over serial port
inside ```SimpleSerial.sendLoop()```.

Serial port is being monitored for incoming packets using ```SimpleSerial.readLoop()``` function.
The function reads incoming packet frames byte by byte. When a complete packet frame is received, it is placed
into **read queue**. Packets are retrieved from this queue with function ```SimpleSerial.read()```.

The length of send and read queues is set by constant ```QUEUE_LEN``` inside ```SimpleSerial.h```.
If a queue is full, packets are discarded (not sent or not received).
