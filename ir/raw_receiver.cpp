/*
                    *****  Raw IR eceiver Implementation  *****
*/

#include "raw_receiver.h"

bool RAW_Receiver::check_sync(uint64_t ts, bool falling)
{
    //  Wait for the first rising edge
    if (sync_ == 0 && !falling)
    {
        return false;
    }

    sync_ = 1;
    return true;
}

bool RAW_Receiver::decode_message(uint16_t &addr, uint16_t &func)
{
    addr = 0;
    func = 0;
    return true;
}

uint32_t RAW_Receiver::read_raw_times(uint32_t *times, uint32_t n_times, uint32_t msec, uint32_t edge_tmo)
{
    times_ = times;
    n_times_ = n_times;
    n_pulse_ = 0;
    count_ = 0;
    set_bit_timeout(edge_tmo);
    bool ret = wait_for_message(msec);
    times_ = nullptr;
    n_times_ = 0;
    return count_;
}

bool RAW_Receiver::timeout(bool msg)
{
    store_pulses();
    return IR_Receiver::timeout(msg);
}

void RAW_Receiver::message_complete(uint64_t ts)
{
    store_pulses();
    IR_Receiver::message_complete(ts);
}

void RAW_Receiver::store_pulses()
{
    count_ = n_pulse_ < n_times_ ? n_pulse_ : n_times_;
    for (int ii = 0; ii < count_; ii++)
    {
        times_[ii] = pulses_[ii];
    }
}
