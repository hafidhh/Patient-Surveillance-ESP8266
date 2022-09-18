#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ArduinoOTA.h>
#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <TimeLib.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

//Heart Rate
MAX30105 particleSensor;
// const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
// byte rates[RATE_SIZE]; //Array of heart rates
// byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
// int beatAvg;

//GPS
TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(13, 15); // The serial connection to the GPS device D7, D8
double latitude , longitude, dist;
int longitude2, latitude2, longitude3, latitude3;
String lat_str , lng_str, jarak, gLatitude_str, gLongitude_str, latlong;
double gLatitude, gLongitude, thresholdDistance = 0.05; //km
bool gtrig = false, htrigh = false, htrig;
byte Second, Minute, Hour, Day, Month;
int Year;

//Firebase (Realtime Database)
FirebaseData fbdo;
String id, fbref;
unsigned long milis, ulang=0;

//Hitung Jarak
double HaverSine(double lat1, double lon1, double lat2, double lon2)
{
    double ToRad = PI/180;
    int R = 6371;   // radius earth in Km

    double x = ((lon2*ToRad)-(lon1*ToRad))*cos(((lat1*ToRad)+(lat2*ToRad))/2);
    double y = ((lat2*ToRad)-(lat1*ToRad));
    double d = sqrt((x*x)+(y*y))*R;
    // Serial.println(String(x,6));
    // Serial.println(String(y,6));
    // Serial.println(String(d,6));
    return d;
}
//Maping Battery
int mapfloat(double x, float in_min, float in_max, int out_min, int out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
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
//Sensor Detak Jantung
void detakjantung()
{
    long irValue = particleSensor.getIR();
    
    if (checkForBeat(irValue) == true)
    {
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();
        
        beatsPerMinute = 60 / (delta / 1000.0);
        
        // if (beatsPerMinute < 255 && beatsPerMinute > 20)
        // {
        //     rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        //     rateSpot %= RATE_SIZE; //Wrap variable
            
        //     //Take average of readings
        //     beatAvg = 0;
        //     for (byte x = 0 ; x < RATE_SIZE ; x++)
        //         beatAvg += rates[x];
        //     beatAvg /= RATE_SIZE;
        // }
    }
    if (irValue < 50000)
        beatsPerMinute=0;

    if (beatsPerMinute>20)
    {
        htrig=false;
    }
    else if (beatsPerMinute<1)
    {
        htrig=true;
    }

    // Serial.print("IR=");
    // Serial.print(irValue);
    // Serial.print(", BPM=");
    // Serial.print(beatsPerMinute);
    // Serial.print(", Avg BPM=");
    // Serial.println(beatAvg);
}

void setup()
{
    // const char* ssid = "ZTE-972ec0"; //ssid of your wifi
    // const char* password = "hf170798"; //password of your 
    const char* ssid = "Test"; //ssid of your wifi
    const char* password = "123...123"; //password of your wifi
    #define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
    #define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"
    pinMode(16, OUTPUT);
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

    setup_OTA();

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

    //Get geofence parameter(titik tengah, radius)
    Firebase.getString(fbdo, "/geofence");
    String geofence = fbdo.stringData();
    //Split String from Firebase (delimiter ,)
    int geofencelength = geofence.length()+1;
    char chargeofence[geofencelength];
    geofence.toCharArray(chargeofence , geofencelength);
    char *token = strtok(chargeofence, ",");
    //String to Double
    double doublegeofence[3];
    for (size_t i = 0; i < 3; i++)
    {
        doublegeofence[i] = atof(token);
        token = strtok(NULL, ",");
    }
    gLatitude = doublegeofence[0];
    gLongitude = doublegeofence[1];
    thresholdDistance = doublegeofence[2]/1000;
    //Set ActiveUsers to True >> true = tampil di apps, false = tidak tampil di apps
    Firebase.setBool(fbdo, "ActiveUsers/"+id, true);
    Serial.println(gLatitude, 6);
    Serial.println(gLongitude, 6);
    Serial.println(String((thresholdDistance*1000),0));

    //Heart Rate Sensor Activated
    digitalWrite(16, HIGH);

    delay(1000);

    // Initialize sensor
    if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) //Use default I2C port, 400kHz speed
    {
        Serial.println("MAX30105 was not found. Please check wiring/power. ");
        while (1);
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");
    
    particleSensor.setup(); //Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
    
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)// if wifi not connected
    {
        Serial.println("DC");
        while (1);
    }

    ArduinoOTA.handle();
    detakjantung();

    while (ss.available() > 0) //while data is available
        if (gps.encode(ss.read())) //read gps data
        {
            //GPS >> Location (Lat,Long)
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
            //GPS >> Date
            if (gps.date.isValid()&&gps.date.isUpdated())
            {
                Day   = gps.date.day();
                Month = gps.date.month();
                Year  = gps.date.year();
            }
            //GPS >> Time
            if (gps.time.isValid()&&gps.time.isUpdated())
            {
                Minute = gps.time.minute();
                Second = gps.time.second();
                Hour   = gps.time.hour();                
            }
        }
        
            //Battery
            double calibration=0.009179688;
            int battery_value = analogRead(A0);
            double voltage_battery = (((battery_value * 3.3) / 512) + calibration);
            int battery_percentage = mapfloat(voltage_battery, 2.8, 3.7, 0, 100);
            if (battery_percentage >= 100)
            {
                battery_percentage = 100;
            }
            if (battery_percentage <= 0)
            {
                battery_percentage = 1;
            }

            //Send Data
            if (millis()>=ulang+4000) 
            {
                // Firebase.getFloat(fbdo, "/gtrig");
                // gtrig = fbdo.boolData();
                Firebase.setInt(fbdo, fbref+"/battery", battery_percentage);
                // Firebase.setInt(fbdo, fbref+"/vbattery", voltage_battery);
                Firebase.setFloat(fbdo, fbref+"/bpm", beatsPerMinute);
                // if (htrig != htrigh)
                // {
                //     Firebase.setBool(fbdo, fbref+"/htrig", htrig);
                //     htrigh = htrig;
                // }
                if (longitude2!=longitude3||latitude2!=latitude3)
                {
                    char Time[]  = "00:00:00";
                    char Date[]  = "2000/00/00";
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

                    Firebase.setString(fbdo, fbref+"/latitude, longitude", latlong);

                    //trig = 1 >> diluar geofence
                    if (gtrig==true)
                    {
                        String logs = fbref+"/logs/"+Date+"/"+Time;
                        Firebase.setString(fbdo, logs, latlong);
                        Firebase.setString(fbdo, fbref+"/jarak",jarak);
                        Firebase.setBool(fbdo, fbref+"/gtrig", gtrig);
                    }
                    if (gtrig==false)
                    {
                        Firebase.setString(fbdo, fbref+"/jarak",jarak);
                        Firebase.setBool(fbdo, fbref+"/gtrig", gtrig);
                    }
                    // Serial.println(longitude2);
                    // Serial.println(latitude2);
                    // Serial.println(dist*1000);
                    longitude3 = longitude2;
                    latitude3 = latitude2;
                }
                ulang = millis();
            }     
}