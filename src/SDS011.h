/* SDS011

Copyright 2018 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#include <functional>

#include <Arduino.h>

typedef std::function<void(float, float)> onDataHandler;  // pm2.5, pm10
typedef std::function<void(uint8_t, uint8_t, uint8_t)> onResponseHandler;
typedef std::function<void(int)> onErrorHandler;

class SDS011 {
 public:
  SDS011();
  ~SDS011();
  void setup(HardwareSerial* serial);
  void onData(onDataHandler handler);
  void onResponse(onResponseHandler handler);
  void onError(onErrorHandler handler);  // -1: CRC error
  void setReportMode(bool mode);  // false: active, true: query
  void setWorkingMode(bool mode);  // false: sleep, true, work
  void setWorkingPeriod(uint8_t period);  // period in minutes: work 30 seconds, sleep period*60 - 30 seconds
  void queryData();
  // void queryData();
  void loop();

 private:
  uint8_t _getCRC(uint8_t buff[]);
  bool _checkCRC(uint8_t buff[], uint8_t crc);
  HardwareSerial* _serial;
  onDataHandler _onData;
  onResponseHandler _onResponse;
  onErrorHandler _onError;
  uint8_t _rxBuff[10];
  uint8_t _txBuff[19];

 public:
  float correct(float raw, float humidity, float factor, float exp);
  enum Correction {
    AMSTERDAM,
    AMERSFOORT,
    VENLO
  };
  float correct(float raw, float humidity, Correction correction);
};
