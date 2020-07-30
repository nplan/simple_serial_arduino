/*
* Testing code. Used for testing with transmission_tester.py script. Receives a packet and compares it to expected payload value based on id.
*/

#ifndef TRANSMISSION_TEST_H
#define TRANSMISSION_TEST_H

#include <SimpleSerial.h>
#include <stdbool.h>

template <class T>
bool compare_arrays(T first[], T second[], uint16_t len) {
    bool equal = true;
    for (uint16_t i=0; i<len; i++) {
        if (first[i] != second[i]) {
            equal = false;
            break;
        }
    }
    return equal;
}

bool is_close(float first, float second, float tol) {
    return (abs(first - second) <= tol);
}

// Test function. Must be called in a continous loop.
void transmission_test(SimpleSerial &ss) {
  ss.loop();
  if (ss.available()) {

    SimpleSerial::Packet packet = ss.read();
    
    switch (packet.id)
    {
    // Test suite version
    case 1: {
        ss.send(1, 4, "v0.1");
        break;
    }
    // Test 1
    case 11: {
        uint8_t b[] = {0};
        if (packet.payload_len == 1 && compare_arrays(packet.payload, b, 1)) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 2
    case 12: {
        uint8_t b[] = "hello";
        if (packet.payload_len == 5 && compare_arrays(packet.payload, b, 5)) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 3
    case 13: {
        int16_t i = 12345;
        if (packet.payload_len == 4 && ss.bytes_2_int(packet.payload) == i) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 4
    case 14: {
        int16_t i = -4321;
        if (packet.payload_len == 4 && ss.bytes_2_int(packet.payload) == i) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 5
    case 15: {
        float f = 4.321;
        if (packet.payload_len == 4 && is_close(ss.bytes_2_float(packet.payload), f, 1e-5)) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 6
    case 16: {
        float f = -123.4;
        if (packet.payload_len == 4 && is_close(ss.bytes_2_float(packet.payload), f, 1e-5)) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 7
    case 17: {
        uint8_t b[] = {ss.start_flag, ss.esc_flag, ss.end_flag};
        if (packet.payload_len == 3 && compare_arrays(packet.payload, b, 3)) {
            ss.send(packet.id, 2, "ok");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 8
    case 18: {
        uint8_t b[] = "hello";
        if (packet.payload_len == 5 && compare_arrays(packet.payload, b, 5)) {
            ss.send(packet.id, 5, "world");
        }
        else
        {
            ss.send(packet.id, 4, "fail");
        }
        break;
    }
    // Test 9
    case 19: {
        ss.send_int(packet.id, 2*ss.bytes_2_int(packet.payload));
        break;
    }
    case 20: {
        ss.send_float(packet.id, 2*ss.bytes_2_float(packet.payload));
        break;
    }
    default:
        break;
    } // end switch
  } // end if
}

#endif // TRANSMISSION_TEST_H