//Include Libraries
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Adafruit_BMP3XX.h"
#include <Adafruit_LSM6DSOX.h>
#include "camera_interface.h"

//I2C Pins
#define I2C_SCL 2
#define I2C_SDA 4

// nRF24 Pins
#define NRF24_SCLK 14
#define NRF24_MISO 13
#define NRF24_MOSI 12
#define NRF24_CS   15
#define NRF24_CE   16

SPIClass SPI2(VSPI);
RF24 radio(NRF24_CE, NRF24_CS);

//address of our radio
const uint8_t address[5] = {0, 0, 0, 0, 1};

// the frequency our radio listens and writes on (2.4 Ghz - 2.525 Ghz)
const uint8_t channel = 125;

// bmp388
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BMP3XX bmp;

// lsm6d
Adafruit_LSM6DSOX sox;

//image frame to send
byte *frame;

void writeBytes(byte message[], int len)
{
  byte msg[32] = {0};
  int i = 0;
  if (len > 32)
  {
    for (i = 0; i < int(len / 32); i++)
    {
      memcpy(msg, message + (i * 32), 32);
      radio.write(&msg, 32);
    }
    
    if (i * 32 < len)
    {
      memcpy(msg, message + (i * 32), len - (i * 32));
      radio.write(&msg, 32);
    }
  } else
  {
    memcpy(msg, message, 32);
    radio.write(&msg, 32);
  }
}

void writeMessage(String message, bool newLine = true)
{
  String packet = message + ((newLine) ? "\r\n" : "");

  writeBytes((byte *)packet.c_str(), strlen(packet.c_str()));
}

void setup()
{
  SPI2.begin(NRF24_SCLK, NRF24_MISO, NRF24_MOSI, NRF24_CS);
  Serial.begin(115200);
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

  //set the address
  radio.setAddressWidth(5);
  radio.openWritingPipe(address);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(channel);
  radio.stopListening();

  writeMessage("Radio Initialized");

  //ESP_CAMERA::init_camera();
  //frame = (byte *)malloc(10000);

  //writeMessage("Camera Initialized");

  Wire.begin(I2C_SDA, I2C_SCL);

  while (!bmp.begin_I2C())
  {
    Serial.println("Failed to find BMP388");
    writeMessage("Failed to find BMP388");
    delay(500);
  }

  Serial.println("Found BMP388");
  writeMessage("Found BMP388");

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  while (!sox.begin_I2C())
  {
    Serial.println("Failed to find LSM6D");
    writeMessage("Failed to find LSM6D");
    delay(500);
  }

  Serial.println("Found LSM6D");
  writeMessage("Found LSM6D");

  // setup IMU
  sox.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
  sox.setGyroRange(LSM6DS_GYRO_RANGE_125_DPS);
  sox.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  sox.setGyroDataRate(LSM6DS_RATE_12_5_HZ);

  Serial.println("Moving to loop");
  writeMessage("Moving to loop");

  delay(500);
}

void loop()
{  
  bmp.performReading();

  // imu events
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  sox.getEvent(&accel, &gyro, &temp);
  
  String telemetry = "TEL,";
  telemetry += String(bmp.temperature) + ",";
  telemetry += String(bmp.readAltitude(SEALEVELPRESSURE_HPA)) + ",";
  telemetry += String(accel.acceleration.x) + ",";
  telemetry += String(accel.acceleration.y) + ",";
  telemetry += String(accel.acceleration.z) + ",";
  telemetry += String(gyro.gyro.x) + ",";
  telemetry += String(gyro.gyro.y) + ",";
  telemetry += String(gyro.gyro.z) + ",TED";
  writeBytes((byte *)telemetry.c_str(), strlen(telemetry.c_str()));

  //int bytes = ESP_CAMERA::get_frame(frame);
  //if (bytes > 0)
  //{
  //  writeBytes(frame, bytes);
  //}

  delay(100);
}
