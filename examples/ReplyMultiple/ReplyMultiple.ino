/*
* Inteprets received payload as a float and replies using same id with a 10x multiple.
*/

// Set simple serial parameters before including
#define SIMPLE_SERIAL_MAX_PAYLOAD_LEN 16
#define SIMPLE_SERIAL_MAX_Q_LEN   8

#include <SimpleSerial.h>

SimpleSerial ss(&Serial, millis);

void setup() {
  Serial.begin(115200);

  delay(100);

}

void loop() {
  delay(10);
  ss.loop();
  if (ss.available()) {
    SimpleSerial::Packet packet = ss.read();
    float f = ss.bytes_2_float(packet.payload);
    float reply = f * 10.;
    ss.send_float(packet.id, reply);
  }
}