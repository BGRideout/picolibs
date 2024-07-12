/*
                    *****  Debuf Flags Class implementation  *****
*/

#include "dbgflag.h"
#include <stdio.h>
#include <ctype.h>
#include <pico/stdio.h>
#include <hardware/timer.h>

uint64_t    DBGFlag::start_time_ = 0;
uint32_t    DBGFlag::flags_ = 0;
uint16_t    DBGFlag::bufptr_ = 0;
char        DBGFlag::buf_[32];

void DBGFlag::enable(uint32_t init_flags)
{
    start_time_ = time_us_64();
    flags_ = init_flags;
    bufptr_ = 0;
    stdio_set_chars_available_callback(char_avail, nullptr);
}

void DBGFlag::disable()
{
    flags_ = 0;
    bufptr_ = 0;
    stdio_set_chars_available_callback(nullptr, nullptr);
}

void DBGFlag::char_avail(void *param)
{
    int ch = getchar_timeout_us(0);
    while (ch != PICO_ERROR_TIMEOUT)
    {
        if (ch == '\r')
        {
            char cmdch = '=';
            uint16_t ii = 0;
            if (bufptr_ > 0)
            {
                if (buf_[0] == '=' || buf_[0] == '+' || buf_[0] == '-')
                {
                    cmdch = buf_[0];
                    ii += 1;
                }
                uint32_t mask = 0;
                while (ii < bufptr_)
                {
                    mask <<= 4;
                    char c = tolower(buf_[ii++]);
                    switch (c)
                    {
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        mask |= (c - '0');
                        break;

                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                        mask |= (c = 'a' + 10);
                        break;

                    default:
                        printf("invalid character '%c'\n", c);
                        mask = 0;
                        ii = bufptr_;
                        cmdch = 0;
                    }
                }

                switch (cmdch)
                {
                case '=':
                    flags_ = mask;
                    printf("DBGFlag set flags to %x\n", flags_);
                    break;

                case '+':
                    flags_ |= mask;
                    printf("DBGFlag added %x to make flags %x\n", mask, flags_);
                    break;

                case '-':
                    flags_ &= ~mask;
                    printf("DBGFlag removed %x to make flags %x\n", mask, flags_);
                    break;
                }
            }
            bufptr_ = 0;
        }
        else if (ch == '\b')
        {
            if (bufptr_ > 0)
            {
                bufptr_ -= 1;
            }
        }
        else
        {
            if (bufptr_ < sizeof(buf_))
            {
                buf_[bufptr_++] = ch;
            }
        }
        ch = getchar_timeout_us(0);
    }
}

void DBGFlag::logTime(bool nl)
{
    uint32_t now = static_cast<uint32_t>((time_us_64() - start_time_ + 500000) / 1000000);
    int hr = now / 3600;
    int min = (now % 3600) / 60;
    int sec = (now % 60);
    printf("%02d:%02.2d:%02.2d%c", hr, min, sec, nl ? '\n' : ' ');
}

void DBGFlag::resetStartTime()
{
    start_time_ = time_us_64();
}