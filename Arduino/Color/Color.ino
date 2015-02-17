/*
An Arduino code example for interfacing with the 
HDJD-S822-QR999 Color Sensor.  Put an object in front of the
sensor and look at the serial monitor to see the values the sensor
is reading.  Scaling factors and gains may have to be adjusted
for your application.

by: Jordan McConnell
 SparkFun Electronics
 created on: 1/24/12
 license: OSHW 1.0, http://freedomdefined.org/OSHW
 
Connect the gain pins of the sensor to digital pins 7 - 12 (or ground).
Connect the led pin to digital 13.
Connect Vr to analog 0, Vg to analog 1, and Vb to analog 2.
*/

// Define pins
const int ledpin = 13;
const int GSR1 = 12;
const int GSR0 = 11;
const int GSG1 = 10;
const int GSG0 = 9;
const int GSB1 = 8;
const int GSB0 = 7;

int redpin = A0;
int greenpin = A1;
int bluepin = A2;

// Sensor read values
int red = 0;
int green = 0;
int blue = 0;

void setup() 
{
  Serial.begin(38400);

  pinMode(ledpin, OUTPUT);
  pinMode(GSR1, OUTPUT);
  pinMode(GSR0, OUTPUT);
  pinMode(GSG1, OUTPUT);
  pinMode(GSG0, OUTPUT);
  pinMode(GSB1, OUTPUT);
  pinMode(GSB0, OUTPUT);

  // Turn on the LED
  digitalWrite(ledpin, HIGH);
  
  // Set the gain of each sensor
  digitalWrite(GSR1, LOW);
  digitalWrite(GSR0, LOW);
  digitalWrite(GSG1, LOW);
  digitalWrite(GSG0, LOW);
  digitalWrite(GSB1, LOW);
  digitalWrite(GSB0, LOW);
}

void loop() 
{
  
  // Read sensors
  // On page 7 of the datasheet, there is a graph of the 
  // spectral responsivity of the chip.  Scaling factors were
  // selected based on this graph so that the gain of each 
  // color is closer to being equal
  red = analogRead(redpin) * 1.5;// * 10;
  green = analogRead(greenpin) * 1.8;// * 1.4;// * 14;
  blue = analogRead(bluepin) * 2.0;// * 1.7;// * 17;

//  int redV =   (( red    * 100)  / 16);
//  int greenV = (( green  * 130 ) / 22);
//  int blueV =  (( blue   * 200 ) / 25); 


  // Print values to the serial monitor
  Serial.print("Red: ");
  Serial.print(red, DEC);
  Serial.print("\t\tGreen: ");
  Serial.print(green, DEC);
  Serial.print("\tBlue: ");
  Serial.println(blue, DEC);

  delay(200);
}

