

#ifndef ioManager_h
#define ioManager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "BaseManager.h"

#define MQTT_DEBUG
#define MQTT_ERROR
#include <Adafruit_MQTT.h>                                 // Adafruit MQTT library
#include <Adafruit_MQTT_Client.h>                           // Adafruit MQTT library

//#include <Adafruit_ssd1306syp.h>                           // Adafruit Oled library for display

typedef struct
{
    String                  Name;
    String                  Value;
} SparkVariable;

//#define AIO_SERVER      "io.adafruit.com"
const int AIO_SERVERPORT  = 1883;

const char MQTT_SERVER[]     = "io.adafruit.com";

class ioManager : public BaseManager
{

  public:

    ioManager( unsigned char pinLed);
    unsigned char sendKPIsToIO(String privateKey, String publicKey);

    String toString();
    void addVariable(String name,String value);

    String getClassName(){return "ioManager";}
  private:
    SparkVariable m_variables[8];
    WiFiClient m_client;
    //Adafruit_MQTT_Client *m_mqtt;//(&client, MQTT_SERVER, AIO_SERVERPORT);
    //Adafruit_MQTT_Publish *m_cur;
    //unsigned char m_pinLed;
    unsigned char m_index;
    void MQTT_connect(Adafruit_MQTT_Client *mqtt);//String privateKey/*AIO_KEY*/, String publicKey /*AIO_USERNAME*/);



};

#endif
