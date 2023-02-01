/**
 * @file Geofencing.cpp
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
#include <FirebaseESP8266.h>
#include <TimeLib.h>
#include <Wire.h>

const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"

#define REPORTING_PERIOD_MS 1000

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(13, 15); // The serial connection to the GPS device D8, D7
FirebaseData fbdo;
FirebaseData fbdo1;

float latitude , longitude, dist;
int longitude2, latitude2, longitude3=0, latitude3=0;
String lat_str , lng_str, logs, jarak, gLatitude_str, gLongitude_str, latlong;
float gLatitude, gLongitude;
float thresholdDistance = 0.05; //km
bool gtrig = false;
unsigned long milis, ulang=0;
byte Second, Minute, Hour, Day, Month;
int Year;
char Time[]  = "00:00:00";
char Date[]  = "2000/00/00";
String id, fbref;

String geofence;
float floatgeofence[3];

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
    fbref = "Users/"+id;
    while (WiFi.status() != WL_CONNECTED)// while wifi not connected
    {
        delay(500);
        Serial.print("."); //print "...."
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());  // Print the IP address

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);

    // Firebase.getString(fbdo, "/geofence");
    // geofence = fbdo.stringData();
    // int geofencelength = geofence.length()+1;
    // char chargeofence[geofencelength];
    // geofence.toCharArray(chargeofence , geofencelength);
    // char *token = strtok(chargeofence, ",");
    // for (size_t i = 0; i < 3; i++)
    // {
    //     floatgeofence[i] = strtof(token,NULL);
    //     token = strtok(NULL, ",");
    // }


    Firebase.getFloat(fbdo, "/glatitude");
    Firebase.getFloat(fbdo1, "/glongitude");
    gLatitude = fbdo.floatData();
    gLongitude = fbdo1.floatData();
    // gLatitude = floatgeofence[0];
    // gLongitude = floatgeofence[1];
    // thresholdDistance = floatgeofence[2]/1000;
    Serial.println(gLatitude, 6);
    Serial.println(gLongitude, 6);
    Serial.println(String((thresholdDistance*1000),0));
}

//geofencing
float HaverSine(float lat1, float lon1, float lat2, float lon2)
{
    float ToRad = PI/180.00;
    float R = 6371.00;   // radius earth in Km
    float dLat = (lat2-lat1) * ToRad;
    float dlon = (lon2-lon1) * ToRad;
    float a = sin(dLat/2) * sin(dLat/2) + cos(lat1 * ToRad) * cos(lat2 * ToRad) * sin(dlon/2) * sin(dlon/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    float d = R * c;
    return d;
}

void loop()
{
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
    // Serial.println("1");
        if (gps.encode(ss.read())) //read gps data
        {
            // Serial.println("2");
            if (gps.location.isValid()&&gps.location.isUpdated()) //check whether gps location is valid
            {
                latitude = gps.location.lat();
                latitude2 = latitude*1000000;
                lat_str = String(latitude, 6); // latitude location is stored in a string
                longitude = gps.location.lng();
                longitude2 = longitude*1000000;
                lng_str = String(longitude, 6); //longitude location is stored in a string
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

                //trig = 1 >> diluar geofencing
                if (gtrig==true)
                {
                    logs = fbref+"/logs/"+Date+"/"+Time;
                    Firebase.setString(fbdo, fbref+"/latitude, longitude", latlong);
                    Firebase.setString(fbdo, logs, latlong);
                    Firebase.setString(fbdo, fbref+"/jarak",jarak);
                    Firebase.setBool(fbdo, fbref+"/gtrig", gtrig);
                }
                if (gtrig==false)
                {
                    Firebase.setString(fbdo, fbref+"/latitude, longitude", latlong);
                    Firebase.setBool(fbdo, fbref+"/gtrig", gtrig);
                    Firebase.setString(fbdo, fbref+"/jarak",jarak);
                }
                /*Serial.println(longitude2);
                Serial.println(latitude2);
                Serial.println(dist*1000);*/
                longitude3 = longitude2;
                latitude3 = latitude2;
                ulang = millis();
            }         
        }
}