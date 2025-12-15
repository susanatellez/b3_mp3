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
int Init_MsgQueue_Mp3(void);
int Init_MsgQueue_Mp3_Info(void);
void MandarComando(uint8_t comm, uint8_t dato_1, uint8_t dato_2);
uint8_t comprobarEstadoTarjeta();
uint8_t getInfoMp3(uint8_t);
//Hilo y cola:
osThreadId_t tid_ThMp3;                        // thread id
osMessageQueueId_t  mid_MsgQueueMp3;
osMessageQueueId_t  mid_MsgQueueMp3Info;
//Comandos
  t_comando msg; /////////////////////////////////////////////////////////////

  uint8_t buffRx[10] = {0};
//Driver
extern ARM_DRIVER_USART Driver_USART2;
ARM_DRIVER_USART *USARTdrv2 = &Driver_USART2;

//Inicialización del módulo
void Init_ModMP3 (void){
  Init_ThMP3();
  Init_MsgQueue_Mp3();
  Init_MsgQueue_Mp3_Info();
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
int Init_MsgQueue_Mp3(void) {
 
  mid_MsgQueueMp3= osMessageQueueNew(4,sizeof(t_comando), NULL);
  if (mid_MsgQueueMp3 == NULL) {
    return (-1);
  }
  return(0);
}
int Init_MsgQueue_Mp3_Info(void) {
 
  mid_MsgQueueMp3Info= osMessageQueueNew(4,sizeof(t_respuesta), NULL);
  if (mid_MsgQueueMp3Info == NULL) {
    return (-1);
  }
  return(0);
}


//FUNCION DEL HILO
void MP3 (void *argument){
  uint8_t com;
  uint8_t dato1;
  uint8_t dato2;
  //t_comando msg;
  t_respuesta msg_respuesta;
  uint8_t estado_tarjeta = 0;
  //INICIALIZACION UART
  USARTdrv2->Initialize(USART_Callback);
  USARTdrv2->PowerControl(ARM_POWER_FULL);
  USARTdrv2->Control(ARM_USART_MODE_ASYNCHRONOUS|ARM_USART_DATA_BITS_8|
                    ARM_USART_PARITY_NONE|ARM_USART_STOP_BITS_1|ARM_USART_FLOW_CONTROL_NONE,9600);
  USARTdrv2->Control(ARM_USART_CONTROL_RX,1);
  USARTdrv2->Control(ARM_USART_CONTROL_TX,1);

  //Inicializacion para elegir la tarjeta sd

  MandarComando(9,0,2);

  osDelay(100);
  
  while(1){

    uint8_t nuevo_estado = comprobarEstadoTarjeta();
    if(estado_tarjeta != nuevo_estado){
      estado_tarjeta = nuevo_estado;
      msg_respuesta.tipo = 0;
      msg_respuesta.info = nuevo_estado;
      osMessageQueuePut(mid_MsgQueueMp3Info, &msg_respuesta,0,0);
    }

  
  
  if(osMessageQueueGet(mid_MsgQueueMp3,&msg,NULL,10U)==osOK){
  switch(msg.tipo){
    case 0:
     com = msg.com;
     dato1 = msg.dat1;
     dato2 = msg.dat2;
     MandarComando(com,dato1,dato2);
     break;

    case 1:
      msg_respuesta.tipo = msg.com;
      msg_respuesta.info = getInfoMp3(msg.com);
      osMessageQueuePut(mid_MsgQueueMp3Info, &msg_respuesta,0,0);
      break;
  }

  };


  }
}

//Funcion constructora de comando
void MandarComando(uint8_t comm, uint8_t dato_1, uint8_t dato_2){
  uint8_t buff[8] = {0};
  buff[0] = 0x7E;//BYTE DE START
  buff[1] = 0xFF;//BYTE DE VERSION
  buff[2] = 0x06;//LONGITUD DEL COMANDO SIN BYTE DE START Y END
  buff[3] = comm; //COMANDO
  buff[4] = 0x00;//Byte de feedback, si hiciera falta activarlo poner a 0x01
  buff[5] = dato_1;
  buff[6] = dato_2;
  buff[7] = 0xEF; //BYTE DE END
  
  USARTdrv2->Send(buff,sizeof(buff));
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  osDelay(20);
}


uint8_t getInfoMp3(uint8_t cmd){

  uint8_t buffCMD[8] ={0};
  buffCMD[0] = 0x7E;//BYTE DE START
  buffCMD[1] = 0xFF;//BYTE DE VERSION
  buffCMD[2] = 0x06;//LONGITUD DEL COMANDO SIN BYTE DE START Y END
  buffCMD[3] = cmd; //COMANDO
  buffCMD[4] = 0x00;//Byte de feedback, si hiciera falta activarlo poner a 0x01
  buffCMD[5] = 0x00;
  buffCMD[6] = 0x00;
  buffCMD[7] = 0xEF; //BYTE DE END
  
  
  USARTdrv2->Send(buffCMD,sizeof(buffCMD));
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  
  USARTdrv2->Receive(buffRx,sizeof(buffRx));
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);

  osDelay(10);
   return  buffRx[6];

}

uint8_t comprobarEstadoTarjeta(){
  uint8_t estadoTarjeta = 0;
  USARTdrv2->Receive(buffRx,sizeof(buffRx));
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  
  if (buffRx[3]== 0x3A){//Tarjeta insertada
    estadoTarjeta = 1;
  }else if(buffRx[3] == 0x3B){//Tarjeta no insertada
    estadoTarjeta = 0;
  }else if(buffRx[3] == 0x40){//file not found
    estadoTarjeta = 2;
  }
  return estadoTarjeta;
}

void USART_Callback(uint32_t event){
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE | ARM_USART_EVENT_TRANSFER_COMPLETE|
  ARM_USART_EVENT_SEND_COMPLETE | ARM_USART_EVENT_TX_COMPLETE;
  if (event & mask ){
    osThreadFlagsSet(tid_ThMp3,0x01);
  }
}