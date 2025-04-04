/*
                    *****  Subclass of IR_Receiver for NEC protocol  *****
*/

#ifndef SAMSUNG_RECEIVER
#define SAMSUNG_RECEIVER

#include "ir_receiver.h"

#ifndef SAMSUNG_BASE_PULSE
#define SAMSUNG_BASE_PULSE 562
#endif
#ifndef SAMSUNG_MESSAGE_LEN
#define SAMSUNG_MESSAGE_LEN 67
#endif

class SAMSUNG_Receiver : public IR_Receiver
{
private:
    uint32_t    address_;           // Address to which object responds (0xffff = all)

protected:
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
    bool check_sync(uint64_t ts, bool falling) override;

    /**
     * @brief   Decode received message
     * 
     * @param   addr    Word to receive address
     * @param   func    Word to receive function code
     */
    bool decode_message(uint16_t &addr, uint16_t &func) override;

    /**
     * @brief   Check bit timeout
     * 
     * @return true if bit timeout to be processed
     */
    bool check_bit_timeout() override;

public:
    /**
     * @brief   Constructor for IR Receiver class
     * 
     * @param   gpio        GPIO numver for reading input
     * @param   address     Address to which this receiver responds (0xffff = all)
     */
    SAMSUNG_Receiver(uint32_t gpio, uint32_t address=0xffff) : IR_Receiver(gpio, SAMSUNG_MESSAGE_LEN), address_(address) {}
    ~SAMSUNG_Receiver() {}

    /**
     * @brief   Decode series of pulses
     * 
     * @param   pulses      Pointer to vector of pulse times
     * @param   n_pulse     Number of pulses
     * @param   addr        Variable to receive address
     * @param   func        Variable to receive function
     * @param   address     Address to match (0xffff = all)
     * 
     * @return  true if successful
     */
    static bool decode(uint32_t const *pulses, uint32_t n_pulse, uint16_t &addr, uint16_t &func, uint16_t address=0xffff);
};

#endif
