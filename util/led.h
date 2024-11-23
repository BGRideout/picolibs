/*
                    *****  LED Class  *****
*/

#ifndef LED_H
#define LED_H

#include <stdint.h>
#include <hardware/gpio.h>
#include <pico/time.h>

class LED
{
private:
    uint32_t            gpio_;          // GPIO for LED otput
    uint32_t            flash_period_;  // Flash period
    uint32_t            flash_pattern_; // Flash pattern
    uint32_t            flash_bits_;    // Flash pattern size
    uint32_t            flash_index_;   // Index to flash bit
    alarm_id_t          flasher_;       // Flash timer

    static int64_t timer_cb(alarm_id_t id, void *udata);
    int64_t do_timer();
    bool check_flash();
    void set_led(bool state);

public:
    /**
     * @brief   LED class constructor
     * 
     * @param   gpio    GPIO number for LED output
     * @param   state   Initial state of LED (true = on)
     */
    LED(uint32_t gpio, bool state=true);

    /**
     * @brief   LED class constructor
     * 
     * @param   led     String "LED" indicates onboard LED or numeric string for GPIO
     * @param   state   Initial state of LED (true = on)
     */
    LED(const char *led="LED", bool state=true);

    /**
     * @brief   Destructor for LED class
     */
    virtual ~LED();

    /**
     * @brief   Set state of LED
     * 
     * @param   state   New state of LED (true = on)
     */
    void setState(bool state) { flash_period_ = 0; gpio_put(gpio_, state); }

    /**
     * @brief   Assign state of LED
     * 
     * @param   state   RHS is new state of LED (true = on)
     */
    void operator = (bool state) { flash_period_ = 0; gpio_put(gpio_, state); }

    /**
     * @brief   Get current state of LED
     * 
     * @return  Current state of LED
     */
    bool isOn() const { return gpio_get(gpio_); }

    /**
     * @brief   Get flash pattern size
     * 
     * @return  Flash pattern bit count
     */
    const uint32_t &flash_bits() const { return flash_bits_; }

    /**
     * @brief   Get flash pattern
     * 
     * @return  Flash pattern
     */
    const uint32_t &flash_pattern() const { return flash_pattern_; }

    /**
     * @brief   Get flash period
     * 
     * @return  Flash period
     */
    const uint32_t &flash_period() const { return flash_period_; }

    /**
     * @brief   Set LED to flashing
     * 
     * @param   period  Period of a flash pattern cycle cycle (msec) (zero turns flash off)
     *
     * @return  true if LED flashing, false if steady 
     */
    bool setFlash(uint32_t period);

    /**
     * @brief   Set flash pattern
     * 
     * @param   pattern Bit mask of on/off states per period
     * @param   bits    Number of bits in pattern
     * 
     * @return  true if LED flashing, false if steady
     */
    bool setFlashPattern(uint32_t pattern=1, uint32_t bits=2);

    /**
     * @brief   Set the flash period in msec
     * 
     * @param   period	Flash period (zero is not flashing)
     */
    void setFlashPeriod(uint32_t period) { flash_period_ = period; }
};

#endif
