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

IR_Receiver::IR_Receiver(uint32_t gpio, uint32_t n_pulse)
  : gpio_(gpio), n_pulse_(0), mx_pulse_(n_pulse), prev_ts_(0), sync_(0),
    rcb_(nullptr), rpt_(nullptr), rpt_addr_(0), rpt_func_(0),
    tmo_(nullptr), msg_timer_(-1), bit_timer_(-1), msg_timeout_(0), bit_timeout_(0), prev_count_(0),
    err_(nullptr)
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

    pulses_ = new uint32_t[mx_pulse_];

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

    sem_init(&sem_, 1, 1);
}

IR_Receiver::~IR_Receiver()
{
    if (bit_timer_ != -1) cancel_alarm(bit_timer_);
    if (msg_timer_ != -1) cancel_alarm(msg_timer_);
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

bool IR_Receiver::wait_for_message(uint32_t timeout_ms)
{
    //  Try to get the semaphore, fail if not acquired
    bool ret = sem_try_acquire(&sem_);
    if (ret)
    {
        //  Acquire semaphore again, will be unlocked when message completes
        if (timeout_ms == 0)
        {
            sem_acquire_blocking(&sem_);
        }
        else
        {
            ret = sem_acquire_timeout_ms(&sem_, timeout_ms);
        }
        //  Be sure semaphore is released
        sem_release(&sem_);
    }
    return ret;
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
    uint8_t psy = sync_;
    if (check_sync(ts, falling))
    {
        if (n_pulse_ < mx_pulse_)
        {
            pulses_[n_pulse_++] = static_cast<uint32_t>(ts - prev_ts_);
        }
        if (n_pulse_ == mx_pulse_)
        {
            message_complete(ts);
        }
    }
    prev_ts_ = ts;

    //  Start timeout when sync_ changes from zero to non-zero
    if (psy == 0 && sync_ != 0)
    {
        start_timeout();
    }
}

void IR_Receiver::message_complete(uint64_t ts)
{
    if (decode_message(rpt_addr_, rpt_func_))
    {
        if (rcb_)
        {
            rcb_(ts, rpt_addr_, rpt_func_, this);
        }
        reset();
    }
    else
    {
        message_error();
    }
}

void IR_Receiver::start_timeout()
{
    if (msg_timeout_ > 0)
    {
        msg_timer_ = add_alarm_in_ms(msg_timeout_, timeout_msg, this, true);
    }
    if (bit_timeout_ > 0)
    {
        prev_count_ = n_pulse_;
        bit_timer_ = add_alarm_in_ms(bit_timeout_, timeout_bit, this, true);
    }
}

int64_t IR_Receiver::timeout_msg(alarm_id_t id, void *user_data)
{
    IR_Receiver *self = static_cast<IR_Receiver *>(user_data);
    if (self->sync_ != 0)
    {
        self->timeout(true);
    }
    self->msg_timer_ = -1;
    return 0;
}

int64_t IR_Receiver::timeout_bit(alarm_id_t id, void *user_data)
{
    uint64_t ret = 0;
    IR_Receiver *self = static_cast<IR_Receiver *>(user_data);
    if (self->sync_ != 0)
    {
        if (self->n_pulse_ == self->prev_count_ && self->check_bit_timeout())
        {
            ret = self->timeout(false);
        }
        else
        {
            self->prev_count_ = self->n_pulse_;
        }
        ret = self->bit_timeout_ * 1000;
    }
    if (ret == 0)
    {
        self->bit_timer_ = -1;
    }
    return ret;
}

bool IR_Receiver::timeout(bool msg)
{
    bool ret = false;
    if (tmo_)
    {
        ret = tmo_(msg, n_pulse_, pulses_, this);
        if (!ret)
        {
            reset();
        }
    }
    else
    {
        message_error();
    }
    return ret;
}

void IR_Receiver::message_error()
{
    if (err_)
    {
        err_(this);
    }
    reset();
}

void IR_Receiver::reset()
{
    sync_ = 0;
    n_pulse_ = 0;
    if (bit_timer_ != -1)
    {
        cancel_alarm(bit_timer_);
        bit_timer_ = -1;
    }
    if (msg_timer_ != -1)
    {
        cancel_alarm(msg_timer_);
        msg_timer_ = -1;
    }
    sem_release(&sem_);
}
