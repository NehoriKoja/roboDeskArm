#include "NewServo.h"

NewServo::NewServo(int pin, int maxDeg, int minDeg)
{
    this->pin = pin;
    this->maxDeg = maxDeg;
    this->minDeg = minDeg;
    this->midDeg = (maxDeg+minDeg)/2;
    this->currentPos = this->midDeg;

    this->servo.attach(pin);
    this->servo.write(this->currentPos);
}

void NewServo::setPos(int pos)
{
    if (pos < minDeg) pos = minDeg;
    if (pos > maxDeg) pos = maxDeg;

    if (currentPos < pos)
    {
        for (int i = currentPos; i <= pos; i++)
        {
            servo.write(i);
            delay(20);
        }
    }
    else if (currentPos > pos)
    {
        for (int i = currentPos; i >= pos; i--)
        {
            servo.write(i);
            delay(20);
        }
    }

    currentPos = pos;
}