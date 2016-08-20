#define MY_RADIO_NRF24
#define MY_RF24_CE_PIN 9
#define MY_RF24_CS_PIN 10

#define DEBUG
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_SIGNING

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h> 
#include <EmonLib.h> 

#define PWR_CHILD_NODE_ID 1

#define UPDATE_INTERVAL_SEC 20000

const int CTS1= A0;        // current sensor 1
const int CTS2= A1;        // current sensor 2
const int CTS3= A2;        // current sensor 3

const  float ACVoltaje= 0.23;
const int CTS_NUM=3;

EnergyMonitor emon[CTS_NUM];  
const int emonIn[CTS_NUM] = {CTS1, CTS2, CTS3};

MyMessage msg[CTS_NUM];

void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("PowerMonitor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  for(int i = 0; i < CTS_NUM; i++)
    present(PWR_CHILD_NODE_ID + i, S_POWER);  
}

void setup() {

#ifdef DEBUG
  Serial.print("Energy Monitor started. ");
  Serial.print("Ver 1.0 Build:");
  Serial.println(__DATE__);
#endif
  
  //http://openenergymonitor.org/emon/buildingblocks/calibration
  for(int i = 0; i < CTS_NUM; i++)
  {
    // calibration.//65 because   SCT-013-100
    emon[i].current(emonIn[i], 65);
    // init message
    msg[i].setSensor(PWR_CHILD_NODE_ID + i);
    msg[i].setType(V_KWH);
  }
}

void loop() {
  for(int i = 0; i < CTS_NUM; i++)
  {
    float irms = emon[i].calcIrms(1480);  // Calculate Irms only
    send(msg[i].set(irms * ACVoltaje, 3));  
#ifdef DEBUG
    Serial.print("CTS:"); Serial.print(i+1);
    Serial.print(" I[A]:"); Serial.print(irms);
    Serial.print(" P[Kw]:"); Serial.println(irms*ACVoltaje);
#endif /* DEBUG */  
  }
  sleep(UPDATE_INTERVAL_SEC);
  
}
