#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "control.h"
#include <Arduino.h>

void Control::begin(){
  int result=dac.begin(xmDAC::SINGLE_CHANNEL_MODE, xmDAC::VREF_VCC);
  dac.write(0xFFF);
}

//---------------------------- Control-ReadData --------------------------------//
void Control::readData(){
  //Debug.println("ControlreadData");
  this->averageCurrent = 0;
  this->averageVoltage = 0;
  this->averageTemp  = 0;

  //for(int i = 0; i < 70; i++) //promedio Current,Voltage,Temperature //40
  //{ implementar mejor manera**
    this->averageCurrent = this->averageCurrent + analogRead(A0);
    vTaskDelay(1);
    this->averageVoltage = this->averageVoltage + analogRead(A1);
    vTaskDelay(1);
    this->averageTemp = this->averageTemp + analogRead(A2);
    vTaskDelay(1);
  //}
/*  this->averageCurrent = this->averageCurrent / 70;//40
  this->averageVoltage = this->averageVoltage / 70;
  this->averageTemp = this->averageTemp / 70;

  this->valcurrent0 = this->averageCurrent - 70.0; //36*/
  valcurrent = this->valcurrent0 * 35.0 / 1023.0;

  valAH = valcurrent * 0.000277 * controlTime.ms() * 0.001;

  this->valvoltage0 = this->averageVoltage - 44.0;
  valvoltage = this->valvoltage0 * 500.0 / 1023.0;

  valtemp = this->averageTemp * 120.0 / 1023.0;
  //vTaskDelay(1);
}
//------------------------------ Control-event -----------------------------------//
void Control::event() {
  //here your logic to control the current
  Debug4.println("ControlEvent");
}
