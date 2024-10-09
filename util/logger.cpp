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
