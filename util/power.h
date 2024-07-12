/*
                    *****  Power Information Class  *****
*/

#ifndef POWER_H
#define POWER_H

class Power
{
public:
    /**
     * @brief   Initialize power access class
     */
    static void enable();

    /**
     * @brief   Deinitialize power accesss class
     */
    static void disable();

    /**
     * @brief   Test if system is running on battery
     * 
     * @return  true if on battery, false if on USB
     */
    static bool onBattery();

    /**
     * @brief   Get battery voltage
     * 
     * @return  Voltage output of battery (VSYS)
     */
    static float batteryVolts();
};

#endif
