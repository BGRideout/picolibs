/*
                    *****  LED Class Implementation  *****
*/

#include "led.h"
#if LIB_PICO_CYW43_ARCH
#include "pico/cyw43_arch.h"
#endif
#include <string.h>
#include <stdlib.h>

LED::LED(uint32_t gpio, bool state)
 : gpio_(gpio), flash_period_(0), flash_pattern_(1), flash_bits_(2), flash_index_(0), flasher_(-1)
{
    gpio_init(gpio_);
    gpio_set_dir(gpio_, true);
    set_led(state);
}

LED::LED(const char *led, bool state)
 : gpio_(0xffffffff), flash_period_(0), flash_pattern_(1), flash_bits_(2), flash_index_(0), flasher_(-1)
{
    if (strcasecmp(led, "led") == 0)
    {
#ifndef CYW43_WL_GPIO_LED_PIN
        gpio_ = PICO_DEFAULT_LED_PIN;
        gpio_init(gpio_);
        gpio_set_dir(gpio_, true);
#endif
    }
    else
    {
        char *ep;
        gpio_ = strtoul(led, &ep, 10);
        gpio_init(gpio_);
        gpio_set_dir(gpio_, true);
    }
    set_led(state);
}

LED::~LED()
{
    if (gpio_ != 0xffffffff)
    {
        gpio_deinit(gpio_);
    }
}

bool LED::get_led() const
{
    bool ret = false;
    if (gpio_ == 0xffffffff)
    {
#ifdef CYW43_WL_GPIO_LED_PIN
        cyw43_thread_enter();
        ret = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
        cyw43_thread_exit();
#endif
    }
    else
    {
        ret = gpio_get(gpio_);
    }
    return ret;
}

void LED::set_led(bool state)
{
    if (gpio_ == 0xffffffff)
    {
#ifdef CYW43_WL_GPIO_LED_PIN
        cyw43_thread_enter();
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
        cyw43_thread_exit();
#endif
    }
    else
    {
        gpio_put(gpio_, state);
    }
}


bool LED::setFlash(uint32_t period)
{
    bool ret = true;
    flash_period_ = period;
    if (flash_index_ >= flash_bits_)
    {
        flash_index_ = 0;
    }
    if (check_flash() && flash_period_ != 0 && flash_period_ > flash_bits_)
    {
        if (flasher_ == -1)
        {
            flasher_ = add_alarm_in_ms(flash_period_ / flash_bits_, timer_cb, this, true);
        }
    }
    else
    {
        ret = false;
        if (flasher_ != -1)
        {
            cancel_alarm(flasher_);
            flasher_ = -1;
        }
    }
    return ret;
}

bool LED::setFlashPattern(uint32_t pattern, uint32_t bits)
{
    flash_pattern_ = pattern;
    flash_bits_ = bits;
    return setFlash(flash_period_);
}

bool LED::check_flash()
{
    bool ret = true;
    uint32_t mask = 0xffffffff;
    mask >>= 32 - flash_bits_;
    uint32_t pat = flash_pattern_ & mask;
    if (pat == 0)
    {
        // All zeroes, turn off
        set_led(false);
        ret = false;
    }
    else if (pat == mask)
    {
        // All ones, turn on
        set_led(true);
        ret = false;
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
        set_led((flash_pattern_ & (1 << flash_index_)) != 0);
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
