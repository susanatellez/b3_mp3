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
  
  uint8_t cont = 0;
  uint16_t len;
  t_msgcom msg;
  while(1){
    len = sprintf(msg.mensaje," Mensaje numero:%d\n",cont);
    msg.longitud = len;
    osMessageQueuePut(mid_MsgQueueCom, &msg, 0, 0);
    cont = (cont<10) ? cont + 1 : 0;
     osDelay(1000);
  }
}

  
