#include "cmsis_os2.h" 
#ifndef __COM_H
#define __COM_H
typedef struct{
  uint16_t longitud;
  char mensaje[95];
}t_msgcom;
void Init_ModCom (void);
#endif