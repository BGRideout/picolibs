/*
                    *****  NEC_Receiver Class Implementation  *****
*/

#include "nec_receiver.h"

bool NEC_Receiver::check_sync(uint64_t ts, bool falling)
{
    bool ret = false;
    if (sync_ == 0)
    {
        if (!falling)
        {
            pulses_[0] = ts;
        }
        else
        {
            uint32_t delta = ts - pulses_[0];
            uint32_t base = base_pulse_ * 16;
            if (delta > base - base_pulse_ && delta < base + base_pulse_)
            {
                sync_ = 1;
                pulses_[0] = ts;
            }
        }        
    }
    else if (sync_ == 1)
    {
        uint32_t delta = ts - pulses_[0];
        uint32_t base = base_pulse_ * 8;
        if (!falling && delta > base - base_pulse_ && delta < base + base_pulse_)
        {
            sync_ = 2;
            pulses_[0] = ts;
            n_pulse_ = 1;
        }
        else
        {
            sync_ = 0;
        }
    }
    else if (sync_ == 2)
    {
        ret = true;
    }
    return ret;
}

bool NEC_Receiver::decode_message(uint16_t &addr, uint16_t &func)
{
    bool ret = false;
    uint8_t data[4];
    int ii = 2;
    for (int jj = 0; jj < 4; jj++)
    {
        data[jj] = 0;
        uint8_t mask = 0x01;
        for (int kk = 0; kk < 8; kk++)
        {
            if ((pulses_[ii] - pulses_[ii - 1]) > 2 * base_pulse_)
            {
                data[jj] |= mask;
            }
            mask <<= 1;
            ii += 2;
        }
    }

    addr = data[0];
    func = data[2];
    if ((data[0] ^ data[1]) != 0xff)
    {
        addr = data[0] + data[1] * 256;
    }
    if ((data[2] ^ data[3]) == 0xff)
    {
        if (addr == address_ || address_ == 0xffff)
        {
            ret = true;
        }
    }
    return ret;
}