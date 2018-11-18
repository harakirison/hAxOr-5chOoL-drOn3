/*
   Small webserver with HTML5 interface that creates PPM signals for flight controller.
   Original source: https://www.instructables.com/id/Wifi-PPM-no-App-Needed/

   Modified and optimized for hAxOr-5chOoL-drOn3
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#define DEBUG 1 // set debug mode when value is 1
#define CPU_MHZ 80
#define CHANNEL_NUMBER 8  //set the number of chanels
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define ON_STATE 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define SIG_PIN 5 //set PPM signal output pin on the arduino
#define AP_MODE 1 //set to WIFI Access Point or Client mode

/* Set these to your desired credentials. */
const char *ssid = "WifiPPM";
const char *password = "Wifi_PPM";

/* PPM[0]=Yaw, PPM[1}=Throttle, PPM[2]=Roll, PPM[3]=Pitch, PPM[4]=Button1, PPM[5]=Button2, PPM[5]=Button3, PPM[5]=Button4 */
int ppm[CHANNEL_NUMBER];

volatile unsigned long next;
volatile unsigned int ppm_running = 1;
unsigned int alivecount = 0;

extern const char index_html[];

ESP8266WebServer server (80);
WebSocketsServer webSocket = WebSocketsServer(81);

/**
   @brief Send PPM signal
*/
void inline ppmISR() {
  static boolean state = true;

  if (state) {  //start pulse
    digitalWrite(SIG_PIN, ON_STATE);
    next = next + (PULSE_LENGTH * CPU_MHZ);
    state = false;
    alivecount++;
  }
  else { //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;

    digitalWrite(SIG_PIN, !ON_STATE);
    state = true;

    if (cur_chan_numb >= CHANNEL_NUMBER) {
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;//
      next = next + ((FRAME_LENGTH - calc_rest) * CPU_MHZ);
      calc_rest = 0;
    }
    else {
      next = next + ((ppm[cur_chan_numb] - PULSE_LENGTH) * CPU_MHZ);
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }
  }
  timer0_write(next);
}

/**
   @brief Sets PPM output to default value
*/
void inline resetPPM() {
  ppm[0] = 1500; ppm[1] = 1100; ppm[2] = 1500; ppm[3] = 1500;
  ppm[4] = 1100; ppm[5] = 1100; ppm[6] = 1100; ppm[7] = 1100;
}

/**
    @brief Handle webserver call of root element
*/
void handleRoot() {
  if (ppm_running == 0) {
    noInterrupts();
    timer0_isr_init();
    timer0_attachInterrupt(ppmISR);
    next = ESP.getCycleCount() + 1000;
    timer0_write(next);
    resetPPM();
    ppm_running = 1;
    interrupts();
  }
  server.send_P(200, "text/html", index_html);
}

/**
   @brief Handle WebSocket events

   @param num
   @param type
   @param payload
   @param length
*/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: {
        resetPPM();
        Serial.println("Connection to WebSocket closed...");
      }
      break;
    case WStype_CONNECTED: {
        // send message to client
        webSocket.sendTXT(num, "Connected");
        resetPPM();
        Serial.println("Connection to WebSocket established...");
      }
      break;
    case WStype_BIN: {
        ppm[payload[0]] = (payload[1] << 8) + payload[2];
        alivecount = 0;

        if (DEBUG == 1)
        {
          Serial.print("Received command on channel: ");
          Serial.print(payload[0]);
          Serial.print(" : ");
          Serial.println((payload[1] << 8) + payload[2]);
        }

      }
      break;
  }
}

/**
   @brief Setup WIFI either with Access-Point mode or Client mode.

   Mode depends on directive AP_MODE:
    1 = Access-Point mode
    0 = Client mode
*/
void inline setupWiFi() {
  if (AP_MODE) {
    WiFi.softAP(ssid, password, 2);
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

/**
   @brief Main setup
*/
void setup() {
  // Setup serial port
  Serial.begin(115200);

  // Setup PPM output pin and set the PPM signal
  // pin to the default state (off)
  pinMode(SIG_PIN, OUTPUT);
  digitalWrite(SIG_PIN, !ON_STATE);

  // Setup WIFI (AP or Client mode)
  setupWiFi();

  // Setup HTTP webserver and register handlers
  server.on("/", handleRoot);
  server.begin();

  // Setup WebSocket server
  webSocket.onEvent(webSocketEvent);
  webSocket.begin();

  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(ppmISR);
  next = ESP.getCycleCount() + 1000;
  timer0_write(next);
  resetPPM();
  interrupts();
}

/**
   @brief Main loop
*/
void loop() {
  webSocket.loop();
  server.handleClient();

  if (alivecount > 1000) {
    resetPPM();
  }
  yield();
}
