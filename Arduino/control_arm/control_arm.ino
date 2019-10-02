#include <Ethernet.h>
#include <EthernetUdp.h>

#define DEVICE_ID 1
#define ENC_1_A 0x01 // pin A8
#define ENC_1_B 0x02 // pin A9
#define ENC_2_A 0x04 // pin A10
#define ENC_2_B 0x08 // pin A11
#define ENC_3_A 0x10 // pin A12
#define ENC_3_B 0x20 // pin A13
#define ENC_4_A 0x40 // pin A14
#define ENC_4_B 0x80 // pin A15

#define PIN_JOYSTICK A7
#define PIN_BUTTON1 14
#define PIN_BUTTON2 15

byte port_k_prev = 0x00;

long pos_1 = 0;
long pos_2 = 0;
long pos_3 = 0;
long pos_4 = 0;

char message[14]; // the bytes we send out over UDP

// master address (this)
IPAddress myip(192, 168, 1, 69);
const int srcPort PROGMEM = 4321;

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x69, 0x69, 0x69, 0x69, 0x69, 0x69 };

// slave address
IPAddress dstip(192, 168, 1, 140);
const int dstPort PROGMEM = 2040;

EthernetUDP Udp;

void setup()
{
  cli();
  // set encoder pins as inputs, enable pullups
  DDRK &= 0x00;
  PORTK |= 0xFF;

  PCICR |= 0x04; // enable port B pin change interrupt
  PCMSK2 |= 0xFF; // enable interrupt for all port B pins
  sei();

  port_k_prev = PINK;

  // buttons
  pinMode(PIN_BUTTON1, INPUT);
  digitalWrite(PIN_BUTTON1, HIGH);
  pinMode(PIN_BUTTON2, INPUT);
  digitalWrite(PIN_BUTTON2, HIGH);
  
  // potentiometers
  pinMode(PIN_JOYSTICK, INPUT);

  // ethernet
  Ethernet.init(10);
  
  Ethernet.begin(mymac, myip);

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
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
  Udp.begin(srcPort);
}

void loop()
{
  readData();
  // the message format (14 bytes total):
  // (-127) | Device ID (1) | yaw (high) | yaw (low) | shoulder (high) ...
  // ... | shoulder (low) | elbow (high) | elbow (low) | wrist pitch (high) ...
  // ... | wrist pitch (low) | wrist roll (high) | wrist roll (low) | buttons | hash
  
  Udp.beginPacket(dstip, dstPort);
  for(int i = 0; i < 14; i++)
  {
    Udp.write(message[i]);
    //Serial.print(int(message[i]));
    //Serial.print(", ");
  }
  Udp.endPacket();
  //Serial.print("\n");
  delay(100);
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
      pos_1--;
    }
    else
    {
      // CW
      pos_1++;
    }
  }
  else if (!(port_k_pins & ENC_1_A) && (port_k_prev & ENC_1_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_1_B)
    {
      // CW
      pos_1++;
    }
    else
    {
      // CCW
      pos_1--;
    }
  }

  // encoder 2
  if ((port_k_pins & ENC_2_A) && !(port_k_prev & ENC_2_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_2_B)
    {
      // CCW
      pos_2--;
    }
    else
    {
      // CW
      pos_2++;
    }
  }
  else if (!(port_k_pins & ENC_2_A) && (port_k_prev & ENC_2_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_2_B)
    {
      // CW
      pos_2++;
    }
    else
    {
      // CCW
      pos_2--;
    }
  }

  // encoder 3
  if ((port_k_pins & ENC_3_A) && !(port_k_prev & ENC_3_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_3_B)
    {
      // CCW
      pos_3--;
    }
    else
    {
      // CW
      pos_3++;
    }
  }
  else if (!(port_k_pins & ENC_3_A) && (port_k_prev & ENC_3_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_3_B)
    {
      // CW
      pos_3++;
    }
    else
    {
      // CCW
      pos_3--;
    }
  }

  // encoder 4
  if ((port_k_pins & ENC_4_A) && !(port_k_prev & ENC_4_A)) // rising edge on pin A
  {
    if(port_k_pins & ENC_4_B)
    {
      // CCW
      pos_4--;
    }
    else
    {
      // CW
      pos_4++;
    }
  }
  else if (!(port_k_pins & ENC_4_A) && (port_k_prev & ENC_4_A)) // falling edge on pin A
  {
    if(port_k_pins & ENC_4_B)
    {
      // CW
      pos_4++;
    }
    else
    {
      // CCW
      pos_4--;
    }
  }
  
  port_k_prev = port_k_pins;
}

void readData()
{  
  message[0] = -127; // Start message byte
  message[1] = DEVICE_ID;

  // pot values are 2 bytes (most significant bits sent first)
  message[2] = (char)(pos_4 >> 8);
  message[3] = (char)(pos_4);

  message[4] = (char)(pos_1 >> 8);
  message[5] = (char)(pos_1);

  message[6] = (char)(pos_2 >> 8);
  message[7] = (char)(pos_2);

  message[8] = (char)(pos_3 >> 8);
  message[9] = (char)(pos_3);

  int tempJoystick = analogRead(PIN_JOYSTICK);
  Serial.println(tempJoystick);
  message[10] = (char)(tempJoystick >> 8);
  message[11] = (char)(tempJoystick);
  
  // buttons - each bit is a different button
  message[12] = 0;
  if(!digitalRead(PIN_BUTTON1)) // 2^0 - open claw
  {
    message[12] = message[12] | 0x01;
    Serial.println("Open");
  }
  if(!digitalRead(PIN_BUTTON2)) // 2^1 - close claw
  {
    message[12] = message[12] | 0x02;
    Serial.println("Close");
  }

  // hash
  message[13] = (message[2] + message[3] + message[4] + message[5] + message[6] + message[7] + message[8] + message[9] + message[10] + message[11] + message[12]) / 11;
}
