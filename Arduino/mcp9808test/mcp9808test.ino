#include <Wire.h>
#include <Adafruit_MCP9808.h>
// Create the MCP9808 temperature sensor object
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();


int buttonPin = 6; 


void setup() {
  Serial.begin (115200);
  pinMode (buttonPin, INPUT);

  // Make sure the sensor is found, you can also pass in a different i2c
  // address with tempsensor.begin(0x19) for example
  if (!tempsensor.begin()) {
    Serial.println("Couldn't find MCP9808!");
    while (1);
  }
   delay(2000);
}

int currState;
int prevState = HIGH;

void loop() {
  // Read and print out the temperature, then convert to *F
  float c = tempsensor.readTempC();
  float f = c * 9.0 / 5.0 + 32;
  Serial.print("Temp: "); Serial.print(c); Serial.print("*C\t"); 
  Serial.print(f); Serial.println("*F");
  currState = digitalRead(buttonPin);
  Serial.print("new state :");
  Serial.println(currState); 
   delay(500);
}


