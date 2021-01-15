#include <Wire.h>
#include <AFE_connect.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;
AFE A; //sensor

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

float EMA_a_low = 0.1;    //initialization of EMA alpha
float EMA_a_high = 0.2;
 
int EMA_S_low = 0;        //initialization of EMA S
int EMA_S_high = 0;
 
int highpass = 0;
int bandpass = 0;
int sensorValue = 0;



void setup() {
  Serial.begin(9600);
  A.init();

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  pinMode(4, OUTPUT);



}

void loop() 

{ 
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  sensorValue = irValue ;            
  
  EMA_S_low = (EMA_a_low*sensorValue) + ((1-EMA_a_low)*EMA_S_low);
  EMA_S_high = (EMA_a_high*sensorValue) + ((1-EMA_a_high)*EMA_S_high);
   
  highpass = sensorValue - EMA_S_low;     //find the high-pass as before (for comparison)
  bandpass = EMA_S_high - EMA_S_low;      //find the band-pass

  Serial.print(bandpass);
//  Serial.print(",");
//  Serial.print(A.get_led2_val());
  Serial.println();

}
