/*
                    *****  Subclass of IR_LED for Sony protocols  *****
*/

#ifndef SONY_TRANSMITTER_H
#define SONY_TRANSMITTER_H

#include "ir_led.h"

#ifndef SONY_BASE_PULSE
#define SONY_BASE_PULSE 600
#endif

class Sony_Transmitter : public IR_LED
{
private:

protected:

public:
    /**
     * @brief   Sony_Transmitter base class constructor
     * 
     * @param   gpio    GPIO pin for transmit
     * @param   msglen  Message length
     */
    Sony_Transmitter(uint32_t gpio, uint32_t msglen) : IR_LED(gpio, 40000, 0.25, msglen) {}

    /**
     * @brief   Sony_Transmitter destructor
     */
    virtual ~Sony_Transmitter() {}

    /**
     * @brief   Set times for an address / function pair
     * 
     * @param   addr    Address for IR command
     * @param   func    Function code for IR command
     */
    virtual void setMessageTimes(uint16_t addr, uint16_t func) override;

    /**
     * @brief   Return the IR protocol
     * 
     * @return  Pointer to protocl name
     */
    virtual const char *protocol() const override { return "Sony"; }

    /**
     * @brief   Return the repeat interval in milliseconds
     * 
     * @return  repeat interval (msec)
     */
    virtual int repeatInterval() const override { return 45; }

    /**
     * @brief   Return the minimum numver of repetitions of the message
     * 
     * @return  number of additional repetitions of the message on transmit
     */
    virtual int minimum_repeats() const { return 2; }

    /**
     * @brief   Return the number of bits in the address
     * 
     * @return  number of address bits
     */
    virtual int address_size() const = 0;

    /**
     * @brief   Return the number of bits in the command
     * 
     * @return  number of command bits
     */
    virtual int command_size() const = 0;
};

class Sony12_Transmitter : public Sony_Transmitter
{
public:
    Sony12_Transmitter(uint32_t gpio) : Sony_Transmitter(gpio, (5 + 7) * 2 + 1) {}
    virtual ~Sony12_Transmitter() {}
    virtual const char *protocol() const override { return "Sony12"; }
    virtual int address_size() const { return 5; };
    virtual int command_size() const { return 7; };
};

class Sony15_Transmitter : public Sony_Transmitter
{
public:
    Sony15_Transmitter(uint32_t gpio) : Sony_Transmitter(gpio, (8 + 7) * 2 + 1) {}
    virtual ~Sony15_Transmitter() {}
    virtual const char *protocol() const override { return "Sony15"; }
    virtual int address_size() const { return 8; };
    virtual int command_size() const { return 7; };
};

#endif
