/* Uses parts from the Adafruit LSM6DS3TR-C demo and SerialToSerialBT demo for the Adafruit ESP32 Feather*/

/* This program is a simple implementation of displaying IMU accelerometer and gyroscope to another device using Bluetooth Classic Serial*/
/* Created by: Matthew Branigan */
/* Modified on: 3/28/2023 */

#include "BluetoothSerial.h"
#include <Adafruit_LSM6DS3TRC.h>
#include <bits/stdc++.h>

/* Checking if Bluetooth is properly enabled on esp32 */
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

/* Making sure serial is posible on installed device */
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

// Global Variables
BluetoothSerial SerialBT;
String device_name = "MarchVR Best Team";
Adafruit_LSM6DS3TRC lsm6ds3trc;
float lower = 8, upper = 15;
int stepCount = 0;
bool above = false, below = false;
float accelAvg[3];
float accelMagAvg;
float variance = 0.1f;
float recentMags[50];
float recentMagAvg = 0.0f;
unsigned int nextElement = 0, up = 0;
bool haveStepped = false, nextStep = false;

void floatToStr(float num)
{
  String s = String(num, 3);
  char c[64];
  s.toCharArray(c, sizeof(c));  
  printToBTSerial(c);
}

void printToBTSerial(const char* str)
{
  size_t len = strlen(str);
  for (size_t i = 0; i < len; i++){
    SerialBT.write(str[i]);
  }
}

void setup() 
{
  // Setup serial debugging
  Serial.begin(115200);
  //Setup Serial Over BlueTooth
  SerialBT.begin(device_name);
  Serial.printf("Bluetooth is on!\n");

  //Find IMU
  if (!lsm6ds3trc.begin_I2C()) {
    Serial.println("Failed to find LSM6DS3TR-C chip. Please connect IMU to I2C connection.");
    while (1) {
      delay(10);
    }
  }

  Serial.println("LSM6DS3TR-C Found!");

  lsm6ds3trc.configInt1(false, false, true); // accelerometer DRDY on INT1
  lsm6ds3trc.configInt2(false, true, false); // gyro DRDY on INT2

  sensors_event_t accel, gyro, temp;
  //Testing average calibration
  Serial.println("Calibrating please wait...");
  for (unsigned int i = 0; i < 100; i++){
    lsm6ds3trc.getEvent(&accel, &gyro, &temp);
    accelAvg[0] += accel.acceleration.x;
    accelAvg[1] += accel.acceleration.y;
    accelAvg[2] += accel.acceleration.z;
  }
  unsigned int maxVal = 0;
  for (unsigned int i = 0; i < 3; i++){
    accelAvg[i] /= 100.0f;
    if (maxVal < accelAvg[i]){
      up = i;
      maxVal = accelAvg[i];
    }
  }
  accelMagAvg = sqrt(pow(accelAvg[0],2) + pow(accelAvg[1],2) + pow(accelAvg[2],2));
  
  Serial.println("Calibration done!");
  

}

void loop() 
{
  // if (!SerialBT.connected()){
  //   Serial.println("Not Connected to Device!");
  //   delay(100);
  //   return;
  // }
    
  // sensors_event_t accel, gyro, temp;
  // lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  // Serial.println("Sending Accel Data!");

  // //Sends accel data x,y,z in that order via bluetooth serial

  // SerialBT.write((uint8_t*)(&accel.acceleration.x), sizeof(float));
  // SerialBT.write((uint8_t*)(&accel.acceleration.y), sizeof(float));
  // SerialBT.write((uint8_t*)(&accel.acceleration.z), sizeof(float));
  

  // // floatToStr(accel.acceleration.x);
  // // floatToStr(accel.acceleration.y);
  // // floatToStr(accel.acceleration.z);

  // Serial.println("Sending Gyro Data!");

  // //Sends gyro data x,y,z in that order via bluetooth serial

  // SerialBT.write((uint8_t*)(&gyro.gyro.x), sizeof(float));
  // SerialBT.write((uint8_t*)(&gyro.gyro.y), sizeof(float));
  // SerialBT.write((uint8_t*)(&gyro.gyro.z), sizeof(float));

  // // floatToStr(gyro.gyro.x);
  // // floatToStr(gyro.gyro.y);
  // // floatToStr(gyro.gyro.z);

  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  
  // if(above){
  //   if (accel.acceleration.z < lower){
  //     // # send step
  //     // #stepCount+=1
  //     above = false;
  //     Serial.print("Up");
  //   }
  // }else{
  //   if (accel.acceleration.z > upper){
  //     //# send step
  //     stepCount++;
  //     above = true;
  //     Serial.print("STEP COUNT: ");
  //     Serial.println(stepCount);
  //   }
  // }

  // delay(500);

  float speed = 0.0f;

  // if (accel.acceleration.z < lower){
  //     // # send step
  //     // #stepCount+=1
  //     above = false;
  //     Serial.print("Up");
  //   }
  // }else{
  //   if (accel.acceleration.z > upper){
  //     //# send step
  //     stepCount++;
  //     above = true;
  // }

  

  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  if (nextElement >= 50) {nextElement = 0;}

  // recentMags[nextElement] = sqrt(pow(accel.acceleration.x,2) + pow(accel.acceleration.y,2) + pow(accel.acceleration.z,2));
  // nextElement++;
  // unsigned int usedElements = 0;
  // for (unsigned int i = 0; i < 100; i++){  
  //   if (recentMags[i] == 0)
  //     continue;
  //   recentMagAvg += recentMags[i];
  //   usedElements++;
  // }  

  // recentMagAvg /= usedElements;

  // if (recentMagAvg > accelMagAvg && !haveStepped) {
  //   haveStepped = true;
  //   stepCount++;    
  //   Serial.print("STEP COUNT: ");
  //   Serial.println(stepCount);
  // }

  // if (recentMagAvg < accelMagAvg && haveStepped){
  //   haveStepped = false;
  // }

  float currAccel[3] = {accel.acceleration.x, accel.acceleration.y, accel.acceleration.z};

  if (currAccel[up] < accelAvg[up]*(1-variance)){
    below = true;
  } else if (currAccel[up] > accelAvg[up]*(1+variance)){
    above = true;
  } else {
    below = false;
    above = false;
  }

  if (above && below){
    below = false;
    above = false;
    if (nextStep){
      stepCount++;
      nextStep = false;
      Serial.print("STEP COUNT: ");
      Serial.println(stepCount);
    } else {
      nextStep = true;
    }
    
  }

  
  
  //Serial.print("Accel_X:");
  //Serial.print(accel.acceleration.x);
  //Serial.print(",");
  //Serial.print("Accel_Y:");
  //Serial.print(accel.acceleration.y);
  //Serial.print(",");
  //Serial.print("Accel_Z:");
  //Serial.print(accel.acceleration.z);
  //Serial.print(",");

  //Serial.print("Gyro_X:");
  //Serial.print(gyro.gyro.x);
  //Serial.print(",");
  //Serial.print("Gyro_Y:");
  //Serial.print(gyro.gyro.y);
  //Serial.print(",");
  //Serial.print("Gyro_Z:");
  //Serial.print(gyro.gyro.z);
  //Serial.print("\n");

  // Serial.print("Speed:");
  // Serial.print(accel.acceleration.z*0.01);
  // Serial.print("\n");
  

}
