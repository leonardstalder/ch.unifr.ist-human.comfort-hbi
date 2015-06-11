//    FILE: dht_test.ino
//  AUTHOR: Leonard Stalder
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
// include libraries for luxometre
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
//include libraries for dht and gas sensor
#include "DHT.h"
#include "MQ135.h"
#define DHTPIN 5  
#define MQ135_PIN 1
#define DHTTYPE DHT11 
// dht object
DHT dht(DHTPIN, DHTTYPE);
// gassensor object initalisation
MQ135 gasSensor = MQ135(MQ135_PIN);
// lux object
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);

void setup()
{
  Serial.begin(9600); 
  dht.begin();
  tsl.begin();
  tsl.enableAutoRange(true); 
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
}

void loop()
{
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // Compute heat index
  float hi = dht.computeHeatIndex(f, h);
  // read CO2 Values
  float rzero = gasSensor.getRZero();
  float ppm = gasSensor.getPPM();
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);
  
  // DISPLAY DATA  
  Serial.print("Illuminance: ");
  Serial.print(event.light);
  Serial.print(" lx\tHumidity: "); 
  Serial.print(h);
  Serial.print(" %\tTemperature: "); 
  Serial.print(t);
  Serial.print(" *C\tHeat index: ");
  Serial.print(dht.convertFtoC(hi));
  Serial.print(" *C\tRzero: ");
  Serial.print(rzero,1);  
  Serial.print("\tPPM (CO2): ");
  Serial.print(ppm,1);
  Serial.print(",\n");
  delay(1000);
}
