/*
                    *****  SAMSUNG_Receiver Class Implementation  *****
*/

#include "samsung_receiver.h"

bool SAMSUNG_Receiver::check_sync(uint64_t ts, bool falling)
{
    bool ret = false;
    if (sync_ == 0)
    {
        n_pulse_ = 0;
        if (falling)
        {
            uint32_t delta = ts - prev_ts_;
            if (compare_pulse(delta, SAMSUNG_BASE_PULSE * 8, SAMSUNG_BASE_PULSE / 2))
            {
                pulses_[n_pulse_++] = delta;
                sync_ = 1;
            }
        }        
    }
    else if (sync_ == 1)
    {
        uint32_t delta = ts - prev_ts_;
        if (!falling && compare_pulse(delta, SAMSUNG_BASE_PULSE * 8, SAMSUNG_BASE_PULSE / 2))
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

bool SAMSUNG_Receiver::decode_message(uint16_t &addr, uint16_t &func)
{
    return decode(pulses_, n_pulse_, addr, func, address_);
}

bool SAMSUNG_Receiver::decode(uint32_t const *pulses, uint32_t n_pulse, uint16_t &addr, uint16_t &func, uint16_t address)
{
    bool ret = false;
    addr = 0;
    func = 0;
    if (n_pulse < 3)
    {
        return false;
    }
    uint8_t data[4];
    int ii = 0;
    if (compare_pulse(pulses[ii], SAMSUNG_BASE_PULSE * 8, SAMSUNG_BASE_PULSE / 2))
    {
        //  Skip sync mark pulse
        ++ii;
    }
    if (compare_pulse(pulses[ii], SAMSUNG_BASE_PULSE * 8, SAMSUNG_BASE_PULSE / 2))
    {
        //  Skip sync space pulse
        ++ii;
    }

    //  Point to marks
    ++ii;

    if (ii != 3 || n_pulse - ii < 64)
    {
        // Incorrect sync or not enough pulses
        return false;
    }

    for (int jj = 0; jj < 4; jj++)
    {
        data[jj] = 0;
        uint8_t mask = 0x01;
        for (int kk = 0; kk < 8; kk++)
        {
            if (pulses[ii] > 2 * SAMSUNG_BASE_PULSE)
            {
                data[jj] |= mask;
            }
            mask <<= 1;
            ii += 2;
        }
    }

    addr = data[0];
    func = data[2];
    if ((data[0] != data[1]))
    {
        addr = data[0] + data[1] * 256;
    }
    if ((data[2] ^ data[3]) == 0xff)
    {
        if (addr == address || address == 0xffff)
        {
            ret = true;
        }
    }
    return ret;
}

bool SAMSUNG_Receiver::check_bit_timeout()
{
    return sync_ == 2;
}
