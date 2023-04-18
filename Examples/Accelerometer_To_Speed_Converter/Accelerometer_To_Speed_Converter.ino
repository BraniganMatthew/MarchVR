/* Uses parts from the Adafruit LSM6DS3TR-C demo and SerialToSerialBT demo for the Adafruit ESP32 Feather V2*/

/* This program is used for converting stepping data to speed data*/
/* Created by: Matthew Branigan */
/* Modified on: 4/13/2023 */

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

//VALUE DEFINES
#define SMOOTHING_ALPHA 0.3f

// Global Variables

  //Bluetooth Variables
  BluetoothSerial SerialBT;
  String device_name = "MarchVR Best Team";
  Adafruit_LSM6DS3TRC lsm6ds3trc;

  //Step Counter Variables
  int stepCount = 0;
  bool above = false, below = false, nextStep = false;
  float variance = 0.1f;

  //Calibration Variables
  float accelAvg[3];
  short accelPos[3];
  unsigned int up = 0;

  //Filtering and Frequency Variables
  float prevVal = 0.0f;
  unsigned long startTime;
  bool resetTime = true;



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
  //Gets 100 samples of idling
  for (unsigned int i = 0; i < 100; i++){
    lsm6ds3trc.getEvent(&accel, &gyro, &temp);
    accelAvg[0] += accel.acceleration.x;
    accelAvg[1] += accel.acceleration.y;
    accelAvg[2] += accel.acceleration.z;
  }
  unsigned int maxVal = 0;
  //Determines what side is up and if the accelerometer is flipped or not
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
  
  Serial.println("Calibration done!");
  

}

void loop() 
{
  //Waits until bluetooth is connected
  if (!SerialBT.connected()){
    Serial.println("Not Connected to Device!");
    delay(100);
    return;
  }

  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  //Starts timer for frequencyt check (might switch condition to stepCount == 0 for better frequnecy accuracy)
  if (resetTime){
    startTime = millis();
    resetTime = false;
  }
  
  //Applies IIR smoothing filter to acceleration
  float currAccel[3] = {accel.acceleration.x, accel.acceleration.y, accel.acceleration.z};
  prevVal = SMOOTHING_ALPHA*prevVal + (1 - SMOOTHING_ALPHA)*currAccel[up];

  //Detects user stepping
  if (prevVal*accelPos[up] > accelPos[up]*accelAvg[up]*(1+variance)){
    above = true;
  } else if (prevVal*accelPos[up] < accelPos[up]*accelAvg[up]*(1-variance) && above){
    below = true;
  }

  //Increaments step counter if user has done a full step
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

  //Calculates speed and sends it to OpenVR if it is high enough
  if (stepCount >= 2){
    //Stop Timer
    unsigned long endTime = millis();
    unsigned long timeDiff = endTime - startTime;


    //Calculate freqeuncy of steps
    float freq = stepCount/((float)timeDiff / 1000);
    float speed = 0.3871*freq - 0.1038;

    //Reset step counter and timer
    stepCount = 0;
    resetTime = true;

    //Calculate speed between 0.0 and 1.0
    Serial.println(freq);
    

    //Send to OpenVR drivers
    if (speed >= 0.1){
      Serial.println(speed);
      //send to vr driver (sending as char string)
      char buffer[5];
      dtostrf(speed, 4, 2, buffer);
      SerialBT.write((uint8_t*)&buffer, sizeof(buffer));
    } else {
      //too slow, not sending
    }
  }  
  

}