//Include Libraries
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "radio_helpers.h"

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
  
  Serial.println("Setup Complete");
  RadioHelpers::writeMessage("Setup Complete");

  delay(50);
}

void loop()
{ 

  Serial.println("Transmitting...");
  RadioHelpers::writeMessage("Hello, World!");
  
  delay(500);
}
