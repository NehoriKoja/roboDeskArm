#include <ESP32Servo.h>

class NewServo
{
    public:
        NewServo(int pin, int maxDeg = 180, int minDig = 0);
        void setPos(int pos);

    private:
        int pin, maxDeg, minDeg, midDeg, currentPos;
        Servo servo;
};