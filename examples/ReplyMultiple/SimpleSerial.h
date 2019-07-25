/*
 * SimpleSerial.h - Library for serial communication.
 */

#ifndef SimpleSerial_h
#define SimpleSerial_h

#include <stdint.h>
#include <stdbool.h>

typedef bool (*AvailableFunType)(void);
typedef uint8_t (*ReadFunType)(void); // tyepdef for read function
typedef int16_t (*WriteFunType)(uint8_t*, uint16_t); // typedef for write function

class SimpleSerial
{
public:
    SimpleSerial(AvailableFunType availableFun, ReadFunType readFun, WriteFunType writeFun);
    bool available(void);
    void read(uint8_t &id, uint8_t &len, uint8_t payload[]);
    void send(uint8_t id, uint8_t len, uint8_t payload[]);
    void sendFloat(uint8_t id, float f);
    void sendInt(uint8_t id, int32_t i);
    void readLoop(uint32_t time);
    void sendLoop(uint32_t time);
    void loop(uint32_t time);

    void confirmReceived(uint8_t id);

    static float bytes2Float(uint8_t bytes[]);
    static void float2Bytes(float f, uint8_t bytes[]);
    static int16_t bytes2Int(uint8_t bytes[]);
    static void int2Bytes(int32_t i, uint8_t bytes[]);

    static const uint16_t MAX_LEN_PYLD = 32;                // Max payload length
    static const uint16_t MAX_PCKT_LEN = 2 * MAX_LEN_PYLD;  // Maximum frame length
    static const uint16_t PACKET_TIMEOUT = 1000;            // Packet receive timeout
    static const uint16_t QUEUE_LEN = 50;                   // Send / receive queue length

    static const uint8_t ESC = 1;
    static const uint8_t START = 2;
    static const uint8_t END = 3;

private:
    AvailableFunType _availableFun;
    ReadFunType _readFun;
    WriteFunType _writeFun;
    void frame(uint8_t id, uint8_t payloadLen, uint8_t payload[],
               uint8_t &packetLen, uint8_t packet[]);
    uint8_t _sendQueue[QUEUE_LEN][MAX_PCKT_LEN];
    uint8_t _sendQueueLengths[QUEUE_LEN];
    uint8_t _sendQueueLen = 0;
    uint8_t _readQueue[QUEUE_LEN][MAX_LEN_PYLD];
    uint8_t _readQueueLengths[QUEUE_LEN];
    uint8_t _readQueueIDs[QUEUE_LEN];
    uint8_t _readQueueLen = 0;

};

#endif
