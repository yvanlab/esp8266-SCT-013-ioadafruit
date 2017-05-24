

#ifndef ElectricManager_h
#define ElectricManager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif
#include "BaseManager.h"
#include "readManager.h"

class ElectricManager : public BaseManager,public ReadManager
{

  public:
    unsigned int m_maxCurrentValue = 0;
    unsigned int m_minCurrentValue = 1026;
    unsigned char m_nbreMeasure = 0;
    unsigned int m_valueCurrent = 0;
    unsigned int m_valuePower = 0;
    unsigned long  m_cumulCurrent = 0;

    double  m_wattHour = 0.0;
    unsigned long  m_previousTimeMeasure = 0;

    unsigned char  m_pinCurrent = A0;

    unsigned int  m_globalIntervalValue = 0;
    unsigned char m_globalNbreValue = 0;

    ElectricManager(unsigned char pinCurrent,unsigned char pinLed, unsigned char address=0);

    String toString();
    unsigned long int readCurrent();
    unsigned long int readCumulCurrent();
    unsigned long int getAverageCurrent();
    double getKWattHour();

    String getClassName(){return "ElectricManager";}

  private:
    const unsigned long twoPeriodsMS = 2000/50;

};

#endif
