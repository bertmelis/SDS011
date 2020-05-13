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

#include <SDS011.h>

#include <math.h>  // for pow()

SDS011::SDS011() :
  _serial(nullptr),
  _onData(nullptr),
  _onResponse(nullptr),
  _onError(nullptr),
  _rxBuff{0x00},
  _txBuff{0x00} {
}

SDS011::~SDS011() {
  // currently nothing to do
}

void SDS011::setup(HardwareSerial* serial) {
  _serial = serial;
  _serial->begin(9600, SERIAL_8N1);
}

void SDS011::setup(SoftwareSerial* serial) {
  _serial = serial;
  _serial->begin(9600, SWSERIAL_8N1);
}

void SDS011::onData(onDataHandler handler) {
  _onData = handler;
}

void SDS011::onResponse(onResponseHandler handler) {
  _onResponse = handler;
}

void SDS011::onError(onErrorHandler handler) {
  _onError = handler;
}

void SDS011::setReportMode(bool mode) {
  memset(_txBuff, 0, sizeof(_txBuff));
  _txBuff[0] = 0xAA;
  _txBuff[1] = 0xB4;
  _txBuff[2] = 0x02;
  _txBuff[3] = 0x01;  // set query mode
  _txBuff[4] = (mode) ? 0x01 : 0x00;  // mode
  _txBuff[15] = 0xFF;
  _txBuff[16] = 0xFF;
  _txBuff[17] = _getCRC(_txBuff);
  _txBuff[18] = 0xAB;
  _serial->write(_txBuff, sizeof(_txBuff));
}

void SDS011::setWorkingMode(bool mode) {
  memset(_txBuff, 0, sizeof(_txBuff));
  _txBuff[0] = 0xAA;
  _txBuff[1] = 0xB4;
  _txBuff[2] = 0x06;
  _txBuff[3] = 0x01;  // set working mode
  _txBuff[4] = (mode) ? 0x01 : 0x00;  // mode
  _txBuff[15] = 0xFF;
  _txBuff[16] = 0xFF;
  _txBuff[17] = _getCRC(_txBuff);
  _txBuff[18] = 0xAB;
  _serial->write(_txBuff, sizeof(_txBuff));
}

void SDS011::setWorkingPeriod(uint8_t period) {
  memset(_txBuff, 0, sizeof(_txBuff));
  _txBuff[0] = 0xAA;
  _txBuff[1] = 0xB4;
  _txBuff[2] = 0x08;
  _txBuff[3] = 0x01;  // set working mode
  _txBuff[4] = period;
  _txBuff[15] = 0xFF;
  _txBuff[16] = 0xFF;
  _txBuff[17] = _getCRC(_txBuff);
  _txBuff[18] = 0xAB;
  _serial->write(_txBuff, sizeof(_txBuff));
}

void SDS011::queryData() {
  memset(_txBuff, 0, sizeof(_txBuff));
  _txBuff[0] = 0xAA;
  _txBuff[1] = 0xB4;
  _txBuff[2] = 0x04;
  _txBuff[15] = 0xFF;
  _txBuff[16] = 0xFF;
  _txBuff[17] = _getCRC(_txBuff);
  _txBuff[18] = 0xAB;
  _serial->write(_txBuff, sizeof(_txBuff));
}

void SDS011::loop() {
  static uint8_t index = 0;
  if (_serial->available()) {  // fill rxBuffer
    _rxBuff[index] = _serial->read();
    if (_rxBuff[0] == 0xAA) {  // advance if HEAD is received
      ++index;
    }
  }
  if (_rxBuff[9] == 0xAB) {  // process when TAIL is received
    if (!_checkCRC(_rxBuff, _rxBuff[8])) {  // 1. check CRC
      if (_onError) _onError(-1);           // 2. signal error on fail
    } else {
      // 2. check message type
      if (_rxBuff[1] == 0xC5) {             // 3. response or data?
        if (_onResponse) _onResponse(_rxBuff[2], _rxBuff[3], _rxBuff[4]);     // 4. signal response
      } else {
        uint16_t pm2_5_raw = (_rxBuff[3] << 8) + _rxBuff[2];
        float pm2_5 = pm2_5_raw / 10.0;
        uint16_t pm10_raw = (_rxBuff[5] << 8) + _rxBuff[4];
        float pm10 = pm10_raw / 10.0;
        if (_onData) _onData(pm2_5, pm10);
      }
    }
    // 4. reset
    memset(_rxBuff, 0x00, sizeof(_rxBuff));
    index = 0;
  }
}

uint8_t SDS011::_getCRC(uint8_t buff[]) {
  uint8_t crc = 0;
  for (uint8_t i = 2; i < 17; ++i) {
    crc += buff[i];
  }
  return crc;
}

bool SDS011::_checkCRC(uint8_t buff[], uint8_t crc) {
  uint8_t crc_calc = 0;
  for (uint8_t i = 2; i < 8; ++i) {
    crc_calc += buff[i];
  }
  return crc == crc_calc;
}

float SDS011::correct(float pm2_5, float humidity, float factor, float exp) {
  if (humidity == 100 || factor == 0) return 0;
  float pm2_5_corr = pm2_5 / pow(factor * (100 - humidity), exp);
  return pm2_5_corr;
}

float SDS011::correct(float pm2_5, float humidity, Correction correction) {
  float factor = 0.0;
  float exponent = 0.0;
  switch (correction) {
    case AMSTERDAM:
      factor = 2.3;
      exponent = -0.38;
      break;
    case AMERSFOORT:
      factor = 3.4;
      exponent = -0.4;
      break;
    case VENLO:
      factor = 3.9;
      exponent = -0.43;
      break;
  }
  return correct(pm2_5, humidity, factor, exponent);
}
