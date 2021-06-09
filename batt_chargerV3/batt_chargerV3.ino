#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "comms.h"
//#include "xmDAC.h"
//#include "Timer.h"
#include "globals.h"
#include <Wire.h>
#include <Arduino.h>

void vUARTTask(void *pvParameters);
void vLEDFlashTask1(void *pvParameters);
void vCONTROLTask2(void *pvParameters);

//StopWatch controlTime;
Control control;
//Program program;
//I2CEEPROM i2c_eeprom(0x50);

//Timer t;

//int disk1 = 0x50;    //24LC256 eeprom chip program-default
//int disk2 = 0x57;    //24LC256 eeprom chip Address

unsigned int address = 1;

char rcvchar=0x00;   // último carácter recibido
bool flagcommand=false;
bool flagbuff = false;
bool flagStep=false;


double valcurrent = 0.0; //solo para mostrar
double valvoltage = 0.0;
double valtemp = 0.0;
float valAH = 0.0;

//*int count = 0;
//*const char* letter = "Off"; //0x49; //I char
//*char letter[3] = "Off";

char stepState[2] = "I";
char timehms[9] = "00:00:00";
char timehmsa[9] = "00:00:00";


//const char* timehms = "00:00:00";
//*String timehms = "00:00:00";
//*String timehmsa = "00:00:00";
//*String nameProg= "";

//char timehms[9];
//char timehmsa[9];
//char nameProg[5];

/*unsigned long Ttime = 0;
unsigned long timeC = 0;
unsigned long timeAc = 0;

int seg = 0;
int mint = 0;
int temMin = 0;
int hor = 0;

String mint0 = "";
String hor0 = "";
String seg0 = "";*/

int LedComms = 17; //C5 activa o desactiva comm
int LedRelay = 20; //D0 activa o desactiva relevador

int ledvolt = 0;

char anbu[1024];  //cfg json
char type[17][10]; //inicio, pausa,carga, fin
float duration[17]; //time
float AmperH[17];
float current[17];
float maxtemp[17];
float mintemp[17];

void setup()
{
  //baudios
  Debug.begin(115200); //115200
  Serial1.begin(115200);
  Wire.begin();
  control.begin();

  const char compile_date[] = __DATE__ " " __TIME__; //fecha-hora-compilacion
  Debug.println(compile_date);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LedComms, OUTPUT);
  pinMode(LedRelay, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LedComms, LOW);
  digitalWrite(LedRelay, LOW);

  //delay(3000);
  //t.every(1000,printMessage); //mostrar valores

  xTaskCreate(vUARTTask,
              (signed portCHAR *)"UARTTask",
              256,
              NULL,
              tskIDLE_PRIORITY + 2,
              NULL);

  xTaskCreate(vLEDFlashTask1,
              (signed portCHAR *)"Task1",
              configMINIMAL_STACK_SIZE,
              //128,
              NULL,
              tskIDLE_PRIORITY + 2,
              NULL);

  xTaskCreate(vCONTROLTask2,
              (signed portCHAR *)"Task2",
              128,
              NULL,
              tskIDLE_PRIORITY + 3,
              NULL);

  vTaskStartScheduler();
}

void vLEDFlashTask1(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    vTaskDelay(500);
    digitalWrite(27, !HIGH);
    vTaskDelay(500);
    digitalWrite(27, !LOW);
    //control.readData();
  }
}


void vCONTROLTask2(void *pvParameters) {
  (void) pvParameters;
  for (;;) {
    //control.readData();
    Debug.println("ControlreadData");
    vTaskDelay(1);
  }
}

void vUARTTask(void *pvParameters) {
  (void) pvParameters;
  //t.every(1000,printMessage);
  for (;;) {
    //t.update();
    /*if(millis()%1000==0){
      printMessage();
    }*/
    if (flagcommand)comms_procesa_comando();
    serialEvent1();
    vTaskDelay(1);
  }
}

void loop()
{
}

/*
void printMessage()
{
    timeC = controlTime.ms()*0.001;
    timehms = currentTime(timeC);

    timeAc = Ttime + (controlTime.ms()*0.001);
    timehmsa = currentTime(timeAc);

    Debug.print("time: ");
    Debug.print(timehms);
    Debug.print(", stopwatch : ");
    Debug.print(controlTime.ms()*0.001);
    Debug.print(", Ttime : ");
    Debug.print(Ttime);
    Debug.print(", AH : ");
    Debug.println(valAH);
}
//---------------------- funcion para formato hor:min:seg ----------------------//
String currentTime(unsigned long temSeg){
  //temSeg = controlTime.ms()*0.001;
  if(temSeg > 3599){
    hor = temSeg / 3600; // horas
    temMin = temSeg - (hor * 3600);
    hor0 = String(hor)+":";
    if (temMin > 59){
      mint = temMin / 60; //minutos
      seg = temMin - (mint * 60); //segundos
      if(seg<10){
        seg0 = "0"+String(seg);
      }
      else{
        seg0 = String(seg);
      }
      if(mint<10){
        mint0 = "0"+String(mint)+":";
      }
      else{
        mint0 = String(mint)+":";
      }
    }
    else{
      seg = temMin;
      if(seg<10){
        seg0 = "0"+String(seg);
      }
      else{
        seg0 = String(seg);
      }
    }
  }
  else if (temSeg > 59){
    mint = temSeg / 60; //minutos
    seg = temSeg - (mint * 60); //segundos

    if (seg<10){
      seg0 = "0"+ String(seg);
    }
    else{
      seg0 = String(seg);
    }
    if(mint<10){
      mint0 = "0"+String(mint)+":";
    }
    else{
      mint0 = String(mint)+":";
    }
    hor0 = "00:";
  }
  else if(temSeg < 59){
    seg = temSeg;
    if (seg < 10){
      seg0 = "0"+String(seg);
    }
    else{
      seg0 = String(seg);
    }
    mint0 = "00:";
    hor0 = "00:";
  }

  return hor0+mint0+seg0;
  //timehms = hor0+mint0+seg0;
}
*/
//------------------ Interrupción recepción serie USART ------------------------//
void serialEvent1(){
    flagbuff = false;
    rcvchar=0x00;                // Inicializo carácter recibido
    while(Serial1.available()){  // Si hay algo pendiente de recibir ...
      rcvchar=Serial1.read();    // lo descargo y ...
      comms_addcbuff(rcvchar);   // lo añado al buffer y ...
    }
}
