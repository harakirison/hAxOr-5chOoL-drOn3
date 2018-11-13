/*
 ESP12E-Test
 Led an dem ESP8266 Modul Typ ESP12E blinken lassen
 und Zahlenfolge über serielle Schnittstelle ausgeben

 Programm erprobt ab Arduino IDE 1.8.7
 Weitere Beispiele unter https://www.mikrocontroller-elektronik.de/
*/

#include <ESP8266WiFi.h>


#define LED 2 //GPIO2 - Led auf dem Modul selbst

int i;

void setup() {
 pinMode(LED, OUTPUT); // Port aus Ausgang schalten

 Serial.begin(115200);
 delay(10);
 
 Serial.println("");
 Serial.println("ESP12E Modul Test"); 
}


void loop() {
 
 digitalWrite(LED, LOW); //Led port ausschalten
 delay(1000); //1 Sek Pause
 digitalWrite(LED, HIGH); //Led port einschlaten
 delay(1000); 
 
 Serial.println(i); 
 i++;
}
