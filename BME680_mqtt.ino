/***************************************************************************
  Sample sketch for using a bme680 air quality sensor with a ESP8266 and sending the result over MQTT once a minute. 

  Written by Erik Lemcke, combined out of the following samples:

  https://learn.adafruit.com/adafruit-bme680-humidity-temperature-barometic-pressure-voc-gas/arduino-wiring-test BME680 code & library by adafruit 
  https://www.home-assistant.io/blog/2015/10/11/measure-temperature-with-esp8266-and-report-to-mqtt/, home assistant mqqt by Paulus Schoutsen

***************************************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"

#define mqtt_server "YOUR MQTT SERVER IP"
#define mqtt_user "YOUR MQTT USERNAME"
#define mqtt_password "YOUR MQTT PASSWORD"

#define co2_topic "bme680/co2"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(115200);
  Serial.println("Startup");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  unsigned long previousMillis = millis();

 if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  delay(500);
}

long lastMsg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  //send a meaage every minute
  if (now - lastMsg > 60 * 1000) {
    lastMsg = now;

    if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

   Serial.print("Gas = ");
   Serial.print(bme.gas_resistance / 1000.0);
   Serial.println(" KOhms");
   
   //This is actually not CO2, but a resistance value. Needs to be converted into something that makes sense... 
   client.publish(co2_topic, String(bme.gas_resistance).c_str(), true);
    }
      
    
      
    
  }
