/*
* A sketch used for testing. Sketch expect a message with certain id and payload.
* If payload matches expected payload for id of received packet, test is passed.
*/

#include <stdbool.h>

#include <SimpleSerial.h>
#include <SoftwareSerial.h>
#include "transmission_test.h"

SimpleSerial ss(&Serial);
SoftwareSerial ser(12, 13);

void setup() {
  Serial.begin(115200);
  ser.begin(57600);
  delay(100);
  ser.println("Booted");
}

void loop() {
    transmission_test(ss);
    delay(10);
}
