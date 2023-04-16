#include "camera_interface.h"

namespace RadioHelpers
{

  bool stream = true;
  bool record = false;
  
  namespace
  {
    RF24 _radio;
  }

  void writeMessage(String message, bool newLine = true);

  void setRadio(RF24 &radio_instance)
  {
    _radio = radio_instance;
  }
  
  String received_message = "";
  
  void radioWrite(byte msg[], int len, bool fast)
  {
    if (fast)
    {
      //_radio.startFastWrite(msg, len, false);
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
      writeMessage("Received: " + received_message);

      if (received_message.indexOf("stream_toggle") != -1)
      {
        writeMessage("Toggling Video Stream...");
        stream = !stream;

        if (stream)
        {
          record = false;
          ESP_CAMERA::init_stream_camera();
        }

        received_message = "";
      }

      if (received_message.indexOf("record_toggle") != -1)
      {
        writeMessage("Toggling Video Recording...");
        record = !record;

        if (record)
        {
          stream = false;
          ESP_CAMERA::init_video_camera();
        }

        received_message = "";
      }
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
  
  void writeMessage(String message, bool newLine)
  {
    String packet = "MSP" + message + ((newLine) ? "\r\nMEP" : "MEP");
  
    writeBytes((byte *)packet.c_str(), strlen(packet.c_str()));
  }
};
