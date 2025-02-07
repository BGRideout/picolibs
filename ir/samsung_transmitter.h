/*
                    *****  Subclass of IR_LED for NEC protocol  *****
*/

#ifndef SAMSUNG_TRANSMITTER_H
#define SAMSUNG_TRANSMITTER_H

#include "ir_led.h"

#ifndef SAMSUNG_BASE_PULSE
#define SAMSUNG_BASE_PULSE 562
#endif
#ifndef SAMSUNG_MESSAGE_LEN
#define SAMSUNG_MESSAGE_LEN 67
#endif

class SAMSUNG_Transmitter : public IR_LED
{
private:

protected:

public:
    /**
     * @brief   SAMSUNG_Transmitter class constructor
     * 
     * @param   gpio    GPIO pin for transmit
     */
    SAMSUNG_Transmitter(uint32_t gpio) : IR_LED(gpio, 38000, 0.25, SAMSUNG_MESSAGE_LEN) {}

    /**
     * @brief   SAMSUNG_Transmitter destructor
     */
    virtual ~SAMSUNG_Transmitter() {}

    /**
     * @brief   Set times for an address / function pair
     * 
     * @param   addr    Address for IR command
     * @param   func    Function code for IR command
     */
    void setMessageTimes(uint16_t addr, uint16_t func) override;

    /**
     * @brief   Return the IR protocol
     * 
     * @return  Pointer to protocl name
     */
    virtual const char *protocol() const override { return "SAM"; }

    /**
     * @brief   Return the repeat interval in milliseconds
     * 
     * @return  repeat interval (msec)
     */
    virtual int repeatInterval() const override { return 110; }

};

#endif
