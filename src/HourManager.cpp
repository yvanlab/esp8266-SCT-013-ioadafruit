
#include "HourManager.h"

//#include <WiFiClient.h>


#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

HourManager::HourManager(unsigned int localPort, unsigned char pinLed) : BaseManager(pinLed) {
  m_localPort = localPort;
  m_udp.begin(m_localPort);

}

unsigned long HourManager::getCurrentEpoch() {
  if (m_epoch == 0) {
    switchOn();
    m_epoch = readNTPDateHour();
    m_timeReference = millis();
    m_today = DateTime(m_epoch+(millis() - m_timeReference)/1000).day();
    switchOff();
  }
  if (m_epoch != 0) {
    return m_epoch+(millis() - m_timeReference)/1000;
  } else
     return 0;
}

boolean  HourManager::isNextDay() {
   DateTime now(getCurrentEpoch());
   if (now.day()!=m_today) {
     m_today = now.day();
     return true;
   }
   return false;
}


String HourManager::toDTString() {
  DateTime now(getCurrentEpoch());
  if (m_epoch!=0)
    return String (now.day())  + "/" + String (now.month())+"/"+String (now.year())+" "+String(now.hour())+":"+String(now.minute())+":"+now.second();

  return "Not date";
}


String HourManager::toUTString(){
  return HourManager::toUTString(millis());
}

String HourManager::toUTString(unsigned long epochMS=0) {
  int sec = epochMS / 1000;
  int min = sec / 60;
  int hr = min / 60;
  return String(hr,DEC) + ":" + String(min % 60,DEC) + ":"  + String(sec % 60,DEC);
}

unsigned long  HourManager::readNTPDateHour(){

  WiFi.hostByName(HM_NTP_SERVER_NAME, timeServerIP);
  sendNTPpacket(timeServerIP);
  delay(1000);

  if  (m_udp.parsePacket()) {
    m_udp.read(m_packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    unsigned long highWord = word(m_packetBuffer[40], m_packetBuffer[41]);
    unsigned long lowWord = word(m_packetBuffer[42], m_packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    setStatus(millis(), 0, "ok");
    return secsSince1900 - seventyYears;
  }
  setStatus(millis(), 1, "KO");
  return 0;

}

unsigned long HourManager::sendNTPpacket(IPAddress& address)
{
    // set all bytes in the buffer to 0
    memset(m_packetBuffer, 0, NTP_PACKET_SIZE);

    m_packetBuffer[0] = 0b11100011;   // li, version, mode
    m_packetBuffer[1] = 0;            // stratum, or type of clock
    m_packetBuffer[2] = 6;            // polling interval
    m_packetBuffer[3] = 0xEC;         // peer clock precision

    // 8 bytes of zero for root delay & root dispersion
    m_packetBuffer[12] = 49;
    m_packetBuffer[13] = 0x4E;
    m_packetBuffer[14] = 49;
    m_packetBuffer[15] = 52;

    // all ntp fields have been given values, send request
    m_udp.beginPacket(address, 123);
    m_udp.write(m_packetBuffer, NTP_PACKET_SIZE);
    m_udp.endPacket();

}
