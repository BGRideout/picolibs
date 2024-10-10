#include "web_set_time.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

static void (*set_time_cb_)() = 0;

void sntp_set_system_time(uint32_t sec)
{
    struct timeval tv = {.tv_sec=sec};
    settimeofday(&tv, 0);
    if (set_time_cb_)
    {
        set_time_cb_();
    }
}

void set_time_set_cb(void(*cb)())
{
    set_time_cb_ = cb;
}
