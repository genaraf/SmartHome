#include <SPI.h>
#include "RF24.h"
#include "RF24SN.h"
#include "RF24SNID.h"
#include "EmonLib.h" 

#define UPDATE_INTERVAL_SEC 20
#define DEBUG
//#define WATCHDOG  // !!! WARNING. Watchdog work only with a new version of bootloader

const int CTS1= A0;        // current sensor 1
const int CTS2= A1;        // current sensor 2
const int CTS3= A2;        // current sensor 3

const  float ACVoltaje= 0.23;
const int CTS_NUM=3;

EnergyMonitor emon[CTS_NUM];  
const int emonIn[CTS_NUM] = {CTS1, CTS2, CTS3};

#define RF24SN_NODE_ID (LEVEL1_ZONE | 0)

#define RF24SN_SERVER_ADDR 0xF0F0F0F000LL
#define RF24_CE_PIN 9
#define RF24_CS_PIN 10

RF24 radio(RF24_CE_PIN, RF24_CS_PIN);
RF24SN rf24sn( &radio, RF24SN_SERVER_ADDR, RF24SN_NODE_ID);

float publishErrCnt = 0;
bool publishErrFlg = false;

void setup() {

#ifdef WATCHDOG
  wdt_enable(WDTO_8S);
#endif

#ifdef DEBUG
  Serial.begin(115200);
  Serial.print("Energy Monitor started. ");
  Serial.print("Ver 0.1 Build:");
  Serial.println(__DATE__);
  
#ifdef WATCHDOG
  Serial.println("Watchdog enabled.");
#endif /* WATCHDOG */
#endif /* DEBUG */

  //http://openenergymonitor.org/emon/buildingblocks/calibration
  // calibration.//65 because   SCT-013-100
  for(int i = 0; i < CTS_NUM; i++)
    emon[i].current(emonIn[i], 65);

  // Init RF24 MQTT
  rf24sn.begin();

}

bool publish(uint8_t sensorId, float value)
{
  bool publishSuccess = rf24sn.publish(sensorId, value);
  if(!publishSuccess)
  {
    publishErrCnt++;
    publishErrFlg = true;

  }
#ifdef DEBUG
  Serial.print("Publish: ");
  Serial.print(sensorId);
  Serial.print(":");
  Serial.print(value);
  Serial.println(publishSuccess?" OK":" Failed");
#endif /* DEBUG */  
   
   return publishSuccess;  
}

void loop() {
#ifdef WATCHDOG
  wdt_reset();
#endif /* WATCHDOG */

  for(int i = 0; i < CTS_NUM; i++)
  {
    float irms = emon[i].calcIrms(1480);  // Calculate Irms only
    publish(PM_CTS1_SENSOR + i, irms * ACVoltaje);  
#ifdef DEBUG
    Serial.print("CTS:"); Serial.print(i+1);
    Serial.print(" I[A]:"); Serial.print(irms);
    Serial.print(" P[Kw]:"); Serial.println(irms*ACVoltaje);
#endif /* DEBUG */  
  }
  delay(20000);
  
}
