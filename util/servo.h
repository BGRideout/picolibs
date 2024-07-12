/*
                    ***** Servo class  *****
*/

#ifndef SERVO_H
#define SERVO_H

#include "pwm.h"

class Servo : public PWM
{
private:

public:
    /**
     * @brief   Constructor for Servo class
     * 
     * @param   gpio    GPIO number for servo control
     */
    Servo(uint32_t gpio) : PWM(gpio, 50) {}

    /**
     * @brief   Destructor for Servo class
     */
    ~Servo() {}

    /**
     * @brief   Set the servo position
     * 
     * @param   angle   Servo position in degrees (-90 to +90)
     */
    void setAngle(float angle);

    /**
     * @brief   Turn off the servo position hold
     */
    void relax();
};

#endif
