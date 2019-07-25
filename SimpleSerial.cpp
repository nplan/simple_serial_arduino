#include "SimpleSerial.h"

SimpleSerial::SimpleSerial(AvailableFunType availableFun, ReadFunType readFun, WriteFunType writeFun) {
    _availableFun = availableFun;
    _readFun = readFun;
    _writeFun = writeFun;
}

/*
 * Returns true if data is available.
 */
bool SimpleSerial::available(void) {
    if (_readQueueLen != 0)
        return true;
    else
        return false;
}

/*
 * Takes payload (in bytes) and its id, frames it into a packet, and places it
 * in send queue.
 */
void SimpleSerial::send(uint8_t id, uint8_t len, uint8_t payload[]) {
    // Do nothing if queue full
    if (_sendQueueLen >= QUEUE_LEN)
        return;
    // Frame packet
    uint8_t packet[MAX_PCKT_LEN];
    uint8_t packetLen;
    this->frame(id, len, payload, packetLen, packet);
    // Place packet in send queue
    uint8_t queueIndex = _sendQueueLen;
    for (uint8_t i = 0; i < packetLen; i++) {
        // Copy data to array
        _sendQueue[queueIndex][i] = packet[i];
    }
    _sendQueueLengths[queueIndex] = packetLen;
    _sendQueueLen++;
}

/*
 * Converts float to 4 bytes and sends using send().
 */
void SimpleSerial::sendFloat(uint8_t id, float f) {
    union u {
        float _f;
        uint8_t b[4];
    } u;
    u._f = f;
    send(id, 4, u.b);
}

/*
 * Converts int to 4 bytes and sends using send();
 */
void SimpleSerial::sendInt(uint8_t id, int32_t i) {
    union u {
        int _i;
        uint8_t b[4];
    } u;
    u._i = i;
    send(id, 4, u.b);
}

/*
 * Frames array of bytes into a packet. Inserts flag bytes, id and length.
 */
void SimpleSerial::frame(uint8_t id, uint8_t payloadLen, uint8_t payload[],
                         uint8_t &packetLen, uint8_t packet[]) {
    // Insert ESC flags
    uint8_t i = 0; // payload index
    uint8_t j = 0; // packet index
    packet[0] = START;
    packet[1] = 0; // packet length
    packet[2] = id;
    j = 3;
    while (i < payloadLen) {
        uint8_t b = payload[i];
        if (b == ESC || b == START || b == END) {
            // Must insert ESC flag
            packet[j] = ESC;
            j++;
        }
        packet[j] = b;
        j++;
        i++;
    }
    packet[j] = END;
    j++;
    packet[1] = j; //packet length
    packetLen = j;
}

/*
 * Continuously running loop. Sends the next packet in send queue.
 */
void SimpleSerial::sendLoop(uint32_t time) {
    if (_sendQueueLen < 1)
        // Return if nothing to send
        return;
        // Loop through waiting packets
    else {
        // Send packet
        uint8_t i = _sendQueueLen - 1;
        _writeFun(_sendQueue[i], _sendQueueLengths[i]);
        _sendQueueLen--;
    }
}

/*
 * Returns and removes the oldest packet in the read queue.
 */
void SimpleSerial::read(uint8_t &id, uint8_t &len, uint8_t payload[]) {
    id = _readQueueIDs[0];
    len = _readQueueLengths[0];
    // Copy from read queue
    for (uint8_t i = 0; i < len; i++) {
        payload[i] = _readQueue[0][i];
    }
    _readQueueLen--;
    // Shift read list forward.
    for (uint8_t i = 0; i < _readQueueLen; i++) {
        for (uint8_t j = 0; j < _readQueueLengths[i]; j++) {
            _readQueue[i][j] = _readQueue[i + 1][j];
        }
    }
}

float SimpleSerial::bytes2Float(uint8_t bytes[]) {
    union u {
        float _f;
        uint8_t b[4];
    } u;
    for (uint8_t i = 0; i < 4; i++) {
        u.b[i] = bytes[i];
    }
    return u._f;
}

void SimpleSerial::float2Bytes(float f, uint8_t bytes[]) {
    union u {
        float _f;
        uint8_t b[4];
    } u;
    u._f = f;
    for (uint8_t i = 0; i < 4; i++) {
        bytes[i] = u.b[i];
    }
}

int16_t SimpleSerial::bytes2Int(uint8_t bytes[]) {
    union u {
        int16_t _i;
        uint8_t b[4];
    } u;
    for (uint8_t i = 0; i < 4; i++) {
        u.b[i] = bytes[i];
    }

    return u._i;
}

void SimpleSerial::int2Bytes(int32_t i, uint8_t bytes[]) {
    union u {
        int16_t _i;
        uint8_t b[4];
    } u;
    u._i = i;
    for (uint8_t i = 0; i < 4; i++) {
        bytes[i] = u.b[i];
    }
}

/*
 * Continuously running loop. Waits for bytes as they arrive and
 * decodes the packet.
 */
void SimpleSerial::readLoop(uint32_t time) {
    if (!_availableFun())
        return;
    static uint8_t i = 0; // Byte counter
    static uint8_t l = 0; // Packet length
    static uint8_t id = 0; //Packet id
    static bool esc = false;
    static uint8_t payload[MAX_LEN_PYLD];
    static uint8_t payload_i = 0;
    static uint32_t startTime = time;

    uint8_t b = _readFun();
    if (i == 0 && b == START) {
        // First byte - START flag. Start count.
        startTime = time;
        i = 1;
        payload_i = 0;
    } else if (i == 1) {
        // Second byte - packet length
        l = b;
        i = 2;
    } else if (i == 2) {
        // Third byte - packet identifier
        id = b;
        i = 3;
    } else if (i > 2) {
        // Data value byte
        if (i > (2 * MAX_LEN_PYLD + 4) || (time - startTime) > PACKET_TIMEOUT) {
            // No END flag. Reset.
            i = 0;
            return;
        }
        if (!esc) {
            // No preceding ESC. Accept flags.
            if (b == ESC)
                // ESC flag. Activate ESC mode.
                esc = true;
            else if (b == END) {
                // End of packet. Check if specified and actual length are equal.
                if (i == l - 1) {
                    // Valid data. Add to read queue.
                    if (_readQueueLen >= QUEUE_LEN) {
                        // Queue full. Delete oldest value
                        // Shift read list forward.
                        for (uint8_t i = 0; i < _readQueueLen; i++) {
                            for (uint8_t j = 0; j < _readQueueLengths[i]; j++) {
                                _readQueue[i][j] = _readQueue[i + 1][j];
                            }
                        }
                        _readQueueLen--;
                    }
                    uint8_t queueIndex = _readQueueLen;
                    for (uint8_t i = 0; i < payload_i; i++) {
                        _readQueue[queueIndex][i] = payload[i];
                    }
                    _readQueueLengths[queueIndex] = payload_i;
                    _readQueueIDs[queueIndex] = id;
                    _readQueueLen++;
                    i = 0;
                } else {
                    // CORRUPTED data. Reset
                    i = 0;
                    esc = false;
                }
                return;
            } else {
                // Normal data byte. Add to array.
                payload[payload_i] = b;
                payload_i++;
            }
        } else {
            // ESC preceding. Ignore flag following ESC byte.
            payload[payload_i] = b;
            payload_i++;
            esc = false;
        }
        i++;
    }
}

void SimpleSerial::loop(uint32_t time) {
    sendLoop(time);
    readLoop(time);
}

void SimpleSerial::confirmReceived(uint8_t id) {
    uint8_t pld[] = {255};
    send(id, 1, pld);
}
