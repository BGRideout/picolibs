/*
                    *****  Sound Class  *****
*/

#ifndef SOUND_H
#define SOUND_H

#include "pwm.h"
#include <stdint.h>

class Sound
{
private:
    PWM                 *pwm_;              // PWM object
    const uint8_t       *data_;             // Sound data pointer
    uint32_t            datasz_;            // Sound data size
    uint32_t            sample_rate_;       // Sound sample rate
    uint32_t            idx_;               // Output index
    uint32_t            slice_num_;         // PWM slice number
    uint32_t            chan_num_;          // PWM channel number
    uint16_t            scale_;             // Scaling
    uint8_t             repeat_;            // Repeat count
    bool                loop_;              // Loop flag
    uint32_t            count_;             // Tone / beep counter
    float               tone_f_;            // Tone frequency
    float               tone_v_;            // Tone volume
    int                 tone_p_;            // Tone counts per cycle

    static void handler();
    void next_pulse();

    Sound();
    ~Sound();
    static Sound        *singleton_;

public:
    /**
     * @brief   Get the Bell singleton
     * 
     * @return  Pointer to Bell singleton
     */
    static Sound *get() { if (!singleton_) { singleton_ = new Sound(); } return singleton_; }
    /**
     * @brief   Start Bell singleton
     * 
     * @param   gpio        GPIO for sound output
     */
    void start(uint32_t gpio);

    /**
     * @brief   Stop Bell singleton
     */
    void stop();

    /**
     * @brief   Set sound data
     * 
     * @param   data        Pointer to sound data (must be persistent)
     * @param   size        Size of sound data
     * @param   sample_rate Sample rate of sound data
     */
    void setSoundData(const uint8_t *data, uint32_t size, uint32_t sample_rate=22050);

    /**
     * @brief   Play sound file once
     * 
     * @param   repeat  Number of rings to perform
     */
    void playOnce(uint8_t repeat=1);

    /**
     * @brief   Play sound file continuously
     */
    void startPlaying();

    /**
     * @brief   Stop continuous play
     */
    void stopPlaying();

    /**
     * @brief   Test if actively playing
     */
    bool isPlaying() const;

    /**
     * @brief   Output a continuous sine wave tone
     * 
     * @param   freq        Frequency of tone
     * @param   msec        Duration of tone (milliseconds)
     * @param   vol         Volume (0.0 to 1.0)
     */
    void tone(uint32_t freq, uint32_t msec, float vol = 1.0);

    /**
     * @brief   Output a continuous square wave
     * 
     * @param   freq        Frequency of tone
     * @param   msec        Duration of tone (milliseconds)
     */
    void beep(uint32_t freq, uint32_t msec);

    /**
     * @brief   Wait for sound to complete
     * 
     * @param   timeout     Maximum wait time (millieconds). Zero is infinite.
     */
    void wait(uint32_t timeout=0);
};

#endif
