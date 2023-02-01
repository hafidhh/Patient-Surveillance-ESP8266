/**
 * @file GPS.cpp
 * @author Hafidh Hidayat (hafidhhidayat@hotmail.com)
 * @brief 
 * @version 1.0.0
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 * Github :
 * https://github.com/hafidhh
 */

#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include <FirebaseESP8266.h>
#include <Wire.h>

const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(13, 15); // The serial connection to the GPS device

double latitude , longitude;
unsigned long milis, ulang=0;
int year , month , date, hour , minute , second;
String year_str, month_str, date_str, t_str , time_str , lat_str , lng_str;
int pm;
WiFiServer server(80);

//OTA
void setup_OTA()
{   
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        }
        else { // U_FS
        type = "filesystem";
    }
    
    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } 
        else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  ss.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //connecting to wifi
  WiFi.hostname("skripsi");
  while (WiFi.status() != WL_CONNECTED)// while wifi not connected
  {
    delay(500);
    Serial.print("."); //print "...."
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());  // Print the IP address

  setup_OTA();
}


void loop()
{
  ArduinoOTA.handle();
  
  while (ss.available() > 0) //while data is available
  // Serial.write(ss.read());
    if (gps.encode(ss.read())) //read gps data
    {
      if (gps.location.isValid()) //check whether gps location is valid
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6); // latitude location is stored in a string
        Serial.print(lat_str);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6); //longitude location is stored in a string
        Serial.print(lng_str);
      }
      if (gps.date.isUpdated()&&gps.date.isValid()) //check whether gps date is valid
      {
        date_str = "";
        year = gps.date.year();
        month = gps.date.month();
        date = gps.date.day();
        
        if (year < 10)
          date_str = date_str+'0';
        date_str = date_str+String(year); // values of date,month and year are stored in a string
        date_str = date_str+" / ";
        if (month < 10)
          date_str = date_str+'0';
        date_str = date_str+String(month); // values of date,month and year are stored in a string
        date_str = date_str+" / ";
        if (date < 10)
          date_str = date_str+'0';
        date_str = date_str+String(date);// values of date,month and year are stored in a string
        
      }
      if (gps.time.isValid())  //check whether gps time is valid
      {
        time_str = "";
        hour = gps.time.hour();
        minute = gps.time.minute();
        second = gps.time.second();
        minute = (minute + 30); // converting to IST
        if (minute > 59)
        {
          minute = minute - 60;
          hour = hour + 1;
        }
        hour = (hour + 7) ; // converting to IST
        if (hour > 23)
          hour = hour - 24;   
        if (hour >= 12)  // checking whether AM or PM
          pm = 1;
        else
          pm = 0;
        hour = hour % 12;
        if (hour < 10)
          time_str = '0';
        time_str += String(hour); //values of hour,minute and time are stored in a string
        time_str += " : ";
        if (minute < 10)
          time_str += '0';
        time_str += String(minute); //values of hour,minute and time are stored in a string
        time_str += " : ";
        if (second < 10)
          time_str += '0';
        time_str += String(second); //values of hour,minute and time are stored in a string
        if (pm == 1)
          time_str += " PM ";
        else
          time_str += " AM ";
      }
    }
 
 WiFiClient client = server.available(); // Check if a client has connected
  if (!client)
  {
    return;
  }
  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>GPS DATA</title> <style>";
  s += "a:link {background-color: YELLOW;text-decoration: none;}";
  s += "table, th, td </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER> GPS DATA</h1>";
  s += "<p ALIGN=CENTER style=""font-size:150%;""";
  s += "> <b>Location Details</b></p> <table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Latitude</th>";
  s += "<td ALIGN=CENTER >";
  s += lat_str;
  s += "</td> </tr> <tr> <th>Longitude</th> <td ALIGN=CENTER >";
  s += lng_str;
  s += "</td> </tr> <tr>  <th>Date</th> <td ALIGN=CENTER >";
  s += date_str;
  s += "</td></tr> <tr> <th>Time</th> <td ALIGN=CENTER >";
  s += time_str;
  s += "</td>  </tr> </table> ";
 
  s += "</body> </html>";

  client.print(s); // all the values are send to the webpage
  delay(100);
}