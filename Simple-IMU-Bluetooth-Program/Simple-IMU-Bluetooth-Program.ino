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

  //Serial.println("Sending Accel Data!");

  //Sends accel data x,y,z in that order via bluetooth serial
  SerialBT.write((uint8_t*)(&accel.acceleration.x), sizeof(float));
  SerialBT.write((uint8_t*)(&accel.acceleration.y), sizeof(float));
  SerialBT.write((uint8_t*)(&accel.acceleration.z), sizeof(float));


  Serial.write((uint8_t*)(&accel.acceleration.x), sizeof(float));
  Serial.write((uint8_t*)(&accel.acceleration.y), sizeof(float));
  Serial.write((uint8_t*)(&accel.acceleration.z), sizeof(float));

  // floatToStr(accel.acceleration.x);
  // floatToStr(accel.acceleration.y);
  // floatToStr(accel.acceleration.z);

  //Serial.println("Sending Gyro Data!");

  SerialBT.write((uint8_t*)(&accel.acceleration.x), sizeof(float));
  SerialBT.write((uint8_t*)(&accel.acceleration.y), sizeof(float));
  SerialBT.write((uint8_t*)(&accel.acceleration.z), sizeof(float));

  //Sends gyro data x,y,z in that order via bluetooth serial

  Serial.write((uint8_t*)(&gyro.gyro.x), sizeof(float));
  Serial.write((uint8_t*)(&gyro.gyro.y), sizeof(float));
  Serial.write((uint8_t*)(&gyro.gyro.z), sizeof(float));

  // floatToStr(gyro.gyro.x);
  // floatToStr(gyro.gyro.y);
  // floatToStr(gyro.gyro.z);

}
