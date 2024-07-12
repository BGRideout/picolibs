/*
                    *****  PWM Class  *****

*/

#ifndef PWM_H
#define PWM_H

#include <stdint.h>

class PWM
{
private:
    uint32_t            gpio_;          // GPIO number
    float               div_;           // Frequency divider
    uint16_t            wrap_;          // Wrap value

public:
    /**
     * @brief   Constructor for PWM output
     * 
     * @param   gpio        GPIO number for output
     * @param   freq        Fequency in Hz
     * @param   duty        Duty cycle (0.0 to 1.0)
     */
    PWM(uint32_t gpio, uint32_t freq, float duty=0.0);

    /**
     * @brief   Destructor for PWM
     */
    virtual ~PWM();

    /**
     * @brief   Set frequency of PWM output
     * 
     * @param   freq        Frequency in Hz
     * 
     * @return true if frequency set, false if out of range
     */
    bool setFrequency(uint32_t freq);

    /**
     * @brief   Set duty cycle for PWM output
     * 
     * @param   duty        Dury cycle (0.0 to 1.0)
     * 
     * @return PWM level count
     */
    uint16_t setDutyCycle(float duty);

    uint32_t freq() const;
    float div() const { return div_; }
    uint32_t wrap() const { return wrap_; }
    uint32_t gpio() const { return gpio_; }
};

#endif