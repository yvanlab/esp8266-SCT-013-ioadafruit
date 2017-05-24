

#ifndef SparkfunManager_h
#define SparkfunManager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "BaseManager.h"

typedef struct
{
    String Name;
    unsigned int Value;
} SparkVariable;


class SparkfunManager : public BaseManager
{

  public:

    SparkfunManager( unsigned char pinLed);
    unsigned char sendKPIsToIO(String privateKey, String publicKey);
    String toString();
    void addVariable(String name, unsigned int value);
    String buildRequest(String privateKey, String publicKey);
    unsigned long m_lastSucessTime = 0;
    String getClassName(){return "SparkfunManager";}
  private:
    SparkVariable m_variables[3];
    unsigned char m_pinLed;
    unsigned char m_index;



};

#endif
