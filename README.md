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

Data is sent in packets. Each packet has an ***id*** in range *0 - 255*. The ***id***
can be used for identifying the topic of the packet.

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
