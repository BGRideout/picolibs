//                  ***** Logger class *****

#ifndef LOGGER_H
#define LOGGER_H

class Logger
{
protected:
    int         debug_level_;           // Debug level

public:
    Logger() : debug_level_(0) {}
    virtual ~Logger() {}

    virtual int print(const char *format, ...);
    virtual int print_debug(int level, const char *format, ...);

    bool isDebug(int level = 1) { return level <= debug_level_; }
    int  debugLevel() const { return debug_level_; }
    void setDebug(int level) { debug_level_ = level; }
};

#endif
