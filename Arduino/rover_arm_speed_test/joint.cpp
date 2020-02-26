#include "joint.h"

joint::joint(int pin, int min, int max, double g, double b)
{
  s = new Servo();
  s->attach(pin);
  s->write(90);
  speed = 90.0;

  setPos = 0;
  pos = 0;
  posPrev = 0;
  posMax = max;
  posMin = min;
  gain = g;
  brake = b;
}

void joint::set(int i)
{
  setPos = i;
}

/*
 * Stops the motor and change the setpoint to the current position.
 */
void joint::freeze()
{
  s->write(90);
  speed = 90.0;
  setPos = pos;
  posPrev = pos;
}

/*
 * Sets the positions to zero and freezes the motor
 * You will need to change the encoder value yourself
 */
void joint::reset()
{
  s->write(90);
  speed = 90.0;
  setPos = 0;
  pos = 0;
  posPrev = 0;
}

/*
 * Updates the servo output speed
 * enc: the encoder position to react to
 * dt: the difference in time since the last call to update()
 */
void joint::update(int enc, int dt)
{
  posPrev = pos;
  pos = enc;

  currSpeed = (pos-posPrev)*1000.0/dt;

  int desiredSpeed = (setPos-pos);

  if (desiredSpeed > 0)
  {
    if (desiredSpeed > SPEED_MAX)
      desiredSpeed = SPEED_MAX;
    else if (desiredSpeed < 10)
      desiredSpeed = 10;
  }
  else
  {
    if (desiredSpeed < -SPEED_MAX)
      desiredSpeed = -SPEED_MAX;
    else if (desiredSpeed > -10)
      desiredSpeed = -10;
  }
  
  if (pos > setPos+ENC_TOLERANCE)
  {
    // should have (-) encoder speed, servo speed > 90
    if (currSpeed < desiredSpeed)
      speed -= gain;
    else if (currSpeed > desiredSpeed)
      speed += gain;
  }
  else if (pos < setPos-ENC_TOLERANCE)
  {
    // should have (+) encoder speeed, servo speed < 90
    if (currSpeed < desiredSpeed)
      speed -= gain;
    else if (currSpeed > desiredSpeed)
      speed += gain;
  }
  else
  {
    if (currSpeed > -SPEED_TOLERANCE && currSpeed < SPEED_TOLERANCE)
      speed = 90.0;
    else if (currSpeed > 0)
      speed += brake;
    else if (currSpeed < 0)
      speed -= brake;
  }

  s->write(speed);
}

/*
 * Getter for the speed of the encoder (ticks/second)
 */
double joint::getSpeed()
{
  return currSpeed;
}

/*
 * Getter for the value written to the servo
 */
double joint::getServoSpeed()
{
  return speed;
}
