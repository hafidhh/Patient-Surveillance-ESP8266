/**
 * @file Geofencing+bat.cpp
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

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoOTA.h>
#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// const char* ssid = "Test"; //ssid of your wifi
// const char* password = "123...123"; //password of your wifi

const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(13, 15); // The serial connection to the GPS device
FirebaseData fbdo;
FirebaseData fbdo1;
FirebaseData fdbo2;

// Connections : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0
//PulseOximeter pox;
 
float BPM, SpO2;
String BPM_str, SpO2_str;
uint32_t tsLastReport = 0;

int battery_value,  battery_percentage;
float voltage_battery, calibration=0;

float latitude , longitude, dist;
int longitude2, latitude2, longitude3=0, latitude3=0;
String lat_str , lng_str, logs, jarak, gLatitude_str, gLongitude_str, latlong;
float gLatitude, gLongitude;
float thresholdDistance = 0.05; //km
bool gtrig = false;

byte Second, Minute, Hour, Day, Month;
int Year;
char Time[]  = "00:00:00";
char Date[]  = "2000/00/00";

String id;

unsigned long milis, ulang=0;

void setup_OTA()
{
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);
    
    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");
    
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
    
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    
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

float HaverSine(float lat1, float lon1, float lat2, float lon2){
    float ToRad = PI/180;
    float R = 6371;   // radius earth in Km

    float x = ((lon2*ToRad)-(lon1*ToRad))*cos(((lat1*ToRad)+(lat2*ToRad))/2);
    float y = ((lat2*ToRad)-(lat1*ToRad));
    float d = sqrt((x*x)+(y*y))*R;
    Serial.println(String(x,6));
    Serial.println(String(y,6));
    Serial.println(String(d,6));
    return d;
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
    id = WiFi.macAddress();
    while (WiFi.status() != WL_CONNECTED)// while wifi not connected
    {
        delay(500);
        Serial.print("."); //print "...."
    }
    Serial.println("");
    Serial.println("WiFi connected");

    setup_OTA();

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Firebase.getFloat(fbdo, "/glatitude");
    Firebase.getFloat(fbdo1, "/glongitude");
    Firebase.getFloat(fdbo2, "/geofenceradius");
    gLatitude = fbdo.floatData();
    gLongitude = fbdo1.floatData();
    thresholdDistance = fdbo2.floatData()/1000;
    Serial.println(gLatitude, 6);
    Serial.println(gLongitude, 6);
    Serial.println(thresholdDistance);
}

void loop()
{
    ArduinoOTA.handle();
    // pox.update();
    // BPM = pox.getHeartRate();
    // SpO2 = pox.getSpO2();
    // BPM_str = String (BPM);
    // SpO2_str = String (SpO2);
    // if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    // {
    //     Serial.print("Heart rate:");
    //     Serial.print(BPM);
    //     //Firebase.setString("001/BPM", BPM_str);
    //     Serial.print(" bpm / SpO2:");
    //     Serial.print(SpO2);
    //     //Firebase.setString("001/SpO2", SpO2_str);
    //     Serial.println(" %");

    //     Serial.print("LEDCURRENT: ");
    //     Serial.println(pox.getRedLedCurrentBias());
        
    //     tsLastReport = millis();
    // }

    while (ss.available() > 0) //while data is available
        if (gps.encode(ss.read())) //read gps data
        {
            //Battery
            battery_value = analogRead(A0);
            voltage_battery = (((battery_value * 3.3) / 1024) * 2 + calibration);
            battery_percentage = mapfloat(voltage_battery, 3.2, 3.8, 0, 100);
            if (battery_percentage >= 100)
            {
                battery_percentage = 100;
            }
            if (battery_percentage <= 0)
            {
                battery_percentage = 1;
            }

            //GPS
            if (gps.location.isValid()&&gps.location.isUpdated()) //check whether gps location is valid
            {
                latitude = gps.location.lat();
                latitude2 = latitude*1000000;
                lat_str = String(latitude, 7); // latitude location is stored in a string
                longitude = gps.location.lng();
                longitude2 = longitude*1000000;
                lng_str = String(longitude, 7); //longitude location is stored in a string
                latlong = lat_str+", "+lng_str;
                dist = HaverSine(gLatitude,gLongitude,latitude,longitude);
                if (dist >= thresholdDistance )
                {
                    gtrig = true;
                    jarak = String((dist - thresholdDistance)*1000);
                }
                else
                {
                    gtrig = false;
                    jarak = "0";
                }
            }
            if (gps.date.isValid()&&gps.date.isUpdated())
            {
                Day   = gps.date.day();
                Month = gps.date.month();
                Year  = gps.date.year();
            }
            
            if (gps.time.isValid()&&gps.time.isUpdated())
            {
                Minute = gps.time.minute();
                Second = gps.time.second();
                Hour   = gps.time.hour();                
            }

            if ((longitude2!=longitude3||latitude2!=latitude3)&&millis()>=ulang+3000) //Kirim Data
            {
                // set current UTC time
                setTime(Hour, Minute, Second, Day, Month, Year);
                // add the offset to get local time UTC+7 (7x3600 >> 1 jam = 3600s))
                adjustTime(25200);

                // update time array
                Time[6] = second() / 10 + '0';
                Time[7] = second() % 10 + '0';
                Time[3]  = minute() / 10 + '0';
                Time[4] = minute() % 10 + '0';
                Time[0]  = hour()   / 10 + '0';
                Time[1]  = hour()   % 10 + '0';
                
                // update date array
                Date[2] = (year()  / 10) % 10 + '0';
                Date[3] =  year()  % 10 + '0';
                Date[5]  =  month() / 10 + '0';
                Date[6] =  month() % 10 + '0';
                Date[8]  =  day()   / 10 + '0';
                Date[9]  =  day()   % 10 + '0';

                //trig = 1 >> diluar geofence
                if (gtrig==true)
                {
                    logs = "Users"+id+"/logs/"+Date+"/"+Time;
                    Firebase.setString(fbdo, "Users"+id+"/latitude, longitude", latlong);
                    Firebase.setString(fbdo, logs, latlong);
                    Firebase.setString(fbdo, "Users"+id+"/jarak",jarak);
                    Firebase.setBool(fbdo, "Users"+id+"/gtrig", gtrig);
                    Firebase.setInt(fbdo, "Users"+id+"/battery", battery_percentage);
                }
                if (gtrig==false)
                {
                    Firebase.setString(fbdo, "Users"+id+"/latitude, longitude", latlong);
                    Firebase.setString(fbdo, "Users"+id+"/jarak",jarak);
                    Firebase.setBool(fbdo, "Users"+id+"/gtrig", gtrig);
                    Firebase.setInt(fbdo, "Users"+id+"/battery", battery_percentage);
                }
                /*Serial.println(longitude2);
                Serial.println(latitude2);
                Serial.println(dist*1000);
                longitude3 = longitude2;
                latitude3 = latitude2;*/
                ulang = millis();
            }         
        }
}