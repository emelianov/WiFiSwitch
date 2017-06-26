# WiFiSwitch

## Resources used

* [Arduino](https://github.com/arduino/Arduino)
* [ESP8266 core for Arduino](https://github.com/esp8266/Arduino)
* [Run 2017.4](https://github.com/emelianov/Run)
* [TinyXML](https://github.com/adafruit/TinyXML)
* [A fork of Jeelab's fantastic RTC library](https://github.com/adafruit/RTClib)
* [ESP8266 WiFi Connection manager with web captive portal](https://github.com/tzapu/WiFiManager)

### Release notes

* To reset network configuration and activate open Access Point mode Pull Up D8 for 3 seconds. Warning! Socket switching operations are suspended while configuration mode is active

* Default IP in Access Point mode is 192.168.4.1

* Set IP address field to blank to use DHCP settings

* After configuration click save and perform ESP reset to activate settings

* Current configuration settings is stored in `config.xml` file

* Firmware stores current state in `state.xml` file. File contains all switching defined but not all of them corresponding actial state an used.
Actualy only schedule settings, feed reaction and group membership saved as it changed and to be restored on power on.

* State changes write operations are chached in RAM and flushed to file system only after 5 seconds after last modification.

* Wave function time set looks line hours:minutes set but really it's interpretated as minutes:seconds

* Sockets 1-4 affected by Wave function