#include "cmsis_os2.h" // CMSIS RTOS header file
#include "com.h"
#include "Driver_USART.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThCom;                        // thread id
osThreadId_t tid_ThComRx;
osMessageQueueId_t  mid_MsgQueueCom;
osMessageQueueId_t  mid_MsgQueueComRx;
//DRIVER UART
extern ARM_DRIVER_USART Driver_USART3;
ARM_DRIVER_USART *USARTdrv = &Driver_USART3;
void ComPC (void *argument);                   // thread function
void ComPCRx (void *argument);
int Init_MsgQueue_Com(void);
int Init_MsgQueue_ComRx(void);
int Init_ThCom(void);
int Init_ThComRx(void);
void USART_Callback_PC(uint32_t event);

extern osMessageQueueId_t mid_MsgQueue_Joy;

//Mensaje a enviar
//char msg[32];
  uint8_t orden[1];
void Init_ModCom(void){
  Init_ThCom();
  Init_MsgQueue_Com();
    Init_ThComRx();
  Init_MsgQueue_ComRx();
}

//Inicialización del hilo 
int Init_ThCom (void) {

  const osThreadAttr_t ThCom_attributes = {
    .name = "ThCom",
    .stack_size = 256U, // <- Nuevo tamaño
  };

  tid_ThCom = osThreadNew(ComPC, NULL, &ThCom_attributes);
  if (tid_ThCom == NULL) {
    return(-1);
  }
 
  return(0);
}

int Init_ThComRx (void) {

  const osThreadAttr_t ThCom_attributes = {
    .name = "ThComRx",
    .stack_size = 256U, // <- Nuevo tamaño
  };

  tid_ThComRx = osThreadNew(ComPCRx, NULL, &ThCom_attributes);
  if (tid_ThComRx == NULL) {
    return(-1);
  }
 
  return(0);
}
 
//Hilo de comunicación con el PC
void ComPC (void *argument) {
  t_msgcom msg;
  USARTdrv->Initialize(USART_Callback_PC);
  USARTdrv->PowerControl(ARM_POWER_FULL);
  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|ARM_USART_DATA_BITS_8|
                    ARM_USART_PARITY_NONE|ARM_USART_STOP_BITS_1|ARM_USART_FLOW_CONTROL_NONE,9600);
  USARTdrv->Control(ARM_USART_CONTROL_RX,1);
  USARTdrv->Control(ARM_USART_CONTROL_TX,1);
  
  while (1) {
  if(osMessageQueueGet(mid_MsgQueueCom,&msg,NULL,10U)==osOK){
  USARTdrv->Send(msg.mensaje,75);
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  }
   //osThreadYield();                            // suspend thread
  }
}

void ComPCRx(void *argument){
uint8_t tecla;
  while(1){
  USARTdrv->Receive(orden,sizeof(orden));
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  switch(orden[0]){
    case 0x70:
      tecla = 0x05;
      osMessageQueuePut(mid_MsgQueue_Joy,&tecla,0,0);
    break;
  }
  }
}
void USART_Callback_PC(uint32_t event){
  uint32_t mask;
  mask =  ARM_USART_EVENT_TRANSFER_COMPLETE|
  ARM_USART_EVENT_SEND_COMPLETE | ARM_USART_EVENT_TX_COMPLETE;
  if (event & mask ){
    osThreadFlagsSet(tid_ThCom,0x01);
  }
  if(event & ARM_USART_EVENT_RECEIVE_COMPLETE ){
    osThreadFlagsSet(tid_ThComRx,0x01);
  }
}
int Init_MsgQueue_Com(void) {
 
  mid_MsgQueueCom= osMessageQueueNew(4,sizeof(t_msgcom), NULL);                              
  if (mid_MsgQueueCom == NULL) {
    return (-1);
  }
  return(0);
}

int Init_MsgQueue_ComRx(void) {
 
  mid_MsgQueueComRx= osMessageQueueNew(4,sizeof(t_msgcom), NULL);                              
  if (mid_MsgQueueComRx == NULL) {
    return (-1);
  }
  return(0);
}

