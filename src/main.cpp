/*********
  
  Kamran R.
  Waqas M.
  Nabil E.

  Robbin Law
  Rui Santos
  
*********/

#include <Arduino.h>          // base library
#include <Adafruit_Sensor.h>  // default sensor library
#include <Adafruit_MPU6050.h> // for MPU6050 sensor
#include <Wire.h>             // for i2c communication
#include <TFT_eSPI.h>         // for TFT Display
#include "TaskScheduler.h"    // for mimic delay

// program contants
#define tftRefreshTime      1000
#define readingRefreshTime  1000

// variable for storing data from sensor
float temperature;
float AcX, AcY, AcZ, GyX, GyY, GyZ;

// tft is global to this file only
TFT_eSPI tft = TFT_eSPI();
// Setup the clock

uint16_t bg = TFT_BLACK;
uint16_t fg = TFT_WHITE;

// declaration of functions for setup
void initSetup();
void initSPIFFS();
void tftSetup();
void mpu6050Setup();

// declaration of functions for performing
void refresh_readings();
void tftDisplay();

// sensor variable
Adafruit_MPU6050 mpu;

// Setup tasks for the task scheduler
Task tftRefreshTask(tftRefreshTime, TASK_FOREVER, &tftDisplay);
Task dataRefreshTask(readingRefreshTime, TASK_FOREVER, &refresh_readings);

// Create the scheduler
Scheduler runner;

// main setup function
void setup()
{
  // calling the initSetup function
  initSetup();
}

// the loop functions
void loop()
{
  // Execute the scheduler runner
  runner.execute();
}

// initial setup
void initSetup()
{
  // Init Serial Monitor
  Serial.begin(115200);

  // starting the Flash File System
  initSPIFFS();

  // the MPU Setup function is called
  mpu6050Setup();

  // get reading for the first time
  refresh_readings();

  // tft Setup
  tftSetup();

  // Start the task scheduler
  runner.init();

  // Add the task to the scheduler
  runner.addTask(tftRefreshTask);
  runner.addTask(dataRefreshTask);

  // Enable the task
  //tftRefreshTask.enable();
  //dataRefreshTask.enable();
  runner.enableAll();
}

// Flash File Setup
void initSPIFFS()
{
  if (!SPIFFS.begin())
  {
    Serial.println("Cannot mount SPIFFS volume...");
    while (1)
      ; // infinite loop
  }
  else
  {
    Serial.println("SPIFFS volume mounted properly");
  }
}

// tft Setup
void tftSetup()
{
  // Setup the TFT
  tft.begin();
  tft.setRotation(1);
  tft.loadFont("NotoSansBold20");
  tft.setTextColor(fg, bg);
  tft.fillScreen(bg);
  tft.setCursor(0, 0);
  tft.println("Hello!");
  tft.println("Searching for the sensor...");
  delay(2000);

  // displaying on the TFT Screen
  tft.fillScreen(bg);
  tft.setCursor(5, 5);
  tft.setTextColor(fg, bg);
  tft.loadFont("NotoSansBold20");
  tft.println("Right now...");
  tft.setTextColor(TFT_YELLOW, bg);

  // Temperature
  tft.fillRect(10, 30, 250, 30, bg);
  tft.setCursor(10, 30);
  tft.printf("Temperature: %6.2f  °C", temperature);

  // Other values
  tft.setCursor(00, 50);
  tft.println("______________________________________");

  // Accelerometer Values
  tft.fillRect(10, 80, 250, 30, bg);
  tft.setCursor(10, 80);
  tft.println("Accelerometer Values");
  tft.setCursor(15, 100);
  tft.printf("X: %6.2f, Y: %6.2f, Z: %6.2f m/s^2\n", AcX, AcY, AcZ);

  // Gyroscope Values
  tft.fillRect(10, 140, 250, 30, bg);
  tft.setCursor(10, 140);
  tft.println("Gyroscope Values");
  tft.setCursor(15, 160);
  tft.printf("X: %6.2f, Y: %6.2f, Z: %6.2f rad/s", GyX, GyY, GyZ);
}

// setting up MPU6050 i2c
void mpu6050Setup()
{
  if (!mpu.begin())
  {
    Serial.println("Could not find MPU6050 Chip, check wiring!");
    while (1)
    {
      Serial.print(".");
      delay(500);
    }
  }
  else
  {
    Serial.println("Found MPU6050 Chip...");
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange())
  {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange())
  {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth())
  {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
}

// reading from sensor
void refresh_readings()
{
  // reading the readings from the MPU6050 Chip
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  temperature = temp.temperature;

  AcX = a.acceleration.x;
  AcY = a.acceleration.y;
  AcZ = a.acceleration.z;

  GyX = g.gyro.x;
  GyY = g.gyro.y;
  GyZ = g.gyro.z;

  // displaying MPU6050 values on the serial output
  Serial.println();
  Serial.println("*** MPU6050 Values ***");
  Serial.printf("Temperature:   %-6.2f °C", temperature);
  Serial.println();
  Serial.printf("Acceleration   X: %5.2f, Y: %5.2f, Z: %5.2f   m/s^2", AcX, AcY, AcZ);
  Serial.println();
  Serial.printf("Rotation       X: %5.2f, Y: %5.2f, Z: %5.2f   rad/s", GyX, GyY, GyZ);
  Serial.println();
}

// refreshing TFT display with the new readings
void tftDisplay()
{
  // displaying only changing values on the TFT Screen
  // Temperature
  tft.fillRect(150, 29, 58, 20, TFT_RED);
  tft.setCursor(150, 30);
  tft.printf("%6.2f", temperature);

  // Accelerometer Values Update
  tft.fillRect(40, 99, 58, 20, TFT_RED);
  tft.setCursor(40, 99);
  tft.printf("%6.2f,", AcX);

  tft.fillRect(119, 99, 58, 20, TFT_RED);
  tft.setCursor(119, 99);
  tft.printf("%6.2f,", AcY);

  tft.fillRect(200, 99, 58, 20, TFT_RED);
  tft.setCursor(200, 99);
  tft.printf("%6.2f ", AcZ);

  // Gyroscope Values Update
  tft.fillRect(40, 159, 58, 20, TFT_RED);
  tft.setCursor(40, 159);
  tft.printf("%6.2f,", GyX);

  tft.fillRect(119, 159, 58, 20, TFT_RED);
  tft.setCursor(119, 159);
  tft.printf("%6.2f,", GyY);

  tft.fillRect(200, 159, 58, 20, TFT_RED);
  tft.setCursor(200, 159);
  tft.printf("%6.2f ", GyZ);
}
