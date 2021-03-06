#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "control.h"
#include <Arduino.h>

void Control::begin(){
  int result=dac.begin(xmDAC::SINGLE_CHANNEL_MODE, xmDAC::VREF_VCC);
  dac.write(0); //FFF
}

//----------------------------- Set Current -----------------------------------//
void Control::setCurrent(float val_control){
  this->val_control = val_control;
}

//------------------------------ Set Time ------------------------------------//
void Control::setTime(unsigned long timeout){
  this->timeout = timeout * 1000;
  if(this->timeout!=0){
    if (this->timeout <= 60000) //diferenciar entre seg. o min-hor
    { //verificar!!!
      this->time1 = 500;
      this->time2 = 900;
    }
  }
}
//------------------------------ Set AmpHour ---------------------------------//
void Control::setAmpHour(float valAmpHour){
  this->valAmpHour = valAmpHour;
  this->timeAH = this->valAmpHour / (this->val_control * 0.000277); //valor en seg
  if(this->timeAH !=0){
    if(this->timeAH <=60){
      this->time1 = 500; //valor ms
      this->time2 = 900;
    }
  }
}
//---------------------------- Set Temperature --------------------------------//
void Control::setTemperature(float maxTemp,float minTemp){
  this->maxTemp = maxTemp;
  this->minTemp = minTemp;
}

//----------------------------- Control-Run ------------------------------------//
void Control::run() {
  this->state = 1;
  flagCapt = false;
  digitalWrite(LedRelay, HIGH);
  if((this->prevstate == 0 || this->prevstate == 3 || this->prevstate == 4 || this->prevstate == 1) && this->state == 1){
    control_cbuff("R");
    this->flagP = false;
    controlTime.start();
    Debug4.println(F("RUN"));
  }
}
//----------------------------- Control-Pause ---------------------------------//
void Control::pause() {
  this->state = 2;
  this->flagEnable = true;
}
//----------------------------- Control-Stop -----------------------------------//
void Control::stop() {
  this->state = 3;
  //digitalWrite(LED_BUILTIN, LOW);
}
//--------------------------- Control Run-Pause --------------------------------//
void Control::runPause(){
  this->state = 4;

  valcurrent = 0;
  valvoltage = 0;
  valtemp = 0;

  controlTime.stop();
  while(this->valrampa > 0)
  {
    this->valrampa--;
    //dac.write(0xFFF-this->valrampa);
    //vTaskDelay(1);
    rampa();
  }

  digitalWrite(LedRelay, LOW);

  if((this->prevstate == 0 || this->prevstate == 3 || this->prevstate == 1 || this->prevstate == 4) && this->state == 4){
    controlTime.start();
  }
}

void Control::rampa(){
  //dac.write(0xFFF-this->valrampa);
  dac.write(this->valrampa);
  vTaskDelay(1);
}

void Control::control_cbuff(const char* stepLetter){
  memset(stepState,0,2);
  strcat(stepState,stepLetter);
}
//------------------------------- Control-StepPause ----------------------------//
void Control::stepPause(unsigned long steptime){
  this->steptime = steptime * 1000;
}
//------------------------------ Control-states --------------------------------//
bool Control::states(){
  return this->flagS;
}
//---------------------------- Control-ReadData --------------------------------//
void Control::readData(){
  this->averageCurrent = 0;
  this->averageVoltage = 0;
  this->averageTemp  = 0;

  for(int i = 0; i < 70; i++) //promedio Current,Voltage,Temperature //40
  {
    this->averageCurrent = this->averageCurrent + analogRead(A0);
    this->averageVoltage = this->averageVoltage + analogRead(A1);
    this->averageTemp = this->averageTemp + analogRead(A2);
  }
  this->averageCurrent = this->averageCurrent / 70;//40
  this->averageVoltage = this->averageVoltage / 70;
  this->averageTemp = this->averageTemp / 70;

  this->valcurrent0 = this->averageCurrent - 70.0; //36
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
  if(this->state == 1)
  {
      if(controlTime.isRunning())//Program running
      {
        if(this->timeout!=0){ //Running with time
          if(controlTime.ms() < this->timeout)
          {
              if(this->flagTemp != false){
                this->flagTemp = false;
                this->t2 = controlTime.ms() + 30000;
                Debug4.print(F("t2:"));
                Debug4.println(this->t2);
              }
              //------------------------- control current ------------------------//
              if(valcurrent < this->val_control)
              {
                if (this->valrampa<0xFFF)
                  this->valrampa++;
              }

              if(valcurrent > this->val_control)
              {
                if(this->valrampa > 0)
                  this->valrampa--;
              }
              //dac.write(0xFFF-this->valrampa);
              dac.write(this->valrampa);
              vTaskDelay(1);
             //------------------------- current Warning -------------------------//
             /*if(controlTime.ms() > this->time1 && controlTime.ms() > this->t2)//toma una muestra cada 1.2s
             {
                Debug4.println(F("save0"));
                this->time1 = this->time1 + this->time2;
                this->tmpVal = valcurrent - 7; //guarda valor temp 3
                if (this->tmpVal<=0 || this->tmpVal>=valcurrent)
                  control_cbuff("W");
             }

             if(this->tmpVal<=0 && controlTime.ms() > this->time2 && controlTime.ms() > this->t2)
             {
               this->flagO = true;
               Debug4.print(F("tmval0"));
               if (this->flagO !=false){
                 control_cbuff("W");
                 this->flagS = true; //flag manda a state = Stop
                 Debug4.println(F("dentro"));
               }
             }

             else if(this->tmpVal>=valcurrent && controlTime.ms() > this->time2 && controlTime.ms() > this->t2)
             {
               if (this->flagO !=true){
                 control_cbuff("W");
                 this->flagO = true;
                 this->t1 = controlTime.ms() + 20000;
                 Debug4.print(F("T1me:"));
                 Debug4.println(this->t1);
               }
               else if(controlTime.ms() > this->t1 && this->flagO != false){
                 this->flagS = true;
                 //Debug4.print(F("ElseTime:"));
               }
             }*/
             //-------------------------- control temp -----------------------//
             //else if(this->maxTemp!=0){
             if(this->maxTemp!=0){
               if(valtemp > this->maxTemp){ //se va a pause
                 this->state = 2; Debug4.println(F("Pause-temp"));
                 control_cbuff("T");
                 this->flagEnable = true;
                 this->flagTemp = true;
               }
             }
          }
          else
          {
            Debug4.println(F("timeout-agotado"));
            this->t1 = 0;
            this->t2 = 0;
            this->time1 = 60000;
            this->time2 = 90000;
            this->tmpVal = 0;
            this->Ttime0 = Ttime;
            Ttime = this->Ttime0 + (controlTime.ms()*0.001);
            totAH = totAH + valAH;
            controlTime.stop();
            flagStep=true;
          }
        }
        else
        {
          //running with Amp-hour
          if(valAH < this->valAmpHour)
          {
             if(this->flagTemp != false){
               this->flagTemp = false;
               this->t2 = controlTime.ms() + 30000;
               //Debug4.print(F("t2:"));
               //Debug4.println(this->t2);
             }
             //---------------------- control current ------------------------//
             if(valcurrent < this->val_control)
             {
               if(this->valrampa<0xFFF)
                 this->valrampa++;
             }

             if(valcurrent > this->val_control)
             {
               if(this->valrampa > 0)
                 this->valrampa--;
             }
             //dac.write(0xFFF-this->valrampa);
             dac.write(this->valrampa);
             vTaskDelay(1);

             //---------------------- current Warning ------------------------//
             /*if(controlTime.ms() > this->time1 && controlTime.ms() > this->t2)
             {
               Debug4.print(F("save1"));
               this->time1 = this->time1 + this->time2;
               this->tmpVal = valcurrent - 7; //guarda valor temp
               if (this->tmpVal<=0 || this->tmpVal>=valcurrent)
                 control_cbuff("W");
             }

             if(this->tmpVal<=0 && controlTime.ms() > this->time2 && controlTime.ms() > this->t2)
             {
               this->flagO = true;
               //Debug4.print(F("tmval1"));
               if (this->flagO !=false){
                 control_cbuff("W");
                 this->flagS = true; //flag manda a state = Stop
                 //Debug4.println(F("dentro2"));
               }
             }

             else if(this->tmpVal>=valcurrent && controlTime.ms() > this->time2 && controlTime.ms() > this->t2)
             {
               if (this->flagO !=true){
                 control_cbuff("W");
                 this->flagO = true;
                 this->t1 = controlTime.ms() + 20000;
                 //Debug4.print(F("T1me2:"));
                 //Debug4.println(this->t1);
               }
               else if(controlTime.ms() > this->t1 && this->flagO != false){
                 this->flagS = true;
               }
             }*/

             //------------------------- control Temp -------------------------//
             if(this->maxTemp!=0){
               if(valtemp > this->maxTemp){ //se va a pause
                 this->state = 2; Debug4.println(F("Pause-temp"));
                 control_cbuff("T");
                 this->flagEnable = true;
                 this->flagTemp = true;
               }
             }
          }
          else
          {
            Debug4.println(F("timeout-agotado"));
            this->t1 = 0;
            this->t2 = 0;
            this->time1 = 60000;
            this->time2 = 90000;
            this->tmpVal = 0;
            this->Ttime0 = Ttime;
            Ttime = this->Ttime0 + (controlTime.ms()*0.001);
            totAH = totAH + valAH;
            controlTime.stop();
            flagStep=true;
          }
        }
      }
  }

//----------------------------------- Step Pause ----------------------------------//
  if(this->state == 4)
  {
    if(controlTime.isRunning())
    {
      if(controlTime.ms() > this->steptime)
      {
        Debug4.println(F("stepPause out"));
        flagStep=true;
        this->Ttime0 = Ttime;
        Ttime = this->Ttime0 + (controlTime.ms()*0.001);
        controlTime.stop(); //*
      }
    }
      vTaskDelay(1);
  }

//------------------------------------- Stop ---------------------------------------//
  if(this->state == 3)
  {
    if(this->prevstate != 3){
      this->time1 = 60000;
      this->tmpVal = 0;
      this->t1 = 0;
      this->t2 = 0;
      this->flagPause = false;
      this->flagEnable = false;

      controlTime.stop();
      Debug4.println(F("STOP"));
    }
    if(this->flagS != false){
      dac.write(0xFFF);
      vTaskDelay(1);
      this->flagO = false;
      this->flagS = false;
      this->valrampa = 0;
      Debug4.println(F("flagS"));
    }
    if(this->valrampa > 0)
    {
      this->valrampa--;
      //dac.write(0xFFF-this->valrampa);
      dac.write(this->valrampa);
      vTaskDelay(1);
    }
    else{
      this->state = 0;
      flagCapt = true;
      control_cbuff("S");
      digitalWrite(LedRelay, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

//----------------------------------- Pause ---------------------------------------//
  if(this->state == 2 && this->flagEnable !=false)
  {
    if(this->prevstate!= 2){
      Debug4.println(F("PAUSE"));
      controlTime.pause();
      if(this->flagTemp != false){
        Debug4.println(F("flagPause"));
        this->flagPause = true;
        this->flagP = false;
      }
      else{
        this->flagP = true;
      }
    }
    if(this->valrampa > 0)
    {
      this->valrampa--;
      //dac.write(0xFFF-this->valrampa);
      dac.write(this->valrampa);
      vTaskDelay(1);
    }
    else{
      digitalWrite(LedRelay, LOW); //solo digitalWrite
      control_cbuff("P");
      this->flagPause = false;
      this->flagEnable = false;
    }
  }

  //------------------------------ Temperature good ------------------------------//

  if(this->prevstate == 2 &&this->state == 2 && this->flagPause !=true && this->flagP != true){
    if(valtemp <= minTemp){
      this->state = 1;
      control_cbuff("R");
      Debug4.println(F("good-temp"));
    }
  }

//------------------------------- Run-play after pause ----------------------------//
  if(this->prevstate == 2 && (this->state == 1 || this->state == 4))
  {
    controlTime.play();
    control_cbuff("R");
    this->t2 = controlTime.ms() + 30000;
    if(this->state==4)digitalWrite(LedRelay, LOW);
    if(this->state==1)digitalWrite(LedRelay, HIGH);
  }

  this->prevstate = this->state;
}
