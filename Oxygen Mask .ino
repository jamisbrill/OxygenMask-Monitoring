//buzzer Define
int buzzer = 12; // set the buzzer control digital IO pin
// Bmp180
#define SEALEVELPRESSURE_HPA (1013.25)
#define seaLevelPressure_hPa 1013.25
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;  //library for the altitude sensor

// Force Resitive Sensor
int forcePin = 16;
int forceReading;

//


/// led setup
// Import required libraries
#ifdef ESP32
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#endif
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//OLED STUFF
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C   // Address may be different for your module, also try with 0x3D if this doesn't work

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// OLED STUFF END



/// SQL STUFF
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
//Wifi Client Setup
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Network stuff
char ssid[] = "";                 // Network Name
char pass[] = "";                 // Network Password
byte mac[6];



WiFiServer SQLserver(3306); //define port for the sql server (default 3306)
IPAddress ip(192, 168, 1, 13); //set the ip for the ESP device (huzzah)
IPAddress gateway(192, 168, 1, 1); //set the ip for the Router / Default Gateway
IPAddress subnet(255, 255, 255, 0); //set the subnet mask

WiFiClient client;  // create client to connect to the sql
MySQL_Connection conn((Client *)&client); //setup a client connection


// Sql Query to sent to the Database
char query[128];

IPAddress server_addr(192, 168, 1, 133);      // MySQL server IP
char user[] = "";                          // MySQL user
char password[] = "";                 // MySQL password
//end sql


// Realtimeclock Setup
#include "RTClib.h"
DateTime now;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
RTC_DS3231 rtc;

// RTC Setup End
Adafruit_BME280 bme; // BM280 Sensor Setup

//const char* ssid = "";
const char* password1 = "";

// Create AsyncWebServer on port 80
AsyncWebServer server(80);

void setup() {


  //initialise all sensors / stuff
  //bmp180 setup
  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 sensor");
    while (1) {}
  }
  // bmp180 end


  // Buzzer Setup
  pinMode(buzzer, OUTPUT); // set pin 8 as output
  //Buzzer Setup End

  // RTCHART
  // Serial port for debugging purposes
  Serial.begin(115200);

  //led setups
  pinMode(15, OUTPUT); // red
  pinMode(14, OUTPUT); //green


  bool status;
  // default settings
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Initialize SPIFFS To write website files to Esp8266
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password1); // insert ssid and password that has been predefined
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi.."); //print this until connected
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());



  // Route for root / web page  // Sends HTTP request to server to post on the graph
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBME280Temperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBME280Humidity().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBME280Pressure().c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readBMP180Altitude().c_str());   //change this to Altitude Next time lo l  find out what c_str does
    Serial.println("    Serial.println(readBMP180Altitude().c_str())");
    Serial.println(readBMP180Altitude().c_str());
  });

  // Start server
  server.begin();

  // RTChart end
  Serial.println("Initialising connection");
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  Serial.println("");
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.println("Wifi-Load");
  }

  Serial.println("");
  Serial.println("WiFi Connected");

  //print mac address in mac format

  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
  Serial.println("");
  Serial.print("Assigned IP: ");
  Serial.print(WiFi.localIP());
  Serial.println("");

  Serial.println("Connecting to database");

  //while making connection to SQL server print . to show currentl;y connecting
  while (conn.connect(server_addr, 3306, user, password) != true) //connect to SQL server using pre set parameters
  {
    delay(200);
    Serial.print ( "." );
  }

  Serial.println("");
  Serial.println("Connected to SQL Server!");

  //turn on the display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

}
// Altitude Averaging Code bmp180

int i = 0; // Iteration
double Altitude[2]; // array to calculate the average

void loop() {
  Serial.print ( "Start Of Loop " ); // debugging purposes

  //rtc
  DateTime now = rtc.now(); //get time
  Serial.print(now.hour(), DEC); //print time
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();


  char datetimeBuffer[20] = ""; //buffer to store the time in
  sprintf(datetimeBuffer, "%02d.%02d", now.hour(), now.minute());  //put data into format
  Serial.println(datetimeBuffer);
  String Time2 = datetimeBuffer;
  Serial.println("Time2");
  Serial.println(Time2);

  // OLED CODE
  display.clearDisplay();           // Required to clear display
  display.setTextColor(WHITE);
  display.setTextSize(1.5);
  display.setCursor(0, 0);            // Set cursor
  display.print("Temperature:");
  display.setCursor(70, 0);         // Set cursor
  display.print(readBME280Temperature());
  display.setCursor(0, 10);         // Set cursor
  display.print("Altitude:");
  display.setCursor(60 , 10);         // Set cursor
  display.print(readBMP180Altitude());

  display.setCursor(0, 20);         // Set cursor
  display.print("Time:");
  display.setCursor(60, 20);         // Set cursor
  display.print(Time2);

  display.setCursor(0, 30);         // Set cursor
  display.print("Humidity:");
  display.setCursor(60, 30);         // Set cursor
  display.print(readBME280Humidity());



  display.display(); // show what has been set
  //OLED END

  // Altitude Averaging Code bmp180

  double Altitudemaths;
  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude()); // Check the current Altitude
  Serial.println(" meters");

  if (i < 3) // Dont run after the set 3 values have been initially set
  {
    Serial.print("Entered Setting Loop I IS :");
    Serial.print(i);
    Serial.println("");
    Serial.println("Altitude Currently is:");
    Serial.println(bmp.readAltitude());


    Altitude[i] = bmp.readAltitude();  // put the current altitude into the altitude array for the first 3 values
    i++;
  }
  //bmp180 end
  Serial.println("Altitude Currently is:");
  Serial.println(bmp.readAltitude());
  Serial.println("");


  if (i > 2) // run when the array is full
  {
    Serial.println("Altitude[0-1-2]");
    Serial.println(Altitude[0]); //debugging purposes
    Serial.println(Altitude[1]); //debugging purposes
    Serial.println(Altitude[2]);  //debugging purposes
    Altitudemaths =  (Altitude[0] +  Altitude[1] +  Altitude[2]) / 3;   //calculate the average altitude


    Serial.println("AltitudeMaths:");
    Serial.println(Altitudemaths); // the average value after three values
  }
  float Humidity = bme.readHumidity(); // set the humidity  to this variable

  // This triggers When the mask has fallen off the patient and the humidity has also fallen (Mask Removed)
  Serial.println(Altitudemaths * 0.95 && Humidity < 50);  // Altitude Fallen by 5% and Humidity below 50%
  if (bmp.readAltitude() < Altitudemaths * 0.95 )
  {
    Serial.println("Start the Alarm");
    AlarmSystem(); // run the alarm system
    Serial.println("Finished Alarm ");
  }
  // The Force Resitive Sensor Detetction Code
  forceReading = analogRead(forcePin);

  if (forceReading == 0) {
    digitalWrite(14, HIGH);   // change the led

  }
  else {
    digitalWrite(14, LOW);   // The green light will show when Force resistive touch is not pressed

  }
  String TempTempStr = readBME280Temperature();
  int TempTemp = TempTempStr.toInt();

  if ( 13 > TempTemp ) {
    Serial.println(TempTemp);
    Serial.println("Start the Alarm (Temp low)");
    AlarmSystem(); // run the alarm system
    Serial.println("Finished Alarm ");
  }

  char default_database[] = "OxygenLog";
  char default_table[]    = "Log";

  // UPDATE QUERY
  // Update all current values to most recent data
  String humidity = readBME280Humidity();
  String pressure = readBME280Pressure();
  String temperature = readBME280Temperature();
  String Time = Time2;
  String Altitude = readBMP180Altitude();

  //create SQL string that will be sent to the server
  // Then append all additional strings to the end of the SQL statement
  String TempSQLStr = "UPDATE OxygenLog.Log SET Humidity=";
  TempSQLStr.concat(humidity);
  String TempSQLStr2 = ",Pressure=";
  TempSQLStr.concat(TempSQLStr2);
  TempSQLStr.concat(pressure);
  String TempSQLStr3 = ",Temperature=";
  TempSQLStr.concat(TempSQLStr3);
  TempSQLStr.concat(temperature);
  String TempSQLStr4 = ",Time=";
  TempSQLStr.concat(TempSQLStr4);
  TempSQLStr.concat(Time);
  String TempSQLStr5 = ",Altitude=";
  TempSQLStr.concat(TempSQLStr5);
  TempSQLStr.concat(Altitude);
  String TempSQLStr6 = " WHERE ID=1";
  TempSQLStr.concat(TempSQLStr6);

  Serial.println("TempSQLStr"); // check what the statement says before sending
  Serial.println(TempSQLStr);

  const char *cstr = TempSQLStr.c_str(); //convert to const char for it to be sent to the server
  sprintf(query, cstr);
  //sprintf(query, INSERT_SQL, Humidity, t);
  Serial.println("Recording data.");
  // Serial.println(query);

  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);

  cur_mem->execute(query);
  delay(10000);
  delete cur_mem;
  // END OF SQL CHANGE STATEMENT

}

String readBME280Humidity() {
  float h = bme.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}


void AlarmSystem()  //Alarm system function Triggers the LED and Buzzer
{

  for (int i = 0; i < 160; i++) {
    digitalWrite(15, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1); // delay 1ms
    digitalWrite(15, LOW); // send low signal to buzzer
    delay(1);
  }

  for (int i = 0; i < 160; i++) {  // make a sound
    digitalWrite(buzzer, HIGH); // send high signal to buzzer
    Serial.println("Buzzer Is running");

    delay(1); // delay 1ms
    digitalWrite(buzzer, LOW); // send low signal to buzzer
    delay(1);
  }
  delay(50);
  for (int j = 0; j < 150; j++) { //make another sound
    digitalWrite(buzzer, HIGH);
    delay(2); // delay 2ms
    digitalWrite(buzzer, LOW);
    delay(2);
  }

}

String readBME280Pressure() {
  float p = bme.readPressure() / 100.0F;
  if (isnan(p)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}

String readBME280Temperature() {
  float t = bme.readTemperature();  // Read temperature as Celsius
  if (isnan(t)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readBMP180Altitude() {
  float a =  bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.println("Altitude From Method");
  Serial.println(a);
  return String(a);
}
