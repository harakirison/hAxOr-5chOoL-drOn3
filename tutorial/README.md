# Programming ESP8266
## Schematic
![ESP8266 Overview!](https://www.mikrocontroller-elektronik.de/wp-content/uploads/2017/02/ESP12E-Pinbelegung-1-768x537.png)

## Documentation
### Software
* [ESP8266 Wifi library](https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html)
* [PID/Rate Settings](http://www.koptertreff.de/f21-tipps-amp-tutorials/t79-tutorial-rates-in-betaflight-einstellen)

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
* Go to Receiver settings and make the following changes 
  * Channel Map value: RTAE1234
  * Stick Low Threshold: 1020
  * Stick Center: 1496
  * Stick High Threshold: 1980
  * RC Deadband: 5
  * Yaw Deadband: 5
* Go to Mode settings and make the following changes
  * ARM / Aux 1 / 1600-2000
  * ANGLE / Aux 2 / 900-1300
  * HORIZON / Aux 2 / 1600-2000
* Go to PID Tuning and make the following changes
  * RC Rate: 0,40 (Roll, Pitch, Yaw)
  * Super Rate: 0,20 (Roll, Pitch, Yaw)
  * RC Expo: 0,70 (Roll, Pitch, Yaw)
* Go to Configuration and make the following changes
  * Enable MOTOR_STOP

## CLI
set motor_pwm_rate = 32000
