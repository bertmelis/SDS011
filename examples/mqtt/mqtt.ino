#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <espMqttClient.h>
#include <SDS011.h>

const char SSID[] = "My_WiFi";
const char PASS[] = "My_Pass";
const IPAddress BROKER = {192, 168, 1, 10};

SDS011 sds011;
espMqttClient mqttClient;
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
  if (!mqttClient.connect()) {
    mqttReconnectTimer.once(2, connectToMqtt);
}

void onMqttConnected(bool sessionPresent) {
  connected = true;
}

void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason) {
  connected = false;
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onSensorData(float pm25Value, float pm10Value) {
  if (connected) {
    mqttClient.publish("/SENSOR/PM2_5", 1, false, String(pm25Value, 1).c_str());
    mqttClient.publish("/SENSOR/PM10", 1, false, String(pm10Value, 1).c_str());
  }
}

void onSensorResponse(uint8_t command, uint8_t set, uint8_t result) {
  // log to MQTT
}

void onSensorError(int8_t error){
  // error happened
  // -1: CRC error
}

void setup() {
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnected);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.setServer(BROKER, 1883);

  sds011.setup(&Serial);
  sds011.onData(onSensorData);
  sds011.onResponse(onSensorResponse);
  sds011.onError(onSensorResponse);
  sds011.setWorkingPeriod(5);

  connectToWifi();
}

void loop() {
  sds011.loop();
}
