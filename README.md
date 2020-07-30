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

### Include in C++ project

Clone library to your project and include: `#include "simple_serial_arduino/src/SimpleSerial.h>`

## Usage

Data is sent in packets. Each packet has an **id** in range *0 - 255*. The **id**
is used for identifying topic and data type of the packet. The library does not know data type for a certain id.
User must keep track of this.

Buffer sizes for sending / receiving packets are set at compile time to make sure library works reliably on limited hardware.
Your should set them based on your needs before inluding the library in your code:

```c++
#define SIMPLE_SERIAL_MAX_PAYLOAD_LEN 16
#define SIMPLE_SERIAL_MAX_Q_LEN   8
#include <SimpleSerial.h>
```

Alternatively you can change the sizes by editing source code (not recommended for Arduino installation).

```c++
#include <SimpleSerial.h>

simple_ser = SimpleSerial(&Serial);

void setup() {
  Serial.begin(115200);
}

void loop() {
  // This function must be called inside main loop as frequently as possible.
  simple_ser.loop();
}

// Send some data
simple_ser.send_int(1, 1234);  // Send packet with id 1 containing an integer
simple_ser.send_float(2, 2.345) // Send packet with id 2 containing a float
simple_ser.send(3, "this is a string") // Send packet containing a string

// Receive data
if (simple_ser.available()) {

  // Read a packet from queue of received packets
  SimpleSerial::Packet packet = ss.read();

  // Convert to desired type
  if (packet.id == 123) {
    int val = simple_ser.bytes_2_int(packet.payload_len, packet.payload)
    // do sth with val
  }
  // Note: there is no way to know the data type of payload.
  // You should use id to keep track of this.
}
```

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

When ```SimpleSerial::send()``` function is called, the payload is framed into a packet
and placed into **send queue**. Packets from this queue are sent over serial port
inside ```SimpleSerial::send_loop()```.

Serial port is being monitored for incoming packets using ```SimpleSerial::read_loop()``` function.
The function reads incoming packet frames byte by byte. When a complete packet frame is received, it is placed
into **read queue**. Packets are retrieved from this queue with function ```SimpleSerial::read()```.

The length of send and read queues is set by define ```SIMPLE_SERIAL_MAX_Q_LEN```.
If a queue is full, packets are discarded (not sent or not received).

## Testing
For testing the library you can use TransmissionTest.ino example or implement your own loop using `transmission_test.h`

Test example is designed to evaluate and reply to packets sent from Python version of this library. You can use
`python3 -m simple_serial.transmission_tester` to run the tests.