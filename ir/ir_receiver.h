/*
                    ***** IR Receiver Class  *****
*/

#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include <stdint.h>
#include <pico/types.h>

class IR_Receiver
{
public:
    /**
     * @brief   Callback for IR message received
     * 
     * @param   t   Timestamp of message received
     * @param   a   Address
     * @param   f   Function code
     */
    typedef void (*ir_rcv_callback)(uint64_t t, uint16_t a, uint16_t f);

protected:
    uint32_t            gpio_;          // GPIO number
    uint64_t            *pulses_;       // On / off times
    uint32_t            n_pulse_;       // Number of pulses
    uint32_t            mx_pulse_;      // Maimum number of pulses
    uint32_t            base_pulse_;    // Base pulse time
    uint16_t            address_;       // Receiver address
    uint8_t             sync_;          // Sync pulse flag
    ir_rcv_callback     rcb_;           // Receive callback

    static IR_Receiver  **receivers_;   // List of receivers
    static uint32_t     n_rcvr_;        // Number of receivers
    static uint32_t     mx_rcvr_;       // Maimum number of receivers

    static void gpio_cb(uint gpio, uint32_t evmask);

    void store_timestamp(uint64_t ts, bool falling);
    void message_complete(uint64_t ts);
    void message_error();
    void reset();

    /*
        Virtual functions for protocol implmentation.

        check_sync looks for message start
        decode_message gets message from pulse times
    */

    /**
     * @brief   Check for message start sync sequence
     * 
     * @details Can use the sync_ counter to determine when sync occurs.
     *          sync_ set to zero by base class before start of new message
     *          but otherwise not altered.
     * 
     * @param   ts      Timestamp of edge
     * @param   falling True if current edge is falling
     * 
     * @return  true if sync sequence received
     */
    virtual bool check_sync(uint64_t ts, bool falling) = 0;

    /**
     * @brief   Decode received message
     * 
     * @param   addr    Word to receive address
     * @param   func    Word to receive function code
     * 
     * @return  true if successful decode
     */
    virtual bool decode_message(uint16_t &addr, uint16_t &func) = 0;

public:
    /**
     * @brief   Constructor for IR Receiver class
     * 
     * @param   gpio        GPIO numver for reading input
     * @param   address     Address to which this receiver responds (0xffff = all)
     * @param   base_pulse  Base pulse time in microseconds
     * @param   n_pulse     Number of values reserved for pulse times
     */
    IR_Receiver(uint32_t gpio, uint16_t address, uint32_t base_pulse, std::size_t n_pulse);

    /**
     * @brief   Destructor for IR Receiver class
     */
    virtual ~IR_Receiver();

    /**
     * @brief   Getter for GPIO number for receiver
     */
    uint32_t gpio() const { return gpio_; }

    /**
     * @brief   Set callback for IR message received
     * 
     * @param   cb      Callback to receive address and function code
     */
    void set_rcv_callback(ir_rcv_callback cb) { rcb_ = cb; }
};

#endif  