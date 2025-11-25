#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>


/*---------------------------------------------------------------------------------------------
----------------------------------- MODULO RELOJ ----------------------------------------------
----------------------------------------------------------------------------------------------*/


//Variables globales del reloj
uint8_t horas = 0 ;
uint8_t minutos = 0;
uint8_t segundos = 0;

//Hilo gestor del reloj
osThreadId_t tid_Clock;


void ThClock (void *argument);


int Init_ThClock (void){

    tid_Clock = osThreadNew(ThClock, NULL, NULL);
  if (tid_Clock == NULL) {
      return(-1);
    }
    return(0);
}

void ThClock (void *argument){
  
  while (1){
      osDelay(1000);
      if(segundos < 59){
        segundos++;
      }else if(minutos < 59){
        segundos = 0;
        minutos++;
      }else if(horas < 23){
        minutos = 0;
        segundos = 0;
        horas++;
      }else{
        horas = 0;
        minutos = 0;
        segundos = 0;
      }
  }
  
}
