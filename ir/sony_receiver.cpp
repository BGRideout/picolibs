/*
                    *****  Sony_Receiver Class Implementation  *****
*/

#include "sony_receiver.h"

bool Sony_Receiver::check_sync(uint64_t ts, bool falling)
{
    bool ret = false;
    if (sync_ == 0)
    {
        n_pulse_ = 0;
        if (falling)
        {
            uint32_t delta = ts - prev_ts_;
            if (compare_pulse(delta, SONY_BASE_PULSE * 4, SONY_BASE_PULSE / 2))
            {
                pulses_[n_pulse_++] = delta;
                sync_ = 1;
            }
        }        
    }
    else if (sync_ == 1)
    {
        uint32_t delta = ts - prev_ts_;
        if (!falling && compare_pulse(delta, SONY_BASE_PULSE * 1, SONY_BASE_PULSE / 2))
        {
            sync_ = 2;
            pulses_[n_pulse_++] = delta;
        }
        else
        {
            reset();
        }
    }
    else if (sync_ == 2)
    {
        ret = true;
    }
    return ret;
}

bool Sony_Receiver::decode_message(uint16_t &addr, uint16_t &func)
{
    return decode(pulses_, n_pulse_, addr, func, address_, address_size_, value_size_);
}

bool Sony_Receiver::decode(uint32_t const *pulses, uint32_t n_pulse, uint16_t &addr, uint16_t &func,
                           uint16_t address, uint32_t addr_size, uint32_t value_size)
{
    bool ret = false;
    addr = 0;
    func = 0;
    if (n_pulse < 1)
    {
        return false;
    }
    uint8_t data[4];
    int ii = 0;
    if (compare_pulse(pulses[ii], SONY_BASE_PULSE * 4, SONY_BASE_PULSE / 2))
    {
        //  Skip sync mark pulse
        ++ii;
    }

    if (ii != 1 || n_pulse - ii < (addr_size + value_size) * 2 + 1 - 1)
    {
        // Incorrect sync or not enough pulses
        return false;
    }

    uint16_t mask = 0x0001;
    for (int jj = 0; jj < value_size; jj++)
    {
        uint32_t space = pulses[ii++];
        uint32_t mark = pulses[ii++];
        if (!compare_pulse(space, SONY_BASE_PULSE, SONY_BASE_PULSE / 2))
        {
            //  Incorrect space duration
            return false;
        }
        if (compare_pulse(mark, SONY_BASE_PULSE * 2, SONY_BASE_PULSE / 2))
        {
            func |= mask;
        }
        else if (!compare_pulse(mark, SONY_BASE_PULSE, SONY_BASE_PULSE / 2))
        {
            //  Incorrect mark duration
            return false;
        }
        mask <<= 1;
    }

    mask = 0x0001;
    for (int jj = 0; jj < addr_size; jj++)
    {
        uint32_t space = pulses[ii++];
        uint32_t mark = pulses[ii++];
        if (!compare_pulse(space, SONY_BASE_PULSE, SONY_BASE_PULSE / 2))
        {
            //  Incorrect space duration
            return false;
        }
        if (compare_pulse(mark, SONY_BASE_PULSE * 2, SONY_BASE_PULSE / 2))
        {
            addr |= mask;
        }
        else if (!compare_pulse(mark, SONY_BASE_PULSE, SONY_BASE_PULSE / 2))
        {
            //  Incorrect mark duration
            return false;
        }
        mask <<= 1;
    }

    if (addr == address || address == 0xffff)
    {
        ret = true;
    }

    return ret;
}

bool Sony_Receiver::check_bit_timeout()
{
    return sync_ == 2;
}
