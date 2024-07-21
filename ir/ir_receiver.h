/*
                    ***** IR Receiver Class  *****
*/

#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include <stdint.h>
#include <pico/types.h>
#include <pico/time.h>
#include <pico/sync.h>

class IR_Receiver
{
public:
    /**
     * @brief   Callback for IR message received
     * 
     * @details Function is called from interrupt. Keep processing short.
     * 
     * @param   t           Timestamp of message received
     * @param   a           Address
     * @param   f           Function code
     * @param   obj         IR_Receiver object pointer
     */
    typedef void (*ir_rcv_callback)(uint64_t t, uint16_t a, uint16_t f, IR_Receiver *obj);

    /**
     * @brief   Callback for read timeout
     * 
     * @details Function is called from interrupt. Keep processing short.
     * 
     * @param   msg         true if message timeout, false if bit / pulse timeout
     * @param   n_pulse     Number of pulses read
     * @param   pulses      Pointer to array of pulse times
     * @param   obj         IR_Receiver object pointer
     * 
     * @return true if buffer to be processed, false if message failed
     */
    typedef bool (*ir_tmo_callback)(bool msg, uint32_t n_pulse, uint32_t const *pulses, IR_Receiver *obj);

    /**
     * @brief   Callback for error
     * 
     * @details Called if the decode function returns a failure or if a timeout
     *          occurs without a timeout callback defined
     * 
     * @param   obj         IR_Receiver object pointer
     */
    typedef void (*ir_error_callback)(IR_Receiver *obj);

protected:
    uint32_t            gpio_;          // GPIO number
    uint32_t            *pulses_;       // On / off times
    uint32_t            n_pulse_;       // Number of pulses
    uint32_t            mx_pulse_;      // Maimum number of pulses
    uint64_t            prev_ts_;       // Previous timestamp
    uint8_t             sync_;          // Sync pulse flag
    ir_rcv_callback     rcb_;           // Receive callback

    ir_rcv_callback     rpt_;           // Repeat callback
    uint16_t            rpt_addr_;      // Repeat address
    uint16_t            rpt_func_;      // Repeat function

    ir_tmo_callback     tmo_;           // Timeout callback
    alarm_id_t          msg_timer_;     // Message timer
    alarm_id_t          bit_timer_;     // Bit receive timeout
    uint32_t            msg_timeout_;   // Message timeout
    uint32_t            bit_timeout_;   // Bit timeout
    uint32_t            prev_count_;    // Previous bit (pulse) count

    ir_error_callback   err_;           // Error callback

    semaphore_t         sem_;           // Semaphore

    static IR_Receiver  **receivers_;   // List of receivers
    static uint32_t     n_rcvr_;        // Number of receivers
    static uint32_t     mx_rcvr_;       // Maximum number of receivers
    static alarm_pool_t *pool_;         // Timer pool

    static void gpio_cb(uint gpio, uint32_t evmask);
    static int64_t timeout_msg(alarm_id_t id, void *user_data);
    static int64_t timeout_bit(alarm_id_t id, void *user_data);

    void store_timestamp(uint64_t ts, bool falling);
    virtual void message_complete(uint64_t ts);
    void start_timeout();
    virtual bool timeout(bool msg);
    virtual void message_error();
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
     *          but otherwise not altered. Timeouts started when this
     *          functurn returns true and sync_ no longer zero.
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

    /**
     * @brief   Check bit timeout
     * 
     * @return true if bit timeout to be processed
     */
    virtual bool check_bit_timeout() { return true; }

    /**
     * @brief   Compare pulse time
     * 
     * @param   pulse       Pulse time
     * @param   nom_pulse   Nominal pulse for compare
     * @param   band        Band either side of numinal pulse
     */
    static inline bool compare_pulse(uint32_t pulse, uint32_t nom_pulse, uint32_t band)
                        {return pulse > nom_pulse - band && pulse < nom_pulse + band; }

public:
    /**
     * @brief   Constructor for IR Receiver class
     * 
     * @param   gpio        GPIO numver for reading input
     * @param   n_pulse     Number of values reserved for pulse times
     */
    IR_Receiver(uint32_t gpio, uint32_t n_pulse);

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

    /**
     * @brief   Set callback for IR repeat code received
     * 
     * @param   cb      Callback for repeat received. Address and code are from
     *                  most recent receive
     */
    void set_rpt_callback(ir_rcv_callback cb) { rpt_ = cb; }

    /**
     * @brief   Set callback for IR message timeout
     * 
     * @param   cb      Callback to receive timeout notification
     */
    void set_tmo_callback(ir_tmo_callback cb) { tmo_ = cb; }

    /**
     * @brief   Set callback for IR message error
     * 
     * @param   cb      Callback to receive error notification
     */
    void set_err_callback(ir_error_callback cb) { err_ = cb; }

    /**
     * @brief   Set message timeout
     * 
     * @param   tmo_msec    Message timeout in milliseconds
     */
    void set_message_timeout(uint32_t tmo_msec) { msg_timeout_ = tmo_msec; }

    /**
     * @brief   Set bit / pulse timeout
     * 
     * @param   tmo_msec    Bit / pulse timeout in milliseconds
     */
    void set_bit_timeout(uint32_t tmo_msec) { bit_timeout_ = tmo_msec; }

    /**
     * @brief   Wait for next message completion
     * 
     * @param   timeout_ms  Maimum time to wait (milliseconds)
     * 
     * @return  true if message complete, false if timed out waiting
     */
    bool wait_for_message(uint32_t timeout_ms=0);
};

#endif  