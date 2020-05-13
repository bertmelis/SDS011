#include <Ticker.h>
#include <SDS011.h>

SoftwareSerial swSerial(D4, D3);

SDS011 sds011;

void setup() {
  Serial.begin(74880);

  sds011.setup(&swSerial);
  sds011.onData([](float pm25Value, float pm10Value) {
    Serial.printf("pm2.5: %.1f\n", pm25Value);
    Serial.printf("pm10:  %.1f\n", pm10Value);
  });
  sds011.onResponse([](uint8_t command, uint8_t set, uint8_t result){
    Serial.printf("command %d - set %d: %d\n", command, set, result);
  });
  sds011.onError([](int8_t error){
    Serial.printf("error: %d\n", error);
  });
  sds011.setWorkingPeriod(5);
}

void loop() {
  sds011.loop();
}
