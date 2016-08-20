/* Hardware DevDuino Sensor Node V2.0 (ATmega 328) 
 * http://www.seeedstudio.com/wiki/DevDuino_Sensor_Node_V3.0_(ATmega_328)
 */
#define MY_RADIO_NRF24
#define MY_RF24_CE_PIN 8
#define MY_RF24_CS_PIN 7

#define DEBUG
#define MY_DEBUG
//#define MY_DEBUG_VERBOSE_SIGNING

#include <SPI.h>
#include <MySensors.h> 

#define TEMP_CHILD_NODE_ID 1
#define HUM_CHILD_NODE_ID 2

#if defined(DEBUG)
unsigned long UPDATE_INTERVAL = 20000;  // Sleep time between reads (in milliseconds)
#else
unsigned long UPDATE_INTERVAL = 1200000;  // Sleep time between reads (in milliseconds)
#endif

#define BUTTON_PIN 4
#define LED_PIN 9

#define MIN_VOLTAGE 2400 // mV

unsigned long updateTime = 0;

MyMessage msgTemp(TEMP_CHILD_NODE_ID, V_TEMP);
MyMessage msgHum(HUM_CHILD_NODE_ID, V_HUM);

int oldBatteryPcnt = 0;

void presentation()  
{   
 // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Garden Sensor", "1.0");

  // Register all sensors to gateway (they will be created as child devices)
  present(TEMP_CHILD_NODE_ID, S_TEMP);  
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(75); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void setup() {
}

void loop() {
   long volt = readVcc();
   if(volt > MIN_VOLTAGE)
   { 
#ifdef DEBUG
    Serial.print("Voltage: "); Serial.println(volt);
#endif 
    // embedded temperature sensor (connected to A2)
//    float t = (((float)analogRead(A2) * MySensors[2].Value / 1024.0) - 0.5)/0.01;
  
    int batteryPcnt = map(volt, 0, 3000, 0, 100);
    if(batteryPcnt > 100) batteryPcnt = 100;
   
    if (oldBatteryPcnt != batteryPcnt) {
      // Power up radio after sleep
      sendBatteryLevel(batteryPcnt);
      oldBatteryPcnt = batteryPcnt;
    }
  }
  sleep(UPDATE_INTERVAL); //sleep for: sleepTime  
}
