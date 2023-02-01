/**
 * @file main.cpp
 * @author Hafidh Hidayat (hafidhhidayat@hotmail.com)
 * @brief Geofencing Device Program
 * @version 1.1.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 * Github :
 * https://github.com/hafidhh
 * https://github.com/hafidhh/Patient-Surveillance-ESP8266
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

// Heart Rate
MAX30105 particleSensor;

const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
long lastwait = 0;

float beatsPerMinute;
int beatAvarage = 0;

bool ready = false;

// GPS
TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(13, 15); // The serial connection to the GPS device D7, D8
double latitude , longitude, dist;
int longitude2, latitude2, longitude3, latitude3;
String lat_str , lng_str, jarak, gLatitude_str, gLongitude_str, latlong;
double gLatitude, gLongitude, thresholdDistance = 0; //km
bool gtrig = false, htrigh = false, htrig = false;
byte Second, Minute, Hour, Day, Month;
int Year;

const byte arraySize = 5;
double latArray[arraySize];
double longArray[arraySize];
byte arraySpot;
double latAvg = 0;
double longAvg = 0;
bool first = false;

// Firebase (Realtime Database)
FirebaseData fbdo;
String id, fbref;
unsigned long milis, ulang=0;

/**
 * @brief Set the up OTA object, OTA Update
 * 
 */
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
	Serial.println("OTA Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}

/**
 * @brief Hitung Jarak
 * 
 * @param lat1 latitude titik tengah geofencing
 * @param lon1 longitude titik tengah geofencing
 * @param lat2 latitude objek
 * @param lon2 llongitude objek
 * @return float distance
 */
float HaverSine(double lat1, double lon1, double lat2, double lon2)
{
	double ToRad = PI/180;
	int R = 6371;   // radius earth in Km

	double x = ((lon2*ToRad)-(lon1*ToRad))*cos(((lat1*ToRad)+(lat2*ToRad))/2);
	double y = ((lat2*ToRad)-(lat1*ToRad));
	double d = sqrt((x*x)+(y*y))*R;
	return d;
}

/**
 * @brief Battery
 * 
 * @param pin Analog pin battery
 * @param calibration calibration voltage
 * @return int Battery Percentage
 */
int Battery(uint8_t pin, double calibration = 0)
{
	int batteryValue = analogRead(pin);
	double voltageBattery = (((batteryValue * 3.3) / 512) + calibration);
	int batteryPercentage = (voltageBattery - 2.8) * 100 / (3.7 - 2.8);
	return batteryPercentage;
}

/**
 * @brief Sensor detak jantung
 * 
 * @return int Beat
 */
int heartRate()
{
	bool avg = false;
	long wait = millis();
	long irValue = particleSensor.getIR();

	if (checkForBeat(irValue) == true)
	{
		// We sensed a beat!
		long delta = millis() - lastBeat;
		lastBeat = millis();

		beatsPerMinute = 60 / (delta / 1000.0);
		avg = true;
		lastwait = millis();
	}

	if (checkForBeat(irValue) == false && (wait - lastwait >= 5000))
	{
		beatsPerMinute = 0;
		avg = true;
		lastwait = millis();
	}

	if (beatsPerMinute < 255 && beatsPerMinute >= 0 && avg == true)
	{
		beatAvarage = 0;
		rateSpot++;
		rateSpot %= RATE_SIZE; // Wrap variable
		rates[rateSpot] = (byte)beatsPerMinute; // Store this reading in the array
		
		// Take average of readings
		for (byte x = 0 ; x < RATE_SIZE ; x++)
			beatAvarage += rates[x];
		beatAvarage /= RATE_SIZE;
	}

	// No finger detected
	if (irValue < 50000 && (wait - lastwait >= 5000))
	{
		beatsPerMinute = 0;
		avg = true;
		lastwait = millis();
	}
	return beatsPerMinute;
}

void setup()
{
	const char* ssid = "Test"; // ssid
	const char* password = "123...123"; // password
	#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
	#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"
	
	pinMode(16, OUTPUT);
	digitalWrite(16, LOW);
	pinMode(16, OUTPUT);
	Serial.begin(115200);
	ss.begin(9600);
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password); // connecting to wifi
	WiFi.hostname("skripsi");
	id = WiFi.macAddress();
	fbref = "Users/"+id;
	while (WiFi.status() != WL_CONNECTED)// while wifi not connected
	{
		delay(500);
		Serial.print("."); // print "...."
	}
	Serial.println("");
	Serial.println("WiFi connected");

	setup_OTA();

	Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

	delay(5000);

	// Get geofence parameter(titik tengah, radius)
	Firebase.getString(fbdo, "/geofence");
	String geofence = fbdo.stringData();
	// Split String from Firebase (delimiter ,)
	int geofencelength = geofence.length()+1;
	char chargeofence[geofencelength];
	geofence.toCharArray(chargeofence , geofencelength);
	char *token = strtok(chargeofence, ",");
	// String to Double
	double doublegeofence[3];
	for (size_t i = 0; i < 3; i++)
	{
		doublegeofence[i] = atof(token);
		token = strtok(NULL, ",");
	}
	gLatitude = doublegeofence[0];
	gLongitude = doublegeofence[1];
	thresholdDistance = doublegeofence[2]/1000;
	// Set ActiveUsers to True >> true = tampil di apps, false = tidak tampil di apps
	Firebase.setBool(fbdo, "ActiveUsers/"+id, true);
	Serial.println(gLatitude, 6);
	Serial.println(gLongitude, 6);
	Serial.println(String((thresholdDistance*1000),0));

	// Heart Rate Sensor Activated
	digitalWrite(16, HIGH);

	delay(5000);

	// Initialize sensor
	if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) // Use default I2C port, 400kHz speed
	{
		Serial.println("MAX30105 was not found. Please check wiring/power. ");
		ready = true;
	}
	
	particleSensor.setup(); // Configure sensor with default settings
	particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
	particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED
}

void loop()
{
	ArduinoOTA.handle();
	int batteryPercentage = Battery(A0, 0.009179688);

	// Heart Rate
	int beat = heartRate();
	if (beat > 10)
	{
		htrig = false;
	}
	else if (beat <= 10)
	{
		htrig = true;
	}

	while (ss.available() > 0) // while data is available
	{
		if (gps.encode(ss.read())) // read gps data
		{
			if (gps.date.isUpdated())
			{
				// GPS >> Date
				Day   = gps.date.day();
				Month = gps.date.month();
				Year  = gps.date.year();
				
			}
			if (gps.time.isUpdated())
			{
				// GPS >> Time
				Hour   = gps.time.hour();
				Minute = gps.time.minute();
				Second = gps.time.second();
			}
			if (gps.location.isUpdated()) // check whether gps location is valid
			{
				// GPS >> Location (Lat,Long)
				latitude = gps.location.lat();
				longitude = gps.location.lng();

				arraySpot++;
				arraySpot %= arraySize; // Wrap variable
				latArray[arraySpot] = latitude; // Store this reading in the array
				longArray[arraySpot] = longitude; // Store this reading in the array

				latAvg = 0;
				longAvg = 0;

				// Take average of readings
				for (byte x = 0 ; x < arraySize ; x++)
				{
					latAvg += latArray[x];
					longAvg += longArray[x];
				}

				if (first == true)
				{
					latAvg /= arraySize;
					longAvg /= arraySize;
				}

				if (arraySpot > 0 && first == false)
				{
					latAvg /= arraySpot;
					longAvg /= arraySpot;
				}

				if (arraySpot == 0 && first == false)
				{
					latAvg /= arraySize;
					longAvg /= arraySize;
					first = true;
				}

				String latAvg_str = String(latAvg , 6); // latitude location is stored in a string
				String lngAvg_str = String(longAvg , 6); //longitude location is stored in a string

				lat_str = String(latitude , 6); // latitude location is stored in a string
				lng_str = String(longitude , 6); // longitude location is stored in a string

				String latlongAvg = latAvg_str+", "+lngAvg_str;
				Serial.println(latlongAvg);

				// Geofencing
				dist = HaverSine(gLatitude,gLongitude,latAvg,longAvg);
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

				// Send Data
				if (millis()>=ulang+4000) 
				{
					char Time[] = "00:00:00";
					char Date[] = "2000/00/00";

					// add the offset to get local time UTC+7 (7x3600 >> 1 jam = 3600s))
					adjustTime(25200);
					// set current UTC time
					setTime(Hour, Minute, Second, Day, Month, Year);

					// update time array
					Time[0] = hour()   / 10 + '0';
					Time[1] = hour()   % 10 + '0';
					Time[3] = minute() / 10 + '0';
					Time[4] = minute() % 10 + '0';
					Time[6] = second() / 10 + '0';
					Time[7] = second() % 10 + '0';
					
					// update date array
					Date[2] = (year()  / 10) % 10 + '0';
					Date[3] =  year()  % 10 + '0';
					Date[5] =  month() / 10 + '0';
					Date[6] =  month() % 10 + '0';
					Date[8] =  day()   / 10 + '0';
					Date[9] =  day()   % 10 + '0';

					Firebase.setInt(fbdo, fbref+"/battery", batteryPercentage);
					Firebase.setFloat(fbdo, fbref+"/bpm", beat);

					if (htrig != htrigh)
					{
						Firebase.setBool(fbdo, fbref+"/htrig", htrig);
						htrigh = htrig;
					}

					// trig = 1 >> diluar geofence
					if (gtrig==true)
					{
						String logs = fbref+"/logs/"+Date+"/"+Time;
						Firebase.setString(fbdo, fbref+"/latitude, longitude", latlongAvg);
						Firebase.setString(fbdo, logs, latlongAvg);
						Firebase.setString(fbdo, fbref+"/jarak",jarak);
						Firebase.setBool(fbdo, fbref+"/gtrig", gtrig);
					}
					if (gtrig==false)
					{
						Firebase.setString(fbdo, fbref+"/latitude, longitude", latlongAvg);
						Firebase.setString(fbdo, fbref+"/jarak",jarak);
						Firebase.setBool(fbdo, fbref+"/gtrig", gtrig);
					}
					ulang = millis();
				}
			}			
		} 
	}

	if (WiFi.status() != WL_CONNECTED)// if wifi not connected
	{
		Serial.println("WiFi >> Disconnected");
		while (1);
	}	    
}