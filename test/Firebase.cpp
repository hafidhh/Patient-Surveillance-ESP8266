#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

const char* ssid = "ZTE-972ec0"; //ssid of your wifi
const char* password = "hf170798"; //password of your wifi

#define FIREBASE_HOST "skripsi-14e87.firebaseio.com"
#define FIREBASE_AUTH "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn"

FirebaseConfig dbconfig;
FirebaseAuth dbauth;
FirebaseData fbdo;
FirebaseData fbdo1;
String id, geofence;

void setup()
{
    dbconfig.api_key = "AIzaSyAnmeZxASLNzNDfzCKE2BV2hiJMHHfJk9M";
    dbconfig.host = "skripsi-14e87.firebaseio.com";
    dbauth.token.uid = "uy3sgMcGjgdXRARWBlJlAp24Za9yFCWZ40WxfArn";
    Serial.begin(115200);
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
    Serial.println(WiFi.macAddress());

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
}

void loop()
{
    Firebase.getString(fbdo, "/geofence");
    geofence = fbdo.stringData();
    Serial.println(geofence);
    Serial.println(fbdo.errorReason());
    delay(3000);
    Firebase.setString(fbdo, id+"/test", "test");
    delay(3000);
}