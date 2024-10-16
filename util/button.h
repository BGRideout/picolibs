/*
                    *****  Button Class  *****
*/

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <hardware/gpio.h>
#include <pico/time.h>
#include <pico/util/queue.h>

class Button
{
public:
    enum ButtonAction
    {
        No_Action,
        Button_Down,
        Button_Up,
        Button_Clicked,
        Button_DoubleClicked,
        Button_Held
    };

    struct ButtonEvent
    {
        uint64_t        time;
        uint32_t        id;
        ButtonAction    action;
    };

    typedef void (*button_cb_t)(struct ButtonEvent &ev, void *user_data);

private:
    uint32_t            id_;                // Button ID
    uint32_t            gpio_;              // GPIO number
    alarm_id_t          bounce_;            // Debounce timer
    alarm_id_t          click_;             // Click timer
    alarm_id_t          hold_;              // Hold timer

    uint32_t            bounce_time_;       // Bounce time (msec)
    uint32_t            click_time_;        // Click time (msec)
    uint32_t            hold_time_;         // Hold time (msec)
    bool                prev_state_;        // Previous state

    static Button       **buttons_;         // List of buttons
    static uint32_t     n_button_;          // Number of transmitters
    static uint32_t     mx_button_;         // Maximum number of transmitters
    static queue_t      queue_;             // Event queue

    static void gpio_cb(uint gpio, uint32_t evmask);
    void do_event();
    uint32_t gpio() const { return gpio_; }

    static int64_t bounce_cb(alarm_id_t id, void *udata);
    static int64_t click_cb(alarm_id_t id, void *udata);
    static int64_t hold_cb(alarm_id_t id, void *udata);
    void button_down();
    void button_up();
    void button_clicked();
    void button_held();

    void start_hold_timer();
    void stop_hold_timer();
    void start_click_timer();
    void stop_click_timer();

    void notify(ButtonAction action);
    button_cb_t btn_cb_;
    void *user_data_;

public:
    /**
     * @brief   Button class constructor
     * 
     * @param   id          Button ID
     * @param   gpio        GPIO number for LED output
     * @param   hold_time   Hold time (msec)
     * @param   click_time  Click time (msec)
     * @param   bounce_time Debounce time (msec)
     */
    Button(uint32_t id, uint32_t gpio, uint32_t hold_time=3000, uint32_t click_time=250, uint32_t bounce_time=7);

    /**
     * @brief   Button class destructor
     */
    ~Button();

    /**
     * @brief   Check if button is down
     * 
     * @return  true if button is down
     */
    bool isDown() const { return !gpio_get(gpio_); }

    /**
     * @brief   Get the next available event from the button
     * 
     * @param   ev  Buffer to receive event
     * 
     * @return  true if an event was available
     */
    static bool getNextEvent(ButtonEvent &ev);

    /**
     * @brief   Set callback to be used instead of event queue
     * 
     * @param   btn_cb      Callback function (null turns off callback)
     * @param   user_data   User data passed to callbck
     */
    void setEventCallback(button_cb_t cb, void *user_data = nullptr) { btn_cb_ = cb; user_data_ = user_data; }
};

#endif
