/* Uses parts from the Adafruit LSM6DS3TR-C demo and SerialToSerialBT demo for the Adafruit ESP32 Feather*/

/* This program is a simple implementation of displaying IMU accelerometer and gyroscope to another device using Bluetooth Classic Serial*/
/* Created by: Matthew Branigan */
/* Modified on: 3/28/2023 */

#include "BluetoothSerial.h"
#include <Adafruit_LSM6DS3TRC.h>

/* Checking if Bluetooth is properly enabled on esp32 */
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

/* Making sure serial is posible on installed device */
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

#define MAX_PREV_VALS 15
#define SMOOTHING_ALPHA 0.3f

// Global Variables
BluetoothSerial SerialBT;
String device_name = "MarchVR Best Team";
Adafruit_LSM6DS3TRC lsm6ds3trc;
int stepCount = 0;
bool above = false, below = false;
float accelAvg[3];
float accelMagAvg;
float variance = 0.15f;
float recentMags[MAX_PREV_VALS], recentTotal = 0.0f;
short accelPos[3];
float recentMagAvg = 0.0f;
unsigned int nextElement = 0, up = 0, recentSize = 0;
bool haveStepped = false, nextStep = false, resetTime = true;
unsigned long startTime;

float prevVal = 0.0f;


// Calculates average of array in O(1) using on spot recalculations
float getAccelAvg()
{
  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  float currAccel[3] = {accel.acceleration.x, accel.acceleration.y, accel.acceleration.z};

  recentTotal -= recentMags[nextElement];
  recentMags[nextElement] = currAccel[up];
  recentTotal += recentMags[nextElement];
  nextElement++;
  recentSize++;

  if (nextElement >= MAX_PREV_VALS)
    nextElement = 0;
  if (recentSize >= MAX_PREV_VALS)
    recentSize = MAX_PREV_VALS;  

  float avg;
  avg = recentTotal / recentSize;
  return avg;
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
    if (maxVal < abs(accelAvg[i])){
      up = i;
      maxVal = abs(accelAvg[i]);
    }
    if (accelAvg[i] < 0)
      accelPos[i] = -1;
    else
      accelPos[i] = 1;
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

  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  if (resetTime){
    startTime = millis();
    resetTime = false;
  }
  

  //float recentAvg = getAccelAvg();
  float currAccel[3] = {accel.acceleration.x, accel.acceleration.y, accel.acceleration.z};
  prevVal = SMOOTHING_ALPHA*prevVal + (1 - SMOOTHING_ALPHA)*currAccel[up];

  if (prevVal*accelPos[up] > accelPos[up]*accelAvg[up]*(1+variance)){
    above = true;
  } else if (prevVal*accelPos[up] < accelPos[up]*accelAvg[up]*(1-variance) && above){
    below = true;
  }// else {
  //  below = false;
  //  above = false;
  // }

  unsigned long endTime = millis();
  unsigned long timeDiff = endTime - startTime;

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

  if (timeDiff >= 10){
    //Calculate freqeuncy of steps
    
    //Reset step counter

    //Calculate speed between 0.0 and 1.0

    //Send to OpenVR drivers
  }  
  
  
  // Serial.print("Accel_X:");
  // Serial.print(accel.acceleration.x);
  // Serial.print(",");
  // Serial.print("Accel_Y:");
  // Serial.print(accel.acceleration.y);
  // Serial.print(",");
  // Serial.print("Accel_Z:");
  // Serial.print(accel.acceleration.z);
  // Serial.print(",");
  // Serial.print("AvgAccel:");
  // Serial.print(accelAvg[up]);
  // Serial.print(",");
  // Serial.print("Filtered:");
  // Serial.println(prevVal);
  // delay(100);

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
