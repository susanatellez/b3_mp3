#include "cmsis_os2.h"        // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "temp.h"                                                           //

/*******************************************************************************
Código para testear el módulo
********************************************************************************/

osThreadId_t tid_Th_Temp_Test;
//int Init_Th_Joy_Test(void);                                                   //lo muevo al .H
void Th_Temp_Test(void *argument);
extern osMessageQueueId_t mid_MsgQueueTemp;


osStatus_t statusQueueTemp;

float tempExtr = 0;                                               //mensaje que sacas de la cola

//Creación del hilo que testea el funcionamiento de las pulsaciones del joystick
int Init_Th_Temp_Test(void) {

  const osThreadAttr_t ThTest_attributes = {
    .name = "ThTest",
    .stack_size = 128U, // <- Nuevo tamaño
  };

  tid_Th_Temp_Test = osThreadNew(Th_Temp_Test, NULL, &ThTest_attributes);
  if (tid_Th_Temp_Test == NULL) {
    return(-1);
  }
  return(0);

}


void Th_Temp_Test (void *argument) {
  
  while (1) {

    statusQueueTemp = osMessageQueueGet(mid_MsgQueueTemp, &tempExtr, NULL, 10U);
  }
}