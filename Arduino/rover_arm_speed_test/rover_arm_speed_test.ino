#include <Servo.h>

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
#define DEBUG_PERIOD 750

// Servo control constants
#define SPEED_PERIOD 20
#define SPEED_GAIN 1
#define SPEED_BRAKE 2
#define SPEED_MAX 100

// encoder positions (updated by interrupt), speeds
volatile int eBase = 0;
int eBaseP = 0;
double speedBase = 0;

volatile int eShoulder = 0;
int eShoulderP = 0;
double speedShoulder = 0;
int setShoulder = 0;

volatile int eElbow = 0;
int eElbowP = 0;
double speedElbow = 0;

volatile int eWristP = 0;
int eWristPP = 0;
double speedWristP = 0;

volatile byte port_k_prev = 0x00; // only used by ISR

// servo objects
Servo sBase;
Servo sShoulder;
Servo sElbow;
Servo sClaw;
Servo sWristP;
Servo sWristR;

// test program vars
String inString = "";
int select = 3; // 3 - default to elbow
bool calibrated = true;
unsigned long timePrint, timeSpeed;




/*
 * Stop motors and set encoder positions to 0
 */
void joints_reset()
{
  sBase.write(90);
  eBase = 0;
  eBaseP = 0;
  speedBase = 0;
  
  sShoulder.write(90);
  eShoulder = 0;
  eShoulderP = 0;
  speedShoulder = 0;

  sElbow.write(90);
  eElbow = 0;
  eElbowP = 0;
  speedElbow = 0;

  sWristP.write(90);
  eWristP = 0;
  eWristPP = 0;
  speedWristP = 0;
}

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
    Serial.print("\tenc: " + String(eShoulder));
    Serial.print("\tspd: " + String(speedShoulder));
    Serial.println("\tsrv: " + String(sShoulder.read()));
  }

  // periodically display encoder positions
  if (millis()-timeSpeed > SPEED_PERIOD)
  {
    unsigned long tempTime = millis();
    int tempShoulder = eShoulder;
    
    int diffTime = tempTime - timeSpeed;
    int diffShoulder = tempShoulder - eShoulderP;
    speedShoulder = (diffShoulder*1000.0)/diffTime;

    eShoulderP = tempShoulder;
    timeSpeed = tempTime;

    int desiredSpeed = setShoulder - eShoulder;
    if (desiredSpeed > SPEED_MAX)
      desiredSpeed = SPEED_MAX;
    if (desiredSpeed < -SPEED_MAX)
      desiredSpeed = -SPEED_MAX;
      
    if (eShoulder < setShoulder)
    {
      if (speedShoulder < 0)
        sShoulder.write(sShoulder.read() - SPEED_BRAKE);
        
      if (speedShoulder < desiredSpeed)
        sShoulder.write(sShoulder.read() - SPEED_GAIN);
      if (speedShoulder > desiredSpeed)
        sShoulder.write(sShoulder.read() + SPEED_GAIN);
      // sShoulder should be < 90
    }
    else if (eShoulder > setShoulder)
    {
      if (speedShoulder > 0)
        sShoulder.write(sShoulder.read() + SPEED_BRAKE);
      
      if (speedShoulder < desiredSpeed)
        sShoulder.write(sShoulder.read() - SPEED_GAIN);
      if (speedShoulder > desiredSpeed)
        sShoulder.write(sShoulder.read() + SPEED_GAIN);
      // sShoulder should be > 90
    }
    else
    {
      sShoulder.write(90);
    }
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
            sBase.write(temp);
            break;
          case 1: // shoulder
            //sShoulder.write(temp);
            setShoulder = temp;
            break;
          case 2: // elbow
            sElbow.write(temp);
            break;
          case 3: // wrist pitch
            sWristP.write(temp);
            break;
        }
      }
      else
      {
        // non-number command
        inString.toLowerCase();
        
        // anything that isn't a number stops all motors
        sBase.write(90);
        sShoulder.write(90);
        sElbow.write(90);
        sClaw.write(90);
        sWristP.write(90);
        sWristR.write(90);
        
        setShoulder = eShoulder;
        
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
          joints_reset();
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
