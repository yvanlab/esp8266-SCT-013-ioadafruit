
#include "SparkfunManager.h"

//#include <WiFiClient.h>


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

SparkfunManager::SparkfunManager( unsigned char pinLed) : BaseManager(pinLed)  {
  m_index       = 0;
}

void SparkfunManager::addVariable(String name, unsigned int value) {
  m_variables[m_index].Name=name;
  m_variables[m_index].Value=value;
  m_index++;

}

String SparkfunManager::buildRequest(String privateKey, String publicKey) {

  String url = "/input/";
  url += publicKey;
  url += "?private_key=";
  url += privateKey;
  unsigned char i = 0;
  while (i < m_index) {
    url += "&" + m_variables[i].Name + "=" + String(m_variables[i].Value);
    i++;
  }

  String res = String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + "data.sparkfun.com" + "\r\n" +
                 "Connection: close\r\n\r\n";
  m_index = 0;

  return res;
}


unsigned char SparkfunManager::sendKPIsToIO(String privateKey, String publicKey) {
  switchOn();
  WiFiClient client;
  if (!client.connect("data.sparkfun.com", 80)) {
      DEBUGLOG("connection failed");
      setStatus(1, "failed");
      switchOff();
      return 1;
  }


  String url = buildRequest(privateKey,publicKey);
  DEBUGLOG("Requesting URL: ");
  DEBUGLOG(url);
  client.print(url);

    // This will send the request to the server

  unsigned long timeout = millis();
  while (client.available() == 0) {
  if (millis() - timeout > 5000) {
    DEBUGLOG(">>> Client Timeout !");
    client.stop();
    setStatus(2, "Timeout");
    switchOff();
    return 2;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    DEBUGLOG(line);
  }

  DEBUGLOG();
  DEBUGLOG("closing connection");
  m_lastSucessTime = (unsigned long) millis();
  setStatus(0, "Ok");
  switchOff();
  return 0;
}

String SparkfunManager::toString() {

  //String res = "public key [" + m_publicKey +"] - Private key["+m_privateKey +"] " ;
  String res = "variables :";
  unsigned char i=0;
  while (i < m_index) {
    res += " - "+m_variables[i].Name + "=" + String(m_variables[i].Value);
    i++;
  }
  return res;

}
