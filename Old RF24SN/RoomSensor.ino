#include <avr/interrupt.h>        // Library to use interrupt
#include <avr/sleep.h>            // Library for putting our arduino into sleep modes
#include <avr/power.h>
#include <avr/wdt.h>

#include <SPI.h>
#include "RF24.h"
#include "RF24SN.h"
#include "DHT.h"

#define POWER_SAVE_MODE
#define UPDATE_INTERVAL_SEC 20
#define DEBUG
//#define WATCHDOG  // !!! WARNING. Watchdog work only with a new version of bootloader

const int LedPin = 13; // external LED or relay connected to pin 13

// NodeId uint8 2Bits - level id + 6bit room id (0-63) 
#define GARDEN_ZONE 0x00
#define LEVEL1_ZONE 0x40
#define LEVEL2_ZONE 0x80
#define LEVEL3_ZONE 0xC0


#define MOTION_SENSOR 1
#define TEMPERATURE_SENSOR 2
#define HUMIDITY_SENSOR 3
#define LIGHT_SENSOR 4
#define POWER_VOLTAGE 128
#define PUBLISH_ERR_CNT 129

#define RF24SN_NODE_ID (LEVEL2_ZONE | 1)
#define RF24SN_SERVER_ADDR 0xF0F0F0F000LL
#define RF24_CE_PIN 9
#define RF24_CS_PIN 10


RF24 radio(RF24_CE_PIN, RF24_CS_PIN);
RF24SN rf24sn( &radio, RF24SN_SERVER_ADDR, RF24SN_NODE_ID);


#define DHTPIN 5     // what digital pin we're connected to

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);


#define PIRSensorPin 2
const int PIRsensorInterrupt = 0; //interrupt 0 at arduino nano pin D2
int lastPIRsensorState = 1;  // previous sensor state
int PIRsensorState = 0;   // current state of the sensor

const int LightSensorPin = 0; // A0 pin
const int LightSensorCtlPin = 4; // D4

volatile int timerIntFlg=0;
volatile int pirIntFlg=0;
int timerIntCnt=0;
bool radioPowerUp = true;
float publishErrCnt = 0;
bool publishErrFlg = false;

// Timer1 Overflow ISR.
ISR(TIMER1_OVF_vect)
{
  /* set type of interrupt flag. */
   if(timerIntFlg == 0)
     timerIntFlg = 1;
}

// PIR sensor ISR  
void wakeUpNow(){
  /* set type of interrupt flag. */
  if(pirIntFlg == 0)  
    pirIntFlg = 1;
}



void setup() {  

#ifdef WATCHDOG
  wdt_enable(WDTO_8S);
#endif

#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("RoomSensor started");
#ifdef WATCHDOG
  Serial.println("Watchdog enabled.");
#endif /* WATCHDOG */
#endif /* DEBUG */

  analogReference(INTERNAL); // built-in reference, equal to 1.1 volts
  
  // Init DHTxx sensor
  dht.begin();

  // Init PIR sensor input pin
  pinMode(PIRSensorPin, INPUT);

  // Init RF24 MQTT
  rf24sn.begin();
}

#ifdef POWER_SAVE_MODE
void enterSleep(void)
{
  if(radioPowerUp)
  {
    radioPowerUp = false;
    radio.powerDown(); // be reduced to 26uA (.026mA) between sending.
    pinMode(11, INPUT); // disable SCK
    pinMode(13, INPUT); // disable MISO
  }
  
  // timer1 configuration
  // set normal timer operation
  TCCR1A = 0x00; 
  
  // Clear the timer counter register.
  TCNT1=0x0000; 
  
  // Configure the prescaler for 1:1024
  TCCR1B = 0x05;
  
  // enable the timer overlow interrupt
  TIMSK1=0x01;

  delay(50);
  
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();


  // Disable all of the unused peripherals
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  // attach interrupt from PIR sensor
  attachInterrupt(PIRsensorInterrupt,wakeUpNow, CHANGE);
 
  // now enter sleep mode
  sleep_mode();
  
  // detach interrupt from PIR sensor
  detachInterrupt(PIRsensorInterrupt);   

  // the program will continue after the timer1 overflow
  sleep_disable(); 
 
  // re-enable the peripherals
  power_all_enable();  
}
#else
void enterSleep(void)
{ // sleep on 4sec
  int sllepCnt = 80;
  while(--sllepCnt)
  {
    delay(50);
    if(digitalRead(PIRSensorPin) != lastPIRsensorState)
    {
      pirIntFlg = 1;
      return;
    }    
  }
  timerIntFlg = 1;
}
#endif /* POWER_SAVE_MODE */

bool publish(uint8_t sensorId, float value)
{
  if(!radioPowerUp)
  {
    radioPowerUp = true;
    pinMode(11, OUTPUT); // enable SCK
    pinMode(13, OUTPUT); // enable MISO
    radio.powerUp();
    delay(5); 
  }
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
  
  PIRsensorState = digitalRead(PIRSensorPin);
  if (PIRsensorState != lastPIRsensorState)
  {
    if (PIRsensorState == 0) 
    {
       digitalWrite(LedPin, LOW);
#ifdef DEBUG
      Serial.print("Sleeping-");            // enable for debugging
      Serial.println(PIRsensorState);   // read status of interrupt pin
#endif /* DEBUG */  
    }
    else 
    {       
      digitalWrite(LedPin, HIGH); 
#ifdef DEBUG
      Serial.print("Awake-");    // enable for debugging
      Serial.println(PIRsensorState);  // read status of interrupt pin   enable for debugging
#endif /* DEBUG */  
    }
    publish(MOTION_SENSOR, PIRsensorState?1:0);
    lastPIRsensorState = PIRsensorState;
    pirIntFlg = 0;
  } else if(timerIntFlg && (++timerIntCnt <= (UPDATE_INTERVAL_SEC / 4)))
  { // waiting next update interval
    timerIntFlg = 0;
    enterSleep();
    return;  
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if(isnan(h) || isnan(t)) {
 #ifdef DEBUG
   Serial.println("Failed to read from DHT sensor!");
    return;
#endif /* DEBUG */  
  } else
  {
    publish(TEMPERATURE_SENSOR, t);
    publish(HUMIDITY_SENSOR, h);
  }
  // enable light sensor
  pinMode(LightSensorCtlPin, OUTPUT); 
  digitalWrite(LightSensorCtlPin, 1);
  delay(5);
  float lightVal = analogRead(LightSensorPin);    // read light sensor
  // disable light sensor
  pinMode(LightSensorCtlPin, INPUT); 
  publish(LIGHT_SENSOR, lightVal);

#ifdef DEBUG
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("\tLight: ");
  Serial.println(lightVal);
#endif /* DEBUG */  

  // publish error counter
  if(publishErrFlg)
  {
    publishErrFlg = !publish(PUBLISH_ERR_CNT, publishErrCnt);    
  }

  // reset interval
  timerIntCnt = 0;
  timerIntFlg = 0;
  
#ifdef POWER_SAVE_MODE
   enterSleep();
#else
  // Wait a few seconds between measurements.
  delay(4000);
#endif /* POWER_SAVE_MODE */  
}
