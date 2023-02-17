

namespace RadioHelpers
{
  namespace
  {
    RF24 _radio;
  }

  void setRadio(RF24 &radio_instance)
  {
    _radio = radio_instance;
  }
  
  String received_message = "";
  
  void radioWrite(byte msg[], int len, bool fast)
  {
    if (fast)
    {
      if (!_radio.writeFast(msg, len))
      {
        _radio.txStandBy();
      }
    } else
    {
      _radio.write(msg, len);
    }
  
    if (_radio.available())
    {
      char ack[_radio.getDynamicPayloadSize()] = {0};
      _radio.read(&ack, sizeof(ack));
  
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
};
