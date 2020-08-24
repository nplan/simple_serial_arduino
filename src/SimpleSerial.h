/*
 * SimpleSerial.h - Library for serial communication.
 */

#ifndef SimpleSerial_h
#define SimpleSerial_h

#include <stdint.h>
#include "Queue.h"

#define MAX_PAYLOAD_LEN 16
#define MAX_QUEUE_LEN   8


class SimpleSerial {
public:
    static const uint16_t max_payload_len = MAX_PAYLOAD_LEN;
    static const uint16_t max_frame_len = 2 * MAX_PAYLOAD_LEN + 4; // Maximum frame length
    static const uint16_t max_rec_q_len = MAX_QUEUE_LEN; // Receive queue length
    static const uint16_t max_send_q_len = MAX_QUEUE_LEN; // Send queue length

    const uint16_t receive_timeout;   // Packet receive timeout
    const uint8_t read_num_bytes;    // number of bytes to read in single readLoop()

    const uint8_t esc_flag = 1;
    const uint8_t start_flag = 2;
    const uint8_t end_flag = 3;

    // Packet structure
    struct Packet {
        uint8_t id;
        uint8_t payload_len;
        uint8_t payload[max_payload_len];
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
    explicit SimpleSerial(T* serial, //< serial interface.
            unsigned long (*time_getter)() = nullptr, // function that returns system time in ms. like millis().
            const uint16_t receive_timeout = 1000, // [ms] time after which packet is discarded if transmission stops 
            const uint8_t read_num_bytes = 8, // number of bytes to read from serial interface in one loop
            const uint8_t esc_flag = 1,
            const uint8_t start_flag = 2,
            const uint8_t end_flag = 3)
                : serial_(new SerialModel<T>(serial))
                , time_getter(time_getter)
                , receive_timeout(receive_timeout)
                , read_num_bytes(read_num_bytes)
                , esc_flag(esc_flag)
                , start_flag(start_flag)
                , end_flag(end_flag)
            {};
    SimpleSerial(const SimpleSerial&) = delete; // delete copy constructor

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

    // Conversions between bytes and int, folat
    static float bytes_2_float(uint8_t const *bytes);
    static void float_2_bytes(float f, uint8_t *bytes);
    static int16_t bytes_2_int(uint8_t const *bytes);
    static void int_2_bytes(int32_t i, uint8_t *bytes);

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
        uint8_t data[max_frame_len];
    };

    // Send / receive queues
    Queue<Frame, max_send_q_len> send_queue;
    Queue<Packet, max_rec_q_len> receive_queue;

    static uint8_t calc_CRC(uint8_t *data, uint8_t len);
    Frame build_frame(Packet packet);

    void read_loop();
    void send_loop();


    unsigned long (*time_getter)() = nullptr;
    uint32_t sys_time();

};

#endif
