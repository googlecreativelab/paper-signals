#ifndef _SERVO_H
#define _SERVO_H

class Servo{
  private:
    int pin;
  public:
    void attach(int pin);
    void write(int angle);
};

#endif