#include "cmsis_os2.h"
#include "mp3.h"
#include "test.h"
#include <stdio.h>

osThreadId_t tid_Th_Test_Mp3;

extern osMessageQueueId_t mid_MsgQueueMp3;

void thTestMp3 (void *argument);

int InitTestMp3 (void){
  
    tid_Th_Test_Mp3 = osThreadNew(thTestMp3, NULL, NULL);
  if (tid_Th_Test_Mp3 == NULL) {
    return(-1);
  }	
  return(0);
}


void thTestMp3 (void *argument){
  

  t_comando msg;
  while(1){
    //Manda la primera cancion
    msg.com = 0x03;
    msg.dat1= 0x00;
    msg.dat2= 0x01;
    osMessageQueuePut(mid_MsgQueueMp3, &msg, 0, 0);
   
    osDelay(10000);
    //Pone pausa
    msg.com = 0x0E;
    msg.dat1= 0x00;
    msg.dat2= 0x00;
    osMessageQueuePut(mid_MsgQueueMp3, &msg, 0, 0);
    osDelay(10000);
    //Da al play
    msg.com = 0x0D;
    msg.dat1= 0x00;
    msg.dat2= 0x00;
    osMessageQueuePut(mid_MsgQueueMp3, &msg, 0, 0);
    osDelay(10000);
  }
}
