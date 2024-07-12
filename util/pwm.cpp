/*
                    *****  PWM Class Implmentation  *****
*/

#include "pwm.h"

#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>
#include <stdio.h>

PWM::PWM(uint32_t gpio, uint32_t freq, float duty) : gpio_(gpio), div_(0), wrap_(0)
{
    gpio_init(gpio_);
    gpio_set_function(gpio_, GPIO_FUNC_PWM);
    setFrequency(freq);
    setDutyCycle(duty);
}

PWM::~PWM()
{
    gpio_deinit(gpio_);
}

bool PWM::setFrequency(uint32_t freq)
{
    bool ret = true;

    uint slice_num = pwm_gpio_to_slice_num(gpio_);
    pwm_config config = pwm_get_default_config();
    wrap_ = 65534;
    div_ = (float)clock_get_hz(clk_sys) / (freq * wrap_);
    if (div_ < 1.0)
    {
        div_ = 1.0;
        wrap_ = (float)clock_get_hz(clk_sys) / (freq * div_);
    }

    if (div_ <= 256.0 && wrap_ > 2)
    {
        pwm_config_set_clkdiv(&config, div_);
        pwm_config_set_wrap(&config, wrap_);
        pwm_set_gpio_level(gpio_, 0);
        pwm_init(slice_num, &config, true); // start the pwm running according to the config
    }
    else if (div_ > 256.0)
    {
        ret = false;
        fprintf(stderr, "Frequency %d is too low\n", freq);
    }
    else
    {
        ret = false;
        fprintf(stderr, "Frequency %d is too high\n", freq);
    }

    return ret;
}

uint16_t PWM::setDutyCycle(float duty)
{
    if (duty < 0.0) duty = 0.0;
    if (duty > 1.0) duty = 1.0;
    uint16_t level = (uint16_t)(duty * wrap_);
    pwm_set_gpio_level(gpio_, level);
    return level;
}

uint32_t PWM::freq() const
{
    return clock_get_hz(clk_sys) / (div_ * wrap_);
}
