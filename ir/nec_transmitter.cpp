/*
                    *****  NEC_Transmitter class implmentation  *****
*/

#include "nec_transmitter.h"


void NEC_Transmitter::setMessageTimes(uint16_t addr, uint16_t func)
{
    uint8_t data[4];
    if (addr > 255)
    {
        data[0] = addr & 0xff;
        data[1] = (addr >> 8) & 0xff;
    }
    else
    {
        data[0] = addr & 0xff;
        data[1] = data[0] ^ 0xff;
    }
    data[2] = func & 0xff;
    data[3] = data[2] ^ 0xff;

    uint32_t times[68];
    int ii = 0;
    times[ii++] = base_pulse_ * 16;
    times[ii++] = base_pulse_ * 8;
    for (int jj = 0; jj < 4; jj++)
    {
        for (int kk = 0; kk < 8; kk++)
        {
            times[ii++] = base_pulse_;
            if ((data[jj] & 0x01) == 0)
            {
                times[ii++] = base_pulse_;
            }
            else
            {
                times[ii++] = base_pulse_ * 3;
            }
            data[jj] >>= 1;
        }
    }
    times[ii++] = base_pulse_;
    setOutputTimes(times, ii);
}
