#ifndef CONTROL_H_
#define CONTROL_H_

#include "xmDAC.h"
#include "StopWatch.h"
//#include "globals.h"

extern StopWatch controlTime;  // solo para mostrar valores
extern bool flagStep;
extern double valcurrent;
extern double valvoltage;
extern double valtemp;
extern int const LedRelay;
extern unsigned long Ttime;
extern char stepState[2];
extern float valAH;
extern float totAH;
extern bool flagCapt;

class Control
{
  public:

  xmDAC dac=xmDAC(xmDAC::DAC_PORT_B);
  float val_control = 0.0;
  float maxTemp = 0.0;
  float minTemp = 0.0;
  float valAmpHour = 0.0;

  bool flagS = false;
  bool flagO = false;
  bool flagTemp = false;
  bool flagPause = false;
  bool flagEnable = false;
  bool flagP = false;

  unsigned long timeout = 0;
  unsigned long steptime = 0;
  unsigned long Ttime0 = 0;

  unsigned long time1 = 60000; //ms ->60seg-1min
  unsigned long time2 = 90000; //1.5 min
  unsigned long t1 = 0;
  unsigned long t2 = 0;
  unsigned long timeAH = 0;

  int state = 0; // 1 = running, 2 = pause, 3 = stop
  int prevstate = 0;
  int valrampa = 0;
  long averageCurrent = 0;
  long averageVoltage = 0;
  long averageTemp  = 0;

  double tmpVal = 0.0;
  double valcurrent0 = 0.0;
  double valvoltage0 = 0.0;
  double valtemp0 = 0.0;
  double valAmpHour0 = 0.0;

  void begin();
  void setCurrent(float val_control);
  void setTemperature(float maxTemp,float minTemp);
  void setTime(unsigned long timeout);
  void setAmpHour(float valAmpHour);
  void stepPause(unsigned long steptime);
  void run();
  void runPause();
  void pause();
  void stop();
  void event();
  void readData();
  bool states();
  void rampa();
  void control_cbuff(const char* stepLetter);
};

#endif
