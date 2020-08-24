/*
* Inteprets received payload as a float and replies using same id with a 10x multiple.
*/

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