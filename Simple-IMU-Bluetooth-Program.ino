/* Uses parts from the Adafruit LSM6DS3TR-C demo and SerialToSerialBT demo for the Adafruit ESP32 Feather*/

/* This program is a simple implementation of displaying IMU accelerometer and gyroscope to another device using Bluetooth Classic Serial*/
/* Created by: Matthew Branigan */

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

// Global Variables
BluetoothSerial SerialBT;
String device_name = "MarchVR Best Team";
Adafruit_LSM6DS3TRC lsm6ds3trc;

//https://stackoverflow.com/questions/2988791/converting-float-to-char
char* floatToStr(float num)
{
  char* buff = new char[64];
  int ret = snprintf(buff, sizeof(buff), "%f", num);
  if (ret == -1)
    return nullptr;
  else
    return buff;
}

void printToBTSerial(char* str)
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
    Serial.println("Failed to find LSM6DS3TR-C chip");
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
  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  Serial.println("Getting x accel");
  printToBTSerial("X-Acceleration: ");
  char* buff = floatToStr(accel.acceleration.x);
  if (buff){
    printToBTSerial(buff);
    SerialBT.write('\n');
    delete buff;    
  }

  Serial.println("Getting y accel");
  printToBTSerial("Y-Acceleration: ");
  buff = floatToStr(accel.acceleration.y);
  if (buff){
    printToBTSerial(buff);
    SerialBT.write('\n');
    delete buff;    
  }

  Serial.println("Getting z accel");
  printToBTSerial("Z-Acceleration: ");
  buff = floatToStr(accel.acceleration.z);
  if (buff){
    printToBTSerial(buff);
    SerialBT.write('\n');
    delete buff;    
  }

  SerialBT.write('\n');

  Serial.println("Getting X Gyro");
  printToBTSerial("X-Gyroscope: ");
  buff = floatToStr(gyro.gyro.x);
  if (buff){
    printToBTSerial(buff);
    SerialBT.write('\n');
    delete buff;
  }

  Serial.println("Getting Y Gyro");the A 
  printToBTSerial("Y-Gyroscope: ");
  buff = floatToStr(gyro.gyro.y);
  if (buff){
    printToBTSerial(buff);
    SerialBT.write('\n');
    delete buff;
  }

  Serial.println("Getting Z Gyro");
  printToBTSerial("Z-Gyroscope: ");
  buff = floatToStr(gyro.gyro.z);
  if (buff){
    printToBTSerial(buff);
    SerialBT.write('\n');
    delete buff;
  }

  SerialBT.write('\n');

  delay(1000);
}
