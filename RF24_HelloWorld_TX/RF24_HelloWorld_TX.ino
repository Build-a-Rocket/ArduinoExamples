#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define NRF24_SCLK 8
#define NRF24_MISO 9
#define NRF24_MOSI 10
#define NRF24_CS   0
#define NRF24_CE   1

RF24 radio(NRF24_CE, NRF24_CS);

// address of our radio
const uint8_t address[5] = {0, 0, 0, 0, 1};

// the frequency our radio listens and writes on (2.4 Ghz - 2.525 Ghz)
const uint8_t channel = 125;

void setup()
{
  Serial.begin(2000000);
  while(!Serial);

  radio.begin();
  radio.powerUp();

  //set the address
  radio.setAddressWidth(5);
  radio.setDataRate(RF24_2MBPS);
  radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(channel);
  radio.openWritingPipe(address);
  radio.startListening();
}

void loop()
{
  String message = "Hello World!\r\n";
  Serial.println("Hello World!");

  writeBytes((byte *)message.c_str(), strlen(message.c_str()));

  delay(1000);
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
