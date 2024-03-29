/* Uses parts from the Adafruit LSM6DS3TR-C demo, SerialToSerialBT demo, BLE_client demo, and BLE_server demo for the Adafruit ESP32 Feather V2*/

/* This program is used for converting stepping data to speed data*/
/* Created by: Matthew Branigan */
/* Modified on: 11/14/2023 */

//#include "BluetoothSerial.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Adafruit_LIS3MDL.h>
//#include <MadgwickAHRS.h>
#include <Adafruit_AHRS.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Vector.h>
#include <Adafruit_NeoPixel.h>


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
#define VBATPIN A13

// Global Variables

  //Bluetooth Variables
  String device_name = "MarchVR Best Team";
  static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
  static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

  bool doConnect = false;
  bool connected = false;
  bool doScan = false;
  bool newBLERsp = false;
  BLERemoteCharacteristic* pRemoteCharacteristic;
  BLEAdvertisedDevice* myDevice;
  String BLE_RSP_ARRAY[MAX_DATA_PARA];
  String BLE_Wrt_Rsp;

  //Step Counter Variables
  int stepCount = 0;
  bool above = false, below = false, nextStep = false;
  float variance = 0.1f;

  //Calibration Variables
  float accelAvg[3];
  short accelPos[3];
  unsigned int up = 0;
  bool sendCali = false;

  //Filtering and Frequency Variables
  float prevVal = 0.0f;
  unsigned long startTime;
  bool resetTime = true;

  //IMU Oridentation Filtering
  Adafruit_LSM6DS3TRC lsm6ds3trc;
  Adafruit_LIS3MDL lis3mdl;
  //Madgwick filter;
  Adafruit_Madgwick filter;
  Adafruit_Sensor_Calibration_EEPROM cal;
  float tk2Ori[3] = {0.0f, 0.0f, 0.0f};

  //Create a NeoPixel object called onePixel that addresses 1 pixel in pin PIN_NEOPIXEL
  Adafruit_NeoPixel onePixel = Adafruit_NeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

  int loopDelay = 35; //milliseconds we delay at end of each loop

  int wakePeriod = 25 * 60000; // 25 minutes ... the number of milliseconds we want to wait before entering sleep mode
  unsigned long startSleepTime = millis(); //get current time to later determine if we timed out for sleep

  int batteryCheckPeriod = .5 * 60000; // 30 seconds ... the number of ms we want to wait before checking battery
  unsigned long startBatteryTime = millis(); // get current time to later determine if time to check
  float measuredvbat = analogReadMilliVolts(VBATPIN) * 2.0f / 1000.0f; // checks battery level

//Classes

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify){
  String value = "";
  for (uint8_t i = 0; i < length; i++){
    value += (char)(pData[i]);
  }
  
  if (value.length() > 0) {
    BLE_Wrt_Rsp = "";
    BLE_Wrt_Rsp += value.c_str();

    newBLERsp = true;
    //Serial.println(value);
  }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;

    onePixel.setPixelColor(0, 200, 0, 0);//set to red to indicate it is disconnected
    onePixel.show();

    Serial.println("onDisconnect");
  }
};

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

  Serial.println("Calibration done!");
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

void splitString(String input, Vector<String>* output, char delim)
{
  uint8_t count = 0;
  //Vector<String> output;
  String tmp = "";

  for (uint8_t i = 0; i < input.length(); i++){
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

void bleResponse()
{
  Vector<String> splitVal;
  splitVal.setStorage(BLE_RSP_ARRAY);
  splitString(BLE_Wrt_Rsp, &splitVal, ';');

  // //Check if valid
  if (splitVal.at(0) != "%"){
    Serial.printf("Invalid Starting Byte: %s", splitVal.at(0).c_str());
    return;
  }
  if (!assertCheckSum(&splitVal)){
    Serial.printf("Failed Sum Check: %s", splitVal.at(splitVal.size()-1).c_str());
    return;
  }

  //Check where it is coming from
  String src = splitVal.at(1);

  //Check where it will be going to
  String dst = splitVal.at(2);

  //Check what command is calling
  String cmd = splitVal.at(3);

  //If to client
  Serial.println(BLE_Wrt_Rsp);
  if (dst == "TK2"){
    if (src == "TK1"){
      //To be added upon
      if (cmd == "PWR"){
        enterSleep();
      }
    } else if (src == "GUI") {
      if (cmd == "CAL"){
        calibrateTracker();
        sendCali = true;
      }

    } else if (src == "DRV") {
      
    }
  
  }
}

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");
    connected = true;
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

void enterSleep(){
  Serial.println("Client going to sleep");
  onePixel.setPixelColor(0, 0, 0, 0);//turn NeoPixel off
  onePixel.show();//update pixel
  esp_deep_sleep_start();//enter sleep
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) { //Stop scanning if the service is there

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks



void setup() 
{
  // Setup serial debugging
  Serial.begin(115200);

  // Setup the NeoPixel -  Doing this sooner in setup() so we know it is on
  onePixel.begin();
  onePixel.clear();
  onePixel.setBrightness(20);
  onePixel.setPixelColor(0, 200, 0, 0);//set to red to indicate it is on but not connected
  onePixel.show();

  //Setup BLE
  BLEDevice::init("MarchVR BLE Client Tracker");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  //Find IMU
  if (!lsm6ds3trc.begin_I2C()) {
    Serial.println("Failed to find LSM6DS3TR-C chip. Please connect IMU to I2C connection.");
    while (1) {
      delay(10);
    }
  }

  Serial.println("LSM6DS3TR-C Found!");


  if (! lis3mdl.begin_I2C()) {
    Serial.println("Failed to find LIS3MDL chip");
    while (1) { 
      delay(10);
    }
  }

  Serial.println("LIS3MDL Found!");

  lsm6ds3trc.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  lsm6ds3trc.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
  lsm6ds3trc.setAccelDataRate(LSM6DS_RATE_26_HZ);
  lsm6ds3trc.setGyroDataRate(LSM6DS_RATE_26_HZ);

  lsm6ds3trc.configInt1(false, false, true); // accelerometer DRDY on INT1
  lsm6ds3trc.configInt2(false, true, false); // gyro DRDY on INT2

  lis3mdl.configInterrupt(false, false, true, // enable z axis
                        true, // polarity
                        false, // don't latch
                        true); // enabled!

  calibrateTracker();

  filter.begin(100);
}

void loop() 
{
  //Waits until bluetooth is connected
  if (doConnect == true) {
    if (connectToServer()) {
      onePixel.setPixelColor(0, 0, 150, 200);//set to blueish green to indicate it is connected battery not yet checked
      onePixel.show();
      Serial.println("We are now connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  if (newBLERsp){
    bleResponse();
    newBLERsp = false;
  }

  
  static unsigned long prevReadingTime = millis();
  sensors_event_t accel, gyro, temp;
  if (millis() - prevReadingTime < 10){
    return;
  } else {
    sensors_event_t mag;
    lsm6ds3trc.getEvent(&accel, &gyro, &temp);
    lis3mdl.read(); 
    lis3mdl.getEvent(&mag);
    cal.calibrate(mag);
    cal.calibrate(accel);
    cal.calibrate(gyro);
    prevReadingTime = millis();
    filter.update(gyro.gyro.x * SENSORS_RADS_TO_DPS, gyro.gyro.y * SENSORS_RADS_TO_DPS, gyro.gyro.z * SENSORS_RADS_TO_DPS, accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, mag.magnetic.x, mag.magnetic.y, mag.magnetic.z);
    //Serial.printf("Yaw: %f Pitch: %f Roll: %f\n", filter.getYaw(), filter.getPitch(), filter.getRoll());
    //if(connected){Serial.println("Connected " + connected);}
  }

  //Check Battery and Change Battery Level
  if(millis() - startBatteryTime >  batteryCheckPeriod){ //only check once every batteryCheckPeriod
    measuredvbat = analogReadMilliVolts(VBATPIN) * 2.0f / 1000.0f;
    startBatteryTime = millis();
    //Serial.println("Checking");
  }
  if(connected){ // ensure we are connected
    if (measuredvbat > 3.79f){
      //High Battery
      //Serial.println("High Battery!");
      onePixel.setPixelColor(0, 0, 250, 0);//green
      onePixel.show();//update pixel
    } else if (measuredvbat < 3.6f){
      //Low Battery
      //Serial.println("Low Battery!");
      onePixel.setPixelColor(255, 165, 0, 0);//orange
      onePixel.show();//update pixel
    } else {
      //Normal Battery
      //Serial.println("Normal Battery!");
      onePixel.setPixelColor(0, 0, 0, 200);//blue
      onePixel.show();//update pixel
    }
  } else if (doScan){
    BLEDevice::getScan()->start(0);  // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
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

  //Serial.println(up);

  //Increaments step counter if user has done a full step
  if ((above && below) || sendCali){
    below = false;
    above = false;
    if (nextStep || sendCali){
      tk2Ori[0] = filter.getRollRadians() / 3.1415926f;
      tk2Ori[1] = filter.getYawRadians() / 3.1415926f;
      tk2Ori[2] = filter.getPitchRadians() / 3.1415926f;
      sendCali = false;
      nextStep = false;
      String tmp;
      tmp = tmp + "%;TK2;TK1;MOT;4;" + tk2Ori[1] + ";" + tk2Ori[0] + ";" + tk2Ori[2] + ";0";
      Vector<String> splitTmp;
      splitTmp.setStorage(BLE_RSP_ARRAY);
      splitString(tmp, &splitTmp, ';');
      tmp.remove(tmp.length()-1);
      tmp = tmp + checkSumCalc(&splitTmp);
      Serial.println(tmp);
      if (connected)
        pRemoteCharacteristic->writeValue(tmp.c_str(), tmp.length());

      startSleepTime = millis();
    } else {
      nextStep = true;
    }
    //
  }

  //Added backup timer to make sure that the client will fall asleep without a connection to the server
  if(millis() - startSleepTime > wakePeriod){
    enterSleep();
  }
}
