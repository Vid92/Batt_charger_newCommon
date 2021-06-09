#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "comms.h"
#include <Arduino.h>

#include "crc.h"

int const lenbuff=1024; // Longitud de buffer, Ajustar 128 512 1024

unsigned int myaddress = 1;   //255
const char beginchar = 0x02; //inicio
const char endchar = 0x04;   //fin
const char runchar = 0x33;   //run
const char pausechar = 0x34; //pause
const char stopchar = 0x35;  //stop
const char combchar = 0x43;  //C = I,V,T
const char writechar = 0x57; //W
const char Ping = 0x64;  //default

/*const char voltchar = 0x56;  //V
const char tempchar = 0x54;  //T
const char stepchar = 0x50;  //P
const char timechar = 0x74;  //t
const char readchar = 0x52;  //R*/

bool flagload = false;
bool flagrun = false;
bool flagpause = false;
float totalTime;
//String totalhms; //total time formato "00:00:00"

//unsigned long Ttime = 0; //variable acum.tiempo
float TempTime1 = 0; // aux AH acumulado
float totAH = 0.0;   //variable acum.AH
int xbuff=0;      // Índice: siguiente char en cbuff
char cbuff[lenbuff]; // Buffer
//bool flagcommand=false;  // Flag para indicar comando disponible

//unsigned char tbuff[lenbuff] = {0};//1024
char tbuff[lenbuff];
char arg[lenbuff]; // Argumento de comando (si lo tiene)
char crc16_high = 0x00;
char crc16_low = 0x00;
int dato = 0;

char final[128];
unsigned char tmp_crc[128]={0};
int lenbff = 0;

//----------------------------- Variables control prueba-------------------------
int count = 0;
char letter[7] = "Off";
char totalhms[9] = "00:00:00"; //extern
char nameProg[6] = "1-SGL";

/*double valcurrent = 0.0; //solo para mostrar
double valvoltage = 0.0;
double valtemp = 0.0;
float valAH = 0.0;

char stepState[2] = "I";
char timehms[9] = "00:00:00";
char timehmsa[9] = "00:00:00";*/

char var_addr[5];
char var_current[5];
char var_voltage[5];
char var_temp[5];
char var_Ah[5];
char var_temptime1[6];
char var_count[3];

char var_run[3];
char var_pause[3];
char var_stop[3];
char var_pass[13] = "ACTION: PASS";
char var_passrun[17] = "ACTION: PASS,RUN";
char var_passpause[19] = "ACTION: PASS,PAUSE";
char var_passtop[18] = "ACTION: PASS,STOP";

char let_value[9] = "VALUE: I";
char let_voltage[3]= ",V";
char let_temp[3]= ",T";
char let_Ah[4]= ",AH";
char let_temptime1[4]= ",AC";
char let_count[3]= ",P";
char let_letter[3]= ",S";
char let_timehms[3]= ",t";
char let_timehmsa[4]= ",Tt";
char let_totalhms[4]= ",TT";
char let_nameProg[3]= ",N";
char let_coma[2] = ",";

char str_addr[5];
char str_current[5];
char str_voltage[5];
char str_temp[5];
char str_Ah[5];
char str_temptime1[6];
char str_count[3];

char str_run[3];
char str_pause[3];
char str_stop[3];
//-------------------------------- Funciones ----------------------------------//

void doCrc(int var_len);


//----------------------------- clean buffer ----------------------------------//
void comms_inicbuff(void){ // Inicia a 0 cbuff
  memset(cbuff,0,1024); //limpia
  xbuff=0;//0x00            // Inicializo el índice de siguiente
  flagbuff=true;                    // carácter
}

//--------------------------- add data to buffer -----------------------------//
int comms_addcbuff(char c){ // Añade a cbuff
  switch(c){
    case endchar:           // Enter -> Habilita Flag para procesar
      cbuff[xbuff++]=c;       // Añade carácter recibido al Buffer
      flagcommand=true;     // Comando en Main
      flagbuff=true;
      break;
    default:
     cbuff[xbuff++]=c;       // Añade carácter recibido al Buffer
     break;
  }
  return 0;
}

//----------------------------------- CRC -------------------------------------//
void doCrc(int var_len){
  lenbff = 0;       //limpia valores
  crc16_high = 0x00;
  crc16_low = 0x00;
  memset(tmp_crc,0,128);

  for(int i=0;i<var_len;i++){
    tmp_crc[i]=final[i];
    lenbff++;
  }
  dato = crc16_SingleBuf(tmp_crc,lenbff); //crc
  crc16_high = highByte(dato);
  crc16_low = lowByte(dato);
}

//---------------------------- Procesa comando -------------------------------//
void comms_procesa_comando(void){
  int i;
  int len=0;
  flagcommand=false;  // Desactivo flag de comando pendiente.
  flagbuff =false;

  for(i=0;i<lenbuff;i++){ // Bucle que pone a 0 todos los
    arg[i]=0x00;      //caracteres en el argumento
    tbuff[i]=0x00;
  }
    Debug.println("Leyendo...");

    //Debug.println(cbuff);
    if(cbuff[0]==beginchar&&cbuff[xbuff-4]==0x03&&cbuff[xbuff-1]==endchar){
      memset(final,0,128); //limpia antes de concatenar
      int n=3;
      for(int x=n;; x++){
        if(cbuff[x]==0x03)break;

        tbuff[x-n]=cbuff[x]; // a partir del 3er byte y hasta 0.
        len++;
      }

      //Debug.println(cbuff);
      //Debug.println(strlen(cbuff));
      /*Debug.println("--------------");
      Debug.println(tbuff);
      Debug.println(len);*/

      if(len<lenbuff){
        dato = crc16_SingleBuf((unsigned char*)tbuff,len);
        crc16_high = highByte(dato);
        crc16_low = lowByte(dato);
      }

      //----------------------------- validacion CRC ------------------------------//
      if(cbuff[xbuff-3]==crc16_low && cbuff[xbuff-2]==crc16_high) //validacion CRC*/
      {
        Debug.println("valido crc");
        if(cbuff[1]==myaddress) //address
        {
            //Debug.println("valido address");
            if(cbuff[2]==Ping){
              Debug.println("PING");
              //address and action:pass
              dtostrf(myaddress, 2, 0, str_addr);
              sprintf(var_addr,"%s",str_addr);
              strcat(final,var_addr);
              strcat(final,var_pass);

              doCrc(strlen(final)); //crc
              digitalWrite(17, HIGH); Serial1.write(2); Serial1.print(myaddress); Serial1.write("ACTION: PASS"); Serial1.write(3); Serial1.write(crc16_low);Serial1.write(crc16_high); Serial1.write(4);vTaskDelay(2); digitalWrite(17, LOW);
            }

            if(cbuff[2]==runchar){  //run
              Debug.println("Run");

              dtostrf(myaddress, 2, 0, str_addr);
              sprintf(var_addr,"%s",str_addr);
              dtostrf(51, 2, 0, str_run);
              sprintf(var_run,"%s",str_run);

              strcat(final,var_addr);
              strcat(final,var_run);
              strcat(final,var_passrun);

              doCrc(strlen(final)); //crc
              digitalWrite(17, HIGH); Serial1.write(2); Serial1.print(myaddress); Serial1.write(51); Serial1.write("ACTION: PASS,RUN"); Serial1.write(3); Serial1.write(crc16_low);Serial1.write(crc16_high); Serial1.write(4); vTaskDelay(2); digitalWrite(17, LOW);
            }

            if(cbuff[2]==pausechar){  //pause
              Debug.println("Pause");
              dtostrf(myaddress, 2, 0, str_addr);
              sprintf(var_addr,"%s",str_addr);
              dtostrf(52, 2, 0, str_pause);
              sprintf(var_pause,"%s",str_pause);

              strcat(final,var_addr);
              strcat(final,var_pause);
              strcat(final,var_passpause);

              doCrc(strlen(final));//crc
              digitalWrite(17, HIGH); Serial1.write(2); Serial1.print(myaddress); Serial1.write(52); Serial1.write("ACTION: PASS,PAUSE"); Serial1.write(3); Serial1.write(crc16_low); Serial1.write(crc16_high); Serial1.write(4); vTaskDelay(2); digitalWrite(17, LOW);
            }

            if(cbuff[2]==stopchar){ //stop
              Debug.println("Stop");

              dtostrf(myaddress, 2, 0, str_addr);
              sprintf(var_addr,"%s",str_addr);

              dtostrf(53, 2, 0, str_stop);
              sprintf(var_stop,"%s",str_stop);

              strcat(final,var_addr);
              strcat(final,var_stop);
              strcat(final,var_passtop);

              doCrc(strlen(final));
              digitalWrite(17, HIGH);Serial1.write(2);Serial1.print(myaddress); Serial1.write(53); Serial1.write("ACTION: PASS,STOP"); Serial1.write(3); Serial1.write(crc16_low); Serial1.write(crc16_high); Serial1.write(4); vTaskDelay(2); digitalWrite(17, LOW);
            }

            if(cbuff[2]==combchar){  //show I,V,T
              Debug.println("Datos");

              //TempTime1 = totAH + valAH;
              dtostrf(myaddress, 2, 0, str_addr);
              sprintf(var_addr,"%s",str_addr);
              dtostrf(valcurrent, 2, 2, str_current);
              sprintf(var_current,"%s",str_current);
              dtostrf(valvoltage, 2, 2, str_voltage);
              sprintf(var_voltage,"%s",str_voltage);
              dtostrf(valtemp, 2, 2, str_temp);
              sprintf(var_temp,"%s",str_temp);
              dtostrf(valAH, 2, 2, str_Ah);
              sprintf(var_Ah,"%s",str_Ah);
              dtostrf(TempTime1, 2, 2, str_temptime1);
              sprintf(var_temptime1,"%s",str_temptime1);
              dtostrf(count, 2, 0, str_count);
              sprintf(var_count,"%s",str_count);

              strcat(final,var_addr);
              strcat(final,let_value);
              strcat(final,var_current);
              strcat(final,let_voltage);
              strcat(final,var_voltage);
              strcat(final,let_temp);
              strcat(final,var_temp);
              strcat(final,let_Ah);
              strcat(final,var_Ah);
              strcat(final,let_temptime1);
              strcat(final,var_temptime1);
              strcat(final,let_count);
              strcat(final,var_count);
              strcat(final,let_letter);
              strcat(final,letter);
              strcat(final,let_timehms);
              strcat(final,timehms);
              strcat(final,let_timehmsa);
              strcat(final,timehmsa);
              strcat(final,let_totalhms);
              strcat(final,totalhms);
              strcat(final,let_nameProg);
              strcat(final,nameProg);
              strcat(final,let_coma);
              strcat(final,stepState);

              Debug.println(final);
              doCrc(strlen(final)); //crc

              /*Debug.print("crc16_high: ");
              Debug.println(crc16_high);
              Debug.print("crc16_low: ");
              Debug.println(crc16_low);*/

              digitalWrite(17, HIGH); Serial1.write(2);Serial1.print(myaddress);  Serial1.write("VALUE: ");Serial1.write("I");Serial1.print(valcurrent); Serial1.write(","); Serial1.write("V"); Serial1.print(valvoltage);Serial1.write(","); Serial1.write("T"); Serial1.print(valtemp);Serial1.write(",");Serial1.write("AH");Serial1.print(valAH);Serial1.write(",");Serial1.write("AC");Serial1.print(TempTime1);Serial1.write(",");Serial1.write("P"); Serial1.print(count);Serial1.write(",");Serial1.write("S");Serial1.write(letter);Serial1.write(","); Serial1.write("t"); Serial1.print(timehms);Serial1.write(",");Serial1.write("Tt"); Serial1.print(timehmsa); Serial1.write(","); Serial1.write("TT"); Serial1.print(totalhms); Serial1.write(",");Serial1.write("N"); Serial1.print(nameProg); Serial1.write(","); Serial1.print(stepState);Serial1.write(3); Serial1.write(crc16_low); Serial1.write(crc16_high); Serial1.write(4);vTaskDelay(2); digitalWrite(17, LOW);
            }

            if(cbuff[2]==writechar){  //writting eeprom Json
              Debug.println("Write");
              if(cbuff[8]==0x5B){ //3  OJO!! 28 0x6B = [
                int i = 3;
                do{ // Extraemos argumento del buffer
                  arg[ i - 3 ] = cbuff[i]; // a partir del 3er byte y hasta 0.
                }while (cbuff[++i]!=0x03);
                Debug.println("writingEEPROM");
                Debug.println(arg);

                dtostrf(myaddress, 2, 0, str_addr);
                sprintf(var_addr,"%s",str_addr);

                strcat(final,var_addr);
                strcat(final,"W");
                strcat(final,var_pass);

                doCrc(strlen(final));//crc
                digitalWrite(17, HIGH); Serial1.write(2); Serial1.print(myaddress); Serial1.write(writechar); Serial1.write("ACTION: PASS"); Serial1.write(3); Serial1.write(crc16_low); Serial1.write(crc16_high); Serial1.write(4); vTaskDelay(2); digitalWrite(17, LOW);
              }
            }

            /*if(cbuff[2]==readchar){ //reading eeprom -IDProgram
                Debug.println("readingIDProgram");
                clearProgram();
                IDread();
                //eepromread();
            }*/
        }
      }
      if(!flagbuff)comms_inicbuff(); // Borro buffer.
      //comms_inicbuff();
      //Debug.println("Procesado"); // Monitorizo procesado.

    }
}
