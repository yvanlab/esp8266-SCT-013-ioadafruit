
#include "ElectricManager.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



ElectricManager::ElectricManager( unsigned char pinCurrent,unsigned char pinLed, unsigned char address)
  : BaseManager(pinLed) , ReadManager(address) {
  m_pinCurrent = pinCurrent;
  pinMode ( pinCurrent, INPUT );

}

double ElectricManager::getInstantKWH() {
  // extrapolation sur 24H a partir d une 1mn de mesure
  double result = m_valueCurrent;
  return ((result/1000.0)*220*60)/(1000.0*3600);
}


/*
{
  "current" : "10250",
  "KWT"  : "40.25",
  "LAST_KWT"  : 20.25
},
*/
String ElectricManager::toString(boolean bJson) {
  if (bJson==JSON_TEXT)
    return  String ("\"current\":\"" + String(m_valueCurrent) +\
            "\",\"KWH\":\"" + String(getInstantKWH()) +\
            "\",\"lastKWT\":\"" + String(m_previousWattHour) +"\"");
  else {
    String mesure = "Measure  sampling["+String (m_nbreMeasure) + "] - Min[" + String(m_minCurrentValue);
    mesure += "] - Max[" + String(m_maxCurrentValue) + "] - Delta[" + String (m_maxCurrentValue-m_minCurrentValue);
    mesure += "] - nbMesure[" + String(m_globalNbreValue) + "]   ";
    mesure += "\nCurrent[" + String (m_valueCurrent)  + " mA]-Watt [" + String(m_valuePower) + "]";
    return mesure;
  }
}


unsigned long int ElectricManager::readCurrent(){
  // read during 2 periods 50 Hz :
  switchOn();
  setChannel();
  m_maxCurrentValue = 0;
  m_minCurrentValue = 1026;
  m_nbreMeasure = 0;
  unsigned long startMS = millis();
  while ((millis()-startMS)< twoPeriodsMS) {
      int currentValue = analogRead(m_pinCurrent);
      //DEBUGLOG (currentValue);
      if (currentValue > m_maxCurrentValue ) m_maxCurrentValue = currentValue;
      if (currentValue < m_minCurrentValue ) m_minCurrentValue = currentValue;
      m_nbreMeasure ++;
  }
  // 535 mv  ==> 2000W    ==> I = 9 000mA
  // 3300 mv ==> 12 336W  ==> I = 56 000mA
  // 1 mv    ==> 3.7 W    ==> I = 17 mA
  // 1 mv    == 3 300/1024  ==  3
  const int coefCurrent = 60; // mA((2000/535)/220)*(3300/1024);
  m_valueCurrent = (m_maxCurrentValue-m_minCurrentValue)*coefCurrent;
  setStatus(0, "ok");
  switchOff();
  return m_valueCurrent;
}

unsigned long int ElectricManager::readCumulCurrent() {
  m_globalIntervalValue += readCurrent();
  m_globalNbreValue++;

}

unsigned long int ElectricManager::getAverageCurrent(){
  unsigned long int average = m_globalIntervalValue/m_globalNbreValue;
  m_globalNbreValue = 0;
  m_globalIntervalValue = 0;

  //calculate watHour
  unsigned long int currentMS = millis();
  m_wattHour += (((currentMS-m_previousTimeMeasure)/1000.0)*(average*220.0))/1000.0; //watt.s
  m_previousTimeMeasure=currentMS;
  return average;
}

double ElectricManager::getKWattHour(){
  //DEBUGLOG(m_wattHour);
  m_previousWattHour = (m_wattHour/(3600.0*1000.0));
  m_wattHour = 0.0;
  return m_previousWattHour;
;
}
