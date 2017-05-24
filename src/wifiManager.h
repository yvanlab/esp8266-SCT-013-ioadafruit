

#ifndef WifiManager_h
#define WifiManager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "BaseManager.h"
class WifiManager  : public BaseManager
{
  public:

    WifiManager(unsigned char pinLed);
    String toString();
    //unsigned char connectSSID();
    unsigned char connectSSID(char *ssid,char *pass);
    unsigned char connectAP();

    String getClassName(){return "WifiManager";}

  private:
    


};

#endif
