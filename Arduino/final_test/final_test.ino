// FILE: final_test.ino
// AUTHOR: Leonard Stalder
// PURPOSE: comfort box
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
#include <MFRC522.h>

#define RST_PIN		13		// pin reset for RFID
#define SS_PIN		4		// pin digital for RFID data
#define MQ135_PIN 0 // what pin we are connected for gaz sensor
#define DHTPIN 5 // what pin we're connected to for temperature
#define DHTTYPE DHT22   // type of temperature sensor DHT 22 OR 11 
#define PIN_ANALOG_IN A4 // Define hardware connections for sound detector
#define analogPinForRV    A1   // change to pins you the analog pins are using
#define analogPinForTMP   A2

#define NODEID 2 // unique node id

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance for RFID reader
DHT dht(DHTPIN, DHTTYPE); //temperature/humidity sensor object
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //lux sensor object
MQ135 gasSensor = MQ135(MQ135_PIN); //gas sensor object
RF24 radio(9,10); //Set up nRF24L01 radio on SPI bus plus pins 9 & 10
const uint64_t pipe_writing = 0xF0F0F0F0E1LL; // Radio pipe address to write messages.
const uint64_t pipe_reading = 0xF0F0F0F0D2LL; // Radio pipe address to receive messages.

// messages structure sended to gateway
struct payload{
unsigned co2:16;
unsigned lux:16;
unsigned hum:12;
unsigned temp:12;
unsigned id:8;
} payload;

const float zeroWindAdjustment =  .22; // Wind sensor calibaration near to zero = smaller value and vice-versa.
int TMP_Therm_ADunits;  //temp termistor value from wind sensor
float RV_Wind_ADunits;    //RV output from wind sensor 
float RV_Wind_Volts;
unsigned long lastMillis;
int TempCtimes100;
float zeroWind_ADunits;
float zeroWind_volts;
float WindSpeed_MPH;



void setup() {
  Serial.begin(9600); 
  while (!Serial);
  dht.begin();
  tsl.begin();
  tsl.enableAutoRange(true); 
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  // medium resolution and speed 
  
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();		// Init MFRC522
  ShowReaderDetails();	// Show details of PCD - MFRC522 Card Reader details
  mfrc522.PCD_AntennaOff();
  
  pinMode(6, OUTPUT);
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

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  payload.temp= (int)(t*10); 
  payload.hum= (int) (h*10);
  payload.id =NODEID;
  sensors_event_t event;
  tsl.getEvent(&event);
  float ppm = gasSensor.getPPM();
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
  Serial.print((float)WindSpeed_MPH);   

  delay(100);
  mfrc522.PCD_AntennaOn(); // set RFID reader to on
  delay(10);

Serial.print("   User Present :  ");
// Look for new cards
	if (  mfrc522.PICC_IsNewCardPresent()) {
                digitalWrite(6, HIGH);
                 if ( mfrc522.PICC_ReadCardSerial()) {
	          // Dump debug info about the card; PICC_HaltA() is automatically called

                 byte x = *(mfrc522.uid.uidByte);
Serial.println(x);
                  mfrc522.PCD_AntennaOff(); // set RFID reader to on
	      }
	
	}else{
          digitalWrite(6, LOW);
          Serial.println(" no ");
        }

  delay(300); 
  radio.powerUp();
  bool ok = radio.write( &payload, sizeof(payload));
  if (ok){
     Serial.println("ok..."); 
     }
  else{
     Serial.println("failed\n\r");
     }
  delay(10);  
  radio.powerDown();
   delay(1000);

}

void ShowReaderDetails() {
	// Get the MFRC522 software version
	byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
	Serial.print(F("MFRC522 Software Version: 0x"));
	Serial.print(v, HEX);
	if (v == 0x91)
		Serial.print(F(" = v1.0"));
	else if (v == 0x92)
		Serial.print(F(" = v2.0"));
	else
		Serial.print(F(" (unknown)"));
	Serial.println("");
	// When 0x00 or 0xFF is returned, communication probably failed
	if ((v == 0x00) || (v == 0xFF)) {
		Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
	}
}
