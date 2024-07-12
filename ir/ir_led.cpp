/*
                    *****  IR_LED Class Implementation  *****
*/

#include "ir_led.h"
#include <pico/time.h>
#include <hardware/timer.h>
#include <stdio.h>

IR_LED          **IR_LED::leds_ = nullptr;
uint32_t        IR_LED::n_led_ = 0;
uint32_t        IR_LED::mx_led_ = 4;
repeating_timer IR_LED::timer_;

IR_LED::IR_LED(uint32_t gpio, uint32_t freq, float duty, uint32_t base_pulse, uint32_t a_times)
 : PWM(gpio, freq), base_pulse_(base_pulse), times_(nullptr), a_times_(a_times), n_times_(0), out_index_(0), duty_(duty), count_(0)
{
    //  Allocate list of LED's on first object creation
    if (n_led_ == 0)
    {
        leds_ = new IR_LED *[mx_led_];
        add_repeating_timer_us(-(uint64_t)base_pulse_, timer_cb, nullptr, &timer_);
    }
    if (n_led_ < mx_led_)
    {
        leds_[n_led_++] = this;
    }
    else
    {
        fprintf(stderr, "IR_LED - Too many LED's defined! Maximum = %d\n", mx_led_);
    }

    times_ = new uint32_t[a_times];
}

IR_LED::~IR_LED()
{
    n_led_ -= 1;
    int jj = 0;
    for (int ii = 0; ii < n_led_; ii++)
    {
        if (leds_[ii] == this)
        {
            jj += 1;
        }
        if (jj != ii)
        {
            leds_[ii] = leds_[jj];
        }
    }
    if (n_led_ == 0)
    {
        delete [] leds_;
        leds_ = nullptr;
        cancel_repeating_timer(&timer_);
    }
    delete [] times_;
}

void IR_LED::setOutputTimes(const uint32_t *times, uint32_t n_times)
{
    stop();
    if (n_times > a_times_)
    {
        delete [] times_;
        a_times_ = n_times + 16;
        times_ = new uint32_t[a_times_];
    }

    n_times_ = n_times;
    if (n_times_ > 0)
    {
        for (int ii = 0; ii < n_times_; ii++)
        {
            times_[ii] = times[ii] / base_pulse_;
        }
    }
}

bool IR_LED::transmit()
{
    bool ret = true;
    stop();
    if (n_times_ > 0)
    {
        count_ = 1;
    }
    return ret;
}

void IR_LED::stop()
{
    count_ = 0;
    out_index_ = 0;
    setDutyCycle(0.0);
}

bool IR_LED::timer_cb(repeating_timer *timer)
{
    for (int ii = 0; ii < n_led_; ii++)
    {
        leds_[ii]->set_next();
    }
    return true;
}

void IR_LED::set_next()
{
    if (count_ > 0)
    {
        if (--count_ == 0)
        {
            if (out_index_ < n_times_)
            {
                setDutyCycle((out_index_ & 1) == 0 ? duty_ : 0.0);
                count_ = times_[out_index_++];
            }
            if (count_ == 0)
            {
                setDutyCycle(0.0);
            }
        }
    }
}
