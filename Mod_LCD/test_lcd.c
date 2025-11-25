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
    mensaje.linea1 = 0;
    sprintf(mensaje.texto1, "Linea de prueba 1");
    mensaje.linea2 = 2;
    sprintf(mensaje.texto2, "Hoooolaaaaa  =) ");

    osMessageQueuePut(mid_MsgQueueLCD, &mensaje, 0, 0);
    osDelay(3000);
    mensaje.linea1 = 0;
    sprintf(mensaje.texto1, "Linea de prueba 2");
    mensaje.linea2 = 2;
    sprintf(mensaje.texto2, "Adiooooos  =( ");

    osMessageQueuePut(mid_MsgQueueLCD, &mensaje, 0, 0);
     osDelay(3000);
  }
}

  

