#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define NRF24_SCLK 8
#define NRF24_MISO 9
#define NRF24_MOSI 10
#define NRF24_CS   0
#define NRF24_CE   1

RF24 radio(NRF24_CE, NRF24_CS);

//address through which two modules communicate.
const uint8_t txa[5] = {0x01,0x00, 0x00, 0x00, 0x00};
const uint8_t rxa[5] = {0x02,0x00, 0x00, 0x00, 0x00};

void setup()
{
  Serial.begin(2000000);
  while(!Serial);
  
  if (!radio.begin())
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
  radio.openReadingPipe(0, rxa);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.setChannel(125);
  radio.startListening();
}

void loop()
{
  //Read the data if available in buffer
  while (radio.available())
  {
    if (radio.getDynamicPayloadSize() < 1)
      return;
    
    byte text[radio.getDynamicPayloadSize()] = {0};
    radio.read(&text, sizeof(text));
    
    if (Serial.available())
    {
      char buf[Serial.available()]; 
      size_t len = Serial.readBytes(buf, sizeof(buf));
      radio.writeAckPayload(0, buf, len);
    }
    
    Serial.write(text, sizeof(text));
  }
}
