//Include Libraries
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "radio_helpers.h"
#include "Adafruit_BMP3XX.h"
#include <Adafruit_LSM6DSOX.h>
#include "camera_interface.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32

// I2C Pins
#define I2C_SCL 2
#define I2C_SDA 4

// nRF24 Pins
#define NRF24_SCLK 14 // out green
#define NRF24_MISO 13 // in  purple
#define NRF24_MOSI 12 // out blue
#define NRF24_CS   15 // out orange
#define NRF24_CE   16 // 3.3 yellow (not used)

// nRF24 Values
#define RF24_CHANNEL 125 // Change this to your team's channel

SPIClass SPI2(VSPI);
RF24 radio(NRF24_CE, NRF24_CS);

//address through which two modules communicate.
const uint8_t txa[5] = {0x01, 0x00, 0x00, 0x00, 0x00};
const uint8_t rxa[5] = {0x02, 0x00, 0x00, 0x00, 0x00};

// bmp
#define SEALEVEL_HPA 1013.25
Adafruit_BMP3XX bmp;

// lsm
Adafruit_LSM6DSOX lsm;

byte *frame;
int frameNum = 0;

void setup()
{
  Serial.begin(115200);
  
  /*if(!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD Card Mount Failure...");
    RadioHelpers::writeMessage("SD Card Mount Failure...");
  }*/
  
  SPI2.begin(NRF24_SCLK, NRF24_MISO, NRF24_MOSI, NRF24_CS);
  
  if (!radio.begin(&SPI2))
  {
    Serial.println("Failed to start radio...");
  } else
  {
    Serial.println("Success!");
  }

  if (!radio.isChipConnected())
  {
    Serial.println("Failed to detect radio");
  } else
  {
    Serial.println("Success!");
  }

  if (!radio.isValid())
  {
    Serial.println("Failed?? Not a real radio??");
  } else
  {
    Serial.println("Success!");
  }
   
  radio.powerUp();

  // configure radio
  radio.openWritingPipe(rxa);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.setChannel(RF24_CHANNEL);
  radio.stopListening();

  RadioHelpers::setRadio(radio);
  RadioHelpers::writeMessage("Radio Initialized");

  Wire.begin(I2C_SDA, I2C_SCL);

  while(!bmp.begin_I2C())
  {
    Serial.println("BMP388 Not Found");
    RadioHelpers::writeMessage("BMP388 Not Found");
    delay(1000);
  }

  Serial.println("BMP Found");
  RadioHelpers::writeMessage("BMP Found");

  // improve data
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  while(!lsm.begin_I2C())
  {
    Serial.println("LSM Not Found");
    RadioHelpers::writeMessage("LSM Not Found");
    delay(1000);
  }

  // set IMU limits
  lsm.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
  lsm.setGyroRange(LSM6DS_GYRO_RANGE_125_DPS);
  lsm.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  lsm.setGyroDataRate(LSM6DS_RATE_12_5_HZ);

  Serial.println("LSM Found");
  RadioHelpers::writeMessage("LSM Found");

  ESP_CAMERA::init_stream_camera();
  frame = (byte *)malloc(10000);

  RadioHelpers::writeMessage("Camera Initialized");
  
  Serial.println("Setup Complete");
  RadioHelpers::writeMessage("Setup Complete");
  delay(50);
}

void loop()
{ 
  double start = millis();
  bmp.performReading();

  // imu events
  sensors_event_t a, g, t;
  lsm.getEvent(&a, &g, &t);
  
  Serial.println("Transmitting...");

  double altitude = bmp.readAltitude(SEALEVEL_HPA);
  double temperature = bmp.temperature;
  
  String telemetry = "TSP,";
  telemetry += String(altitude) + ",";
  telemetry += String(temperature) + ",";
  telemetry += String(a.acceleration.x * 2) + ",";
  telemetry += String(a.acceleration.y * 2) + ",";
  telemetry += String(a.acceleration.z * 2) + ",";
  telemetry += String(g.gyro.x) + ",";
  telemetry += String(g.gyro.y) + ",";
  telemetry += String(g.gyro.z) + ",";
  telemetry += String(millis()) + ",";
  telemetry += "TEP\n";
  
  RadioHelpers::writeBytes((byte *)telemetry.c_str(), strlen(telemetry.c_str()));

  if (RadioHelpers::stream)
  {
    int bytes = ESP_CAMERA::get_frame(frame, true);
    if (bytes > 0)
    {
      RadioHelpers::writeBytes(frame, bytes);
    }
  } else if (RadioHelpers::record) {

    ESP_CAMERA::get_frame(NULL, false, frameNum);  
    frameNum++;
  }


  double stopTime = millis() - start;
  if (stopTime < 33)
  {
    delay((int)(33 - stopTime));
  }
}
