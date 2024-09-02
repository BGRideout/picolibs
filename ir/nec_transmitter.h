/*
                    *****  Subclass of IR_LED for NEC protocol  *****
*/

#ifndef NEC_TRANSMITTER_H
#define NEC_TRANSMITTER_H

#include "ir_led.h"

#ifndef NEC_BASE_PULSE
#define NEC_BASE_PULSE 562
#endif
#ifndef NEC_MESSAGE_LEN
#define NEC_MESSAGE_LEN 67
#endif

class NEC_Transmitter : public IR_LED
{
private:

protected:

public:
    /**
     * @brief   NEC_Transmitter class constructor
     * 
     * @param   gpio    GPIO pin for transmit
     */
    NEC_Transmitter(uint32_t gpio) : IR_LED(gpio, 38000, 0.25, NEC_MESSAGE_LEN) {}

    /**
     * @brief   NEC_Transmitter destructor
     */
    virtual ~NEC_Transmitter() {}

    /**
     * @brief   Set times for an address / function pair
     * 
     * @param   addr    Address for IR command
     * @param   func    Function code for IR command
     */
    void setMessageTimes(uint16_t addr, uint16_t func);

};

#endif
