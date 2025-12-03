#include "cmsis_os2.h" // CMSIS RTOS header file
#include "mp3.h"
#include "Driver_USART.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"



//******************************************************************************
//--------------------------------------MP3-------------------------------------
//******************************************************************************


//CABECERAS:
void USART_Callback(uint32_t event);
int Init_ThMP3(void);
void MP3 (void *argument);
int Init_MsgQueue_Com(void);
//Hilo y cola:
osThreadId_t tid_ThMp3;                        // thread id
osMessageQueueId_t  mid_MsgQueueMp3;

//Driver
extern ARM_DRIVER_USART Driver_USART2;
ARM_DRIVER_USART *USARTdrv = &Driver_USART2;

//Inicialización del módulo
void Init_ModMP3 (void){
  Init_ThMP3();
  Init_MsgQueue_Com();
}

//Inicialización del hilo
int Init_ThMP3(void){
  tid_ThMp3 = osThreadNew(MP3, NULL, NULL);
  if (tid_ThMp3 == NULL) {
    return(-1);
  }
 
  return(0);

}

//Inicializacion de la cola
int Init_MsgQueue_Com(void) {
 
  mid_MsgQueueMp3= osMessageQueueNew(4,1, NULL);                                //CAMBIAR VALOR DE (1) cuando se sepa el tamaño del mensaje de la trama!!!
  if (mid_MsgQueueMp3 == NULL) {
    return (-1);
  }
  return(0);
}


//FUNCION DEL HILO
void MP3 (void *argument){
  
  //INICIALIZACION UART
  USARTdrv->Initialize(USART_Callback);
  USARTdrv->PowerControl(ARM_POWER_FULL);
  USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS|ARM_USART_DATA_BITS_8|
                    ARM_USART_PARITY_NONE|ARM_USART_STOP_BITS_1|ARM_USART_FLOW_CONTROL_NONE,9600);
  USARTdrv->Control(ARM_USART_CONTROL_RX,1);
  USARTdrv->Control(ARM_USART_CONTROL_TX,1);


  while(1){
  
  
  }
}



void USART_Callback(uint32_t event){
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_TRANSFER_COMPLETE|
  ARM_USART_EVENT_SEND_COMPLETE | ARM_USART_EVENT_TX_COMPLETE;
  if (event & mask ){
    osThreadFlagsSet(tid_ThMp3,0x01);
  }
}