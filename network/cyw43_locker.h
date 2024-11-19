//                  ***** Locking class for cyw43 access control  *****

#ifndef CYW43_LOCKER_H
#define CYW43_LOCKER_H

#include "pico/cyw43_arch.h"

class CYW43Locker
{
private:
    static inline int  lock_count_ = 0;

public:
    /**
     * @brief   Constructor
     * 
     * @details
     *  Locks the cyw43 access until object destroyed
     */
    inline CYW43Locker() {lock();}

    /**
     * @brief   Destructor
     * 
     * @details
     *  Locks the cyw43 access until object destroyed normally
     *  when th object goes out of scope
     */
    inline ~CYW43Locker() {unlock();}

    /**
     * @brief   Lock access
     */
    static inline void lock() {cyw43_arch_lwip_begin(); ++lock_count_;}

    /**
     * @brief   Unlock access
     */
    static inline void unlock() {cyw43_arch_lwip_end(); --lock_count_;}
};

#endif

