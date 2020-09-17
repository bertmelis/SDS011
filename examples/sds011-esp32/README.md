The ESP32 [has three serial ports](https://quadmeup.com/arduino-esp32-and-3-hardware-serial-ports/). This example shows how to connect the SDS011 to a port that is different from the default one, so that we can still print to the default serial port and see what's going on:

```command
$ platformio run --target upload && platformio device monitor --port /dev/cu.SLAB_USBtoUART --baud 115200
```
