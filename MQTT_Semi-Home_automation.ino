#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SSID"
#define WLAN_PASS       "Password"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "USERNAME"
#define AIO_KEY         "AIO KEY"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup feeds for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish Temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temp");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
// Setup feeds subscribing to changes.
Adafruit_MQTT_Subscribe LED = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/LED");
Adafruit_MQTT_Subscribe AC = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/AC");
Adafruit_MQTT_Subscribe Switch = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Switch");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  Serial.begin(115200);
  dht.begin();
  delay(10);
pinMode(D2,OUTPUT);
pinMode(D3,OUTPUT);
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription 
  mqtt.subscribe(&LED);
 mqtt.subscribe(&AC);
 mqtt.subscribe(&Switch);
}

uint32_t x=0;

void loop() {
 
  MQTT_connect();
  float h = dht.readHumidity();
  
  float t = dht.readTemperature();// Read temperature as Celsius (the default)
  
  float f = dht.readTemperature(true);// Read temperature as Fahrenheit (isFahrenheit = true)
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &LED) {
      Serial.println((char *)LED.lastread);
      int state=atoi((char*)LED.lastread);
       digitalWrite(D2,state);
    }
   if (subscription == &AC) {
      Serial.println((char *)AC.lastread);
      int state1=atoi((char*)AC.lastread);
      digitalWrite(D3,state1);
    }
    if (subscription == &Switch) {
      Serial.println((char *)Switch.lastread);
      int value=atoi((char*)Switch.lastread);
      if(t>31 && value>0);
      {
        digitalWrite(D3,value);
        }
      
    }
  }

 // Now we can publish stuff!
  Serial.print(F("\nSending Temp and Humidity Values "));
 
  if (! Humidity.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! Temp.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  delay(1000);

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
