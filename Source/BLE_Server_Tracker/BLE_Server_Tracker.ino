/* Uses parts from the Adafruit LSM6DS3TR-C demo, SerialToSerialBT demo, BLE_client demo, and BLE_server demo for the Adafruit ESP32 Feather V2*/

/* This program is used for converting stepping data to speed data*/
/* Created by: Matthew Branigan */
/* Modified on: 9/29/2023 */

//#include "BluetoothSerial.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <MadgwickAHRS.h>
#include <Vector.h>

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
#define MAX_DATA_PARA   20

// Global Variables

  //Bluetooth Variables
  String device_name = "MarchVR BLE Server Tracker";
  #define serviceUUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
  #define charUUID_TRK "beb5483e-36e1-4688-b7f5-ea07361b26a8"
  #define charUUID_GUI "aad41096-f795-4b3b-83bb-858051e5e284"
  #define charUUID_DRV "22d7a034-791d-49f6-a84e-ef78ab2473ad"
  BLEServer* pServer = NULL;
  BLECharacteristic* pCharacteristic_TRK = NULL;
  BLECharacteristic* pCharacteristic_GUI = NULL;
  BLECharacteristic* pCharacteristic_DRV = NULL;
  bool isConnected = false, prevConnected = false, newBLERsp = false;
  String BLE_Wrt_Rsp;
  String BLE_RSP_ARRAY[MAX_DATA_PARA];

  //Step Counter Variables
  int stepCount = 0;
  bool above = false, below = false, nextStep = false;
  float variance = 0.1f;
  bool trackerStep1 = false, trackerStep2 = false;

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
      isConnected = true;
      BLEDevice::startAdvertising();
      Serial.println("Device Connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      isConnected = false;
    }
};

void splitString(String input, Vector<String>* output, char delim)
{
  uint8_t count = 0;
  //Vector<String> output;
  String tmp = "";

  for (uint8_t i = 0; i < input.length(); i++){
    
    // if (count >= MAX_DATA_PARA){
    //   Serial.println("INPUT TOO LARGE. QUITTING");
    //   break;
    // }
    char tmpChar = input.charAt(i);
    if (tmpChar == delim){
      count++;
      output->push_back(tmp.c_str());
      tmp = "";
      continue;
    }
    tmp += tmpChar;
    if (i+1 == input.length()){
      output->push_back(tmp.c_str());
    }
  }
}

uint8_t checkSumCalc(Vector<String>* input)
{
  uint8_t calcSum = 0;
  //Will XOR everything, execpt for the start and end byte
  for (uint8_t i = 1; i < input->size()-1; i++){
    for (uint8_t j = 0; j < input->at(i).length(); j++){
      calcSum = calcSum ^ (uint8_t)input->at(i).charAt(j);
    }
  }
  return calcSum;
}

bool assertCheckSum(Vector<String>* input)
{
  uint8_t sum = input->at(input->size()-1).toInt();
  uint8_t calcSum = checkSumCalc(input);
  if (sum == calcSum){
    //Serial.printf("They're equal!\n");
    return true;
  } else {
    Serial.printf("Recieved Sum: %d is different from calculated sum: %d\n", sum, calcSum);
    return false;
  }
}

//Calibrates Sensor on call
void calibrateTracker()
{


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

  filter.begin(26);

  Serial.println("Calibration done!");
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue().c_str();
      

      if (value.length() > 0) {
        BLE_Wrt_Rsp = "";
        BLE_Wrt_Rsp += value.c_str();

        newBLERsp = true;
        //Serial.println(value);
      }
    }
};

void bleResponse()
{
  Vector<String> splitVal;
  splitVal.setStorage(BLE_RSP_ARRAY);
  splitString(BLE_Wrt_Rsp, &splitVal, ';');

  // //Check if valid
  if (splitVal.at(0) != "%"){
    Serial.println("Invalid Starting Byte");
    return;
  }
  if (!assertCheckSum(&splitVal)){
    Serial.println("Failed Sum Check");
    return;
  }

  //Check where it is coming from
  String src = splitVal.at(1);

  //Check where it will be going to
  String dst = splitVal.at(2);

  //Check what command is calling
  String cmd = splitVal.at(3);

  //If to server
  if (dst == "TK1"){
    if (src == "TK2"){
      trackerStep2 = true;
    } else if (src == "GUI") {
      if (cmd == "CAL"){
        calibrateTracker();
      }

    } else if (src == "DRV") {
      
    }
  
  }
  
  //Else to tracker 1
   else if (dst == "TK2"){
    String tmp = BLE_Wrt_Rsp;
    const char* tmp_c = tmp.c_str();
    pCharacteristic_TRK->setValue((uint8_t*)tmp_c, tmp.length());
    pCharacteristic_TRK->notify();
  }

  //Else to GUI
  else if (dst == "GUI"){
    String tmp = BLE_Wrt_Rsp;
    const char* tmp_c = tmp.c_str();
    pCharacteristic_GUI->setValue((uint8_t*)tmp_c, tmp.length());
    pCharacteristic_GUI->notify();
  }

  //Else to Driver
  else if (dst == "DRV"){
    String tmp = BLE_Wrt_Rsp;
    const char* tmp_c = tmp.c_str();
    pCharacteristic_DRV->setValue((uint8_t*)tmp_c, tmp.length());
    pCharacteristic_DRV->notify();
  }
}

void setup() 
{
  // Setup serial debugging
  Serial.begin(115200);

  //Setup BLE
  //BLEDevice::init("MarchVR BLE Server Tracker");
  BLEDevice::init("ESP32");
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(serviceUUID);

  // Create a BLE Characteristic
  pCharacteristic_TRK = pService->createCharacteristic(
                      charUUID_TRK,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic_GUI = pService->createCharacteristic(
                    charUUID_GUI,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );
  
  pCharacteristic_DRV = pService->createCharacteristic(
                    charUUID_DRV,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );

  // Create a BLE Descriptor
  pCharacteristic_TRK->addDescriptor(new BLE2902());
  pCharacteristic_TRK->setCallbacks(new MyCallbacks());

  pCharacteristic_GUI->addDescriptor(new BLE2902());
  pCharacteristic_GUI->setCallbacks(new MyCallbacks());

  pCharacteristic_DRV->addDescriptor(new BLE2902());
  pCharacteristic_DRV->setCallbacks(new MyCallbacks());


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

  lsm6ds3trc.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm6ds3trc.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm6ds3trc.setAccelDataRate(LSM6DS_RATE_26_HZ);
  lsm6ds3trc.setGyroDataRate(LSM6DS_RATE_26_HZ);

  lsm6ds3trc.configInt1(false, false, true); // accelerometer DRDY on INT1
  lsm6ds3trc.configInt2(false, true, false); // gyro DRDY on INT2

  calibrateTracker();

}

void loop() 
{
  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);

  //Update the IMU with new data
  static unsigned long prevTime = millis();
  if (millis() - prevTime >= 39){
    prevTime = millis();
    filter.updateIMU(gyro.gyro.x, gyro.gyro.y, gyro.gyro.z, accel.acceleration.x, accel.acceleration.y, accel.acceleration.z);
  }
 
  //If a device disconnects
  if (!isConnected && prevConnected){
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    prevConnected = isConnected;
  }

  //If new device connects
  if (isConnected && !prevConnected){
    prevConnected = isConnected;
  }
  
  if (newBLERsp){
    bleResponse();
    newBLERsp = false;
  }

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
      //stepCount++;
      trackerStep1 = true;
      nextStep = false;
      Serial.println("TRACKER STEP 1");
      //Serial.println(stepCount);
    } else {
      nextStep = true;
    }
    
  }

  //Calculates speed and sends it to OpenVR if it is high enough
  if (trackerStep1 && trackerStep2){
    //Stop Timer
    unsigned long endTime = millis();
    unsigned long timeDiff = endTime - startTime;


    //Calculate freqeuncy of steps
    float freq = 2/((float)timeDiff / 1000);
    float speed = 0.3871*freq - 0.1038;

    //Reset step counter and timer
    stepCount = 0;
    resetTime = true;
    trackerStep1 = false;
    trackerStep2 = false;

    //Calculate speed between 0.0 and 1.0
    Serial.println(freq);
    

    //Send to OpenVR drivers
    if (speed >= 0.1){
      Serial.println(speed);
      //send to vr driver (sending as char string)
      char buffer[5];
      dtostrf(speed, 4, 2, buffer);
      //SerialBT.write((uint8_t*)&buffer, sizeof(buffer));

      //Send data with orienation to all connected devices
      String tmp;
      tmp = tmp + "%;TK1;DRV;MOT;4;" + filter.getYaw() + ";" + filter.getPitch() + ";" + filter.getRoll() + ";" + speed + ";0";
      Vector<String> splitTmp;
      splitTmp.setStorage(BLE_RSP_ARRAY);
      splitString(tmp, &splitTmp, ';');
      tmp.remove(tmp.length()-1);
      tmp = tmp + checkSumCalc(&splitTmp);
      const char* tmp_c = tmp.c_str();
      pCharacteristic_DRV->setValue((uint8_t*)tmp_c, tmp.length());
      pCharacteristic_DRV->notify();
      Serial.println(tmp);
    } else {
      //too slow, not sending
    }
  }  
}