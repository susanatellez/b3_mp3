#include "cmsis_os2.h"        // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "pot.h"                                                                //

/*******************************************************************************
Código para testear el módulo
********************************************************************************/

#define VREF 3.3f
#define RESOLUTION_12B 4096.0f

osThreadId_t tid_Th_Joy_Test;
//int Init_Th_Joy_Test(void);                                                   //lo muevo al .H
void Th_Joy_Test(void *argument);

extern osMessageQueueId_t mid_MsgQueue_POT;
osStatus_t statusQueueTest;
uint8_t MsgQueue_test = 0x00;                                                //mensaje que sacas de la cola

//Creación del hilo que testea el funcionamiento de las pulsaciones del joystick
int Init_Th_Joy_Test(void) {

  const osThreadAttr_t ThTest_attributes = {
    .name = "ThTest",
    .stack_size = 512U,                                                   //Habrá que cambiarlo
  };

  tid_Th_Joy_Test = osThreadNew(Th_Joy_Test, NULL, &ThTest_attributes);
  if (tid_Th_Joy_Test == NULL) {
    return(-1);
  }
  return(0);

}


void Th_Joy_Test (void *argument) {

  while (1) {

    statusQueueTest = osMessageQueueGet(mid_MsgQueue_POT, &MsgQueue_test, NULL, 10U);
    osDelay(1000);

  }
}

