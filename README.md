# WeMos-Fermentation-Control
Control your fermentation temperature with web interface and WiFi

Parts:
- ESP8266 WeMos-D1R2
- DS18B20 temperature sensor + 4K7 resistor
- 2 channel relay module for Arduino

Pins:
- D4 temperature sensor data
- D5 relay HEAT
- D6 relay COOL

How to setup Wemos D1? Instructions can be found example from here: http://www.instructables.com/id/Programming-the-WeMos-Using-Arduino-SoftwareIDE/?ALLSTEPS

Remember install onewire library to your Arduino IDE https://github.com/adafruit/MAX31850_OneWire
