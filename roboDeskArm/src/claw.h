#include <ESP32Servo.h>

class Claw
{
    public:
        Claw(int pin, int openPos, int closePos = 0);

        void open();
        void close();

        bool isClose();

    private:
        int pin, posOpen, posClose;
        bool isClawClose = true;
        Servo servo;

        void setPos(int pos);
        void setClawOpen();
        void setClawClose();
};