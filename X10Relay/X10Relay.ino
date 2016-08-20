#define MY_RADIO_NRF24
#define MY_RF24_CE_PIN 9
#define MY_RF24_CS_PIN 10

#define DEBUG
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_SIGNING

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <Wire.h>    // Required for I2C communication
#include <OneWire.h>
#include <DallasTemperature.h>
#include <MySensors.h> 
#include "X10rf.h"


#define UPDATE_TIME 600000
#define X10_CMD_NODE_ID 20
#define X10_TEMP_NODE_ID 21

#define RELEY_NODE_ID 1

#define NUMBER_OF_RELAYS 16

MyMessage msgCmd(X10_CMD_NODE_ID, V_IR_RECORD);
MyMessage msgTemp(X10_TEMP_NODE_ID, V_TEMP);

unsigned long nextUpdateTime;

uint8_t port0, port1;
volatile bool newState;
volatile int newUnit = -1;
volatile unsigned short newCommand = 0;
 
void rfX10Event(char house, byte unit, byte command, bool isRepeat);

// Data wire is plugged into port 4 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


// X10 RF Receiver Library
X10rf x10rf = X10rf(
  0, // Receive Interrupt Number (0 = Standard Arduino External Interrupt)
  2, // Receive Interrupt Pin (Pin 2 must be used with interrupt 0)
  rfX10Event // Event triggered when RF message is received
);

void presentation()  
{   
   // Send the sketch version information to the gateway and Controller
  sendSketchInfo("X10 Relay", "1.0");

  for (int i = 0; i < NUMBER_OF_RELAYS; i++) {
    // Register all sensors to gw (they will be created as child devices)
    present(RELEY_NODE_ID + i, S_LIGHT);
  }
  present(X10_CMD_NODE_ID, S_CUSTOM);
  present(X10_TEMP_NODE_ID, S_TEMP);
}

void receive(const MyMessage &message)
{
  if ((message.type==V_LIGHT) && (message.sensor >= RELEY_NODE_ID) && (message.sensor <  RELEY_NODE_ID + NUMBER_OF_RELAYS))
  {
     // Write some debug info
#ifdef DEBUG
     Serial.print("Incoming change for Relay:");
     Serial.print(message.sensor - RELEY_NODE_ID + 1);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
#endif
     newState = message.getBool();
     newUnit = message.sensor - RELEY_NODE_ID;
  } 
}

bool updatePCF8574(int portNum)
{
  uint8_t _value;
  uint8_t _address;
  if(portNum == 0)
  {
    _address = 0x20;
    _value = port0;
  } else
  {
    _address = 0x21;
    _value = port1;
  }
  //return;
  /* Start communication and send GPIO values as byte */
  Wire.beginTransmission(_address);
  Wire.write(_value);
  Wire.endTransmission();
  return true;
}

// port [0..15] state 0-off 1-on 
bool writeGpioPort(byte port, byte state)
{
  Serial.print("Rele: ");
  Serial.print(port);
  Serial.println((state > 0)?"-ON":"-OFF");

  saveState(port, state);
  if(port < 8)
  {
    if(state > 0)
      port0 |= 1 << port;
    else
      port0 &= ~(1 << port);
    updatePCF8574(0); 
  } else
  {
    if(state > 0)
      port1 |= (1 << (port - 8));
    else
      port1 &= ~(1 << (port - 8));
    updatePCF8574(1); 
  }
  return true;
}

// Process commands received from X10 compatible RF remote
void rfX10Event(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat)
  {
    unsigned int cmd = (house -'A' + 1)& 0xF;
    cmd = cmd << 4;
    cmd += unit &0xF;
    cmd = cmd << 8;
    cmd += command;
    newCommand = cmd;
   
    if((house == 'A') && (unit != 0)  && ((command == CMD_ON) || (command == CMD_OFF)))
    {
      newState = (command == CMD_ON);
      newUnit = unit - 1;
    }
  }
}

void setup() {
  x10rf.begin();
  Wire.begin();
  // restore state
  for(int i=0; i <= 15; i++)
    writeGpioPort(i, (loadState(i) == true));
  updatePCF8574(0);
  updatePCF8574(1);
  sensors.begin();
  nextUpdateTime = millis() + UPDATE_TIME;
}

void loop() {
  if(newCommand)  {
    send(msgCmd.set(newCommand));
#ifdef DEBUG
    Serial.println(newCommand, HEX);
#endif
    newCommand = 0;
               
  }
  if(newUnit >= 0)  {
    writeGpioPort(newUnit, newState); 
    newUnit = -1; 
  }
  if(millis() > nextUpdateTime)
  {
    sensors.requestTemperatures(); // Send the command to get temperatures
    float t = sensors.getTempCByIndex(0);
    send(msgTemp.set(t, 2));
#ifdef DEBUG
    Serial.print("Temperature: "); Serial.println(t);
#endif
    nextUpdateTime = millis() + UPDATE_TIME;
  }
}
