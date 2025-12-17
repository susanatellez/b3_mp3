#include "cmsis_os2.h" // CMSIS RTOS header file
#include "mp3.h"
#include "Driver_USART.h"
#include <stdio.h>
#include <string.h> // para el memset
#include "stm32f4xx.h"



//******************************************************************************
//--------------------------------------MP3-------------------------------------
//******************************************************************************

//DRIVER
extern ARM_DRIVER_USART Driver_USART2;
ARM_DRIVER_USART *USARTdrv2 = &Driver_USART2;

//HILO
osThreadId_t tid_ThMp3;                        // thread id
osThreadId_t tid_ThMp3_Recep;
int Init_ThMP3(void);
int Init_ThMP3_Recep(void);
void MP3 (void *argument);
void MP3_Recep(void *argument);
void USART_Callback(uint32_t event);

//COLA
osMessageQueueId_t  mid_MsgQueueMp3;      //de aquí extraeremos
osMessageQueueId_t  mid_MsgQueueMp3Info; //en esta metemos mensajes
int Init_MsgQueue_Mp3(void);
int Init_MsgQueue_Mp3_Info(void);

//FUNCIONES AUXILIARES
void MandarComando(uint8_t comm, uint8_t dato_1, uint8_t dato_2);
uint8_t comprobarEstadoTarjeta();
uint8_t getInfoMp3(uint8_t);
void flushMp3Rx(void);


//Comandos
uint32_t statusGet = 0;
t_comando msg;              //mensaje que recibes
t_respuesta msg_reply; //la que envias

uint8_t estado_tarjeta = 0x01; //Asumimos que Hay tarjta
uint8_t fold_ini =0;
uint8_t tracks_ini =0;

uint8_t buffTx[8] = {0};  //MANDAR COMANDO
uint8_t buffCMD[8] = {0}; //MANDA COMANDO ESPERANDO RESPUESTA
uint8_t buffRx[10] = {0}; //RECIBE RESPUESTA


//Inicialización del módulo
void Init_ModMP3 (void){
  Init_MsgQueue_Mp3();
  Init_MsgQueue_Mp3_Info();
  Init_ThMP3();
  Init_ThMP3_Recep();

}

//Inicialización del hilo
int Init_ThMP3(void){
  tid_ThMp3 = osThreadNew(MP3, NULL, NULL);
  if (tid_ThMp3 == NULL) {
    return(-1);
  }
  return(0);
}

int Init_ThMP3_Recep(void){
  tid_ThMp3_Recep = osThreadNew(MP3_Recep, NULL, NULL);
  if (tid_ThMp3_Recep == NULL) {
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

  //INICIALIZACION UART
  USARTdrv2->Initialize(USART_Callback);
  USARTdrv2->PowerControl(ARM_POWER_FULL);
  USARTdrv2->Control(ARM_USART_MODE_ASYNCHRONOUS|ARM_USART_DATA_BITS_8|
                    ARM_USART_PARITY_NONE|ARM_USART_STOP_BITS_1|ARM_USART_FLOW_CONTROL_NONE,9600);
  USARTdrv2->Control(ARM_USART_CONTROL_RX,1);
  USARTdrv2->Control(ARM_USART_CONTROL_TX,1);

  //Inicializacion para elegir la tarjeta sd
  MandarComando(9,0,2);
//  tracks_ini =  getInfoMp3(0x48);
//  osDelay(1000);
//  fold_ini = getInfoMp3(0x4F);
//  osDelay(1000);
//  
//  if((tracks_ini!=0)||(fold_ini!=0)){
//    estado_tarjeta = 1;
//    msg_reply.tipo = 0;
//    msg_reply.info = estado_tarjeta;
//    osMessageQueuePut(mid_MsgQueueMp3Info, &msg_reply,0,0);
//  }else{
//    estado_tarjeta = 0;
//    msg_reply.tipo = 0;
//    msg_reply.info = estado_tarjeta;
//    osMessageQueuePut(mid_MsgQueueMp3Info, &msg_reply,0,0);
//  }
//  osDelay(100);
  
  while(1){
    uint8_t comando; 
    statusGet = osMessageQueueGet(mid_MsgQueueMp3,&msg,NULL,10U);
    comando = msg.com;
  if(statusGet == osOK){
    statusGet = 0;
    switch(msg.tipo){
      case 0:
      MandarComando(msg.com,msg.dat1,msg.dat2);
      osDelay(20);
      break;
      case 1:
      msg_reply.tipo = comando;
      msg_reply.info = getInfoMp3(comando);
      osMessageQueuePut(mid_MsgQueueMp3Info, &msg_reply,0,0);
      break;
    }

  } 


  }
}


void MP3_Recep(void *argument){
  while(1){
    uint8_t nuevo_estado = comprobarEstadoTarjeta();
    if(estado_tarjeta != nuevo_estado){
      estado_tarjeta = nuevo_estado;
      msg_reply.tipo = 0;
      msg_reply.info = nuevo_estado;
      osMessageQueuePut(mid_MsgQueueMp3Info, &msg_reply,0,0);
    }
  }
}
//Funcion constructora de comando
void MandarComando(uint8_t comm, uint8_t dato_1, uint8_t dato_2){               // = HAZ ESTO

  buffTx[0] = 0x7E;//BYTE DE START
  buffTx[1] = 0xFF;//BYTE DE VERSION
  buffTx[2] = 0x06;//LONGITUD DEL COMANDO SIN BYTE DE START Y END
  buffTx[3] = comm; //COMANDO
  buffTx[4] = 0x00;//Byte de feedback, si hiciera falta activarlo poner a 0x01
  buffTx[5] = dato_1;
  buffTx[6] = dato_2;
  buffTx[7] = 0xEF; //BYTE DE END
  
  USARTdrv2->Send(buffTx,sizeof(buffTx));
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  osDelay(20);
}


uint8_t getInfoMp3(uint8_t cmd){                                                // = DIME ESTO

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
  osDelay(20);
  
//  USARTdrv2->Receive(buffRx,sizeof(buffRx));
//  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
//  osDelay(20);

  return  comprobarEstadoTarjeta();

}

uint8_t comprobarEstadoTarjeta(){
  uint8_t estadoTarjeta = 1;
  USARTdrv2->Receive(buffRx,sizeof(buffRx));
  osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
  
  if (buffRx[3]== 0x3A){//Tarjeta insertada
    estadoTarjeta = 1;
  }else if(buffRx[3] == 0x3B){//Tarjeta no insertada
    estadoTarjeta = 0;
  }else if(buffRx[3] == 0x40){//file not found
    estadoTarjeta = 2;
  }else if (buffRx[3] == 0x3D){
    estadoTarjeta = 3;
  }else{
    estadoTarjeta = buffRx[6];
  }
  return estadoTarjeta;
}

void USART_Callback(uint32_t event){
  uint32_t mask;
  mask =  ARM_USART_EVENT_TRANSFER_COMPLETE|
  ARM_USART_EVENT_SEND_COMPLETE | ARM_USART_EVENT_TX_COMPLETE;
  if (event & mask ){
    osThreadFlagsSet(tid_ThMp3,0x01);
  }
  if (event & ARM_USART_EVENT_RECEIVE_COMPLETE ){
    osThreadFlagsSet(tid_ThMp3_Recep,0x01);
  }
}

