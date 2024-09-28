/*
                    *****  Subclass of IR_Receiver for Sony protocols  *****
*/

#ifndef SONY_RECEIVER
#define SONY_RECEIVER

#include "ir_receiver.h"

#ifndef SONY_BASE_PULSE
#define SONY_BASE_PULSE 600
#endif

class Sony_Receiver : public IR_Receiver
{
private:
    uint32_t    address_size_;      // Address size
    uint32_t    value_size_;        // Value size
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
    Sony_Receiver(uint32_t gpio, uint32_t address_size, uint32_t value_size, uint32_t address=0xffff)
     : IR_Receiver(gpio, (address_size + value_size) * 2 + 1), address_size_(address_size), value_size_(value_size), address_(address) {}
    ~Sony_Receiver() {}

    /**
     * @brief   Decode series of pulses
     * 
     * @param   pulses      Pointer to vector of pulse times
     * @param   n_pulse     Number of pulses
     * @param   addr        Variable to receive address
     * @param   func        Variable to receive function
     * @param   address     Address to match (0xffff = all)
     * @param   addr_size   Address size
     * @param   value_size  Value size
     * 
     * @return  true if successful
     */
    static bool decode(uint32_t const *pulses, uint32_t n_pulse, uint16_t &addr, uint16_t &func,
                       uint16_t address, uint32_t addr_size, uint32_t value_size);
};


class Sony12_Receiver : public Sony_Receiver
{
public:
    Sony12_Receiver(uint32_t gpio, uint32_t address=0xffff) : Sony_Receiver(gpio, 5, 7, address) {}

    static bool decode(uint32_t const *pulses, uint32_t n_pulse, uint16_t &addr, uint16_t &func, uint16_t address=0xffff)
        { return Sony_Receiver::decode(pulses, n_pulse, addr, func, address, 5, 7); } 
};


class Sony15_Receiver : public Sony_Receiver
{
public:
    Sony15_Receiver(uint32_t gpio, uint32_t address=0xffff) : Sony_Receiver(gpio, 8, 7, address) {}

    static bool decode(uint32_t const *pulses, uint32_t n_pulse, uint16_t &addr, uint16_t &func, uint16_t address=0xffff)
        { return Sony_Receiver::decode(pulses, n_pulse, addr, func, address, 8, 7); } 
};

#endif
