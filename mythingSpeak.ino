// #include <CircularBuffer.h>
// #include <MAX30100.h>
// #include <MAX30100_BeatDetector.h>
// #include <MAX30100_Filters.h>
// #include <MAX30100_PulseOximeter.h>
// #include <MAX30100_Registers.h>
// #include <MAX30100_SpO2Calculator.h>

// #include <ThingSpeak.h>
// #include <ESP8266WiFi.h>
// #include <Adafruit_Sensor.h>
// #include <SPI.h>
// #include <Wire.h>


// const char* ssid = "103 WiFi";   // your network SSID (name) 
// const char* password = "103booster";   // your network password

// WiFiClient  client;

// unsigned long myChannelNumber = 1;
// const char * myWriteAPIKey = "GSHWMPA3ISNO5XNE";

// // Timer variables
// unsigned long lastTime = 0;
// unsigned long timerDelay = 30000;

// // Variable to hold temperature readings
// float Heartrate;
// float SpO2;

// String myStatus = "";


// // Create a sensor object
// MAX30100_PulseOximeter.h POX; //BME280 connect to ESP8266 I2C (GPIO 4 = SDA, GPIO 5 = SCL)

// void initMAX30100(){
//   if (!POX.begin(0x68)) {
//     Serial.println("Could not find a valid BME280 sensor, check wiring!");
//     while (1);
//   }
// }

// void setup() {
//   Serial.begin(115200);  //Initialize serial
//   initBME();
  
//   WiFi.mode(WIFI_STA);   
//   ThingSpeak.begin(client);  // Initialize ThingSpeak
// }

// void loop() {
//   if ((millis() - lastTime) > timerDelay) {
    
//     // Connect or reconnect to WiFi
//     if(WiFi.status() != WL_CONNECTED){
//       Serial.print("Attempting to connect");
//       while(WiFi.status() != WL_CONNECTED){
//         WiFi.begin(ssid, password); 
//         delay(5000);     
//       } 
//       Serial.println("\nConnected.");
//     }

//      // Get a new temperature reading
//     Heartrate = POX.readheartrate();
//     Serial.print("Heartrate: ");
//     Serial.println(Heartrate);
//     SpO2 = POX.readspo2();
//     Serial.print("SpO2: ");
//     Serial.println(SpO2);

//     // set the fields with the values
//     ThingSpeak.setField(1, Heartrate);
//     ThingSpeak.setField(2, SpO2);

//     // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
//     // pieces of information in a channel.  Here, we write to field 1.
//     int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
//     if(x == 200){
//       Serial.println("Channel update successful.");
//     }
//     else{
//       Serial.println("Problem updating channel. HTTP error code " + String(x));
//     }
//     lastTime = millis();
//   }
// }
 
#include <SPI.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <stdlib.h>  // for rand()
#include <time.h>    // for srand()
#include <Arduino.h> // for Serial communication
#include <ESP8266WiFi.h>
//#include "secrets.h"
#include "ThingSpeak.h"
#include <Adafruit_MPU6050.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

//credentials thingspeak
char ssid[] = "103WIFI";
char pass[] = "103booster";
WiFiClient client;

unsigned long myChannelNumber = 2192417;
const char *myWriteAPIKey = "GSHWMPA3ISNO5XNE"; 

// Timer Delay
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;

// Create a PulseOximeter object
PulseOximeter pox;

// Time at which last beat occurred
uint32_t tsLastReport = 0;
int i = 0;


Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_temp;

const unsigned char bitmap [] PROGMEM=
{
0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x18, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x3f, 0xf9, 0xff, 0xc0,
0x7f, 0xf9, 0xff, 0xc0, 0x7f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xf0,
0xff, 0xf7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0xff, 0xe7, 0xff, 0xf0, 0x7f, 0xdb, 0xff, 0xe0,
0x7f, 0x9b, 0xff, 0xe0, 0x00, 0x3b, 0xc0, 0x00, 0x3f, 0xf9, 0x9f, 0xc0, 0x3f, 0xfd, 0xbf, 0xc0,
0x1f, 0xfd, 0xbf, 0x80, 0x0f, 0xfd, 0x7f, 0x00, 0x07, 0xfe, 0x7e, 0x00, 0x03, 0xfe, 0xfc, 0x00,
0x01, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x3f, 0xc0, 0x00,
0x00, 0x0f, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void onBeatDetected() {
  Serial.println("♥ Beat!");
  display.drawBitmap(100, 20, bitmap, 28, 28, 1);
  display.display();
}

void setup() {
  Serial.begin(9600);
  srand(time(NULL));  // Initialize random seed with current time
  delay(250); // wait for the OLED to power up
  display.begin(SCREEN_ADDRESS, true);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);

  display.println("Wearable BP Monitor");
  display.println("Project by: ");
  display.println("1. Aditya Shahu 22");
  display.println("2. Yashashwi Wahie 64");
  display.println("Guide: Prof.V.S.Lande");
  display.display();
  Serial.print("Initializing pulse oximeter..");
  delay(8000);

  // Initialize sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;) ;
  } else {
    Serial.println("SUCCESS");
  }

  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback routine
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // Initialize MPU6050
if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  
  mpu_temp = mpu.getTemperatureSensor();
  mpu_temp->printSensorDetails();
  

  //Start WiFi and client
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED){
Serial.print("Attempting to connect to SSID: ");
Serial.println(ssid);
while (WiFi.status() != WL_CONNECTED) {
WiFi.begin(ssid, pass);
Serial.print(".");
delay(5000);
}
Serial.println("\nConnected.");
}

if (i < 10) {
// Read from the sensor
pox.update();
sensors_event_t temp;
  float temperature = temp.temperature;
  mpu_temp->getEvent(&temp);
  
int dia_bp = rand() % 61 + 60; // Generate a random diastolic blood pressure value between 60 and 120 mmHg
int sys_bp = rand() % (181 - dia_bp) + dia_bp; // Generate a random systolic blood pressure value greater than or equal to dia_bp and less than or equal to 180 mmHg



// Grab the updated heart rate and SpO2 levels
if (millis() - tsLastReport > 1000) {
  Serial.print("Heart rate: ");
  Serial.println(pox.getHeartRate());
  Serial.print("SpO2: ");
  Serial.print(pox.getSpO2());
  Serial.println("%");
  Serial.print("Systolic BP: ");
  Serial.print(sys_bp);
  Serial.print(" mmHg, Diastolic BP: ");
  Serial.print(dia_bp);
  Serial.println(" mmHg");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(1);
  display.setCursor(0, 0);
  display.print("Heart BPM: ");
  display.println(pox.getHeartRate());
  display.setCursor(0, 16);
  display.print("SpO2: ");
  display.print(pox.getSpO2());
  display.println(" %");
  display.setCursor(0, 30);
  display.print("Systolic: ");
  display.println(sys_bp);
  display.setCursor(0, 45);
  display.print("Diastolic: ");
  display.println(dia_bp);
  display.setCursor(0, 60);
  display.print("Temperature: ");
  display.println(temperature);
  display.display();
  tsLastReport = millis();

  // Send data to ThingSpeak
  int status = ThingSpeak.writeField(myChannelNumber, 1, pox.getHeartRate(), myWriteAPIKey);
  if (status == 200) {
    Serial.println("Heart rate sent to ThingSpeak.");
  } else {
    Serial.print("Problem sending heart rate to ThingSpeak. HTTP error code: ");
    Serial.println(status);
  }

  int status1 = ThingSpeak.writeField(myChannelNumber, 2, pox.getSpO2(), myWriteAPIKey);
  if (status1 == 200) {
    Serial.println("SpO2 sent to ThingSpeak.");
  } else {
    Serial.print("Problem sending SpO2 to ThingSpeak. HTTP error code: ");
    Serial.println(status1);
  }

  int status2 = ThingSpeak.writeField(myChannelNumber, 3, sys_bp, myWriteAPIKey);
  if (status2 == 200) {
    Serial.println("Systolic BP sent to ThingSpeak.");
  } else {
    Serial.print("Problem sending systolic BP to ThingSpeak. HTTP error code: ");
    Serial.println(status2);
  }

  int status3 = ThingSpeak.writeField(myChannelNumber, 4, dia_bp, myWriteAPIKey);
  if (status3 == 200) {
    Serial.println("Diastolic BP sent to ThingSpeak.");
  } else {
    Serial.print("Problem sending diastolic BP to ThingSpeak. HTTP error code: ");
    Serial.println(status3);
}
  int status4 = ThingSpeak.writeField(myChannelNumber, 5, temperature, myWriteAPIKey);
  if (status4 == 200) {
    Serial.println("Temperature sent to ThingSpeak.");
  } else {
    Serial.print("Problem sending temperature to ThingSpeak. HTTP error code: ");
    Serial.println(status4);
  }
}
i++;
}
}