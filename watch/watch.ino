// If using the Adafruit 2.8\" TFT Arduino shield, the line:
// #define USE_ADAFRUIT_SHIELD_PINOUT
// should appear in the library header (Adafruit_TFT.h)

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

#define SENSOR_DHT22
//#define SENSOR_BMP085

#include <DS1307.h>

// Init the DS1307
DS1307 rtc(20, 21);

#ifdef SENSOR_DHT22
#include <DHT.h>
#define DHTPIN A8
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
#endif

#ifdef SENSOR_BMP085
#include <Wire.h>
#include <BMP085.h>
BMP085 dps = BMP085();      // Digital Pressure Sensor 
#endif

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define YP A1  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 7   // can be a digital pin
#define XP 6   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

//Подключение дисплея TFT01-22SP
Adafruit_TFTLCD myGLCD(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

int cnt = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
#ifdef SENSOR_DHT22
  // DHT22 Init
  dht.begin();
#endif

#ifdef SENSOR_BMP085
  // init LCD
  // BMP-085 Init
  Wire.begin();
  dps.init(MODE_ULTRA_HIGHRES, 25000, true);  // 250 meters, true = using meter units
                  // this initialization is useful if current altitude is known,
                  // pressure will be calculated based on TruePressure and known altitude.
#endif

  myGLCD.reset();  
  uint16_t identifier = myGLCD.readID();
  myGLCD.begin(identifier);
  myGLCD.fillScreen(BLACK);
  myGLCD.setRotation(45);

  // Set the clock to run-mode
/*
  rtc.halt(true);
  // The following lines can be commented out to use the values already stored in the DS1307
  rtc.setDOW(SUNDAY);        // Set Day-of-Week to SUNDAY
  rtc.setTime(10, 16, 0);     // Set the time to 12:00:00 (24hr format)
  rtc.setDate(23, 9, 2015);   // Set the date to October 3th, 2010
  rtc.halt(false);
  
  // Set SQW/Out rate to 1Hz, and enable SQW
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
*/
}

void loop() {
  long Temperature = 0, Pressure = 0, Humidity = 0;
  static bool togle = false;
  
  myGLCD.setTextSize(10);
  myGLCD.setCursor(320, 70);
  myGLCD.setTextColor(togle?GREEN:BLACK);  
  myGLCD.print(".");
  togle = !togle;
  
  if((cnt++ % 60) == 0)
  {
     #ifdef SENSOR_BMP085
        long Temperature = 0, Pressure = 0;
        dps.getTemperature(&Temperature);
        dps.getPressure(&Pressure);
        Pressure /= 100;
        Serial.print("  Temperature(*C):");
        Serial.println(Temperature);
        Serial.print("  Pressure(hPa):");
        Serial.println(Pressure);
    #endif 
   #ifdef SENSOR_DHT22
        // put your main code here, to run repeatedly:
       // Reading temperature or humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        Humidity = h;
        Temperature = t;
        // check if returns are valid, if they are NaN (not a number) then something went wrong!
        if (isnan(t) || isnan(h)) {
          Serial.println("Failed to read from DHT");
        } else {
          Serial.print(" Humidity: "); 
          Serial.print(h);
          Serial.print(" %\t");
          Serial.print(" Temperature: "); 
          Serial.print(t);
          Serial.println(" *C");
        }
    #endif
      
      myGLCD.setTextColor(WHITE);  
      myGLCD.setTextSize(3);
      myGLCD.fillScreen(BLACK);
      if(Temperature != 0)
      {
        myGLCD.setCursor(40, 10);
        myGLCD.print(Temperature);
        myGLCD.print("'C");
      }
      if(Humidity)
      {
        myGLCD.setCursor(180, 10);
        myGLCD.print(Humidity);
        myGLCD.print("%");
      }
      myGLCD.setCursor(65, 75);
      myGLCD.setTextColor(GREEN);  
      myGLCD.setTextSize(9);
      myGLCD.print(rtc.getTimeStr(FORMAT_SHORT));
      myGLCD.setCursor(80, 190);
      myGLCD.setTextColor(YELLOW);  
      myGLCD.setTextSize(4);
      myGLCD.print(rtc.getDateStr());
  }
  // Wait one second before repeating
  delay(1000);

}
