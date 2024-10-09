//                  ***** Logger class *****

#ifndef LOGGER_H
#define LOGGER_H

class Logger
{
public:
    Logger() {}
    virtual ~Logger() {}

    virtual int print(const char *format, ...);
};

#endif
