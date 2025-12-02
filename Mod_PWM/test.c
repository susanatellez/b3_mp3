#include "cmsis_os2.h"                                                          // CMSIS RTOS header file

#include "test.h"                                                               //
#include "PWM.h"                                                                //

/*******************************************************************************
 *      Thread para testear el MÓDULO PWM OUTPUT
 *******************************************************************************/

// HILO DEL TEST
osThreadId_t tid_ThTest;                                                        // thread id
//int Init_ThTest (void);
void ThTest (void *argument);                                                   // thread function

// COLA DEL TEST
extern osMessageQueueId_t mid_MsgQueue_PWM;

uint8_t MsgQueue_Test;


int Init_ThTest (void) {

  const osThreadAttr_t ThTest_attributes = {
    .name = "ThTest",
    .stack_size = 256U, // <- Nuevo tamaño
  };

  tid_ThTest = osThreadNew(ThTest, NULL, &ThTest_attributes);
  if (tid_ThTest == NULL) {
    return(-1);
  }
  return(0);
}


void ThTest (void *argument) {
  while (1) {

//    MsgQueue_Test = 0x01;                                                       //DO
//    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
//    osDelay(200);

//    MsgQueue_Test = 0x10;                                                       //MI
//    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
//    osDelay(200);

//    MsgQueue_Test = 0x11;                                                       //SOL
//    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
//    osDelay(200);

//    MsgQueue_Test = 0x10;                                                       //MI
//    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
//    osDelay(200);

//    MsgQueue_Test = 0x01;                                                       //DO
//    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
//    osDelay(200);

//    MsgQueue_Test = 0x00;                                                       //SILENCIO
//    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
//    osDelay(1000);

    MsgQueue_Test = 0x01;                                                       //DO
    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
    osDelay(200);

    MsgQueue_Test = 0x00;                                                       //SILENCIO
    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

    MsgQueue_Test = 0x11;                                                       //SOL
    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
    osDelay(200);

    MsgQueue_Test = 0x00;                                                       //SILENCIO
    osMessageQueuePut(mid_MsgQueue_PWM, &MsgQueue_Test, 0U, 0U);
    osDelay(1000);

  }
}



