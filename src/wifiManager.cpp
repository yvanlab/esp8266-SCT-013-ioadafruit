
#include "WifiManager.h"

//#include <WiFiClient.h>


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif


#define EE_ADR_SSID 0
#define MAX_SSID 32
#define MAX_PASS 64
#define EE_ADR_PASS 100

WifiManager::WifiManager(unsigned char pinLed) : BaseManager(pinLed){

  }

String WifiManager::toString(){
  return "Mode["+String(WiFi.getMode())+\
  "] SSID["+WiFi.SSID()+\
  "] LocalIP["+WiFi.localIP().toString()+\
  "] softIP[" + WiFi.softAPIP().toString() + \
  "] status[" + String(WiFi.status(),DEC) + "]" ;
}



unsigned char WifiManager::connectSSID(char *ssid, char *pass){
  if (!ssid[0] || !pass[0])
    return WL_NO_SSID_AVAIL;

  WiFi.begin ( ssid, pass );
  unsigned char inbTest = 0;
  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED && inbTest < 20) {
    switchOn();
    delay ( 500 );
    Serial.print ( "." );
    Serial.print (WiFi.status());
    inbTest++;
    switchOff();
  }
  WiFi.softAP("Rhrrr");
  MDNS.begin ( "rhrrr" );
  setStatus(millis(), WiFi.status(), "ssid");
  return WiFi.status();
}

unsigned char WifiManager::connectAP(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.softAP("Rhrrrap");
  MDNS.begin ( "rhrrr", WiFi.softAPIP() );
  setStatus(millis(), WiFi.status(), "ap");
  return WiFi.status();
}
