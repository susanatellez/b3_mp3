#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "com.h"
#include "Driver_USART.h"
#include <stdio.h>
#include <string.h>
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThCom;                        // thread id
 
void ComPC (void *argument);                   // thread function
void USART_Callback(uint32_t event);
int Init_ThCom (void) {
 
  tid_ThCom = osThreadNew(ComPC, NULL, NULL);
  if (tid_ThCom == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ComPC (void *argument) {
 
  while (1) {

    osThreadYield();                            // suspend thread
  }
}

void USART_Callback(uint32_t event){
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_TRANSFER_COMPLETE|
  ARM_USART_EVENT_SEND_COMPLETE | ARM_USART_EVENT_TX_COMPLETE;
  if (event & mask ){
    osThreadFlagsSet(tid_ThCom,0x01);
  }
  
}
