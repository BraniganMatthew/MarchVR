/* Uses parts from the Adafruit LSM6DS3TR-C demo, SerialToSerialBT demo, BLE_client demo, and BLE_server demo for the Adafruit ESP32 Feather V2*/

/* This program is used for converting stepping data to speed data*/
/* Created by: Matthew Branigan */
/* Modified on: 9/4/2023 */

//#include "BluetoothSerial.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <MadgwickAHRS.h>

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
  String device_name = "MarchVR Best Team";
  static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
  BLEServer* pServer = NULL;
  BLECharacteristic* pCharacteristic = NULL;

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

  //IMU Oridentation Filtering
  Adafruit_LSM6DS3TRC lsm6ds3trc;
  Madgwick filter;

//Classes
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);
        Serial.println();
        Serial.println("*********");
        stepCount++;
      }
    }
};




void setup() 
{
  // Setup serial debugging
  Serial.begin(115200);

  //Setup BLE
  BLEDevice::init("MarchVR BLE Server Tracker");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      charUUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());


  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

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

  filter.begin(25);

  Serial.println("Calibration done!");
  

}

void loop() 
{
  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  filter.updateIMU(gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);

  //Starts timer for frequency check (might switch condition to stepCount == 0 for better frequnecy accuracy)
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
      //SerialBT.write((uint8_t*)&buffer, sizeof(buffer));
      String tmp;
      tmp = tmp + "Yaw: " + filter.getYaw() + " Pitch: " + filter.getPitch() + " Roll: " + filter.getRoll();
      Serial.println(tmp);
    } else {
      //too slow, not sending
    }
  }  
  
  delay(40);
}