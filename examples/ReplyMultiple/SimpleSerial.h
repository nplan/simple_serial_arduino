/*
 * SimpleSerial.h - Library for serial communication.
 */

#ifndef SimpleSerial_h
#define SimpleSerial_h

#include <stdint.h>
#include <stdbool.h>

const uint16_t MAX_LEN_PYLD = 8;                // Max payload length
const uint16_t MAX_PCKT_LEN = 2 * MAX_LEN_PYLD; // Maximum frame length
const uint16_t PACKET_TIMEOUT = 1000;           // Packet receive timeout
const uint8_t QUEUE_LEN = 5;                    // Send / receive queue length
const uint8_t SEND_BYTES = 4;                   // Number of bytes to send in each send loop

const uint8_t ESC = 1;
const uint8_t START = 2;
const uint8_t END = 3;

typedef uint8_t (*ReadFunType)(void); // tyepdef for read function
typedef uint8_t (*WriteFunType)(uint8_t*, uint8_t); // typedef for write function
typedef bool (*AvailableFunType)(void);

class SimpleSerial
{
  public:
    SimpleSerial(AvailableFunType availableFun, ReadFunType readFun, WriteFunType writeFun);
    bool available(void);
    void read(uint8_t &id, uint8_t &len, uint8_t bytearray[]);
    float bytes2Float(uint8_t len, uint8_t bytes[]);
    void float2Bytes(float f, uint8_t bytes[]);
    int16_t bytes2Int(uint8_t len, uint8_t bytes[]);
    void int2Bytes(int32_t i, uint8_t bytes[]);
    void send(uint8_t id, uint8_t bytesLen, uint8_t bytes[]);
    void sendFloat(uint8_t id, float f);
    void sendInt(uint8_t id, int32_t i);
    void readLoop(uint32_t time);
    void sendLoop(uint32_t time);
    void loop(uint32_t time);
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
