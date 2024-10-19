//                  ***** Logger class implementation  *****

#include "logger.h"
#include <stdio.h>
#include <stdarg.h>

int Logger::print(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int ret = vprintf(format, ap);
    va_end(ap);
    return ret;
}

int Logger::print_debug(int level, const char *format, ...)
{
    int ret = 0;
    if (isDebug(level))
    {
        va_list ap;
        va_start(ap, format);
        ret = vprintf(format, ap);
        va_end(ap);
    }
    return ret;
}