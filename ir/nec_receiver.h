/*
                    *****  Subclass of IR_Receiver for NEC protocol  *****
*/

#ifndef NEC_RECEIVER
#define NEC_RECEIVER

#include "ir_receiver.h"
#include <stdio.h>

class NEC_Receiver : public IR_Receiver
{
private:

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
    NEC_Receiver(uint32_t gpio, uint32_t address=0xffff) : IR_Receiver(gpio, address, 562, 65) {}
    ~NEC_Receiver() {}
};

#endif
