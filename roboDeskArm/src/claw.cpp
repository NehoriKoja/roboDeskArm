#include "claw.h"

Claw::Claw(int pin, int openPos, int closePos = 0)
{
    this->pin = pin;
    this->posOpen = openPos;
    this->posClose = closePos;
    this->servo.attach(this->pin);
}

void Claw::open()
{
    setPos(this->posOpen);
    setClawOpen();
}

void Claw::close()
{
    setPos(this->posClose);
    setClawClose();
}

bool Claw::isClose()
{
    return this->isClawClose;
}

void Claw::setPos(int pos)
{
    if (pos == this->posClose && !isClose())
    {
        for (int i = this->posOpen; i > pos; i--)
        {
            servo.write(i);
            delay(20);
        }
    }
    else if (pos == this->posOpen && isClose())
    {
        for (int i = this->posClose; i < pos; i++)
        {
            servo.write(i);
            delay(20);
        }
    }
}

void Claw::setClawOpen()
{
    this->isClawClose = false;
}

void Claw::setClawClose()
{
    this->isClawClose = true;
}