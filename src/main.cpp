#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
// #include <NTPClient.h>
#include "Timer.h"
#include <Adafruit_BMP280.h>
#include <BH1750.h>

Adafruit_BMP280 bmp; // I2C

#include <Adafruit_Sensor.h>
#include <DHT.h>
// #include <DHT_U.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
// #define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT dht(DHTPIN, DHTTYPE);

BH1750 lightMeter(0x23);



//https://kit.alexgyver.ru/tutorials/fastbot/
// https://github.com/GyverLibs/FastBot
//esp_http_client.h
//https://dzen.ru/media/kotyara12/sozdanie-telegram-bota-63367831df5d464b372abee7?utm_referer=dzen.ru

//BH1750 lightMeter(0x23);

Timer timerPressure(600000);  //замер освещённости
Timer timerTemper(6000);  

const char* ssid = "ivanych";
const char* password = "stroykomitet";
const char* mqtt_server = "192.168.1.34";
// volatile bool ir_motion{};
// const int MOVPIN = 33; //датчик движения
// const int ONE_LED = 12; //управление включением света
// OneLed light_t(ONE_LED, 0, 20);
WiFiClient espClient;
PubSubClient client(espClient);
// const char* msg_motion= "balcony/motion";
// const char* msgLightOn= "balcony/lightOn";
// const char* msgLightOff="balcony/lightOff";
const char* msgTemper="balcony/temper";
const char* msgDHTemper="balcony/temperDHT";
const char* msgPressure="balcony/pressure";
const char* msgHumidity="balcony/humidity";
const char* msgLight="balcony/light";
String pressure{};
//-----------------------------------
void setup_wifi() {

  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  randomSeed(micros());
  delay(1000);
}
//-----------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    String clientId = "TTGO_Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      // client.subscribe(msgPressure, 0);
      // client.subscribe("aisle/ledBlinkG");
    } else {
      delay(1000);
    }
  }
}
//-----------------------------------
void setup_280(){
  Serial.println(F("BMP280 Forced Mode Test."));

  //if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_4000); /* Standby time. */

}
//-----------------------------------
void setup() {
  Serial.begin(115200);
  Wire.begin(); //SDA, SCL);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);
  //---------------------------
  reconnect();
  setup_280();

  dht.begin();

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }
}
//----------------------------------------
void controlWiFi(){
  if(WiFi.status() != WL_CONNECTED) {
    delay(10000);
    ESP.restart();
  }
}
//----------------------------------------
void loop() {
  controlWiFi();  //проверка подключения wifi
  
  client.loop();

  if(timerTemper.getTimer()){
    timerTemper.setTimer();
    if (bmp.takeForcedMeasurement()) {
      double temp = bmp.readTemperature();
      client.publish( msgTemper, String(temp).c_str() );
      if(timerPressure.getTimer()){
        timerPressure.setTimer();
          double pressure = bmp.readPressure() * 0.00750062;
          client.publish( msgPressure, String(pressure).c_str() );
      }
    }
    client.publish(msgDHTemper, String(dht.readTemperature()).c_str());
    client.publish(msgHumidity, String(dht.readHumidity()).c_str());

    if (lightMeter.measurementReady()) {
      float lux = lightMeter.readLightLevel();
      client.publish(msgLight, String(lux).c_str());
    }
  }
  
}