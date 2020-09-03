/*
 * SimpleSerial.h - Library for serial communication.
 */

#ifndef SimpleSerial_h
#define SimpleSerial_h

#include <stdint.h>
#include <string.h>
#include "SimpleQueue.h"


class SimpleSerial {
public:
    const uint16_t max_payload_len_;
    const uint16_t max_frame_len_ = 2 * max_payload_len_ + 4; // Maximum frame length

    const uint16_t receive_timeout;   // Packet receive timeout
    const uint8_t read_num_bytes;    // number of bytes to read in single readLoop()

    const uint8_t esc_flag = 1;
    const uint8_t start_flag = 2;
    const uint8_t end_flag = 3;

    // Packet structure
    struct Packet {
        uint8_t id;
        uint8_t payload_len;
        uint8_t *payload;
    public:
        Packet()
            : id(0)
            , payload_len(0)
            , payload(new uint8_t[1])
            {payload[0] = 0;}
        Packet(uint8_t id, uint8_t payload_len)
                : id(id)
                , payload_len(payload_len)
                , payload(new uint8_t[payload_len])
        {}
        Packet(uint8_t id, uint8_t payload_len, const uint8_t *payload)
            : id(id)
            , payload_len(payload_len)
            , payload(new uint8_t[payload_len])
            {memcpy(this->payload, payload, payload_len);}
        ~Packet() {delete [] payload;}
        Packet(const Packet &old_packet) {
            id = old_packet.id;
            payload_len = old_packet.payload_len;
            payload = new uint8_t[payload_len];
            memcpy(payload, old_packet.payload, payload_len);
        }
        Packet(Packet&& old_packet) {
            id = old_packet.id;
            payload_len = old_packet.payload_len;
            payload = old_packet.payload;
            old_packet.payload = nullptr;
        }
        Packet& operator=(const Packet& rhs) {
            if (this == &rhs)
                return *this;
            id = rhs.id;
            payload_len = rhs.payload_len;
            delete[] payload;
            payload = new uint8_t[payload_len];
            void * dest = memcpy(payload, rhs.payload, payload_len);
            return *this;
        }
    };

    /*
    * Constructor. Serial interface can by any object that has these 3 functions:
    *   virtual uint8_t available() = 0;
    *   virtual uint8_t read() = 0;
    *   virtual uint8_t write(uint8_t b[], uint8_t len) = 0;
    * 
    * Other arguments are optional.
    * */
    template <class T>
    explicit SimpleSerial(T* serial, // serial interface.
            uint16_t max_payload_len = 16, // max payload len
            uint16_t max_queue_len = 8, // max send/receive queue len
            unsigned long (*time_getter)() = nullptr, // function that returns system time in ms. like millis().
            const uint16_t receive_timeout = 500, // [ms] time after which packet is discarded if transmission stops
            const uint8_t read_num_bytes = 4, // number of bytes to read from serial interface in one loop
            const uint8_t esc_flag = 1,
            const uint8_t start_flag = 2,
            const uint8_t end_flag = 3)
                : serial_(new SerialModel<T>(serial))
                , max_payload_len_(max_payload_len)
                , time_getter(time_getter)
                , receive_timeout(receive_timeout)
                , read_num_bytes(read_num_bytes)
                , esc_flag(esc_flag)
                , start_flag(start_flag)
                , end_flag(end_flag)
                , incoming_payload_(new uint8_t[max_payload_len])
                , receive_queue(max_queue_len)
                , send_queue(max_queue_len)
            {};
    SimpleSerial(const SimpleSerial&) = delete; // delete copy constructor
    ~SimpleSerial() {
        delete [] incoming_payload_;
    };

    // Returns true if packets are available to read
    bool available();

    // Return a packet from receive queue
    Packet read();

    // Send packet with id, length and payload array
    void send(uint8_t id, uint8_t len, uint8_t const payload[]);

    // Send float
    void send_float(uint8_t id, float f);

    // Send int
    void send_int(uint8_t id, int32_t i);

    // Handler loop. Must be called periodically from main program.
    void loop();

    // Sends "ok" as payload
    void confirm_received(uint8_t id);

private:
    // Using type erasure pattern for serial interface
    class SerialConcept {
    public:
        virtual uint8_t available() = 0;
        virtual uint8_t read() = 0;
        virtual uint8_t write(uint8_t b[], uint8_t len) = 0;
    };

    template <class T>
    class SerialModel : public SerialConcept {
    public:
        explicit SerialModel(T* serial) : serial_(serial) {};
        uint8_t available() override { return serial_->available(); };
        uint8_t read() override { return serial_->read(); };
        uint8_t write(uint8_t b[], uint8_t len) override { return serial_->write(b, len);} ;
    private:
        T* serial_;
    };

    SerialConcept* serial_;

    // Frame structure
    struct Frame {
        uint8_t len;
        uint8_t *data;
    public:
        Frame()
            : len(0)
            , data(new uint8_t[1])
            {data[0] = 0;}
        explicit Frame(uint8_t len)
                : len(len)
                , data(new uint8_t[len])
        {}
        Frame(uint8_t len, const uint8_t *data)
            : len(len)
            , data(new uint8_t[len])
            {memcpy(this->data, data, len);}

        ~Frame() {delete [] data;}
        Frame(const Frame& old_frame) {
            len = old_frame.len;
            data = new uint8_t[len];
            memcpy(data, old_frame.data, len);
        }
        Frame(Frame&& old_frame) {
            len = old_frame.len;
            data = old_frame.data;
            old_frame.data = nullptr;
        }
        Frame& operator=(const Frame& rhs){
            if (this == &rhs)
                return *this;
            len = rhs.len;
            delete[] data;
            data = new uint8_t[rhs.len];
            memcpy(data, rhs.data, len);
            return *this;
        }
    };

    // Send / receive queues
    SimpleQueue<Frame> send_queue;
    SimpleQueue<Packet> receive_queue;

    static uint8_t calc_CRC(uint8_t *data, uint8_t len);
    Frame build_frame(Packet packet);

    void read_loop();
    void send_loop();

    uint8_t *incoming_payload_;

    unsigned long (*time_getter)() = nullptr;
    uint32_t sys_time();

};

// Conversions between bytes and int, folat
namespace byte_conversion {
    float bytes_2_float(uint8_t const *bytes);
    void float_2_bytes(float f, uint8_t *bytes);
    int32_t bytes_2_int(uint8_t const *bytes);
    void int_2_bytes(int32_t i, uint8_t *bytes);
}


#endif
