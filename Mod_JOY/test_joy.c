#include "cmsis_os2.h"        // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "joystick.h"                                                           //

/*******************************************************************************
Código para testear el módulo
********************************************************************************/

osThreadId_t tid_Th_Joy_Test;
//int Init_Th_Joy_Test(void);                                                   //lo muevo al .H
void Th_Joy_Test(void *argument);
extern osMessageQueueId_t mid_MsgQueue_Joy;

uint32_t sizeQueueJoy = 0;
osStatus_t statusQueueJoy;

uint8_t MsgQueue_JoyExtr = 0x00;                                                //mensaje que sacas de la cola

//Creación del hilo que testea el funcionamiento de las pulsaciones del joystick
int Init_Th_Joy_Test(void) {

  tid_Th_Joy_Test = osThreadNew(Th_Joy_Test, NULL, NULL);
  if (tid_Th_Joy_Test == NULL) {
    return(-1);
  }
  return(0);

}


void Th_Joy_Test (void *argument) {
  
  while (1) {
    //con el osDelay y ayuda del watches veríamos si la cola de mensajes funciona bien
    //osDelay(2000);                                                              //comentar para que funcione normal
    sizeQueueJoy = osMessageQueueGetCount (mid_MsgQueue_Joy);
    statusQueueJoy = osMessageQueueGet(mid_MsgQueue_Joy, &MsgQueue_JoyExtr, NULL, 10U);
  }
}
