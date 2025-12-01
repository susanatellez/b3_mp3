#include "cmsis_os2.h"
#include "com.h"
#include "test_com.h"
#include <stdio.h>

osThreadId_t tid_Th_Test_Com;

extern osMessageQueueId_t mid_MsgQueueCom;

void thTestCom (void *argument);

int InitTestCom (void){
    tid_Th_Test_Com = osThreadNew(thTestCom, NULL, NULL);
  if (tid_Th_Test_Com == NULL) {
    return(-1);
  }	
  return(0);
}


void thTestCom (void *argument){
  char mensaje[32];
  uint8_t cont = 0;
  while(1){
    sprintf(mensaje,"/n Mensaje numero:%d",cont);
    osMessageQueuePut(mid_MsgQueueCom, &mensaje, 0, 0);
    cont = (cont<10) ? cont + 1 : 0;
     osDelay(1000);
  }
}

  
