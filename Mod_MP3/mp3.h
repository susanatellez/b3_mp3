#include "cmsis_os2.h" 
#ifndef __MP3_H
#define __MP3_H

typedef struct{
  uint8_t tipo; //0 si es de envio de comando normal, 1  si es con respuesta y necesita comando de envio p.ej ver nº carpetas
  uint8_t com;  //Numero azul del datasheet
  uint8_t dat1; //Sexto número del comado (En negrita)
  uint8_t dat2; //Séptimo numero del comando (En negrita)
}t_comando;
typedef struct{
  uint8_t tipo; //0 si es de tipo hay o no tarjeta, si es peticion de dato nº de algo, es el nº de comando
  uint8_t info; //0 no hay tarjeta, 1 hay tarjeta, numero de carpetas, archivos o lo que sea
}t_respuesta;
void Init_ModMP3 (void);
#endif