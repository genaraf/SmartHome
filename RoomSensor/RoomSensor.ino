#define MY_RADIO_NRF24
#define MY_RF24_CE_PIN 9
#define MY_RF24_CS_PIN 10

#define DEBUG
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_SIGNING

// Enabled repeater feature for this node
//#define MY_REPEATER_FEATURE

#include <MySensors.h> 
#include <DHT.h>

#define UPDATE_INTERVAL 20000

const int LedPin = 13; // external LED or relay connected to pin 13

#define MOTION_SENSOR_NODE_ID 1
#define TEMPERATURE_SENSOR_NODE_ID 2
#define HUMIDITY_SENSOR_NODE_ID 3
#define LIGHT_SENSOR_NODE_ID 4

#define DHTPIN 5     // what digital pin we're connected to

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);


MyMessage msgMotion(MOTION_SENSOR_NODE_ID, V_TRIPPED);
MyMessage msgTemp(TEMPERATURE_SENSOR_NODE_ID, V_TEMP);
MyMessage msgHum(HUMIDITY_SENSOR_NODE_ID, V_HUM);
MyMessage msgLight(LIGHT_SENSOR_NODE_ID, V_LIGHT_LEVEL);


#define PIRSensorPin 2
int lastPIRsensorState = 1;  // previous sensor state
int PIRsensorState = 0;   // current state of the sensor

const int LightSensorPin = 0; // A0 pin
const int LightSensorCtlPin = 4; // D4

void presentation()  
{   
   // Send the sketch version information to the gateway and Controller
  sendSketchInfo("RoomSensor", "1.0");
  present(MOTION_SENSOR_NODE_ID, S_MOTION);
  present(TEMPERATURE_SENSOR_NODE_ID, S_TEMP);
  present(HUMIDITY_SENSOR_NODE_ID, S_HUM);
  present(LIGHT_SENSOR_NODE_ID, S_LIGHT_LEVEL);
}


void setup() {  
#ifdef DEBUG
  Serial.println("RoomSensor started");
#endif /* DEBUG */

  analogReference(INTERNAL); // built-in reference, equal to 1.1 volts
  
  // Init DHTxx sensor
  dht.begin();

  // Init PIR sensor input pin
  pinMode(PIRSensorPin, INPUT);
}


void loop() {
     
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
    send(msgMotion.set(PIRsensorState?1:0));
    lastPIRsensorState = PIRsensorState;
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
    send(msgTemp.set(t, 2));
    send(msgHum.set(h, 2));
  }
  // enable light sensor
  pinMode(LightSensorCtlPin, OUTPUT); 
  digitalWrite(LightSensorCtlPin, 1);
  delay(5);
  uint16_t lightVal = analogRead(LightSensorPin);    // read light sensor
  // disable light sensor
  pinMode(LightSensorCtlPin, INPUT); 
  send(msgLight.set(lightVal));

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

  sleep(digitalPinToInterrupt(PIRSensorPin), CHANGE, UPDATE_INTERVAL);
}
