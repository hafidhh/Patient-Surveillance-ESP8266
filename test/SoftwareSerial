//Simple_SoftwareSerial_GPS_Echo

//Reads characters from a GPS using Software Serial and echoes them to the Arduino Serial Monitor at 115200 baud.   

//Important Note
//--------------
//Make sure the #defines below for the GPS TX and GPS RX pin numbers below match what you have connected and that the GPS
//baud rate used by the GPS is correct, its often 9600 baud but not always. 

#define GPSTX 15 //pin number for GPS TX output - data from Arduino into GPS
#define GPSRX 13 //pin number for GPS RX input - to Arduino from GPS
#define GPSBaud 9600 //GPS Baud rate
#define Serial_Monitor_Baud 115200   //this is baud rate used for the Arduino IDE Serial Monitor

#include <Arduino.h>
#include <SoftwareSerial.h> 

#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>

SoftwareSerial GPSserial(GPSRX, GPSTX);  

void setup()
{
 Serial.begin(Serial_Monitor_Baud);   //start Serial console ouput
 GPSserial.begin(GPSBaud);//start softserial for GPS at defined baud rate
}
    
void loop()
{
  while (GPSserial.available() > 0)
  Serial.write(GPSserial.read());
}