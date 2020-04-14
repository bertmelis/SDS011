# SDS011

[![Build Status](https://travis-ci.com/bertmelis/SDS011.svg?branch=master)](https://travis-ci.com/bertmelis/SDS011)

SDS011 particle matter sensor library for the Arduino framework for ESP8266.  
Not tested, but should also work on ESP32

This is yet another SDS011 library, this time completely non blocling. It does come with a `loop()`-method to poll the serial port.

## Installation

* For Arduino IDE: see [the Arduino Guide](https://www.arduino.cc/en/Guide/Libraries#toc4)
* For Platformio: see the [Platfomio guide](http://docs.platformio.org/en/latest/projectconf/section_env_library.html)

## Usage

You cannot use Serial on ESP8266 as there's only 1 full UART available which will be used by the sensor.

To do something useful you can combine this lib with MQTT (like [async-mqtt-client](https://github.com/marvinroger/async-mqtt-client)) as in the example below.

```C++
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <SDS011.h>

const char SSID[] = "My_WiFi";
const char PASS[] = "My_Pass";
const IPAddress BROKER = {192, 168, 1, 10};

SDS011 sds011;
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

bool connected = false;

void connectToWifi() {
  WiFi.begin(SSID, PASS);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  mqttClient.connect();
}

void onMqttConnected(bool sessionPresent) {
  connected = true;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  connected = false;
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void setup() {
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnected);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(BROKER, 1883);

  sds011.setup(&Serial);
  sds011.onData([](float pm25Value, float pm10Value) {
    if (connected) {
      mqttClient.publish("/SENSOR/PM2_5", 1, false, String(pm25Value, 1).c_str());
      mqttClient.publish("/SENSOR/PM10", 1, false, String(pm10Value, 1).c_str());
    }
  });
  sds011.onResponse([](uint8_t command, uint8_t set, uint8_t result){
    // command has been executed
  });
  sds011.onError([](int8_t error){
    // error happened
    // -1: CRC error
  });
  sds011.setWorkingPeriod(5);

  connectToWifi();
}

void loop() {
  sds011.loop();
}

```

## To Do

- adjust readings based on humidity
- possible timeout after sending a command
- implement missing commands
- multiple sensors (apparently the firmware supports this. However, they implemented hardware using 1-1 UART) ???

## Issues, improvements?

Please create a ticket or pull request.
