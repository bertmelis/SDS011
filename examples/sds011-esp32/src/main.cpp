#include <HardwareSerial.h>
#include <SDS011.h>

HardwareSerial usbSerial(0);
HardwareSerial sds011Serial(1);
SDS011 sds011;

void setup() {
  usbSerial.begin(115200);
  sds011.setup(&sds011Serial, 14, 12);  // Rx on GPIO14; Tx on GPIO12

  sds011.onData([](float pm25Value, float pm10Value) {
    usbSerial.println("Data arrived: PM2.5 = " + String(pm25Value, 1) + " μg/m³; PM10 = " + String(pm10Value, 1) + " μg/m³");
  });

  sds011.onResponse([](uint8_t command, uint8_t set, uint8_t result) {
    usbSerial.println("Response to command 0x" + String(command, HEX) + " received: 0x" + String(result, HEX));
  });

  sds011.onError([](int8_t error) {
    usbSerial.println("Error occurred: 0x" + String(error, HEX));
  });

  sds011.setWorkingPeriod(1);
  usbSerial.println("Data should appear on the default serial port in less than a minute");
}

void loop() {
  sds011.loop();
}
