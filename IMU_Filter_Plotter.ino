/*This program filters the accelerometer data outputted by the LSM6DS3TR-C due to noise using an IIR filter 
given by the equation y[n] = alpha*y[n-1] + (1-alpha)*x[n] provided by Signals and Systems professor
Dr. Harley*/
/*After filtering, we plot the original data and the filtered data using the serial plotter*/
/*Created by Maria Carmona*/
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
#define SMOOTHING_ALPHA 0.3f

// Global Variables
BluetoothSerial SerialBT;
String device_name = "MarchVR Best Team";
Adafruit_LSM6DS3TRC lsm6ds3trc;
float fx, fy, fz = 0.0f;
void setup() {
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

void loop() {
  sensors_event_t accel, gyro, temp;
  lsm6ds3trc.getEvent(&accel, &gyro, &temp);
  
  float currAccel[3] = {accel.acceleration.x, accel.acceleration.y, accel.acceleration.z};
  //Applying filter to data using alpha = 0.3
  fx = SMOOTHING_ALPHA*fx + (1-SMOOTHING_ALPHA)*currAccel[0];
  fy = SMOOTHING_ALPHA*fy + (1-SMOOTHING_ALPHA)*currAccel[1];
  fz = SMOOTHING_ALPHA*fz + (1-SMOOTHING_ALPHA)*currAccel[2];
  // Plotting using the serial plotter
  Serial.print("Accel_X:");
  Serial.print(currAccel[0]);
  Serial.print(",");
  Serial.print("Accel_Y:");
  Serial.print(currAccel[1]);
  Serial.print(",");
  Serial.print("Accel_Z:");
  Serial.print(currAccel[2]);
  Serial.print(",");
  Serial.print("filterx:");
  Serial.print(fx);
  Serial.print(",");
  Serial.print("filtery:");
  Serial.print(fy);
  Serial.print(",");
  Serial.print("filterz:");
  Serial.println(fz);

}
