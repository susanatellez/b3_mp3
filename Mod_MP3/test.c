#include "cmsis_os2.h"
#include "mp3.h"
#include "test.h"
#include <stdio.h>

osThreadId_t tid_Th_Test_Mp3;

extern osMessageQueueId_t mid_MsgQueueMp3;

void thTestMp3 (void *argument);
t_comando msgTest;

int InitTestMp3 (void){
  
    tid_Th_Test_Mp3 = osThreadNew(thTestMp3, NULL, NULL);
  if (tid_Th_Test_Mp3 == NULL) {
    return(-1);
  }	
  return(0);
}


void thTestMp3 (void *argument){
    //Volumen al maximo
    msgTest.tipo = 0;
    msgTest.com = 0x06;
    msgTest.dat1= 0x00;
    msgTest.dat2= 0x05;
    osMessageQueuePut(mid_MsgQueueMp3, &msgTest, 0, 0);
    
//    //PLAY
//    msgTest.tipo = 0;
//    msgTest.com = 0x0D;
//    msgTest.dat1= 0x00;
//    msgTest.dat2= 0x00;
//    osMessageQueuePut(mid_MsgQueueMp3, &msgTest, 0, 0);
//    osDelay(2000);
    
    //Manda la primera cancion
    msgTest.tipo = 0;
    msgTest.com = 0x0F;
    msgTest.dat1= 0x01;
    msgTest.dat2= 0x01;
    osMessageQueuePut(mid_MsgQueueMp3, &msgTest, 0, 0);


  while(1){

    osDelay(10000);
    msgTest.tipo = 0;
    msgTest.com = 0x01;
    msgTest.dat1= 0x00;
    msgTest.dat2= 0x00;
    osMessageQueuePut(mid_MsgQueueMp3, &msgTest, 0, 0);

//    //Pone pausa
//    msg.com = 0x0E;
//    msg.dat1= 0x00;
//    msg.dat2= 0x00;
//    osMessageQueuePut(mid_MsgQueueMp3, &msg, 0, 0);
//    osDelay(10000);
//    //Da al play
//    msg.com = 0x0D;
//    msg.dat1= 0x00;
//    msg.dat2= 0x00;
//    osMessageQueuePut(mid_MsgQueueMp3, &msg, 0, 0);
//    osDelay(10000);
  }
}
