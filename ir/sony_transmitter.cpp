/*
                    *****  Sony_Transmitter class implmentation  *****
*/

#include "sony_transmitter.h"


void Sony_Transmitter::setMessageTimes(uint16_t addr, uint16_t func)
{
    uint32_t times[42];
    int ii = 0;
    times[ii++] = SONY_BASE_PULSE * 4;
    uint16_t mask = 1;
    for (int jj = 0; jj < command_size(); jj++, mask <<= 1)
    {
        if ((func & mask) == 0)
        {
            times[ii++] = SONY_BASE_PULSE;
            times[ii++] = SONY_BASE_PULSE;
        }
        else
        {
            times[ii++] = SONY_BASE_PULSE;
            times[ii++] = SONY_BASE_PULSE * 2;
        }
    }
    mask = 1;
    for (int jj = 0; jj < address_size(); jj++, mask <<= 1)
    {
        if ((addr & mask) == 0)
        {
            times[ii++] = SONY_BASE_PULSE;
            times[ii++] = SONY_BASE_PULSE;
        }
        else
        {
            times[ii++] = SONY_BASE_PULSE;
            times[ii++] = SONY_BASE_PULSE * 2;
        }
    }
    setOutputTimes(times, ii);
}
