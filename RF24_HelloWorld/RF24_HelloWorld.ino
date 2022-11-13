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
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(channel);
  radio.startListening();
}

void loop()
{
  byte text[32] = {0};
  //Read the data if available in buffer
  while (radio.available())
  { 
    radio.read(&text, sizeof(text));
    Serial.write(text, sizeof(text));
  }
}
