/*
 * Small webserver with HTML5 interface that creates PPM signals for flight controller.
 * Original source: https://www.instructables.com/id/Wifi-PPM-no-App-Needed/
 * 
 * Modified and optimized for hAxOr-5chOoL-drOn3
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

#define CPU_MHZ 80
#define CHANNEL_NUMBER 8  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1100  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define ON_STATE 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define SIG_PIN 5 //set PPM signal output pin on the arduino
#define AP_MODE 1 //set to WIFI Access Point or Client mode

/* Set these to your desired credentials. */
const char *ssid = "WifiPPM";
const char *password = "Wifi_PPM";

volatile unsigned long next;
volatile unsigned int ppm_running = 1;
int ppm[CHANNEL_NUMBER];
unsigned int alivecount = 0;

const char* serverIndex = "<form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
extern const char index_html[];

ESP8266WebServer server (80);
WebSocketsServer webSocket = WebSocketsServer(81);

void inline ppmISR(void) {
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

/*
 * Handle webserver call of root element
 */
void handleRoot() {
  if (ppm_running == 0) {
    noInterrupts();
    timer0_isr_init();
    timer0_attachInterrupt(ppmISR);
    next = ESP.getCycleCount() + 1000;
    timer0_write(next);
    
    for (int i = 0; i < CHANNEL_NUMBER; i++) {
      ppm[i] = CHANNEL_DEFAULT_VALUE;
    }
    
    ppm_running = 1;
    interrupts();
  }
  server.send_P(200, "text/html", index_html);
}

/*
 * Handle WebSocket events
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED: {
        ppm[0] = 1100; ppm[1] = 1500; ppm[2] = 1500; ppm[3] = 1500;
        ppm[4] = 1100; ppm[5] = 1100; ppm[6] = 1100; ppm[7] = 1100;
        Serial.println("Disconnect");
      }
      break;
    case WStype_CONNECTED: {
        // send message to client
        webSocket.sendTXT(num, "Connected");
        ppm[0] = 1100; ppm[1] = 1500; ppm[2] = 1500; ppm[3] = 1500;
        ppm[4] = 1100; ppm[5] = 1100; ppm[6] = 1100; ppm[7] = 1100;
        Serial.println("Connect");
      }
      break;
    case WStype_BIN: {
        ppm[payload[0]] = (payload[1] << 8) + payload[2];
        alivecount = 0;
        Serial.println("Command");
      }
      break;
  }
}

/*
 * Setup WIFI either with Access-Point mode or Client mode.
 * Depends on directive AP_MODE:
 *  1 = Access-Point mode
 *  0 = Client mode
 */
void setupWiFi() {
  if(AP_MODE) {
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

/*
 * Main setup
 */
void setup() {
  // Setup Serial port
  Serial.begin(115200);


  // Setup PPM output
  pinMode(SIG_PIN, OUTPUT);
  digitalWrite(SIG_PIN, !ON_STATE); //set the PPM signal pin to the default state (off)

  // Setup WIFI (AP or Client)
  setupWiFi();

  // Register webserver handlers
  server.onNotFound(handleRoot);
  server.on("/", handleRoot);
  
  server.on("/update", HTTP_GET, []() {
    noInterrupts();
    timer0_detachInterrupt();
    ppm_running = 0;
    interrupts();
    delay(500);
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/upload", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        //         Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        //          Update.printError(Serial);
      }
    }
    else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        //         Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        //          Update.printError(Serial);
      }
      //       Serial.setDebugOutput(false);
    }
    yield();
  });

  server.begin();

  // Init WebSocket server
  webSocket.onEvent(webSocketEvent);
  webSocket.begin();

  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(ppmISR);
  next = ESP.getCycleCount() + 1000;
  timer0_write(next);
  for (int i = 0; i < CHANNEL_NUMBER; i++) {
    ppm[i] = CHANNEL_DEFAULT_VALUE;
  }
  interrupts();
}

/*
 * Main loop
 */
void loop() {
  webSocket.loop();
  server.handleClient();
  if (alivecount > 1000) {
    for (int i = 0; i < 4; i++) {
      ppm[i] = 900;
    }
    for (int i = 4; i < 8; i++) {
      ppm[i] = 1100;
    }
  }

  yield();
}
