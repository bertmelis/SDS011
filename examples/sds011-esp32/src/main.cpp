#include <Arduino.h>
#include <SDS011.h>

SDS011 sds011;

void onSensoirData(float pm25Value, float pm10Value) {
  Serial.println("Data arrived: PM2.5 = " + String(pm25Value, 1) + " μg/m³; PM10 = " + String(pm10Value, 1) + " μg/m³");
};

void onSensorResponse(uint8_t command, uint8_t set, uint8_t result) {
  Serial.println("Response to command 0x" + String(command, HEX) + " received: 0x" + String(result, HEX));
};

void onSensorError(int8_t error) {
  Serial.println("Error occurred: 0x" + String(error, HEX));
};

void setup() {
  Serial.begin(115200);
  sds011.setup(&Serial1, 14, 12);  // Rx on GPIO14; Tx on GPIO12

  sds011.onData(onSensorData);

  sds011.onResponse(onSensorResponse);

  sds011.onError(onSensorError);

  sds011.setWorkingPeriod(1);
  Serial.println("Data should appear on the default serial port in less than a minute");
}

void loop() {
  sds011.loop();
}
