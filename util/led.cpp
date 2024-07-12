/*
                    *****  LED Class Implementation  *****
*/

#include "led.h"

LED::LED(uint32_t gpio, bool state)
 : gpio_(gpio), flash_period_(0), flash_pattern_(1), flash_bits_(2), flash_index_(0), flasher_(-1)
{
    gpio_init(gpio_);
    gpio_set_dir(gpio_, true);
    gpio_put(gpio_, state);
}

LED::~LED()
{
    gpio_deinit(gpio_);
}

void LED::setFlash(uint32_t period)
{
    flash_period_ = period;
    flash_index_ = 0;
    if (flash_period_ != 0 && flash_period_ > flash_bits_)
    {
        if (flasher_ == -1)
        {
            flasher_ = add_alarm_in_ms(flash_period_ / flash_bits_, timer_cb, this, true);
        }
    }
    else
    {
        if (flasher_ != -1)
        {
            cancel_alarm(flasher_);
            flasher_ = -1;
        }
    }
}

bool LED::setFlashPattern(uint32_t pattern, uint32_t bits)
{
    bool ret = false;
    if (pattern != 0 && bits > 1)
    {
        flash_index_ = 0;
        flash_pattern_ = pattern;
        flash_bits_ = bits;
        ret = true;
    }

    return ret;
}

int64_t LED::timer_cb(alarm_id_t id, void *udata)
{
    LED *self = static_cast<LED *>(udata);
    return self->do_timer() * -1000;
}

int64_t LED::do_timer()
{
    int64_t ret = 0;
    if (flash_period_ > 0)
    {
        ret = flash_period_ / flash_bits_;
        gpio_put(gpio_, (flash_pattern_ & (1 << flash_index_)) != 0);
        if (++flash_index_ >= flash_bits_)
        {
            flash_index_ = 0;
        }
    }
    if (ret == 0)
    {
        flasher_ = -1;
    }
    return ret;
}
