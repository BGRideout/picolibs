/*
                    *****  SAMSUNG_Transmitter class implmentation  *****
*/

#include "samsung_transmitter.h"

void SAMSUNG_Transmitter::setMessageTimes(uint16_t addr, uint16_t func)
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
        data[1] = data[0];
    }
    data[2] = func & 0xff;
    data[3] = data[2] ^ 0xff;

    uint32_t times[68];
    int ii = 0;
    times[ii++] = SAMSUNG_BASE_PULSE * 8;
    times[ii++] = SAMSUNG_BASE_PULSE * 8;
    for (int jj = 0; jj < 4; jj++)
    {
        for (int kk = 0; kk < 8; kk++)
        {
            times[ii++] = SAMSUNG_BASE_PULSE;
            if ((data[jj] & 0x01) == 0)
            {
                times[ii++] = SAMSUNG_BASE_PULSE;
            }
            else
            {
                times[ii++] = SAMSUNG_BASE_PULSE * 3;
            }
            data[jj] >>= 1;
        }
    }
    times[ii++] = SAMSUNG_BASE_PULSE;
    setOutputTimes(times, ii);
}
