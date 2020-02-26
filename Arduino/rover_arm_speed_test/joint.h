#include <Servo.h>

#ifndef joint_h
#define joint_h

#define SPEED_MAX 100
#define SPEED_TOLERANCE 10
#define ENC_TOLERANCE 2

class joint
{
  private:
    int setPos; // desired position
    int pos, posPrev, posMax, posMin; // encoder positions
    double gain, brake; // tuning constants
    Servo* s;
    double speed; // speed to write to the servo
    double currSpeed; // speed the encoder sees
    
  public:
    joint(int pin, int min, int max, double g, double b);
    void set(int i);
    void update(int enc, int dt);
    void freeze();
    void reset();

    // getters for debug
    double getSpeed();
    int getServoSpeed();
};

#endif
