#include "SimpleSerial.h"
#include <string.h>

/*
 * Returns true if data is available.
 */
bool SimpleSerial::available() {
    return receive_queue.count();
}

/*
 * Takes payload (in bytes) and its id, frames it into a packet, and places it
 * in send queue.
 */
void SimpleSerial::send(uint8_t id, uint8_t len, uint8_t const *payload) {
    // Check len
    if (len > max_payload_len_) return;

    // Frame packet
    Packet packet(id, len, payload);
    Frame frame = build_frame(packet);

    // Place packet in send queue
    send_queue.push(frame);
}

/*
 * Converts float to 4 bytes and sends using send().
 */
void SimpleSerial::send_float(uint8_t id, float f) {
    union u {
        float _f = 0.;
        uint8_t b[4];
    } u;
    u._f = f;
    send(id, 4, u.b);
}

/*
 * Converts int to 4 bytes and sends using send();
 */
void SimpleSerial::send_int(uint8_t id, int32_t i) {
    union u {
        int32_t _i = 0;
        uint8_t b[4];
    } u;
    u._i = i;
    send(id, 4, u.b);
}

/*
 * Frames array of bytes into a packet. Inserts flag bytes, id and length.
 */
SimpleSerial::Frame SimpleSerial::build_frame(Packet packet) {
    uint8_t id = packet.id;
    uint8_t payload_len = packet.payload_len;
    uint8_t *payload = packet.payload;

    if (payload_len > max_payload_len_) return Frame();

    // Calculate CRC
    uint8_t crc = calc_CRC(payload, payload_len);

    // Extend array to make place for CRC
    uint8_t payload_new[payload_len + 1];
    memcpy(payload_new, payload, payload_len);
    payload = payload_new;
    payload_len++;

    // Insert CRC byte to the end of payload
    payload[payload_len - 1] = crc;

    Frame frame(max_frame_len_);

    // Insert ESC flags
    uint8_t i = 0; // payload byte index
    uint8_t j = 0; // frame byte index
    frame.data[0] = start_flag;
    frame.data[1] = 0; // frame length
    frame.data[2] = id;
    j = 3;
    while (i < payload_len) {
        uint8_t b = payload[i];
        if (b == esc_flag || b == start_flag || b == end_flag) {
            // Must insert ESC flag
            frame.data[j] = esc_flag;
            j++;
            frame.data[j] = b;
            j++;
            i++;
        }
        else {
            frame.data[j] = b;
            j++;
            i++;
        }
    }
    frame.data[j] = end_flag;
    j++;
    frame.data[1] = j; //frame length
    frame.len = j;
    return frame;
}

/*
 * Continuously running loop. Sends the next packet in send queue.
 */
void SimpleSerial::send_loop() {
    if (send_queue.count() <= 0)
        // Return if nothing to send
        return;
    else {
        // Send packet
        Frame frame = send_queue.pop();
        serial_->write(frame.data, frame.len);
        }
}

/*
 * Returns and removes the oldest packet in the read queue.
 */
SimpleSerial::Packet SimpleSerial::read() {
    return receive_queue.pop();
}

/*
 * Continuously running loop. Waits for bytes as they arrive and
 * decodes the packet. Reads *read_num_bytes* in one iteration.
 */
void SimpleSerial::read_loop() {
    for (uint8_t k = 0; k < read_num_bytes; ++k) {

        uint32_t time = sys_time();

        if (!serial_->available())
            return;

        uint8_t b = serial_->read();
        if (byte_count == 0 && b == start_flag) {
            // First byte - START flag. Start count.
            start_time = time;
            byte_count = 1;
            payload_i = 0;
            esc_active = false;
        } else if (byte_count == 1) {
            // Second byte - packet length
            received_frame_len = b;
            byte_count = 2;
        } else if (byte_count == 2) {
            // Third byte - packet identifier
            received_id = b;
            byte_count = 3; 
        } else if (byte_count > 2) {
            // Data value byte
            if (byte_count > (max_frame_len_) || (time - start_time) > receive_timeout) {
                // No END flag. Reset.
                byte_count = 0;
                return;
            }
            if (!esc_active) {
                // No preceding ESC. Accept flags.
                if (b == esc_flag)
                    // ESC flag. Activate ESC mode.
                    esc_active = true;
                else if (b == end_flag) {
                    // End of packet. Check if specified and actual length are equal.
                    uint8_t crc_received = incoming_payload_[payload_i - 1];
                    uint8_t crc_calculated = calc_CRC(incoming_payload_, payload_i - 1);
                    payload_i--;

                    if ((byte_count == received_frame_len - 1) && (crc_received == crc_calculated)) {
                        // Valid data. Add to read queue.
                        Packet packet(received_id, payload_i, incoming_payload_);
                        receive_queue.push(packet);
                        byte_count = 0;
                    } else {
                        // CORRUPTED data. Reset
                        byte_count = 0;
                        esc_active = false;
                    }
                    return;
                } else {
                    // Normal data byte. Add to array.
                    if (payload_i < max_payload_len_) {
                        incoming_payload_[payload_i] = b;
                        payload_i++;
                    }
                    else {
                        // Restart
                        byte_count = 0;
                        return;
                    }

                }
            } else {
                // ESC preceding. Ignore flag following ESC byte.
                incoming_payload_[payload_i] = b;
                payload_i++;
                esc_active = false;
            }
            byte_count++;
        }
    }
}

void SimpleSerial::loop() {
    send_loop();
    read_loop();
}

void SimpleSerial::confirm_received(uint8_t id) {
    uint8_t pld[] = "ok";
    send(id, 2, pld);
}

/*
 * Return system time if time_getter function is set, otherwise return 0.
 */
uint32_t SimpleSerial::sys_time() {
    uint32_t time;
    if(time_getter) {
        time = (uint32_t) time_getter();
    }
    else {
        time = 0;
    }
    return time;
}

// Automatically generated CRC function from python crcmod
// CRC-8; polynomial: 0x107
uint8_t SimpleSerial::calc_CRC(uint8_t *data, uint8_t len)
{
    static const uint8_t table[256] = {
            0x00U,0x07U,0x0EU,0x09U,0x1CU,0x1BU,0x12U,0x15U,
            0x38U,0x3FU,0x36U,0x31U,0x24U,0x23U,0x2AU,0x2DU,
            0x70U,0x77U,0x7EU,0x79U,0x6CU,0x6BU,0x62U,0x65U,
            0x48U,0x4FU,0x46U,0x41U,0x54U,0x53U,0x5AU,0x5DU,
            0xE0U,0xE7U,0xEEU,0xE9U,0xFCU,0xFBU,0xF2U,0xF5U,
            0xD8U,0xDFU,0xD6U,0xD1U,0xC4U,0xC3U,0xCAU,0xCDU,
            0x90U,0x97U,0x9EU,0x99U,0x8CU,0x8BU,0x82U,0x85U,
            0xA8U,0xAFU,0xA6U,0xA1U,0xB4U,0xB3U,0xBAU,0xBDU,
            0xC7U,0xC0U,0xC9U,0xCEU,0xDBU,0xDCU,0xD5U,0xD2U,
            0xFFU,0xF8U,0xF1U,0xF6U,0xE3U,0xE4U,0xEDU,0xEAU,
            0xB7U,0xB0U,0xB9U,0xBEU,0xABU,0xACU,0xA5U,0xA2U,
            0x8FU,0x88U,0x81U,0x86U,0x93U,0x94U,0x9DU,0x9AU,
            0x27U,0x20U,0x29U,0x2EU,0x3BU,0x3CU,0x35U,0x32U,
            0x1FU,0x18U,0x11U,0x16U,0x03U,0x04U,0x0DU,0x0AU,
            0x57U,0x50U,0x59U,0x5EU,0x4BU,0x4CU,0x45U,0x42U,
            0x6FU,0x68U,0x61U,0x66U,0x73U,0x74U,0x7DU,0x7AU,
            0x89U,0x8EU,0x87U,0x80U,0x95U,0x92U,0x9BU,0x9CU,
            0xB1U,0xB6U,0xBFU,0xB8U,0xADU,0xAAU,0xA3U,0xA4U,
            0xF9U,0xFEU,0xF7U,0xF0U,0xE5U,0xE2U,0xEBU,0xECU,
            0xC1U,0xC6U,0xCFU,0xC8U,0xDDU,0xDAU,0xD3U,0xD4U,
            0x69U,0x6EU,0x67U,0x60U,0x75U,0x72U,0x7BU,0x7CU,
            0x51U,0x56U,0x5FU,0x58U,0x4DU,0x4AU,0x43U,0x44U,
            0x19U,0x1EU,0x17U,0x10U,0x05U,0x02U,0x0BU,0x0CU,
            0x21U,0x26U,0x2FU,0x28U,0x3DU,0x3AU,0x33U,0x34U,
            0x4EU,0x49U,0x40U,0x47U,0x52U,0x55U,0x5CU,0x5BU,
            0x76U,0x71U,0x78U,0x7FU,0x6AU,0x6DU,0x64U,0x63U,
            0x3EU,0x39U,0x30U,0x37U,0x22U,0x25U,0x2CU,0x2BU,
            0x06U,0x01U,0x08U,0x0FU,0x1AU,0x1DU,0x14U,0x13U,
            0xAEU,0xA9U,0xA0U,0xA7U,0xB2U,0xB5U,0xBCU,0xBBU,
            0x96U,0x91U,0x98U,0x9FU,0x8AU,0x8DU,0x84U,0x83U,
            0xDEU,0xD9U,0xD0U,0xD7U,0xC2U,0xC5U,0xCCU,0xCBU,
            0xE6U,0xE1U,0xE8U,0xEFU,0xFAU,0xFDU,0xF4U,0xF3U,
    };

    uint8_t crc = 0x00;

    while (len > 0)
    {
        crc = table[*data ^ (uint8_t)crc];
        data++;
        len--;
    }
    return crc;
}

float byte_conversion::bytes_2_float(uint8_t const *bytes) {
    union u {
        float _f;
        uint8_t b[4];
    } u {};
    for (uint8_t i = 0; i < 4; i++) {
        u.b[i] = bytes[i];
    }
    return u._f;
}

void byte_conversion::float_2_bytes(float f, uint8_t *bytes) {
    union u {
        float _f;
        uint8_t b[4];
    } u {};
    u._f = f;
    for (uint8_t i = 0; i < 4; i++) {
        bytes[i] = u.b[i];
    }
}

int32_t byte_conversion::bytes_2_int(uint8_t const *bytes) {
    union u {
        int32_t _i;
        uint8_t b[4];
    } u {};
    for (uint8_t i = 0; i < 4; i++) {
        u.b[i] = bytes[i];
    }

    return u._i;
}

void byte_conversion::int_2_bytes(int32_t i, uint8_t *bytes) {
    union u {
        int32_t _i;
        uint8_t b[4];
    } u {};
    u._i = i;
    for (uint8_t i = 0; i < 4; i++) {
        bytes[i] = u.b[i];
    }
}