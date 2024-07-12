/*
                    *****  Debug Flags Class  *****

    Monitor stdin for intreactive settig of debug flags.

    Type =nn (or just nn) <cr>  to set flag mask to nn where nn is a hex number
    Type +nn to or the new flags into existing set
    Type -nn too remove the specified flags from the active set
*/

#ifndef DBGFLAG_H
#define DGBFLAG_H

#include <stdint.h>

class DBGFlag
{
private:
    static uint64_t         start_time_;
    static uint32_t         flags_;
    static uint16_t         bufptr_;
    static char             buf_[32];

    static void char_avail(void *param);

public:
    /**
     * @brief   Enable interactive debug flags setting
     * 
     * @param   init_flags  Initial setting of flags
     */
    static void enable(uint32_t init_flags = 0);

    /**
     * @brief   Disable interactibe debug flags
     */
    static void disable();

    /**
     * @brief   Get current flag settings
     * 
     * @return  Current flags
     */
    static const uint32_t &flags() { return flags_; }

    /**
     * @brief   Test if flags are set
     * 
     * @param   Mask oof flags to test
     * 
     * @return  true if any flags in mask are set
     */
    static bool isSet(uint32_t mask) { return (mask & flags_) != 0; }

    /**
     * @brief   Print the elapsed time since boot
     * 
     * @param   nl          Add newline if true, space if false
     */
    static void logTime(bool nl = false);

    /**
     * @brief   Reset start time
     */
    static void resetStartTime();
};

#endif
