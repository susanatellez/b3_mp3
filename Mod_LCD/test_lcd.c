#include "cmsis_os2.h"
#include "lcd.h"
#include "test_lcd.h"


osThreadId_t tid_Th_Test_LCD;

extern osMessageQueueId_t mid_MsgQueueLCD;

void thTestLCD (void *argument);

int InitTestLCD (void){
    tid_Th_Test_LCD = osThreadNew(thTestLCD, NULL, NULL);
  if (tid_Th_Test_LCD == NULL) {
    return(-1);
  }	
  return(0);
}


void thTestLCD (void *argument){
  MsgLCD_t mensaje;
  
  while(1){
  sprintf(mensaje.texto,"Linea enviada desde test");
  mensaje.linea = 1;
  osMessageQueuePut(mid_MsgQueueLCD, &mensaje,0,0);
  sprintf(mensaje.texto,"Prueba de escribir otra");
  mensaje.linea = 3;
  osMessageQueuePut(mid_MsgQueueLCD, &mensaje,0,0);
  }
}

  

