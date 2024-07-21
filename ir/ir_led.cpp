/*
                    *****  IR_LED Class Implementation  *****
*/

#include "ir_led.h"
#include <pico/time.h>
#include <hardware/timer.h>
#include <hardware/irq.h>
#include <stdio.h>

#ifndef MAX_SIM_IR_LED
#define MAX_SIM_IR_LED  1       // Maximim simultaneous outputting LED's
#endif

uint16_t        IR_LED::nled_ = 0;
alarm_pool_t    *IR_LED::pool_ = nullptr;

IR_LED::IR_LED(uint32_t gpio, uint32_t freq, float duty, uint32_t a_times)
 : PWM(gpio, freq), times_(nullptr), a_times_(a_times), n_times_(0), out_index_(0), timer_(-1), duty_(duty)
{
    if (nled_ == 0)
    {
        pool_ = alarm_pool_create_with_unused_hardware_alarm(MAX_SIM_IR_LED);
        uint alarm_num = alarm_pool_hardware_alarm_num(pool_);
        irq_set_priority(TIMER_IRQ_0 + alarm_num, PICO_HIGHEST_IRQ_PRIORITY);
    }
    ++nled_;
    times_ = new uint32_t[a_times];
}

IR_LED::~IR_LED()
{
    if (timer_ != -1)
    {
        cancel_alarm(timer_);
    }
    delete [] times_;
    if(--nled_ == 0)
    {
        uint alarm_num = alarm_pool_hardware_alarm_num(pool_);
        irq_set_priority(TIMER_IRQ_0 + alarm_num, PICO_DEFAULT_IRQ_PRIORITY);
        alarm_pool_destroy(pool_);
        pool_ = nullptr;
    }
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
            times_[ii] = times[ii];
        }
    }
}

bool IR_LED::transmit()
{
    bool ret = true;
    stop();
    if (n_times_ > 0)
    {
        out_index_ = 0;
        timer_ = alarm_pool_add_alarm_in_us(pool_, 10, timer_cb, this, true);
    }
    return ret;
}

void IR_LED::stop()
{
    if (timer_ != -1)
    {
        alarm_pool_cancel_alarm(pool_, timer_);
        timer_ = -1;
    }
    out_index_ = 0;
    setDutyCycle(0.0);
}

int64_t IR_LED::timer_cb(alarm_id_t id, void *user_data)
{
    IR_LED *self = static_cast<IR_LED *>(user_data);
    return -self->set_next();
}

int64_t IR_LED::set_next()
{
    int64_t ret = 0;
    if (out_index_ < n_times_)
    {
        setDutyCycle((out_index_ & 1) == 0 ? duty_ : 0.0);
        ret = times_[out_index_++];
    }
    if (ret == 0)
    {
        timer_ = -1;
        setDutyCycle(0.0);
    }
    return ret;
}
