#include "cmsis_os2.h" 
#ifndef __MP3_H
#define __MP3_H
typedef struct{
  uint8_t com;
  uint8_t dat1;
  uint8_t dat2;
}t_comando;

void Init_ModMP3 (void);
#endif