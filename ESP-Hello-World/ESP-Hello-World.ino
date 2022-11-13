//Include Libraries
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

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

  Serial.println("Moving to loop");
  writeMessage("Moving to loop");

  delay(500);
}

void loop()
{
  Serial.println("Hello World!");
  writeMessage("Hello World!");
  delay(1000);
}
