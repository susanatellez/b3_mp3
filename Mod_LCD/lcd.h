#ifndef __LCD_H
#define __LCD_H
#include "cmsis_os2.h"  
//Tipo de mensaje recibido por el LCD que indica linea y texto
typedef struct {
    uint8_t linea;
    char texto[32];

} MsgLCD_t;

void initModLCD (void);
#endif