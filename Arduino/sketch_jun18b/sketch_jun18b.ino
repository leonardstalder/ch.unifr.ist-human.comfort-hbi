#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <Wire.h>
#include "DHT.h"
#include <printf.h>

// FILE: dht_test.ino
// AUTHOR: Leonard Stalder
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino

#define DHTPIN 5 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// messages structure
struct payload{
unsigned hum:12;
unsigned temp:12;
unsigned id:8;
} payload;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe_writing = 0xF0F0F0F0E1LL;
const uint64_t pipe_reading = 0xF0F0F0F0D2LL;

void setup() {
  Serial.begin(9600); 
  printf_begin();
  dht.begin();
  payload.id=0x00; 
  radio.begin();
  radio.setAutoAck(1,1);
  radio.setPayloadSize(4);
  radio.setChannel(90);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  // opent writing pipe to 
  radio.openWritingPipe(pipe_writing);
  // open reading Pipe for ack. 
  radio.openReadingPipe(1,pipe_reading); 
  
  delay(3000);
  radio.printDetails();
}

void loop() {
  //radio.printDetails();
  // Read temperature as Celsius
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  payload.id++;
  payload.temp= (int)(t*10); 
  payload.hum= (int) (h*10); 
    Serial.print(" Humidity: "); 
  Serial.print(h);
  Serial.print(" %\tTemperature: "); 
  Serial.println(t);
  radio.powerUp();
  retry:
  bool ok = radio.write( &payload, sizeof(payload));
  if (ok){
     Serial.println("ok...");
     radio.powerDown();
     delay(1000);}
  else{
     Serial.println("failed, try again...\n\r");
     delay(2000);
     goto retry;}
   



}
