/*
                    *****  Button Class Implementation  *****
*/

#include "button.h"
#include <stdio.h>

Button      **Button::buttons_ = nullptr;
uint32_t    Button::n_button_ = 0;
#ifndef MAX_BUTTONS
#define MAX_BUTTONS     8
#endif
uint32_t    Button::mx_button_ = MAX_BUTTONS;
queue_t     Button::queue_;

Button::Button(uint32_t id, uint32_t gpio, uint32_t hold_time, uint32_t click_time, uint32_t bounce_time)
  : id_(id), gpio_(gpio), bounce_(-1), click_(-1), hold_(-1),
    bounce_time_(bounce_time), click_time_(click_time), hold_time_(hold_time), btn_cb_(nullptr), user_data_(nullptr)
{
    if (buttons_ == nullptr)
    {
        buttons_ = new Button *[mx_button_];
        n_button_ = 0;
    }
    if (n_button_ < mx_button_)
    {
        buttons_[n_button_++] = this;
    }
    else
    {
        fprintf(stderr, "Button - Too many buttons defined! Maximum = %d\n", mx_button_);
    }

    gpio_init(gpio_);
    gpio_set_dir(gpio_, GPIO_IN);
    gpio_set_pulls(gpio_, true, false);
    prev_state_ = gpio_get(gpio_);
    if (n_button_ == 1)
    {
        queue_init(&queue_, sizeof(struct ButtonEvent), 64);
        gpio_set_irq_enabled_with_callback(gpio_, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true, gpio_cb);
    }
    else
    {
        gpio_set_irq_enabled(gpio_, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, true);
    }
}

Button::~Button()
{
    gpio_set_irq_enabled(gpio_, GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE, false);
    gpio_deinit(gpio_);
    n_button_ -= 1;
    int jj = 0;
    for (int ii = 0; ii < n_button_; ii++)
    {
        if (buttons_[ii] == this)
        {
            jj += 1;
        }
        if (jj != ii)
        {
            buttons_[ii] = buttons_[jj];
        }
    }
    if (n_button_ == 0)
    {
        delete [] buttons_;
        buttons_ = nullptr;
        queue_free(&queue_);
    }
}

bool Button::getNextEvent(ButtonEvent &ev)
{
    bool ret = queue_try_remove(&queue_, &ev);
    if (!ret)
    {
        ev = {time_us_64(), 0, No_Action};
    }

    return ret;
}

void Button::gpio_cb(uint gpio, uint32_t evmask)
{
    uint64_t ts = time_us_64();
    for (int ii = 0; ii < n_button_; ii++)
    {
        Button *btn = buttons_[ii];
        if (btn->gpio() == gpio)
        {
            btn->do_event();
        }
    }
}

void Button::do_event()
{
    if (bounce_ == -1)
    {
        bounce_ = add_alarm_in_ms(bounce_time_, bounce_cb, this, true);
    }
}

int64_t Button::bounce_cb(alarm_id_t id, void *udata)
{
    Button *self = static_cast<Button *>(udata);
    bool state = self->isDown();
    if (state != self->prev_state_)
    {
        self->prev_state_ = state;
        if (state)
        {
            self->button_down();
        }
        else
        {
            self->button_up();
        }
    }
    self->bounce_ = -1;
    return 0;
}

void Button::button_down()
{
    notify(Button_Down);
    start_hold_timer();
}

void Button::button_up()
{
    notify(Button_Up);
    if (hold_ != -1)
    {
        if (click_ == -1)
        {
            start_click_timer();
        }
        else
        {
            stop_click_timer();
            notify(Button_DoubleClicked);
        }
    }
    stop_hold_timer();
}

void Button::button_clicked()
{
    notify(Button_Clicked);
    stop_click_timer();
}

void Button::button_held()
{
    notify(Button_Held);
    stop_click_timer();
    stop_hold_timer();
}

void Button::start_hold_timer()
{
    stop_hold_timer();
    hold_ = add_alarm_in_ms(hold_time_, hold_cb, this, true);
}

void Button::stop_hold_timer()
{
    if (hold_ != -1)
    {
        cancel_alarm(hold_);
        hold_ = -1;
    }
}

int64_t Button::hold_cb(alarm_id_t id, void *udata)
{
    Button *self = static_cast<Button *>(udata);
    self->hold_ = -1;
    self->button_held();
    return 0;
}

void Button::start_click_timer()
{
    stop_click_timer();
    click_ = add_alarm_in_ms(click_time_, click_cb, this, true);
}

void Button::stop_click_timer()
{
    if (click_ != -1)
    {
        cancel_alarm(click_);
        click_ = -1;
    }
}

int64_t Button::click_cb(alarm_id_t id, void *udata)
{
    Button *self = static_cast<Button *>(udata);
    self->click_ = -1;
    self->button_clicked();
    return 0;
}

void Button::notify(ButtonAction action)
{
    ButtonEvent ev = {time_us_64(), id_, action};
    if (btn_cb_)
    {
        btn_cb_(ev, user_data_);
    }
    else
    {
        queue_try_add(&queue_, &ev);
    }
}
