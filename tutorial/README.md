# Programming ESP8266
## Schematic
![ESP8266 Overview!](https://www.mikrocontroller-elektronik.de/wp-content/uploads/2017/02/ESP12E-Pinbelegung-1-768x537.png)

## Documentation
### Software
* [ESP8266 Wifi library](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html)

### Hardware
* [F3 Evo Soldering](https://www.instructables.com/id/How-to-Solder-the-F3-EVO-Brushed-Flight-Controller/)

## Setting up Development Environment
* Download and install Arduino softwarefrom https://www.arduino.cc/en/Main/Software
* Add additional Boards Manager URL (File->Preferences->Settings): http://arduino.esp8266.com/stable/package_esp8266com_index.json
* Install Wifi Module (Tools -> Board -> Boards Manager): Search for "esp8266"
* Install WebSockets library (Tools -> Manage Library)
* Change Settings (Tools)
  * Baud: 115200 (921600 works as well but not as reliable as 115200)
  * Board: SparkFun ESP 8266 Thing
  * Port: dev/ttyUSB0 (or the corresponding COM port)

## Flashing the program
* Set ESP to flash mode by setting GPIO0 to GND
* Power on ESP
* Verify and upload program
  * [Blinky](https://github.com/harakirison/hAxOr-5chOoL-drOn3/tree/master/src/blinky) (test program with flashing led light)
  * [Drone HTML5 controller](https://github.com/harakirison/hAxOr-5chOoL-drOn3/tree/master/src/wifippm)

## Settings in BetaFlight
* Go to Receiver settings
* Make the following settings 
  * Channel Map value: RTAE1234
  * Stick Low Threshold: 1020
  * Stick Center: 1496
  * Stick High Threshold: 1980
  * RC Deadband: 5
  * Yaw Deadband: 5
