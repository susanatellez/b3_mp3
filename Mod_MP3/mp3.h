#include "cmsis_os2.h" 
#ifndef __MP3_H
#define __MP3_H
typedef struct{
  uint8_t com;  //Numero azul del datasheet
  uint8_t dat1; //Sexto número del comado (En negrita)
  uint8_t dat2; //Séptimo numero del comando (En negrita)
}t_comando;

void Init_ModMP3 (void);
#endif