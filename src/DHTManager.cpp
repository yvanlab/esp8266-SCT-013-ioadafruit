
#include "DHTManager.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



DHTManager::DHTManager( unsigned char pinDHT,unsigned char pinLed)
  : BaseManager(pinLed) {
  m_pinDHT=pinDHT;
}


String DHTManager::toString() {
  String mesure = "Temperature["+String (getTemperature()) + "] - Humidity[" + String(getHumidity())+"]";
  return mesure;
}

float DHTManager::getTemperature() {
  read();
  return m_DHT.temperature;
}

float DHTManager::getHumidity(){
  read();
  return m_DHT.humidity;
}

int8_t DHTManager::read(){
  // read during 2 periods 50 Hz :
  if ((millis() - getLastUpdate()) < 2000UL)
    return DHTLIB_OK;

  switchOn();
  int8_t chk = m_DHT.read(m_pinDHT);
  if (chk != DHTLIB_OK) {
    setStatus(millis(), chk, "KO",true);
  } else {
    setStatus(millis(), chk, "OK",false);
  }

#ifdef MCPOC_TEST
  switch (chk)
  {
    case DHTLIB_OK:
		Serial.print("OK,\t");
		break;
    case DHTLIB_ERROR_CHECKSUM:
		Serial.print("Checksum error,\t");
		break;
    case DHTLIB_ERROR_TIMEOUT:
		Serial.print("Time out error,\t");
		break;
    case DHTLIB_ERROR_CONNECT:
        Serial.print("Connect error,\t");
        break;
    case DHTLIB_ERROR_ACK_L:
        Serial.print("Ack Low error,\t");
        break;
    case DHTLIB_ERROR_ACK_H:
        Serial.print("Ack High error,\t");
        break;
    default:
		Serial.print("Unknown error,\t");
		break;
  }
#endif
  switchOff();
  return chk;
}
