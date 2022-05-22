#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <FirebaseESP8266.h>
#include <TimeLib.h>
#include <Wire.h>
#include "MAX30105.h"
 
#include "heartRate.h"

const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"

SoftwareSerial ss(13, 15); // The serial connection to the GPS device
FirebaseData fbdo;
FirebaseData fbdo1;
FirebaseData fdbo2;
 
MAX30105 particleSensor;
 
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
 
float beatsPerMinute;
int beatAvg;

String id;
bool htrig;
bool htrigh = true;

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

    Serial.println("Initializing...");
    
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

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}
 
void loop()
{
    ArduinoOTA.handle();
    long irValue = particleSensor.getIR();

    // if (checkForBeat(irValue) == true)
    // {
    //     htrig = true;
    // }
    // else if(checkForBeat(irValue) == false)
    // {
    //     htrig = false;
    // }

    // if (irValue<5000)
    // {
    //     htrig = false;
    // }
    // else if (irValue>=5000)
    // {
    //     htrig = true;
    // }


    
    if (checkForBeat(irValue) == true)
    {
        //We sensed a beat!
        long delta = millis() - lastBeat;
        lastBeat = millis();
        
        beatsPerMinute = 60 / (delta / 1000.0);
        
        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
            rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
            rateSpot %= RATE_SIZE; //Wrap variable
            
            //Take average of readings
            beatAvg = 0;
            for (byte x = 0 ; x < RATE_SIZE ; x++)
                beatAvg += rates[x];
            beatAvg /= RATE_SIZE;
        }
    }
    if (irValue < 50000)
        beatsPerMinute=0;

    if (beatsPerMinute>=1)
    {
        htrig=false;
    }
    else if (beatsPerMinute<1)
    {
        htrig=true;
    }
    
    if (htrig != htrigh)
    {
        Firebase.setBool(fbdo, id+"/htrig", htrig);
        Serial.println(fbdo.errorReason());
        htrigh = htrig;
    }
    
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.println(beatAvg);


    // htrig = false;
    
    // if (irValue < 50000)
    // {
    //     Serial.print(" No finger?");
    //     htrig = true;
    // }
        
    // Serial.println();

    // if (htrig != htrigh)
    // {
    //     Firebase.setBool(fbdo, id+"/htrig", htrig);
    //     Serial.println(fbdo.errorReason());
    //     htrigh = htrig;
    // }
    
}