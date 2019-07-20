#include "SimpleSerial.h"

SimpleSerial ss([]{Serial.available();},
                []{Serial.read();},
                [](uint8_t* buf, uint8_t len){Serial.write(buf, len);});

void setup() {
  Serial.begin(115200);
  delay(100);
}

void loop() {
  delay(10);
  ss.loop(millis());
  if (ss.available()) {
    uint8_t payload[MAX_LEN_PYLD];
    uint8_t id;
    uint8_t len;
    ss.read(id, len, payload);
    float n = ss.bytes2Float(len, payload);
    ss.sendFloat(id, n*10.);
  }
}

bool available(void) {
  return Serial.available();
}

uint8_t read(void) {
  return Serial.read();  
}

uint8_t write(uint8_t* buf, uint8_t len) {
  return Serial.write(buf, len);
}
