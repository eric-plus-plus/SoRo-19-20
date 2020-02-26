#include "joint.h"

// Arduino Mega pins to attach Talon controllers to
#define PIN_S_BASE 30
#define PIN_S_SHOULDER 31
#define PIN_S_ELBOW 32
#define PIN_S_CLAW 33
#define PIN_S_WRISTP 34
#define PIN_S_WRISTR 35

// Arduino Mega pins to attach limit switches to
#define PIN_L_BASE 36
#define PIN_L_SHOULDER_MIN 37
#define PIN_L_SHOULDER_MAX 38
#define PIN_L_ELBOW_MIN 39
#define PIN_L_ELBOW_MAX 40
#define PIN_L_WRISTP_MIN 41 // unused?
#define PIN_L_WRISTP_MAX 42 // unused?
#define PIN_L_CLAW 43

// Port K bits (pins) for encoders
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

// Debug constants
#define DEBUG_PERIOD 500

// Servo control constants
#define SPEED_PERIOD 50 // if you make this too low then the speed accuracy won't be very good

// encoder positions (updated by interrupt), speeds
volatile int eBase = 0;
volatile int eShoulder = 0;
volatile int eElbow = 0;
volatile int eWristP = 0;
volatile byte port_k_prev = 0x00; // only used by ISR

// test program vars
String inString = "";
int select = 3; // 3 - default to elbow
bool calibrated = true;
unsigned long timePrint, timeSpeed, timeSpeedPrev;

// joint objects
joint *jBase;
joint *jShoulder;
joint *jElbow;
joint *jWristP;
Servo sWristR;
Servo sClaw;

void setup() {
  jBase = new joint(PIN_S_BASE, MIN_BASE, MAX_BASE, 1.0, 0.5);
  jShoulder = new joint(PIN_S_SHOULDER, MIN_SHOULDER, MAX_SHOULDER, 1.0, 0.5);
  jElbow = new joint(PIN_S_ELBOW, MIN_ELBOW, MAX_ELBOW, 1.0, 0.5);
  jWristP = new joint(PIN_S_WRISTP, MIN_WRISTP, MAX_WRISTP, 1.0, 0.5);
  
  sWristR.attach(PIN_S_WRISTR);
  sWristR.write(90);
  sClaw.attach(PIN_S_CLAW);
  sClaw.write(90);

  // setup encoder pins and interrupts
  cli();
  DDRK &= 0x00; // set encoder pins (all 8 port k bits) as inputs
  PORTK |= 0xFF; // enable pullups on encoder pins
  PCICR |= 0x04; // enable port K pin change interrupt
  PCMSK2 |= 0xFF; // enable interrupt for all port K pins
  sei();

  port_k_prev = PINK;

  // setup limit switch pins (uses pull up resistors)
  pinMode(PIN_L_BASE, INPUT);
  digitalWrite(PIN_L_BASE, HIGH);
  pinMode(PIN_L_SHOULDER_MIN, INPUT);
  digitalWrite(PIN_L_SHOULDER_MIN, HIGH);
  pinMode(PIN_L_SHOULDER_MAX, INPUT);
  digitalWrite(PIN_L_SHOULDER_MAX, HIGH);
  pinMode(PIN_L_ELBOW_MIN, INPUT);
  digitalWrite(PIN_L_ELBOW_MIN, HIGH);
  pinMode(PIN_L_ELBOW_MAX, INPUT);
  digitalWrite(PIN_L_ELBOW_MAX, HIGH);
  pinMode(PIN_L_WRISTP_MIN, INPUT);
  digitalWrite(PIN_L_WRISTP_MIN, HIGH);
  pinMode(PIN_L_WRISTP_MAX, INPUT);
  digitalWrite(PIN_L_WRISTP_MAX, HIGH);
  pinMode(PIN_L_CLAW, INPUT);
  digitalWrite(PIN_L_CLAW, HIGH);
  
  // setup serial
  Serial.begin(9600);
}

void loop() {
  // periodically display encoder positions
  if (millis()-timePrint > DEBUG_PERIOD)
  {
    timePrint = millis();
    Serial.print("enc: " + String(eShoulder));
    Serial.print("\tspd: " + String(jShoulder->getSpeed()));
    Serial.println("\tsrv: " + String(jShoulder->getServoSpeed()));
  }

  // periodically display encoder positions
  if (millis()-timeSpeed > SPEED_PERIOD)
  {
    timeSpeedPrev = timeSpeed;
    timeSpeed = millis();

    int timeDiff = timeSpeed-timeSpeedPrev;
    jBase->update(eBase, timeDiff);
    jShoulder->update(eShoulder, timeDiff);
    jElbow->update(eElbow, timeDiff);
    jWristP->update(eWristP, timeDiff);
  }
  
  // read and handle serial terminal commands
  while (Serial.available() > 0)
  {
    int inChar = Serial.read();
    if (inChar == '\n')
    {
      // don't allow commands when calibrating
      if (!calibrated)
      {
        Serial.println("Wait for me to finish calibrating!");
        break;
      }
      
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
        // number command - set the position
        int temp = inString.toInt();
        Serial.println(temp);
        
        switch(select)
        {
          case 0: // base
            jBase->set(temp);
            break;
          case 1: // shoulder
            jShoulder->set(temp);
            break;
          case 2: // elbow
            jElbow->set(temp);
            break;
          case 3: // wrist pitch
            jWristP->set(temp);
            break;
        }
      }
      else
      {
        // non-number command
        inString.toLowerCase();

        // TODO: stop all motors here
        
        if (inString == "base" || inString == "b")
        {
          select = 0;
          Serial.println("Selected Base");
        }
        else if (inString == "shoulder" || inString == "s")
        {
          select = 1;
          Serial.println("Selected Shoulder");
        }
        else if (inString == "elbow" || inString == "e")
        {
          select = 2;
          Serial.println("Selected Elbow");
        }
        else if (inString == "wrist" || inString == "w")
        {
          select = 3;
          Serial.println("Selected Wrist");
        }
        else if (inString == "reset" || inString == "r")
        {
          // TODO
          Serial.println("Reset positions");
        }
        else if (inString == "zero" || inString == "z")
        {
          Serial.println("This isn't implemented yet.");
        }
        else
        {
          Serial.println("Stopping (unknown command)");
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
