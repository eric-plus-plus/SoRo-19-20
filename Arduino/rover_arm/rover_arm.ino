#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>

#define ENC_1_A 0x01 // pin A8
#define ENC_1_B 0x02 // pin A9
#define ENC_2_A 0x04 // pin A10
#define ENC_2_B 0x08 // pin A11
#define ENC_3_A 0x10 // pin A12
#define ENC_3_B 0x20 // pin A13
#define ENC_4_A 0x40 // pin A14
#define ENC_4_B 0x80 // pin A15

//pin values
char _yawMotor = A0;//Pin for yaw (base) motor
char _shoulderMotor = A1;//Pin for shoulder motor
char _elbowMotor = 4;//Pin for elbow motor
char _wristPitchMotor = A3;//Pin for wrist pitch motor
char _wristRollMotor = A4;//Pin for wrist motor

char _yawPot = A5;//Pin for yaw (base) potentiometer
char _shoulderPot = A6;//Pin for shoulder potentiometer
char _elbowPot = A2;//Pin for elbow potentiometer
char _wristPitchPot = 0;//Pin for wrist pitch potentiometer
char _wristRollPot = 1;//Pin for wrist roll potentiometer

Servo yaw, shoulder, elbow, wristPitch;

//limit switches
#define LIMIT_BASE 36
#define LIMIT_SHOULDER_MIN 37
#define LIMIT_SHOULDER_MAX 38
#define LIMIT_ELBOW_MIN 39
#define LIMIT_ELBOW_MAX 40
#define LIMIT_WRIST_MIN 41
#define LIMIT_WRIST_MAX 42
#define LIMIT_CLAW 43

bool limitBase = false;
bool limitShoulderMin = false;
bool limitShoulderMax = false;
bool limitElbowMin = false;
bool limitElbowMax = false;
bool limitWristMin = false;
bool limitWristMax = false;
bool limitClaw = false;

//UDP stuff

#define DEVICE_ID 1

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45
};
IPAddress ip(192, 168, 1, 140);

unsigned int localPort = 2040;      // local port to listen on

// packet related variables
char packetBuffer[30];  // buffer to hold incoming packet
EthernetUDP Udp;

//End UDP stuff

//Arm stuff

// message data to store
int potYaw;//Base
int potShoulder;
int potElbow;
int potWristPitch;
int potWristRoll;
bool buttonClawOpen;
bool buttonClawClose;

// encoder values
int controlPos[] = {0, 0, 0, 0};
byte port_k_prev = 0x00;

//yaw (base) angle (perpendicular to ground)
float yawAngle = 0;
//shoulder angle (perpendicular to ground)
float shoulderAngle = 0;
//elbow angle (perpendicular to ground)
float elbowAngle = 0;
//wrist pitch angle (perpendicular to ground)
float wristPitchAngle = 0;

void setup() {
  cli();
  // set encoder pins as inputs, enable pullups
  DDRK &= 0x00;
  PORTK |= 0xFF;

  PCICR |= 0x04; // enable port B pin change interrupt
  PCMSK2 |= 0xFF; // enable interrupt for all port B pins
  sei();

  port_k_prev = PINK;
  
  Serial.begin(9600);
  
  //UDP stuff
  Ethernet.init(10);  // Most Arduino shields
  Ethernet.begin(mac, ip);

  // Open serial communications and wait for port to open:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  Udp.begin(localPort);
  //End UDP stuff

  //Start arm stuff
  yaw.attach(30);//TODO: update this line
  shoulder.attach(32);
  elbow.attach(31);
  wristPitch.attach(34);

  //set the pinmode of the motor ports to be output
  pinMode(_yawMotor, OUTPUT);
  pinMode(_shoulderMotor, OUTPUT);
  pinMode(_elbowMotor, OUTPUT);
  pinMode(_wristPitchMotor, OUTPUT);
  pinMode(_wristRollMotor, OUTPUT);

  //set the pinmode of the potentiometer ports to be input
  pinMode(_yawPot, INPUT);
  pinMode(_shoulderPot, INPUT);
  pinMode(_elbowPot, INPUT);
  pinMode(_wristPitchPot, INPUT);
  pinMode(_wristRollPot, INPUT);

  // limit switches
  pinMode(LIMIT_BASE, INPUT);
  pinMode(LIMIT_SHOULDER_MIN, INPUT);
  pinMode(LIMIT_SHOULDER_MAX, INPUT);
  pinMode(LIMIT_ELBOW_MIN, INPUT);
  pinMode(LIMIT_ELBOW_MAX, INPUT);
  pinMode(LIMIT_WRIST_MIN, INPUT);
  pinMode(LIMIT_WRIST_MAX, INPUT);
  pinMode(LIMIT_CLAW, INPUT);

  digitalWrite(LIMIT_BASE, HIGH);
  digitalWrite(LIMIT_SHOULDER_MIN, HIGH);
  digitalWrite(LIMIT_SHOULDER_MAX, HIGH);
  digitalWrite(LIMIT_ELBOW_MIN, HIGH);
  digitalWrite(LIMIT_ELBOW_MAX, HIGH);
  digitalWrite(LIMIT_WRIST_MIN, HIGH);
  digitalWrite(LIMIT_WRIST_MAX, HIGH);
  digitalWrite(LIMIT_CLAW, HIGH);
  
  //End arm stuff
  zero_encoders();
}

void loop() {
  limit_switches();
  
  if(read_data())
  {
    yawAngle = potYaw*3/10;
    shoulderAngle = potShoulder*3/10;
    elbowAngle = potElbow*3/10;
    wristPitchAngle = potWristPitch*3/10;

//    Serial.print(yawAngle);
//    Serial.print(", ");
//    Serial.print(shoulderAngle);
//    Serial.print(", ");
//    Serial.print(elbowAngle);
//    Serial.print(", ");
//    Serial.print(wristPitchAngle);
//    Serial.print(", ");
//    Serial.print(buttonClawOpen);
//    Serial.print(", ");
//    Serial.println(buttonClawClose);   
  }

  Serial.print(float(controlPos[0])*3.0/10.0);
  Serial.print(", ");
  Serial.print(float(controlPos[1])*3.0/10.0);
  Serial.print(", ");
  Serial.print(float(controlPos[2])*3.0/10.0);
  Serial.print(", ");
  Serial.println(float(controlPos[3])*3.0/10.0);

  //move_joint(yawAngle, float(controlPos[3])*3.0/10.0, &yaw);
  //move_joint_backwards(shoulderAngle, float(controlPos[0])*3.0/10.0, &shoulder);
  move_joint_backwards(elbowAngle, float(controlPos[1])*3.0/10.0, &elbow);
  //move_joint(wristPitchAngle, float(controlPos[2])*3.0/10.0, &wristPitch);
  delay(10);
}

void move_joint(float desiredPos, float currPos, Servo* joint)
{
  if(true)//if(desiredPosrround((90 * dif / 360.0) * 10.0 ) / 10.0ound((90 * dif / 360.0) * 10.0 ) / 10.0 >= 0)
  {
    float dif = currPos - desiredPos;
    float distance = round((90 * dif / 360.0) * 10.0 ) / 10.0;
    if(distance != 0)
    {
      if(distance > 0)
      {
        distance += 2;
        if(distance > 90)
          distance = 90;
      }
      if(distance < 0)
      {
        distance -= 2;
        if(distance < -90)
          distance = -90;
      }
    }
    Serial.println(distance);
    joint->write(90 - distance);
  }
}

void move_joint_backwards(float desiredPos, float currPos, Servo* joint)
{
  if(true)//if(desiredPos >= 0)
  {
    float dif = currPos - desiredPos;
    float distance = dif;
    if(distance != 0)
    {
      if(distance > 0)
      {
        if(distance < 5)
          distance = 5;
        if(distance > 90)
          distance = 90;
      }
      if(distance < 0)
      {
        if(distance > -5)
          distance = -5;
        if(distance < -90)
          distance = -90;
      }
      joint->write(90 + distance);
    }
    else
    {
      joint->write(90);
    }
    //Serial.print(desiredPos);
    //Serial.print("  ");
    //Serial.println(distance);
    
    
  }
}

bool read_data()
{
  // the message format (14 bytes total):
  // (-127) | Device ID (1) | yaw (high) | yaw (low) | shoulder (high) ...
  // ... | shoulder (low) | elbow (high) | elbow (low) | wrist pitch (high) ...
  // ... | wrist pitch (low) | wrist roll (high) | wrist roll (low) | buttons | hash
  
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize == 0)
  {
    return false;
  }
  
  // read the packet into packetBufffer
  Udp.read(packetBuffer, packetSize);
      
//  // serial debug
//  Serial.println("Raw data");
//  for(int i = 0; i < packetSize; i++)
//  {
//    Serial.print((int)(packetBuffer[i]));
//    Serial.print(", ");
//  }
//  Serial.print("\n");
  
  if(packetSize != 14)
  {
    Serial.println("Message is wrong length!");
    return false;
  }

  if(packetBuffer[0] != -127)
  {
    Serial.println("Message does not start with -127!");
    return false;
  }

  if(packetBuffer[1] != 1)
  {
    Serial.println("Wrong device id!");
    return false;
  }
  
  char hash = (packetBuffer[2] + packetBuffer[3] + packetBuffer[4] + packetBuffer[5] + packetBuffer[6] + packetBuffer[7] + packetBuffer[8] + packetBuffer[9] + packetBuffer[10] + packetBuffer[11] + packetBuffer[12]) / 11;
  if(hash != packetBuffer[13])
  {
    Serial.println("Wrong hash!");
    return false;
  }

  potYaw = (int)(packetBuffer[2] << 8) + (byte)packetBuffer[3];
  potShoulder = (int)(packetBuffer[4] << 8) + (byte)packetBuffer[5];
  potElbow = (int)(packetBuffer[6] << 8) + (byte)packetBuffer[7];
  potWristPitch = (int)(packetBuffer[8] << 8) + (byte)packetBuffer[9];
  potWristRoll = (int)(packetBuffer[10] << 8) + (byte)packetBuffer[11];

  buttonClawOpen = ((packetBuffer[12] & 0x01) > 0);  // 2^0 bit
  buttonClawClose = ((packetBuffer[12] & 0x02) > 0); // 2^1 bit

  return true;
}

void limit_switches()
{
  if(digitalRead(LIMIT_SHOULDER_MIN) == LOW)
  {
    controlPos[0] = 0;
  }

  //if(digitalRead(LIMIT_SHOULDER_MAX) == LOW)
  //{
    //controlPos[0] = 0;
  //}

  if(digitalRead(LIMIT_ELBOW_MIN) == LOW)
  {
    controlPos[1] = 0;
  }

  //if(digitalRead(LIMIT_ELBOW_MAX) == LOW)
  //{
    //controlPos[1] = 0;
  //}

  if(digitalRead(LIMIT_WRIST_MIN) == LOW)
  {
    controlPos[2] = 0;
  }

  //if(digitalRead(LIMIT_WRIST_MAX) == LOW)
  //{
    //controlPos[2] = 0;
  //}

  //if(digitalRead(LIMIT_CLAW) == LOW)
  //{
    //controlPos[0] = 0;
  //}
}

void zero_encoders()
{
  Serial.println("Zeroing");
  
  while(digitalRead(LIMIT_SHOULDER_MIN) == HIGH)
    shoulder.write(100);
  shoulder.write(90);

  while(digitalRead(LIMIT_ELBOW_MIN) == HIGH)
    elbow.write(100);
  elbow.write(90);
}

ISR(PCINT2_vect) // pin change interrupt for D8 to D13
{
  byte port_k_pins = PINK;

  // encoder 1
  if ((port_k_pins & ENC_1_A) && !(port_k_prev & ENC_1_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_1_B)
    {
      // CCW
      controlPos[0]--;
    }
    else
    {
      // CW
      controlPos[0]++;
    }
  }
  else if (!(port_k_pins & ENC_1_A) && (port_k_prev & ENC_1_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_1_B)
    {
      // CW
      controlPos[0]++;
    }
    else
    {
      // CCW
      controlPos[0]--;
    }
  }

  // encoder 2
  if ((port_k_pins & ENC_2_A) && !(port_k_prev & ENC_2_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_2_B)
    {
      // CCW
      controlPos[1]--;
    }
    else
    {
      // CW
      controlPos[1]++;
    }
  }
  else if (!(port_k_pins & ENC_2_A) && (port_k_prev & ENC_2_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_2_B)
    {
      // CW
      controlPos[1]++;
    }
    else
    {
      // CCW
      controlPos[1]--;
    }
  }

  // encoder 3
  if ((port_k_pins & ENC_3_A) && !(port_k_prev & ENC_3_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_3_B)
    {
      // CCW
      controlPos[2]--;
    }
    else
    {
      // CW
      controlPos[2]++;
    }
  }
  else if (!(port_k_pins & ENC_3_A) && (port_k_prev & ENC_3_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_3_B)
    {
      // CW
      controlPos[2]++;
    }
    else
    {
      // CCW
      controlPos[2]--;
    }
  }

  // encoder 4
  if ((port_k_pins & ENC_4_A) && !(port_k_prev & ENC_4_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_4_B)
    {
      // CCW
      controlPos[3]--;
    }
    else
    {
      // CW
      controlPos[3]++;
    }
  }
  else if (!(port_k_pins & ENC_4_A) && (port_k_prev & ENC_4_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_4_B)
    {
      // CW
      controlPos[3]++;
    }
    else
    {
      // CCW
      controlPos[3]--;
    }
  }
  
  port_k_prev = port_k_pins;
}
