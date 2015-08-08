// FILE: sketch_juin18c.ino
// AUTHOR: Leonard Stalder
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <printf.h>
#include "MQ135.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include "DHT.h"

#define MQ135_PIN 0 // what pin we are connected for gaz sensor
#define DHTPIN 5 // what pin we're connected to for temperature
#define NODEID 2 // unique node id
#define DHTTYPE DHT22   // type of temperature sensor DHT 22 OR 11 
#define PIN_ANALOG_IN A4 // Define hardware connections for sound detector
#define analogPinForRV    A1   // change to pins you the analog pins are using
#define analogPinForTMP   A2

DHT dht(DHTPIN, DHTTYPE); //temperature/humidity sensor object
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //lux sensor object
MQ135 gasSensor = MQ135(MQ135_PIN); //gas sensor object

const float zeroWindAdjustment =  .065; // negative numbers yield smaller wind speeds and vice versa.

int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor 
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;

// messages structure
struct payload{
unsigned co2:16;
unsigned lux:16;
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
  dht.begin();
  tsl.begin();
  tsl.enableAutoRange(true); 
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  radio.begin();
  radio.setAutoAck(1,1);
  radio.setPayloadSize(8);
  radio.setChannel(90);
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.openWritingPipe(pipe_writing);
  radio.openReadingPipe(1,pipe_reading); 

}

void loop() {
  //radio.printDetails();
  // Read temperature as Celsius
  delay(50);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  payload.temp= (int)(t*10); 
  payload.hum= (int) (h*10);
  payload.id =NODEID;
  sensors_event_t event;
  tsl.getEvent(&event);
  float ppm = gasSensor.getRZero();
  payload.co2 = ppm;
  payload.lux= (int) event.light;

  // these are all derived from regressions from raw data as such they depend on a lot of experimental factors
  // such as accuracy of temp sensors, and voltage at the actual wind sensor, (wire losses) which were unaccouted for.
  TMP_Therm_ADunits = analogRead(analogPinForTMP);
  RV_Wind_ADunits = analogRead(analogPinForRV);
  RV_Wind_Volts = (RV_Wind_ADunits *  0.0048828125);
  TempCtimes100 = (0.005 *((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits)) - (16.862 * (float)TMP_Therm_ADunits) + 9075.4;  
  zeroWind_ADunits = -0.0006*((float)TMP_Therm_ADunits * (float)TMP_Therm_ADunits) + 1.0727 * (float)TMP_Therm_ADunits + 47.172;  //  13.0C  553  482.39
  zeroWind_volts = (zeroWind_ADunits * 0.0048828125) - zeroWindAdjustment;  
  WindSpeed_MPH =  pow(((RV_Wind_Volts - zeroWind_volts) /.2300) , 2.7265);    
  
  int sound_value = analogRead(PIN_ANALOG_IN); 
  
  Serial.print("Illuminance: ");
  Serial.print(event.light);
  Serial.print(" lx\tCO2: "); 
  Serial.print(ppm);
  Serial.print(" ppm\tHumidity: "); 
  Serial.print(h);
  Serial.print(" %\tTemperature: "); 
  Serial.print(t); 
  Serial.print("Celcius\tvalue analog sound : ");
  Serial.print(sound_value);
  Serial.print("   WindSpeed MPH ");
  Serial.println((float)WindSpeed_MPH);   
  radio.powerUp();
  bool ok = radio.write( &payload, sizeof(payload));
  if (ok){
     Serial.println("ok...");
     }
  else{
     Serial.println("failed\n\r");
     }
  radio.powerDown();
  delay(1000);
}
