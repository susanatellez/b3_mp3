#include "cmsis_os2.h"                                                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "principal.h"                                                          //
#include "joystick.h"                                                           //

/*******************************************************************************
 *                           MODULO PRINCIPAL..................................*
********************************************************************************/

#define MODO_REPOSO       0x01
#define MODO_REPRODUCCION 0x02
#define MODO_HORA         0x03

#define HORA              0x01
#define TEMPERATURA       0x02
#define JOYSTICK          0x03
#define POT_2             0x04

// HILO PRINCIPAL = TEST.C
osThreadId_t tid_principal;
//int Init_ThPrincipal (void);                                                  //en el .H

int modo_actual;
void ThPrincipal (void *argument);

typedef struct {
    uint8_t  source;  // ¿Quién lo envía? (FUENTE_JOYSTICK, FUENTE_TEMP...)
    uint32_t msg;    // El dato (código de tecla, valor de temperatura, etc.)
} mensaje_t;

extern osMessageQueueId_t mid_MsgQueue_Joy;                                     //faltan los demás

osStatus_t statusPrincipal;
osMessageQueueId_t mid_MsgQueue_Principal;
mensaje_t MsgQueue_Principal;

int Init_ThPrincipal (void){ //Inicializamos hilo y cola
  
  tid_principal = osThreadNew(ThPrincipal, NULL, NULL);               //aquí le cambiamos el tamaño?
  if (tid_principal == NULL) {
    return(-1);
  }
  
  mid_MsgQueue_Principal = osMessageQueueNew(16, sizeof(uint8_t), NULL);
  if (mid_MsgQueue_Principal == NULL) {
    return (-1);
  }
  
  return(0);
}

void ThPrincipal (void *argument){
  
  modo_actual = MODO_REPOSO;
  
  while(1){
    
    statusPrincipal = osMessageQueueGet(mid_MsgQueue_Principal, &MsgQueue_Principal, NULL, osWaitForever);
    
    switch(modo_actual){

      case MODO_REPOSO :
        //[añadir código]
        if(MsgQueue_Principal.source == JOYSTICK){
          if(MsgQueue_Principal.msg == 0x05){
            modo_actual = MODO_REPRODUCCION;
          }
        } //faltan más cosas pero es la idea
        break;
        
      case MODO_REPRODUCCION :
        if(MsgQueue_Principal == 0x05){
          modo_actual = MODO_HORA;
        } //faltan más cosas pero es la idea
        break;
        
      case MODO_HORA :
        //[añadir código]
        if(MsgQueue_Principal == 0x05){
          modo_actual = MODO_REPOSO;
        } //faltan más cosas pero es la idea
        break;
        
      default:
        break;

    }
  
  } //fin del while(1)
}