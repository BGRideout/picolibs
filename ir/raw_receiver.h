/*
                    *****  Subclass of IR_Receiver for raw data  *****
*/

#ifndef RAW_RECEIVER
#define RAW_RECEIVER

#include "ir_receiver.h"

class RAW_Receiver : public IR_Receiver
{
private:
    uint32_t        *times_;            // Vector to receive times
    uint32_t        n_times_;           // Size of vector
    uint32_t        *countptr_;         // Pointer to variable to receive count
    uint32_t        count_;             // Number of pulses stored

protected:
    /**
     * @brief   Check for message start sync sequence
     * 
     * @details Can use the sync_ counter to determine when sync occurs.
     *          sync_ set to zero by base class before start of new message
     *          but otherwise not altered.
     * 
     * @param   ts      Timestamp of edge
     * @param   falling True if current edge is falling
     * 
     * @return  true if sync sequence received
     */
    bool check_sync(uint64_t ts, bool falling) override;

    /**
     * @brief   Decode received message
     * 
     * @param   addr    Word to receive address
     * @param   func    Word to receive function code
     */
    bool decode_message(uint16_t &addr, uint16_t &func) override;

    bool timeout(bool msg) override;
    void message_complete(uint64_t ts) override;
    void store_pulses();

public:
    /**
     * @brief   Constructor for IR Receiver class
     * 
     * @param   gpio        GPIO numver for reading input
     * @param   edges       Number of edges to be received
     */
    RAW_Receiver(uint32_t gpio, uint32_t edges)
        : IR_Receiver(gpio, edges), times_(nullptr), n_times_(0), countptr_(nullptr), count_(0) {}
    ~RAW_Receiver() {}

    /**
     * @brief   Set the buffer to receive the time data
     *
     * @param   times       Pointer to buffer to receive times
     * @param   n_times     Number of elements in time array
     * @param   count       Pointer to location to receive number of times read
     */
    void set_times(uint32_t *times, uint32_t n_times, uint32_t *count) { times_ = times; n_times_ = n_times; countptr_ = count; }
    
    /**
     * @brief   Read a raw list of timestamps from the IR receiver
     * 
     * @param   times       Array to receive time intervals
     * @param   n_times     Number of elements in times array
     * @param   msec        Time to wait for pulses (milliseconds)
     * @param   edge_tmo    Timeout between edges
     * 
     * @return  Number of time intervals received
     */
    uint32_t read_raw_times(uint32_t *times, uint32_t n_times, uint32_t msec, uint32_t edge_tmo = 0);
};

#endif
