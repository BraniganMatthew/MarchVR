/* Uses parts from the Adafruit LSM6DS3TR-C demo and SerialToSerialBT demo for the Adafruit ESP32 Feather*/

/* This program is a simple implementation of displaying IMU accelerometer and gyroscope to another device using Bluetooth Classic Serial*/
/* Created by: Matthew Branigan */
/* Modified on: 3/28/2023 */

#include "BluetoothSerial.h"
#include <Adafruit_LSM6DS3TRC.h>
#include <bits/stdc++.h>
#include <algorithm>
#include <Ewma.h>
#include <movingAvg.h>
#define WINDOW_SIZE 2.0
int ind = 0;
// float valx = 0.0;
// float valy = 0.0;
// float valz = 0.0;
// float sumx = 0.0;
// float sumy = 0.0;
// float sumz = 0.0;
// float readx[WINDOW_SIZE];
// float ready[WINDOW_SIZE];
// float readz[WINDOW_SIZE];
// float averagex = 0.0;
// float averagey = 0.0;
// float averagez = 0.0;
Ewma Filter1(0.15);

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
String device_name = "MarchVR Best Team 2";
Adafruit_LSM6DS3TRC lsm6ds3trc;
float lower = 8, upper = 15;
int stepCount = 0;
bool above = false, below = false;
// float xacc[1000] = {0};
// float yacc[1000] = {0};
// float zacc[1000] = {0};
// float mag[1000] = {0};
// float threshold = 0.0;

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
  //Serial.println(calibrate(xacc,yacc,zacc));

}

void loop() 
{
  // if (!SerialBT.connected()){
  //   Serial.println("Not Connected to Device!");
  //   delay(100);
  //   return;
  // }
    
  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);
  float accx = accel.acceleration.x;
  float fx = Filter1.filter(accx);
  float accy = accel.acceleration.y;
  float fy = Filter1.filter(accy);
  float accz = accel.acceleration.z;
  float fz = Filter1.filter(accz);
  Serial.print("Accel_X:");
  Serial.print(accx);
  Serial.print(",");
  Serial.print("Accel_Y:");
  Serial.print(accy);
  Serial.print(",");
  Serial.print("Accel_Z:");
  Serial.print(accz);
  Serial.print(",");
  Serial.print("filterx:");
  Serial.print(fx);
  Serial.print(",");
  Serial.print("filtery:");
  Serial.print(fy);
  Serial.print(",");
  Serial.print("filterz:");
  Serial.println(fz);

  delay(100);

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

  //sensors_event_t accel, gyro, temp;
  //lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  
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

  // if (accel.acceleration.z < 8.0f){
  //   below = true;
  // } else if (accel.acceleration.z > 15.0f){
  //   above = true;
  // } else {
  //   below = false;
  //   above = false;
  // }
  // if (above && below){
  //   below = false;
  //   above = false;
  //   stepCount++;
  //   Serial.print("STEP COUNT: ");
  //   Serial.println(stepCount);
  // }
  //Serial.println(calibrate(xacc,yacc,zacc));
  // for (int k = 0; k < 100; k++){
  //   lsm6ds3trc.getEvent(&accel, &gyro, &temp);
  //   Serial.print("Accel_X:");
  //   Serial.print(accel.acceleration.x);
  //   Serial.print(",");
  //   Serial.print("Accel_Y:");
  //   Serial.print(accel.acceleration.y);
  //   Serial.print(",");
  //   Serial.print("Accel_Z:");
  //   Serial.print(accel.acceleration.z);
  //   Serial.print(",");

  //   Serial.print("Gyro_X:");
  //   Serial.print(gyro.gyro.x);
  //   Serial.print(",");
  //   Serial.print("Gyro_Y:");
  //   Serial.print(gyro.gyro.y);
  //   Serial.print(",");
  //   Serial.print("Gyro_Z:");
  //   Serial.println(gyro.gyro.z);

  // }
  

}
// float calibrate(float xacc[1000],float yacc[1000],float zacc[1000])
// {
//   sensors_event_t accel, gyro, temp;
//   lsm6ds3trc.getEvent(&accel, &gyro, &temp);
//   float maxVal = 0.0f, minVal = 100.0f;
//   for (int i = 0; i < 1000; i++){
//     xacc[i] = accel.acceleration.x;
//     delay(5);
//     yacc[i] = accel.acceleration.y;
//     delay(5);
//     zacc[i] = accel.acceleration.z;
//     delay(5);
//     mag[i] = sqrt(pow(xacc[i],2) + pow(yacc[i],2) + pow(zacc[i],2));
//     maxVal = max(maxVal, mag[i]);
//     minVal = min(minVal, mag[i]);
//   }
//   float threshold = (maxVal - minVal)/2.0;
//   return threshold;
// }