//Include Libraries
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// nRF24 Pins
#define NRF24_SCLK 14 // out green
#define NRF24_MISO 13 // in  purple
#define NRF24_MOSI 12 // out blue
#define NRF24_CS   15 // out orange
#define NRF24_CE   16 // 3.3 yellow (not used)

SPIClass SPI2(VSPI);
RF24 radio(NRF24_CE, NRF24_CS);

//address through which two modules communicate.
const uint8_t txa[5] = {0x01, 0x00, 0x00, 0x00, 0x00};
const uint8_t rxa[5] = {0x02, 0x00, 0x00, 0x00, 0x00};

String received_message = "";

void radioWrite(byte msg[], int len, bool fast)
{
  if (fast)
  {
    if (!radio.writeFast(msg, len))
    {
      radio.txStandBy();
    }
  } else
  {
    radio.write(msg, len);
  }

  if (radio.available())
  {
    char ack[radio.getDynamicPayloadSize()] = {0};
    radio.read(&ack, sizeof(ack));

    received_message = String(ack);

    Serial.println("Received: " + received_message);
  }
}

void writeBytes(byte message[], int len)
{
  byte msg[32] = {0};
  int i = 0;
  if (len > 32)
  {
    for (i = 0; i < int(len / 32); i++)
    {
      memcpy(msg, message + (i * 32), 32);
      radioWrite(msg, 32, true);
    }
    
    if (i * 32 < len)
    {
      memcpy(msg, message + (i * 32), len - (i * 32));
      radioWrite(msg, 32, true);
    }
  } else
  {
    memcpy(msg, message, 32);
    radioWrite(msg, 32, false);
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

  // configure radio
  radio.openWritingPipe(rxa);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.setChannel(125);
  radio.stopListening();

  writeMessage("Radio Initialized");
  
  Serial.println("Setup Complete");
  writeMessage("Setup Complete");

  delay(50);
}

void loop()
{ 

  writeMessage("Hello, World!");

  if (received_message != "")
  {
    writeMessage("Echo: " + received_message);
    received_message = "";
  }
  
  delay(500);
}
