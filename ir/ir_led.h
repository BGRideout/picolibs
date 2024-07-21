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
    uint32_t                *times_;        // On / off output durations
    uint32_t                a_times_;       // Allocated size of times array
    uint32_t                n_times_;       // Number of durations
    uint32_t                out_index_;     // Output time index
    alarm_id_t              timer_;         // Timer
    float                   duty_;          // Active duty cycle

    static uint16_t         nled_;          // Number of LED's defined
    static alarm_pool_t     *pool_;         // Alarm pool
    static int64_t timer_cb(alarm_id_t id, void *user_data);
    int64_t set_next();

public:
    /**
     * @brief   Constructor for IR_LED class
     * 
     * @param   gpio        GPIO number for IR output
     * @param   freq        Pulse frequency (PWM)
     * @param   duty        PWM duty cycle when outputting
     * @param   a_times     Number of slots to preallocate for times
     */
    IR_LED(uint32_t gpio, uint32_t freq, float duty, uint32_t a_times);

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
