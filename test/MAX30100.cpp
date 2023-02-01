/**
 * @file MAX30100.cpp
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

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <TimeLib.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"

const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"

#define REPORTING_PERIOD_MS 2000

SoftwareSerial ss(13, 15); // The serial connection to the GPS device
FirebaseData fbdo;
FirebaseData fbdo1;
FirebaseData fdbo2;

// Connections : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0
PulseOximeter pox;
MAX30100 sensor;
 
float BPM, SpO2;
String BPM_str, SpO2_str;
uint32_t tsLastReport = 0;
String id;
bool htrig;
bool htrigh = true;


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
    Serial.println(WiFi.localIP());  // Print the IP address
    Serial.print("Initializing Pulse Oximeter..");
 
    if (!pox.begin())
    {
        Serial.println("FAILED");
        pox.shutdown();
        delay(5000);
        ESP.restart();
    }
    else
    {
        Serial.println("SUCCESS");
    }

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

void loop()
{
    pox.update();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    BPM_str = String (BPM);
    SpO2_str = String (SpO2);

    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
        Serial.print("Heart rate:");
        Serial.print(BPM);
        //Firebase.setString("001/BPM", BPM_str);
        Serial.print(" bpm / SpO2:");
        Serial.print(SpO2);
        //Firebase.setString("001/SpO2", SpO2_str);
        Serial.println(" %");

        Serial.print("LEDCURRENT: ");
        Serial.println(pox.getRedLedCurrentBias());

        if (BPM == 0)
        {
            htrig = 1;
            Serial.println(htrig);
            Serial.println(htrigh);
        }

        if (BPM > 0)
        {
            htrig = 0;
            Serial.println(htrig);
            Serial.println(htrigh);
        }
        
        tsLastReport = millis();
    }

    if (htrigh != htrig)
    {
        pox.shutdown();
        delay(5000);
        if (htrig == 1)
        {
            //Firebase.setBool(fbdo, id+"/htrig", true);
            if (Firebase.setBool(fbdo, id+"/htrig", true))
            {
                Serial.println("true");
                pox.resume();
                htrigh = htrig;
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }
            delay(5000);
            
        }
    
        else if (htrig == 0)
        {
            //Firebase.setBool(fbdo, id+"/htrig", false);
            if (Firebase.setBool(fbdo, id+"/htrig", false))
            {
                Serial.println("false");
                pox.resume();
                htrigh = htrig;
            }
            else
            {
                Serial.println(fbdo.errorReason());
            }
            delay(5000);
        }
    }
}