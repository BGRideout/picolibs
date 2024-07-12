/*
                    *****  Servo Class Implementation  *****
*/

#include "servo.h"

void Servo::setAngle(float angle)
{
    if (angle < -90.0) angle = -90.0;
    if (angle > 90.0) angle = 90.0;
    float duty = (angle + 90.0) / 180.0 * 0.095 + 0.025;
    uint16_t level = setDutyCycle(duty);
}

void Servo::relax()
{
    setDutyCycle(0);
}