#include "cmsis_os2.h"                                                          // CMSIS RTOS header file

#include "test.h"                                                               //
#include "rgb.h"                                                                //

/*******************************************************************************
 *      Thread para testear el MÓDULO PWM OUTPUT
 *******************************************************************************/

// HILO DEL TEST
osThreadId_t tid_ThTest;                                                        // thread id
//int Init_ThTest (void);
void ThTest (void *argument);                                                   // thread function

// COLA DEL TEST
extern osMessageQueueId_t mid_MsgQueue_RGB;
uint8_t MsgQueue_Test;


int Init_ThTest (void) {

  const osThreadAttr_t ThTest_attributes = {
    .name = "ThTest",
    .stack_size = 128U, // <- Nuevo tamaño
  };

  tid_ThTest = osThreadNew(ThTest, NULL, &ThTest_attributes);
  if (tid_ThTest == NULL) {
    return(-1);
  }
  return(0);
}


void ThTest (void *argument) {
  while (1) {

    MsgQueue_Test = 0x01;                                                       //ROJO
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x02;                                                       //AMARILLO
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x03;                                                       //VERDE
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x04;                                                       //AZUL OSCURO
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x05;                                                       //AZUL CIAN
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x06;                                                       //ROSITA
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x07;                                                       //BLANCO
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x00;                                                       //
    osMessageQueuePut(mid_MsgQueue_RGB, &MsgQueue_Test, 0U, 0U);
    osDelay(3000);

  }
}