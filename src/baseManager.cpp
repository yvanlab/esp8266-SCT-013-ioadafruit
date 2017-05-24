

#include "BaseManager.h"
#include "HourManager.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

BaseManager::BaseManager(unsigned char pinLed){
  m_pinLed = pinLed;
  m_changed = true;
}

void BaseManager::switchOn(){
  digitalWrite ( m_pinLed, LOW );
}
void BaseManager::switchOff(){
  digitalWrite ( m_pinLed, HIGH );
}

String BaseManager::log(){
  m_changed = false;
  return getClassName()+":" + HourManager::toUTString(m_timeStampStatus) + " [" + String(m_codeStatus) + "]:" + m_description;
}
