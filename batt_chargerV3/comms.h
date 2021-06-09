#ifndef COMMS_H_
#define COMMS_H_
#include "control.h"
//#include "cfeeprom.h"
//#include "program.h"
//#include "Timer.h"
#include "globals.h"

extern char rcvchar;
extern bool flagcommand;
extern bool flagbuff;
//*extern Program program;
extern char timehms[9];
extern char timehmsa[9];

//*extern char timehms;
//*extern char timehmsa;

void comms_inicbuff(void);        // Borra buffer
int comms_addcbuff(char c);       // añade carácter recibido al buffer
void comms_procesa_comando(void); // Procesa comando
//int aux_crc(String val);

const char* aux_time(void);

#endif
