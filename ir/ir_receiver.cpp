/*
                    *****  IR Receiver Class Implementation
*/

#include "ir_receiver.h"
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/timer.h>
#include <stdio.h>

IR_Receiver **IR_Receiver::receivers_ = nullptr;
uint32_t    IR_Receiver::n_rcvr_ = 0;
uint32_t    IR_Receiver::mx_rcvr_ = 4;

IR_Receiver::IR_Receiver(uint32_t gpio, uint16_t address, uint32_t base_pulse, std::size_t n_pulse)
 : gpio_(gpio), n_pulse_(0), mx_pulse_(n_pulse), base_pulse_(base_pulse), address_(address), sync_(0)
{
    if (receivers_ == nullptr)
    {
        receivers_ = new IR_Receiver *[mx_rcvr_];
        n_rcvr_ = 0;
    }
    if (n_rcvr_ < mx_rcvr_)
    {
        receivers_[n_rcvr_++] = this;
    }
    else
    {
        fprintf(stderr, "IR_Receiver - Too many receiverss defined! Maximum = %d\n", mx_rcvr_);
    }

    pulses_ = new uint64_t[mx_pulse_];

    gpio_init(gpio_);
    gpio_set_dir(gpio_, GPIO_IN);
    gpio_disable_pulls(gpio_);
    if (n_rcvr_ == 1)
    {
        gpio_set_irq_enabled_with_callback(gpio_, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, gpio_cb);
    }
    else
    {
        gpio_set_irq_enabled(gpio_, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true);
    }
}

IR_Receiver::~IR_Receiver()
{
    gpio_set_irq_enabled(gpio_, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false);
    gpio_deinit(gpio_);
    n_rcvr_ -= 1;
    int jj = 0;
    for (int ii = 0; ii < n_rcvr_; ii++)
    {
        if (receivers_[ii] == this)
        {
            jj += 1;
        }
        if (jj != ii)
        {
            receivers_[ii] = receivers_[jj];
        }
    }
    if (n_rcvr_ == 0)
    {
        delete [] receivers_;
        receivers_ = nullptr;
    }
    delete [] pulses_;
}

void IR_Receiver::gpio_cb(uint gpio, uint32_t evmask)
{
    uint64_t ts = time_us_64();
    for (int ii = 0; ii < n_rcvr_; ii++)
    {
        IR_Receiver *ir = receivers_[ii];
        if (ir->gpio() == gpio)
        {
            ir->store_timestamp(ts, (evmask & GPIO_IRQ_EDGE_FALL) == 0);
        }
    }
}

void IR_Receiver::store_timestamp(uint64_t ts, bool falling)
{
    if (check_sync(ts, falling))
    {
        if (n_pulse_ < mx_pulse_)
        {
            uint32_t delta = ts - pulses_[n_pulse_];
            if (delta > base_pulse_ * 5)
            {
                pulses_[n_pulse_++] = ts;
            }
            else
            {
                message_error();
            }
        }
        else
        {
            message_complete(ts);
        }
    }
}

void IR_Receiver::message_complete(uint64_t ts)
{
    uint16_t addr;
    uint16_t func;
    if (decode_message(addr, func))
    {
        if (rcb_)
        {
            rcb_(ts, addr, func);
        }
        reset();
    }
    else
    {
        message_error();
    }
}

void IR_Receiver::message_error()
{
    reset();
}

void IR_Receiver::reset()
{
    sync_ = 0;
    n_pulse_ = 0;
}