/*
                    *****  Sound Class Implementation
*/

#include "sound.h"
#include <hardware/pwm.h>
#include <hardware/irq.h>
#include <pico/time.h>
#include <stdio.h>
#include <math.h>

Sound *Sound::singleton_ = nullptr;

Sound::Sound() : pwm_(nullptr), data_(nullptr), datasz_(0), sample_rate_(22050),
    idx_(0), slice_num_(0), chan_num_(0), scale_(0), repeat_(0), loop_(false),
    count_(0), tone_f_(0.0), tone_v_(1.0), tone_p_(0)
{
}

Sound::~Sound()
{
    if (pwm_) delete pwm_;
}

void Sound::start(uint32_t gpio)
{
    pwm_ = new PWM(gpio, sample_rate_);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, handler);
    slice_num_ = pwm_gpio_to_slice_num(gpio);
    chan_num_ = pwm_gpio_to_channel(gpio);
    pwm_set_irq_enabled(slice_num_, true);
}

void Sound::stop()
{
    pwm_set_irq_enabled(slice_num_, false);
    irq_set_enabled(PWM_IRQ_WRAP, false);
    irq_remove_handler(PWM_IRQ_WRAP, handler);
    delete this;
    singleton_ = nullptr;
}

void Sound::setSoundData(const uint8_t *data, uint32_t size, uint32_t sample_rate)
{
    stopPlaying();
    wait();
    data_ = data;
    datasz_ = size;

    //  Calculate scale
    uint8_t high = data_[0];
    uint8_t low = high;
    for (int ii = 1; ii < datasz_ - 1; ii++)
    {
        if (data_[ii] > high) high = data_[ii];
        if (data_[ii] < low) low = data_[ii];
    }
    float scaleh = (float)(0xff - 0x80) / (float)(high - 0x80);
    float scalel = (float)(0x80 - 0x00) / (float)(0x80 - low);
    float scale = scaleh > scalel ? scaleh : scalel;
    scale_ = (uint16_t)(scale * 0x80);
}

void Sound::playOnce(uint8_t repeat)
{
    if (!irq_is_enabled(PWM_IRQ_WRAP))
    {
        idx_ = 0;
        repeat_ = repeat;
        irq_set_enabled(PWM_IRQ_WRAP, true);
    }
    else if (repeat_ < 8)
    {
        repeat_ += repeat;
    }
}

void Sound::startPlaying()
{
    loop_ = true;
    irq_set_enabled(PWM_IRQ_WRAP, true);
}

void Sound::stopPlaying()
{
    repeat_ = 0;
    loop_ = false;
}

bool Sound::isPlaying() const
{
    return irq_is_enabled(PWM_IRQ_WRAP);
}

void Sound::tone(uint32_t freq, uint32_t msec, float vol)
{
    if (freq > 0 && msec > 0 && vol >= 0.0 && vol <= 1.0)
    {
        stopPlaying();
        wait();
        tone_f_ = freq * 2.0 * M_PI / sample_rate_;
        tone_v_ = vol;
        tone_p_ = sample_rate_ / freq;
        count_ = msec * sample_rate_ / 1000;
        idx_ = 0;
        irq_set_enabled(PWM_IRQ_WRAP, true);
    }
}

void Sound::beep(uint32_t freq, uint32_t msec)
{
    if (freq > 0 && msec > 0)
    {
        stopPlaying();
        wait();
        pwm_->setFrequency(freq);
        pwm_->setDutyCycle(0.5);
        count_ = msec * freq / 1000;
        irq_set_enabled(PWM_IRQ_WRAP, true);
    }
}

void Sound::wait(uint32_t timeout)
{
    uint64_t end_time = 0xffffffffffffffffll;
    if (timeout > 0)
    {
        end_time = time_us_64() + timeout * 1000;
    }
    while (irq_is_enabled(PWM_IRQ_WRAP) && time_us_64() < end_time)
    {
        sleep_us(10);
    }
}

void Sound::handler()
{
    singleton_->next_pulse();
}

void Sound::next_pulse()
{
    pwm_clear_irq(slice_num_);
    if (count_ == 0)
    {
        //  Playing Sound sound.  Set next sample level.
        if (idx_ < datasz_)
        {
            uint16_t lvl = ((data_[idx_++] - 0x80) * scale_ + 0x4000) * pwm_->wrap() / 0x8000;
            pwm_set_chan_level(slice_num_, chan_num_, lvl);
        }
        else
        {
            if (repeat_ > 0)
            {
                --repeat_;
            }
            if (loop_ || repeat_ > 0)
            {
                idx_ = 0;
            }
            else
            {
                pwm_->setDutyCycle(0.0);
                irq_set_enabled(PWM_IRQ_WRAP, false);
            }
        }
    }
    else
    {
        //  Constant beep. Count down time
        --count_;
        if (count_ > 0)
        {
            if (tone_p_ > 0)
            {
                float a = (idx_ % tone_p_) * tone_f_ - M_PI;
                // Quick sine calculation
                float s = a * M_PI_4;
                if (a < 0)
                {
                    s += 4.0 / M_PI / M_PI * a * a;
                }
                else
                {
                    s -= 4.0 / M_PI / M_PI * a * a;
                }
                pwm_->setDutyCycle(tone_v_ * s);
                ++idx_;
            }
        }
        else
        {
            tone_p_ = 0;
            idx_ = 0;
            pwm_->setFrequency(sample_rate_);
            pwm_->setDutyCycle(0.0);
            if (!loop_ && repeat_ == 0)
            {
                irq_set_enabled(PWM_IRQ_WRAP, false);
            }
        }
    }
}
