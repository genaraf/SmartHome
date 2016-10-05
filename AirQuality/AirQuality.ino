#define MY_RADIO_NRF24
#define MY_RF24_CE_PIN 9
#define MY_RF24_CS_PIN 10

#define DEBUG
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_SIGNING

#include <MySensors.h> 
#include <Wire.h>
#include <AM2320.h>

AM2320 th;
 
#define SHARP_MEASURE_PIN A0
#define SHARP_LED_POWER_PIN 4
#define CO2_SENSOR_PWM_PIN 5
 
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
 
float calcVoltage = 0;
float dustDensity = 0;

float valAIQ =0.0;
float lastAIQ =0.0;

unsigned long SLEEP_TIME = 30*1000; // Sleep time between reads (in milliseconds)

#define TEMPERATURE_SENSOR_CHIELD_ID 1
#define HUMIDITY_SENSOR_CHIELD_ID 2
#define CO2_CHILD_ID 3
#define DUST_SENSOR_CHIELD_ID 4

MyMessage msgTemp(TEMPERATURE_SENSOR_CHIELD_ID, V_TEMP);
MyMessage msgHum(HUMIDITY_SENSOR_CHIELD_ID, V_HUM);
MyMessage dustMsg(DUST_SENSOR_CHIELD_ID, V_LEVEL);
MyMessage msgPPM(CO2_CHILD_ID, V_LEVEL);
MyMessage msgPPM2(CO2_CHILD_ID, V_UNIT_PREFIX);

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Air Quality", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  present(DUST_SENSOR_CHIELD_ID, S_DUST);  
  present(TEMPERATURE_SENSOR_CHIELD_ID, S_TEMP);
  present(HUMIDITY_SENSOR_CHIELD_ID, S_HUM);
  present(CO2_CHILD_ID, S_AIR_QUALITY);  
  send(msgPPM2.set("ppm"));

}
 
void setup(){
  pinMode(SHARP_LED_POWER_PIN,OUTPUT);
  pinMode(CO2_SENSOR_PWM_PIN, INPUT);
  Wire.begin();
}
 
void loop(){

  float voMeasured = 0;
  for(int i = 0; i < 10; i++)
  {
    digitalWrite(SHARP_LED_POWER_PIN, LOW); // power on the LED
    delayMicroseconds(samplingTime);
   
    voMeasured += analogRead(SHARP_MEASURE_PIN); // read the dust value
   
    delayMicroseconds(deltaTime);
    digitalWrite(SHARP_LED_POWER_PIN, HIGH); // turn the LED off
    delayMicroseconds(sleepTime);
  } 
  voMeasured /= 10;
  
  // 0 - 3.3V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (3.3 / 1024.0);
 
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = (0.17 * calcVoltage) * 1000;

#ifdef DEBUG  
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
 
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);
 
  Serial.print(" - Dust Density (ug/m3): ");
  Serial.println(dustDensity);
#endif  
  send(dustMsg.set((int)ceil(dustDensity))); 

  switch(th.Read()) {
#ifdef DEBUG  
    case 2:
      Serial.println("AM2320 CRC failed");
      break;
    case 1:
      Serial.println("AM2320 Sensor offline");
      break;
#endif
    case 0:
#ifdef DEBUG  
      Serial.print("humidity: ");
      Serial.print(th.h);
      Serial.print("%, temperature: ");
      Serial.print(th.t);
      Serial.println("*C");
#endif
      send(msgHum.set((int)th.h));
      send(msgTemp.set(th.t, 2));
      break;
  }

  //wait for the pin to go LOW and measure HIGH time
  while(digitalRead(CO2_SENSOR_PWM_PIN) == HIGH) {;}
  unsigned long duration = pulseIn(CO2_SENSOR_PWM_PIN, HIGH, 2000000);
  long co2ppm = 5 * ((duration/1000) - 2);
#ifdef DEBUG  
  Serial.print("CO2: ");
  Serial.print(co2ppm);
  Serial.println("ppm");
#endif
  send(msgPPM.set((long)ceil(co2ppm)));  
  
  sleep(SLEEP_TIME);
}
