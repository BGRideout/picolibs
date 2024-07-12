/*
                    *****  IR Led Output class
*/

#ifndef IR_LED_H
#define IR_LED_H

#include "pwm.h"
#include <pico/time.h>

class IR_LED : public PWM
{
protected:
    uint32_t                base_pulse_;    // Base pulse (usec)
    uint32_t                *times_;        // On / off output durations
    uint32_t                a_times_;       // Allocated size of times array
    uint32_t                n_times_;       // Number of durations
    uint32_t                out_index_;     // Output time index
    float                   duty_;          // Active duty cycle
    uint8_t                 count_;         // Pulse counter

    static IR_LED           **leds_;        // List of transmitters
    static uint32_t         n_led_;         // Number of transmitters
    static uint32_t         mx_led_;        // Maximum number of transmitters

    static repeating_timer  timer_;         // Pulse timer

    static bool timer_cb(repeating_timer *timer);

    void set_next();

public:
    /**
     * @brief   Constructor for IR_LED class
     * 
     * @param   gpio        GPIO number for IR output
     * @param   freq        Pulse frequency (PWM)
     * @param   duty        PWM duty cycle when outputting
     * @param   base_pulse  Pulse width in microseconds for counter of 1
     * @param   a_times     Number of slots to preallocate for times
     */
    IR_LED(uint32_t gpio, uint32_t freq, float duty, uint32_t base_pulse, uint32_t a_times);

    /**
     * @brief   Destructor for IR_LED class
     */
    virtual ~IR_LED();

    /**
     * @brief   Set a series of output durations
     * 
     * @param   times   List of on/off times in microseconds (even on, odd off)
     * @param   n_times Number of time values
     */
    void setOutputTimes(const uint32_t *times, uint32_t n_times);

    /**
     * @brief   Start transmitting output pattern
     * 
     * @return  true if transmission started
     */
    bool transmit();

    /**
     * @brief   Stop active transmission
     */
    void stop();
};

#endif
