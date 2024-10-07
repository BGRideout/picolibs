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
    int                     repeat_;        // Repeat count

    static uint16_t         nled_;          // Number of LED's defined
    static alarm_pool_t     *pool_;         // Alarm pool
    static int64_t timer_cb(alarm_id_t id, void *user_data);
    int64_t set_next();

    void                    *user_data_;    // User callback data
    void (*done_cb_)(IR_LED *led, void *user_data);

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
    virtual bool transmit();

    /**
     * @brief   Send repeat message
     * 
     * @return  true if transmission started
     */
    virtual bool repeat();

    /**
     * @brief   Stop active transmission
     */
    void stop();

    /**
     * @brief   Set callback for completion of send
     * 
     * @param   done_cb     Pointer to function to be called on completion
     * @param   user_data   Pointer to be passed to callback function
     */
    void setDoneCallback(void (*done_cb)(IR_LED *led, void *user_data), void *user_data) { done_cb_ = done_cb; user_data_ = user_data; }

    /**
     * @brief   Set times for an address / function pair
     * 
     * @param   addr    Address for IR command
     * @param   func    Function code for IR command
     */
    virtual void setMessageTimes(uint16_t addr, uint16_t func) {}

    /**
     * @brief   Return the IR protocol
     * 
     * @return  Pointer to protocl name
     */
    virtual const char *protocol() const { return ""; }

    /**
     * @brief   Return the repeat interval in milliseconds
     * 
     * @return  repeat interval (msec)
     */
    virtual int repeatInterval() const { return 0; }

    /**
     * @brief   Return the minimum numver of repetitions of the message
     * 
     * @return  number of additional repetitions of the message on transmit
     */
    virtual int minimum_repeats() const { return 0; }
};

#endif
