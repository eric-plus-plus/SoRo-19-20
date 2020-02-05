#include <Servo.h>
#include <PID_v1.h>

// Advanced PID Rover Arm Tester (incomplete)
// Written for the Arduino Mega on the rover's arm
// Open the serial monitor to use

// This program lets you set manually set the position of each joint.
// Talons should be calibrated to the range of servo inputs 10-170.
// A position of 0 encoder ticks should be the default position.

// Caution: This code will not stop you from breaking the arm


// TODO: incorporate limit switches
//       method for zeroing positions


// Arduino Mega pins to attach Talon controllers to
#define PIN_S_BASE 30
#define PIN_S_SHOULDER 31
#define PIN_S_ELBOW 32
#define PIN_S_CLAW 33
#define PIN_S_WRISTP 34
#define PIN_S_WRISTR 35

// Port K bits for encoders
#define ENC_BASE_A 0x01     // pin A8
#define ENC_BASE_B 0x02     // pin A9
#define ENC_SHOULDER_A 0x04 // pin A10
#define ENC_SHOULDER_B 0x08 // pin A11
#define ENC_ELBOW_A 0x10    // pin A12
#define ENC_ELBOW_B 0x20    // pin A13
#define ENC_WRISTP_A 0x40   // pin A14
#define ENC_WRISTP_B 0x80   // pin A15

// Encoder position limits
#define MIN_BASE -500          // update this!
#define MAX_BASE 500           // update this!
#define MIN_SHOULDER 0
#define MAX_SHOULDER 470
#define MIN_ELBOW 0
#define MAX_ELBOW 310
#define MIN_WRISTP -278
#define MAX_WRISTP 295

// encoder positions - updated by interrupt
volatile int eBase = 0;
volatile int eShoulder = 0;
volatile int eElbow = 0;
volatile int eWristP = 0;
volatile byte port_k_prev = 0x00; // only used by ISR

// servo objects
Servo sBase;
Servo sShoulder;
Servo sElbow;
Servo sClaw;
Servo sWristP;
Servo sWristR;

// PID stuff
double pidSetBase, pidInBase, pidOutBase;
PID pidBase(&pidInBase, &pidOutBase, &pidSetBase, 1, 1, 0, P_ON_E, REVERSE); // numbers are P, I, D

double pidSetShoulder, pidInShoulder, pidOutShoulder;
PID pidShoulder(&pidInShoulder, &pidOutShoulder, &pidSetShoulder, 1, 1, 0, P_ON_E, REVERSE); // numbers are P, I, D

double pidSetElbow, pidInElbow, pidOutElbow;
PID pidElbow(&pidInElbow, &pidOutElbow, &pidSetElbow, 1, 1, 0, P_ON_E, REVERSE); // numbers are P, I, D

double pidSetWristP, pidInWristP, pidOutWristP;
PID pidWristP(&pidInWristP, &pidOutWristP, &pidSetWristP, 1, 1, 0, P_ON_E, REVERSE); // numbers are P, I, D

// test program vars
String inString = "";
int select = 3; // 3 - default to elbow
unsigned long timePrint;

void setup() {
  // setup servos
  sBase.attach(PIN_S_BASE);
  sShoulder.attach(PIN_S_SHOULDER);
  sElbow.attach(PIN_S_ELBOW);
  sClaw.attach(PIN_S_CLAW);
  sWristP.attach(PIN_S_WRISTP);
  sWristR.attach(PIN_S_WRISTR);
  
  sBase.write(90);
  sShoulder.write(90);
  sElbow.write(90);
  sClaw.write(90);
  sWristP.write(90);
  sWristR.write(90);

  // setup encoder pins and interrupts
  cli();
  DDRK &= 0x00; // set encoder pins (all 8 port k bits) as inputs
  PORTK |= 0xFF; // enable pullups on encoder pins
  PCICR |= 0x04; // enable port K pin change interrupt
  PCMSK2 |= 0xFF; // enable interrupt for all port K pins
  sei();

  port_k_prev = PINK;
  
  // setup serial
  Serial.begin(9600);

  Serial.println("Enter a number to set a speed:");
  Serial.println("90 is still (if motor controllers are calibrated).");
  Serial.println("Talon motor controllers should be calibrated to range of 10-170.");
  Serial.println();
  Serial.println("Enter a character to select motors:");
  Serial.println("A = base");
  Serial.println("B = shoulder");
  Serial.println("C = elbow (default)");
  Serial.println("D = wrist pitch");
  
  // setup PID stuff
  pidBase.SetOutputLimits(-255, 255);
  pidInBase = map(eBase, MIN_BASE, MAX_BASE, -255, 255); // map encoder range to 0-255
  pidSetBase = map(0, MIN_BASE, MAX_BASE, -255, 255);
  pidBase.SetSampleTime(20);
  pidBase.SetMode(AUTOMATIC);

  pidShoulder.SetOutputLimits(-255, 255);
  pidInShoulder = map(eShoulder, MIN_SHOULDER, MAX_SHOULDER, -255, 255); // map encoder range to 0-255
  pidSetShoulder = map(0, MIN_SHOULDER, MAX_SHOULDER, -255, 255);
  pidShoulder.SetSampleTime(20);
  pidShoulder.SetMode(AUTOMATIC);

  pidElbow.SetOutputLimits(-255, 255);
  pidInElbow = map(eWristP, MIN_ELBOW, MAX_ELBOW, -255, 255); // map encoder range to 0-255
  pidSetElbow = map(0, MIN_ELBOW, MAX_ELBOW, -255, 255);
  pidElbow.SetSampleTime(20);
  pidElbow.SetMode(AUTOMATIC);

  pidWristP.SetOutputLimits(-255, 255);
  pidInWristP = map(eWristP, MIN_WRISTP, MAX_WRISTP, -255, 255); // map encoder range to 0-255
  pidSetWristP = map(0, MIN_WRISTP, MAX_WRISTP, -255, 255);
  pidWristP.SetSampleTime(20);
  pidWristP.SetMode(AUTOMATIC);
}

void loop() {
  // handle PID stuff periodically
  pidInBase = map(eBase, MIN_BASE, MAX_BASE, -255, 255); // map encoder range to 0-255
  pidBase.Compute();
  sBase.write(map(pidOutBase, -255, 255, 10, 170));

  pidInShoulder = map(eShoulder, MIN_SHOULDER, MAX_SHOULDER, -255, 255); // map encoder range to 0-255
  pidShoulder.Compute();
  sShoulder.write(map(pidOutShoulder, -255, 255, 10, 170));
  
  pidInElbow = map(eElbow, MIN_ELBOW, MAX_ELBOW, -255, 255); // map encoder range to 0-255
  pidElbow.Compute();
  sElbow.write(map(pidOutElbow, -255, 255, 10, 170));
  
  pidInWristP = map(eWristP, MIN_WRISTP, MAX_WRISTP, -255, 255); // map encoder range to 0-255
  pidWristP.Compute();
  sWristP.write(map(pidOutWristP, -255, 255, 10, 170));
  
  // read and handle serial terminal commands
  while (Serial.available() > 0)
  {
    int inChar = Serial.read();
    if (inChar == '\n')
    {
      // check for number
      bool num = true;
      if (inString.length() == 0)
        num = false;
      for (int i = 0; i < inString.length(); i++)
      {
        if (i == 0)
        {
          if (!isDigit(inString[i]) && inString[i] != '-')
          {
            num = false;
            break;
          }
        }
        else if (!isDigit(inString[i]))
        {
          num = false;
          break;
        }
      }
      if (num)
      {
        // if a number, set the position
        int temp = inString.toInt();
        Serial.println(temp);
        
        switch(select)
        {
          case 0: // base
            pidSetBase = map(temp, MIN_BASE, MAX_BASE, -255, 255);
            break;
          case 1: // shoulder
            pidSetShoulder = map(temp, MIN_SHOULDER, MAX_SHOULDER, -255, 255);
            break;
          case 2: // elbow
            pidSetElbow = map(temp, MIN_ELBOW, MAX_ELBOW, -255, 255);
            break;
          case 3: // wrist pitch
            pidSetWristP = map(temp, MIN_WRISTP, MAX_WRISTP, -255, 255);
            break;
        }
      }
      else
      {
        // anything that isn't a number changes all setpoints to the current position (should stop them)
        pidSetBase = map(eBase, MIN_BASE, MAX_BASE, -255, 255);
        pidSetShoulder = map(eShoulder, MIN_SHOULDER, MAX_SHOULDER, -255, 255);
        pidSetElbow = map(eElbow, MIN_ELBOW, MAX_ELBOW, -255, 255);
        pidSetWristP = map(eWristP, MIN_WRISTP, MAX_WRISTP, -255, 255);

        if (inString == "A" || inString == "a")
        {
          select = 0;
          Serial.println("Selected Base");
        }
        else if (inString == "B" || inString == "b")
        {
          select = 1;
          Serial.println("Selected Shoulder");
        }
        else if (inString == "C" || inString == "c")
        {
          select = 2;
          Serial.println("Selected Elbow");
        }
        else if (inString == "D" || inString == "d")
        {
          select = 3;
          Serial.println("Selected Claw");
        }
        else
        {
          Serial.println("IDK what that means bro");
        }
      }

      inString = "";
    }
    else
    {
      // add character to string
      inString += (char)inChar;
    }
  }

  // periodically display encoder positions
  if (millis()-timePrint > 750)
  {
    timePrint = millis();
    //Serial.println("B " + String(eBase) + "\tS " + String(eShoulder) + "\tE " + String(eElbow) + "\tW " + String(eWristP));
    
    Serial.print("B: " + String(pidSetBase) + ", " + String(pidOutBase));
    Serial.print("\tS: " + String(pidSetShoulder) + ", " + String(pidOutShoulder));
    Serial.print("\tE: " + String(pidSetElbow) + ", " + String(pidOutElbow));
    Serial.println("\tW: " + String(pidSetWristP) + ", " + String(pidOutWristP));
  }
}

ISR(PCINT2_vect) // pin change interrupt for pins A8 to A15 (update encoder positions)
{
  byte port_k_pins = PINK;

  // base
  if ((port_k_pins & ENC_BASE_A) && !(port_k_prev & ENC_BASE_A)) // rising edge on A
  {
    if (port_k_pins & ENC_BASE_B)
      eBase--; // CCW
    else
      eBase++; // CW
  }
  else if (!(port_k_pins & ENC_BASE_A) && (port_k_prev & ENC_BASE_A)) // falling edge on A
  {
    if (port_k_pins & ENC_BASE_B)
      eBase++; // CW
    else
      eBase--; // CCW
  }

  // shoulder
  if ((port_k_pins & ENC_SHOULDER_A) && !(port_k_prev & ENC_SHOULDER_A)) // rising edge on A
  {
    if (port_k_pins & ENC_SHOULDER_B)
      eShoulder--; // CCW
    else
      eShoulder++; // CW
  }
  else if (!(port_k_pins & ENC_SHOULDER_A) && (port_k_prev & ENC_SHOULDER_A)) // falling edge on A
  {
    if (port_k_pins & ENC_SHOULDER_B)
      eShoulder++; // CW
    else
      eShoulder--; // CCW
  }

  // elbow
  if ((port_k_pins & ENC_ELBOW_A) && !(port_k_prev & ENC_ELBOW_A)) // rising edge on A
  {
    if (port_k_pins & ENC_ELBOW_B)
      eElbow--; // CCW
    else
      eElbow++; // CW
  }
  else if (!(port_k_pins & ENC_ELBOW_A) && (port_k_prev & ENC_ELBOW_A)) // falling edge on A
  {
    if (port_k_pins & ENC_ELBOW_B)
      eElbow++; // CW
    else
      eElbow--; // CCW
  }

  // wrist pitch
  if ((port_k_pins & ENC_WRISTP_A) && !(port_k_prev & ENC_WRISTP_A)) // rising edge on A
  {
    if (port_k_pins & ENC_WRISTP_B)
      eWristP--; // CCW
    else
      eWristP++; // CW
  }
  else if (!(port_k_pins & ENC_WRISTP_A) && (port_k_prev & ENC_WRISTP_A)) // falling edge on A
  {
    if (port_k_pins & ENC_WRISTP_B)
      eWristP++; // CW
    else
      eWristP--; // CCW
  }
  
  port_k_prev = port_k_pins;
}
