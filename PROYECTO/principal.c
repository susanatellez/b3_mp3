#include "cmsis_os2.h"                                                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "principal.h"                                                          //

/*******************************************************************************
 *                           MODULO PRINCIPAL..................................*
********************************************************************************/

#define MODO_REPOSO       0x01
#define MODO_REPRODUCCION 0x02
#define MODO_HORA         0x03

#define EV_JOY            0x01
#define EV_POT           0x02
#define EV_TEMP           0x03

uint8_t modo_actual;
uint8_t evento = 0X00;

// HILO PRINCIPAL = TEST.C
osThreadId_t tid_principal;
//int Init_ThPrincipal (void);                                                  //en el .H
void ThPrincipal (void *argument);

//COLAS DE OTROS MÓDULOS
extern osMessageQueueId_t mid_MsgQueue_Joy;                                     //faltan los demás
extern osMessageQueueId_t mid_MsgQueue_POT;
extern osMessageQueueId_t mid_MsgQueueTemp;
extern osMessageQueueId_t mid_MsgQueueMp3;                                      //FALTA mod MP3

//COLAS DEL H-PRINCIPAL ASOCIADAS A LOS MÓDULOS
uint8_t msgJoy = 0x00;
uint8_t msgPot = 0x00;
float msgTemp = 0x00;
t_comando msgMP3;                                                               //FALTA mod MP3

int Init_ThPrincipal (void){ //Inicializamos hilo y cola
  
  tid_principal = osThreadNew(ThPrincipal, NULL, NULL);               //aquí le cambiamos el tamaño?
  if (tid_principal == NULL) {
    return(-1);
  }
  
  return(0);
}

void ThPrincipal (void *argument){
  
  modo_actual = MODO_REPOSO;
  
  while(1){
    
    if (osMessageQueueGet(mid_MsgQueue_Joy, &msgJoy, NULL, 10) == osOK) {
          evento = EV_JOY;
    }
      // Si no hubo joystick, probar POT
    else if (osMessageQueueGet(mid_MsgQueue_POT, &msgPot, NULL, 10) == osOK) {
          evento = EV_POT;
    }
      // Si tampoco, probar TEMP
    else if (osMessageQueueGet(mid_MsgQueueTemp, &msgTemp, NULL, 10) == osOK) {
          evento = EV_TEMP;
    }
    
    switch(modo_actual){                                                        //SWITCH PRINCIPAL
    
      case MODO_REPOSO :                                                        //MODO REPOSO
        
        //[añadir código]
        
        break;                                                                  //FIN modo reposo
        
      case MODO_REPRODUCCION :                                                  //MODO REPDRODUCCIÓN
        
        switch(evento){
          case EV_JOY:
            evento = 0x00; //borramos flag
            if(msgJoy == 0x50){
              //AÑADIR WAKE UP/SLEEP
              //
              modo_actual = MODO_HORA;
            }
            break;
          case EV_POT:
            
            break;
        }
        
        break;                                                                  //FIN modo reproducción
        
      case MODO_HORA :                                                          //MODO_HORA
        
        //[añadir código]
        
        break;                                                                  //FIN modo hora
        
      default:
        modo_actual = MODO_REPOSO;
        break;
     
    } //FIN del switch principal
  
  } //fin del while(1)
  
}