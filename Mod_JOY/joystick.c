#include "cmsis_os2.h"        // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "joystick.h"                                                           //
/*******************************************************************************
MODULO JOYSTICK: encargado de detectar e identificar una pulsación en el joystick
********************************************************************************/

static GPIO_InitTypeDef GPIO_InitStruct;                                        //están en
//static void initModJoy (void);                                                //el .H

// HILO DEL JOYSTICK
osThreadId_t tid_joy;                                                           //no me va si lo muevo al .H      ??!
uint32_t statusJoy = 0x0000;
int Init_ThJoy (void);
void ThJoy (void *argument);

// VARIABLES PARA GESTIONAR LAS PULSACIONES CORTAS Y LARGAS
//typedef struct {                                                              //está en al .H
//    uint16_t pin;
//    GPIO_TypeDef *port;
//} mygpio_pin;
mygpio_pin pin_presionado;
uint16_t pin_activo;

// TIMER VIRTUAL
osTimerId_t timerJoy;
int Init_timerJoy (void);
void TimerJoy_Callback(void *arg);

// QUEUE
osMessageQueueId_t mid_MsgQueue_Joy;
uint32_t sizeQueueJoy = 0;
osStatus_t statusQueueJoy = 0x000000000;
uint8_t MsgQueue_Joy = 0x00;                                                    //mensaje de la cola
int Init_MsgQueue_Joy(void);


/* INICIALIZACIÓN MÓDULO DEL JOYSTICK 
 * donde se configuran e inicializan los pines
 * se inicializa hilo del joystick
 * se inicializa el timer virtual que gestiona las pulsaciones
 * y se inicializa la cola de mensajes (desde aquí meto mensajes en la cola)
 */

void initModJoy (void) {

 //Habilito reloj, configuro pines y los inicializo

  //Gestos RIGHT(PB11) y UP(PB10) del joystick
  __HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  //Gestos DOWN(PE12), LEFT(PE14) y CENTER(PE15) del joystick
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  //Habilito interrupciones externas de los pines configurados
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  //Inicializo hilo del joystick
  Init_ThJoy();
  
  //Inicializo el timer virtual que gestiona las pulsaciones
  Init_timerJoy();
  
  //inicializo cola de mesnsajes (desde aquí envío)
  Init_MsgQueue_Joy();

}


// INICIALIZACIÓN DEL HILO DEL JOYSTICK
int Init_ThJoy (void) {
  tid_joy = osThreadNew(ThJoy, NULL, NULL);
  if (tid_joy == NULL) {
    return(-1);
  }
  return(0);
}


/* FUNCIÓN DEL HILO DEL JOYSTICK
 * que espera la flag proveniente del fichero de interrupciones externas
 * y comprueba si se trata de una pulsacion corta
 */
void ThJoy (void *argument) {

  while(1){

    statusJoy = osThreadFlagsWait(0x1000, osFlagsWaitAny, osWaitForever);

    //Inicializo timer cada que llegue flag de que ha sido pulsado un botón del joystick
    if (statusJoy == 0x1000){
      statusJoy = 0x0000; //borramos el flag
      pin_activo = pin_presionado.pin; //guardamos el pin que ha sido presionado
      osDelay(50); //espero los primeros rebotes del rising
      osTimerStart(timerJoy, 950U); //inicializamos timer de 1seg-50ms esperados antes
      
      do {
        if(HAL_GPIO_ReadPin(pin_presionado.port, pin_presionado.pin) == 0){//si se ha dejado de pulsar el gesto del joystick
          // Verificar si el temporizador sigue activo
          if (osTimerIsRunning(timerJoy)){ //Si lo está se trata de una pulsación corta
            osTimerStop(timerJoy); // Detener el temporizador de pulsación larga
                                   // de manera que no saltará la callback
            switch(pin_activo){ //depende del pin presionado enviamos un mensaje u otro
              case 0x0800:
                MsgQueue_Joy = 0x02;
                osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
                break;
              case 0x0400:
                MsgQueue_Joy = 0x01;
                osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
                break;
              case 0x1000:
                MsgQueue_Joy = 0x03;
                osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
                break;
              case 0x4000:
                MsgQueue_Joy = 0x04;
                osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
                break;
              case 0x8000:
                MsgQueue_Joy = 0x05;
                osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
                break;
              default:
                //ns que poner aquí
              break;
            } //fin del switch
          // EL ELSE SOBRA??
          } else { //si el timer ya se ha acabado la función callback del timer ya ha sido llamada = PULSACION LARGA
           break; //CON ESTO SALDRÍA DEL DO-WHILE CREO . . .
          }
          
        }//fin del if(pin_status==0)
        
      } while (1);
      
    } //fin del if (statusJoy == 0x1000)
  }

}

//inicialización del timer del joystick
int Init_timerJoy (void){

  timerJoy = osTimerNew(TimerJoy_Callback, osTimerOnce, NULL, NULL);

  if (timerJoy != NULL) { // Si se ha creado el timer correctamente
    return 0;
  } else {
    return -1;
  }

  return NULL;

}

void TimerJoy_Callback(void *arg){                                              //para PULSACIONES LARGAS

  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 1){                               //RIGHT
    MsgQueue_Joy = 0x20;
    osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
  } else if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 1){                        //UP
    MsgQueue_Joy = 0x10;
    osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
  } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == 1){                        //DOWN
    MsgQueue_Joy = 0x30;
    osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
  } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == 1){                        //LEFT
    MsgQueue_Joy = 0x40;
    osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
  } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == 1){                        //CENTER
    MsgQueue_Joy = 0x50;
    osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
  }

}

int Init_MsgQueue_Joy(void){

  mid_MsgQueue_Joy = osMessageQueueNew(16, sizeof(uint8_t), NULL);              //REVISARLO
  if (mid_MsgQueue_Joy == NULL) {
    return (-1);
  }
  return(0);

}


/* Código para testear el módulo                                                */

osThreadId_t tid_Th_Joy_Test;
//int Init_Th_Joy_Test(void);                                            //lo muevo al .H
void Th_Joy_Test(void *argument);

//Creación del hilo que testea el funcionamiento de las pulsaciones del joystick
int Init_Th_Joy_Test(void) {

  tid_Th_Joy_Test = osThreadNew(Th_Joy_Test, NULL, NULL);
  if (tid_Th_Joy_Test == NULL) {
    return(-1);
  }
  return(0);

}


void Th_Joy_Test (void *argument) {

  while (1) {
    
    //con el osDelay y ayuda del watches veríamos si la cola de mensajes funciona bien
    //osDelay(2000);                                                              //comentar para que funcione normal
    sizeQueueJoy = osMessageQueueGetCount (mid_MsgQueue_Joy);
    statusQueueJoy = osMessageQueueGet(mid_MsgQueue_Joy, &MsgQueue_Joy, NULL, 10U);
    
  }

}

