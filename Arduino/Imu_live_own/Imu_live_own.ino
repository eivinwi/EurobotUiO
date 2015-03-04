/*****************************************************************
Hardware setup: This library supports communicating with the
LSM9DS0 over either I2C or SPI. If you're using I2C, these are
the only connections that need to be made:
    LSM9DS0 --------- Arduino
     SCL ---------- SCL (A5 on older 'Duinos')
     SDA ---------- SDA (A4 on older 'Duinos')
     VDD ------------- 3.3V
     GND ------------- GND
(CSG, CSXM, SDOG, and SDOXM should all be pulled high jumpers on 
  the breakout board will do this for you.)

If you're using SPI, here is an example hardware setup:
    LSM9DS0 --------- Arduino
          CSG -------------- 9
          CSXM ------------- 10
          SDOG ------------- 12
          SDOXM ------------ 12 (tied to SDOG)
          SCL -------------- 13
          SDA -------------- 11
          VDD -------------- 3.3V
          GND -------------- GND
Development environment specifics:
    IDE: Arduino 1.0.5
    Hardware Platform: Arduino Pro 3.3V/8MHz
    LSM9DS0 Breakout Version: 1.0
*****************************************************************/

// The SFE_LSM9DS0 requires both the SPI and Wire libraries.
// Unfortunately, you'll need to include both in the Arduino
// sketch, before including the SFE_LSM9DS0 library.
#include <SPI.h> // Included for SFE_LSM9DS0 library
#include <Wire.h>
#include <SFE_LSM9DS0.h>

///////////////////////
// Example I2C Setup //
///////////////////////
// Comment out this section if you're using SPI
// SDO_XM and SDO_G are both grounded, so our addresses are:
#define LSM9DS0_XM  0x1D // Would be 0x1E if SDO_XM is LOW
#define LSM9DS0_G   0x6B // Would be 0x6A if SDO_G is LOW
// Create an instance of the LSM9DS0 library called `dof` the
// parameters for this constructor are:
// [SPI or I2C Mode declaration],[gyro I2C address],[xm I2C add.]
LSM9DS0 dof(MODE_I2C, LSM9DS0_G, LSM9DS0_XM);

///////////////////////
// Example SPI Setup //
///////////////////////
/* // Uncomment this section if you're using SPI
#define LSM9DS0_CSG  9  // CSG connected to Arduino pin 9
#define LSM9DS0_CSXM 10 // CSXM connected to Arduino pin 10
LSM9DS0 dof(MODE_SPI, LSM9DS0_CSG, LSM9DS0_CSXM);
*/

// Do you want to print calculated values or raw ADC ticks read
// from the sensor? Comment out ONE of the two #defines below
// to pick:
#define PRINT_CALCULATED
//#define PRINT_RAW

#define PRINT_SPEED 200 // 500 ms between prints


void setup() {
  Serial.begin(38400);
  // Use the begin() function to initialize the LSM9DS0 library.
  // You can either call it with no parameters (the easy way):
  uint16_t status = dof.begin();
  // begin() returns a 16-bit value which includes both the gyro 
  // and accelerometers WHO_AM_I response. You can check this to
  // make sure communication was successful.
/*
  Serial.print("LSM9DS0: 0x");
  Serial.println(status, HEX);
  Serial.println("Should be 0x49D4");

  Serial.print("Scale_values: ");
  Serial.print(gRes);
  Serial.print(aRes);
  Serial.println(mRes);*/
}


void loop() {
  Serial.print("L ");
  printGyro();  // Print "G gx gy gz"
  printAccel(); // Print "A ax ay az"
  printMag();   // Print "M mx my mz"
  Serial.println();

  delay(PRINT_SPEED);
}

void printGyro(){
  // update IMU's gx, gy and gz values
  dof.readGyro();
  Serial.print(dof.gx);
  Serial.print(" ");
  Serial.print(dof.gy);
  Serial.print(" ");
  Serial.print(dof.gz);
  Serial.print(" ");
}

void printAccel(){
  // update IMU's ax, ay and az values
  dof.readAccel();
  Serial.print(dof.ax);
  Serial.print(" ");
  Serial.print(dof.ay);
  Serial.print(" ");
  Serial.print(dof.az);
  Serial.print(" ");
}

void printMag() {
  // update IMU's mx, my and mz values
  dof.readMag();
  Serial.print(dof.mx);
  Serial.print(" ");
  Serial.print(dof.my);
  Serial.print(" ");
  Serial.print(dof.mz);
}