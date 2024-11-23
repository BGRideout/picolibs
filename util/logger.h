//                  ***** Logger class *****

#ifndef LOGGER_H
#define LOGGER_H

/**
 * @class   Logger
 * 
 * This class provides a debug logging facility.
 * 
 * It can be subclassed (e.g. FileLogger class) to direct logs
 * to persistent storage. This base class writes messages to
 * stdout.
 */
class Logger
{
protected:
    int         debug_level_;           // Debug level

public:
    /**
     * @brief   Constructor
     */
    Logger() : debug_level_(0) {}

    /**
     * @brief   Destructor
     */
    virtual ~Logger() {}

    /**
     * @brief   Print data usig pritf semantics
     */
    virtual int print(const char *format, ...);

    /**
     * @brief   Print message if debug level at or above specified level
     * 
     * @param   level   Debug level at which this message is printed
     * @param   format  Remaining parameters as for printf
     */
    virtual int print_debug(int level, const char *format, ...);

    /**
     * @brief   Test if dbug level at or above specified level
     */
    bool isDebug(int level = 1) { return level <= debug_level_; }

    /**
     * @brief   Return current debug level
     */
    int  debugLevel() const { return debug_level_; }

    /**
     * @brief   Set debug level
     * 
     * @param   level   New debug level
     */
    void setDebug(int level) { debug_level_ = level; }
};

#endif
