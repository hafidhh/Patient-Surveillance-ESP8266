#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
 
const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"

FirebaseData fbdo;

uint32_t tlastreport, tlastreportfb;
int milis;
 
int analogInPin  = A0;    // Analog input pin
int sensorValue;          // Analog Output of Sensor
float calibration = 0; // Check Battery voltage using multimeter & add/subtract the value
int bat_percentage;
String id;
 
WiFiClient client;
 
void setup()
{
    Serial.begin(115200);
    Serial.println("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print("*");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    id = WiFi.macAddress();

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
 
void loop()
{
    if (millis()-tlastreport>1000)
    {
        sensorValue = analogRead(analogInPin);
    
        float voltage = (((sensorValue * 3.3) / 1024) * 2 + calibration); //multiply by two as voltage divider network is 100K & 100K Resistor
    
        bat_percentage = mapfloat(voltage, 3.2, 3.8, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
    
        if (bat_percentage >= 100)
        {
            bat_percentage = 100;
        }
    
        if (bat_percentage <= 0)
        {
            bat_percentage = 1;
        }
        Serial.print("Analog Value = ");
        Serial.print(sensorValue);
        Serial.print("\t Output Voltage = ");
        Serial.print(voltage);
        Serial.print("\t Battery Percentage = ");
        Serial.println(bat_percentage);
        tlastreport=millis();
    }
    if (millis()-tlastreportfb>300000)
    {
        Firebase.pushInt(fbdo, id+"/Battery", bat_percentage);
        Serial.println(fbdo.errorReason());
        tlastreportfb=millis();
    }
    
}