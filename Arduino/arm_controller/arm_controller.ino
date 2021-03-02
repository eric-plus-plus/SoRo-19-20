#include <Servo.h>

#define BASE_PIN A0
#define SHOULDER_PIN A5
#define ELBOW_PIN 4
#define CLAW_L_PIN 7
#define CLAW_R_PIN 8
#define ENC_L_PIN 10
#define ENC_R_PIN 12

const char DEVICE_ID = 1;

// pin mapping
Servo base, shoulder, elbow, clawL, clawR;
// pin  3, 5, 6, 9, 10, and 11 are for analogWrite
const int wristL1 = 5, wirstL2 = 6, wristR1 = 3, wristR2 = 11;

// math constants
const float ticksPerRev = 1669656;
const float ticksRatio = 360.0 / ticksPerRev;
const int wristTopSpeed = 127;// 255 max for analog write
const int motorDeadZone = 1;

char myHash = 90;
int elbow_pos = 135;
int wristTheta_dest = 0;
int wristPhi_dest = 0;
int clawR_pos = 120;
int clawL_pos = 58;
int shoulder_pos = 135;
int base_pos = 0;
long lTicks = 0; // ticks since last update
long rTicks = 0;
bool rForward = true;
bool lForward = true;

int wristTheta = 0;
int wristPhi = 0;


char incomingByte;
int bytesRead = 0;

char serResp[] = "1,190,200!!!!!!!!!!!!!!!!!!!!!!!!!\n";
char serialHash;
bool inTransmission = false;

//setup wheels
void setup() {
  base.attach(BASE_PIN);
  shoulder.attach(SHOULDER_PIN);
  elbow.attach(ELBOW_PIN);
  clawL.attach(CLAW_L_PIN);
  clawR.attach(CLAW_R_PIN);
  shoulder.write(shoulder_pos);
  elbow.write(elbow_pos);
  clawL.write(clawL_pos);
  clawR.write(clawR_pos);

  attachInterrupt(digitalPinToInterrupt(ENC_L_PIN), leftEncoder, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_R_PIN), rightEncoder, RISING);
  Serial.begin(9600);
  delay(10);
}

void loop() {
  if (Serial.available() > 4) // trying to read from an empty buffer usually breaks stuff
  { /*

      incomingByte = Serial.read();
      if(incomingByte == -127)
      {
      Serial.println("start flag");
      if(Serial.read() != DEVICE_ID)
      {
       Serial.println("incorrect id");
       // transmission is invalid, clear buffer and then reply with device id
       Serial.flush();
       Serial.write(-126);
       Serial.write(DEVICE_ID);
      }
      else{
       Serial.println("reading message");
       shoulder_pos = uint8_t(Serial.read());
       elbow_pos = uint8_t(Serial.read());
       clawL_pos = uint8_t(Serial.read());
       clawR_pos = uint8_t(Serial.read());
       serialHash = Serial.read();
       myHash = (shoulder_pos + elbow_pos+clawL_pos+ clawR_pos)/4;
       if(myHash == serialHash)
       {
         sprintf(serResp, "%d\t%d\t%d\t%d", shoulder_pos, elbow_pos, clawL_pos, clawR_pos);
         Serial.println(serResp);
         updateServos();
       } else {
         sprintf(serResp, "%d\t%d\t%d\t%d!!!!!!!", shoulder_pos, elbow_pos, clawL_pos, clawR_pos);
         Serial.println(serResp);
       }
      }
      }
      }
      else
      {
      //Serial.println("no bytes to read");
      //Serial.read();// dispose packets untill the start flag(-127)
      }

      /**/
    incomingByte = Serial.read();
    if (incomingByte == -127 && !inTransmission) {
      inTransmission = true;
      bytesRead = 0;
    } else if (inTransmission) {
      switch (bytesRead) {
        case 0: // device id
          if (incomingByte != DEVICE_ID) {
            // transmission is invalid, clear buffer and then reply with device id
            Serial.flush();
            Serial.write(-126);
            Serial.write(DEVICE_ID);
            inTransmission = false;
          }
          break;
        case 1:
          base_pos = uint8_t(incomingByte);
          break;
        case 2: // shoulder
          shoulder_pos = uint8_t(incomingByte);
          break;
        case 3: // elbow
          elbow_pos = uint8_t(incomingByte);
          break;
        case 4:
          wristTheta_dest = int8_t(incomingByte);// converts from int to float. change the transmission
          break;
        case 5:
          wristPhi_dest = int8_t( incomingByte);// TODO change the transmission to be a float for precision
          break;
        case 6:
          clawL_pos = uint8_t(incomingByte);
          break;
        case 7:
          clawR_pos = uint8_t(incomingByte);
          break;
        case 8: // hash (last byte recieved in transmission)
          serialHash = incomingByte;
          inTransmission = false;
          myHash = (base_pos + shoulder_pos + elbow_pos + wristTheta_dest + (wristPhi_dest) + clawL_pos + clawR_pos) / 7;
          if (myHash == serialHash) {
            sprintf(serResp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", base_pos, shoulder_pos, elbow_pos, (int8_t)wristTheta_dest, (int8_t)wristPhi_dest, clawL_pos, clawR_pos, myHash);
            Serial.println(serResp);
            updateServos();
          } else {
            sprintf(serResp, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d!!!!!!!", base_pos, shoulder_pos, elbow_pos, (int8_t)wristTheta_dest, (int8_t)wristPhi_dest, clawL_pos, clawR_pos, myHash);
            Serial.println(serResp);
          }
          break;
      }
      bytesRead++;
    }
  }/**/
}

void updateServos() {

  base.write(base_pos);
  shoulder.write(shoulder_pos);
  elbow.write(elbow_pos);
  clawL.write(clawL_pos);
  clawR.write(clawR_pos);

  // Right now we can't rotate and tilt ar the same time we'll need to fix this later
  if (wristPhi_dest > 0) {
    // rotate right
    //Serial.println(wristPhi_dest);
    analogWrite(5, 200);//wristPhi_dest << 1);
    analogWrite(6, 0);
    analogWrite(3, 0);
    analogWrite(11, 200);//wristPhi_dest << 1);
    return;
  }
  if (wristPhi_dest < 0) {

    analogWrite(5, 0);
    analogWrite(6, 200);//abs(wristPhi_dest)*2);
    analogWrite(3, 200);//abs(wristPhi_dest)*2);
    analogWrite(11, 0);
    return;
  }
  if (wristTheta_dest > 0) {
    // rotate right
    analogWrite(5, 200);//wristTheta_dest << 1);
    analogWrite(6, 0);
    analogWrite(3, 200);//wristTheta_dest << 1);
    analogWrite(11, 0);
    return;
  }
  if (wristTheta_dest < 0) {
    // rotate right
    analogWrite(5, 0);
    analogWrite(6, 200);//abs(wristTheta_dest)*2);
    analogWrite(3, 0);
    analogWrite(11, 200);//abs(wristTheta_dest)*2);
    return;
  }
  if (wristPhi_dest == 0 && wristTheta_dest == 0) {
    analogWrite(5, 0);
    analogWrite(6, 0);
    analogWrite(3, 0);
    analogWrite(11, 0);
  }


}
/*
   this interuptes the processor to count the number of pulses that come from the encoder.

   Warning this can happen as much as 1,669,656 times a second, and the arduino clock is only 16mhz
*/
void leftEncoder() {
  lTicks++;
}
/*
   same as left encoder
*/
void rightEncoder() {
  rTicks++;
}

void calibrate() {


}
