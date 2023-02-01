/**
 * @file MAX30100_IR.cpp
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
// Sampling is tightly related to the dynamic range of the ADC.
// refer to the datasheet for further info
#define SAMPLING_RATE                       MAX30100_SAMPRATE_50HZ

// The LEDs currents must be set to a level that avoids clipping and maximises the
// dynamic range
#define IR_LED_CURRENT                      MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT                     MAX30100_LED_CURR_27_1MA

// The pulse width of the LEDs driving determines the resolution of
// the ADC (which is a Sigma-Delta).
// set HIGHRES_MODE to true only when setting PULSE_WIDTH to MAX30100_SPC_PW_1600US_16BITS
#define PULSE_WIDTH                         MAX30100_SPC_PW_1600US_16BITS
#define HIGHRES_MODE                        true

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
bool htrig, htrigh;


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
 
    // if (!pox.begin())
    // {
    //     Serial.println("FAILED");
    //     ESP.restart();
    // }
    // else
    // {
    //     Serial.println("SUCCESS");
    // }

    if (!sensor.begin())
    {
        Serial.println("FAILED");
        ESP.restart();
    }
    else
    {
        Serial.println("SUCCESS");
    }

    sensor.setMode(MAX30100_MODE_SPO2_HR);
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    sensor.setLedsPulseWidth(PULSE_WIDTH);
    sensor.setSamplingRate(SAMPLING_RATE);
    sensor.setHighresModeEnabled(HIGHRES_MODE);

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

void loop()
{
    uint16_t ir, red;

    sensor.update();

    while (sensor.getRawValues(&ir, &red)) {
        Serial.print(ir);
        Serial.print('\t');
        Serial.println(red);
        if (ir > 40000)
        {
            htrig = 1;
        }
        
        if (ir < 40000)
        {
            htrig = 0;
        }
    }

    if (htrigh != htrig)
    {
        if (htrig == 1)
        {
            Firebase.setBool(fbdo, id+"/htrig", true);
            Serial.println("true");
            htrigh = htrig;
        }
    
        if (htrig == 0)
        {
            Firebase.setBool(fbdo, id+"/htrig", false);
            Serial.println("false");
            htrigh = htrig;
        }
    }
}