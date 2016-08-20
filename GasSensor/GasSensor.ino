#define MY_RADIO_NRF24
#define MY_RF24_CE_PIN 9
#define MY_RF24_CS_PIN 10

#define MY_DEBUG
//#define MY_DEBUG_VERBOSE_SIGNING

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <SPI.h>
#include <MySensors.h> 

#define MQ2_CHILD_NODE_ID 1
#define MQ7_CHILD_NODE_ID 2
#define BAZZER_NODE_ID 3
#define BATTON_NODE_ID 4
#define SENSOR_SENSE_ID  5

unsigned long SLEEP_TIME = 1000;  // Sleep time between reads (in milliseconds)
unsigned long UPDATE_INTERVAL = 30000; // Time between publishing  (in milliseconds)
unsigned long ALARM_DISABLE_INTERVAL = 180000; // disable buzzer when button was pressed  (in milliseconds)

#define MQ2_SENSOR_ANALOG_PIN 1
#define MQ7_SENSOR_ANALOG_PIN 0
#define ALARM_RESET_BTN 4
#define BUZZER_PIN 5
#define RED_LED 6
#define GREEN_LED 7
#define BLUE_LED 8

// EPROM Address
#define EPROM_ALARM_LEVEL 1

// default alarm levels
#define DEF_ALARM_LEVEL 150

int alarmLevel;
unsigned long updateTime = 0;
unsigned long alarmDisableTime = ALARM_DISABLE_INTERVAL;
bool buzzerOn = false;
MyMessage msg1(MQ2_CHILD_NODE_ID, V_LEVEL);
MyMessage msg2(MQ7_CHILD_NODE_ID, V_LEVEL);
MyMessage alarmLevelMsg(SENSOR_SENSE_ID, V_DIMMER);

void setLed(int led) {
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  if(led)
    digitalWrite(led, HIGH); 
}

void presentation()  
{   
 // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Sensor Propane MQ-2, CO2 MQ-7", "1.1");

  // Register all sensors to gateway (they will be created as child devices)
  present(MQ2_CHILD_NODE_ID, S_AIR_QUALITY);  
  present(MQ7_CHILD_NODE_ID, S_AIR_QUALITY);
  present(BAZZER_NODE_ID, S_BINARY); 
  present(BATTON_NODE_ID, S_BINARY);
  present(SENSOR_SENSE_ID, S_DIMMER);
  
}

void receive(const MyMessage &message)
{
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_LIGHT)
  {
    buzzerOn = message.getBool();
    Serial.print("Received Buzzer state :");
    Serial.print(buzzerOn?"On":"Off");
  } else if(message.type==V_DIMMER) 
  {
    int sensVal = atoi( message.data );
    if ((sensVal >= 0) && (sensVal <= 100)) 
    {
      alarmLevel = sensVal * 10;
      saveState(EPROM_ALARM_LEVEL, sensVal);
    }
    Serial.print( "Set new alarm level :" );
    Serial.println(alarmLevel);  
  }
   
}

void setup() {
  pinMode(MQ2_SENSOR_ANALOG_PIN, INPUT);
  pinMode(MQ7_SENSOR_ANALOG_PIN, INPUT);
  pinMode(ALARM_RESET_BTN, INPUT);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  setLed(RED_LED);
  delay(500);
  setLed(GREEN_LED);
  delay(500);
  setLed(BLUE_LED);
  
  // restore alarm level
  int sensValue=loadState(EPROM_ALARM_LEVEL);   
  alarmLevel = ((sensValue >= 0) && (sensValue <= 100))?sensValue*10:DEF_ALARM_LEVEL;
  
  Serial.print("Alarm level :");
  Serial.println(alarmLevel);
  send(alarmLevelMsg.set(alarmLevel / 10));
}

void loop() {
  int mq2Val = analogRead(MQ2_SENSOR_ANALOG_PIN); // read the value from the pot
  Serial.print("MQ-2:");
  Serial.println(mq2Val);
  send(msg1.set(mq2Val));

  int mq7Val = analogRead(MQ7_SENSOR_ANALOG_PIN); // read the value from the pot
  Serial.print("MQ-7:");
  Serial.println(mq7Val);
  send(msg2.set(mq7Val));

  if(updateTime >= UPDATE_INTERVAL)
  {
    updateTime = 0;
    if(send(msg1.set(mq2Val), true) && send(msg2.set(mq7Val), true))
      setLed(GREEN_LED);
    else  
      setLed(RED_LED);
  }
  
  if(!digitalRead(ALARM_RESET_BTN))
  {
    alarmDisableTime = ALARM_DISABLE_INTERVAL;  
    Serial.println("Alarm temporary disabled");
  }
  
  if((alarmLevel > 0) && ((mq2Val > alarmLevel) || (mq7Val > alarmLevel)))
  {
      setLed(BLUE_LED);
      if(!alarmDisableTime)
        digitalWrite(BUZZER_PIN, HIGH);
      
  }

  // buzzer activated by command
  if(buzzerOn) digitalWrite(BUZZER_PIN, HIGH);
  
  wait(SLEEP_TIME); //sleep for: sleepTime
  
  digitalWrite(BUZZER_PIN, buzzerOn?HIGH:LOW);  
  updateTime += SLEEP_TIME;
  
  // check when alarm disactivation time expired
  if(alarmDisableTime)
  {
    if(alarmDisableTime > SLEEP_TIME)
      alarmDisableTime -= SLEEP_TIME;
    else
    { 
      alarmDisableTime = 0;
      Serial.println("Alarm activated");
    }
  }    
  setLed(0);
}
