#include "cmsis_os2.h"        // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "joystick.h"                                                           //

/*******************************************************************************
MODULO JOYSTICK: encargado de detectar e identificar una pulsación en el joystick
                 se detecta si ha sido una pulsacion corta o larga              
                 dependiendo de cuando haya finalizado / a partir de un segundo
********************************************************************************/

static GPIO_InitTypeDef GPIO_InitStruct;                                        //están en
//static void initModJoy (void);                                                //el .H

// HILO DEL JOYSTICK
osThreadId_t tid_joy;                                                           //no me va si lo muevo al .H
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

// TIMER VIRTUAL 1, GESTIONA REBOTES
osTimerId_t timerJoyRebotes;
int Init_timerJoyRebotes (void);
void TimerJoyRebotes_Callback(void *arg);


// TIMER VIRTUAL 2, PARA LAS PULSACIONES LARGAS
osTimerId_t timerJoyPulsacionLarga;
int Init_timerJoyPulsacionLarga (void);
void TimerJoyPulsacionLarga_Callback(void *arg);


// TIMER VIRTUAL 3 espera 50s para el flanco de bajada 
osTimerId_t timerJoyRebotesBajada;
int Init_timerJoyRebotesBajada (void);
void TimerJoyRebotesBajada_Callback(void *arg);

// QUEUE
osMessageQueueId_t mid_MsgQueue_Joy;
uint8_t MsgQueue_Joy = 0x00;                                                    //mensaje de metes a la cola
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
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;                           //RISING-FALLING para saber...
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  //Gestos DOWN(PE12), LEFT(PE14) y CENTER(PE15) del joystick
  __HAL_RCC_GPIOE_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;                           //...cuando empieza y acaba
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  //Habilito interrupciones externas de los pines configurados
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  //Inicializo hilo del joystick
  Init_ThJoy();
  
  //Inicializo el timer virtual que gestiona las pulsaciones
  Init_timerJoyRebotes();
  Init_timerJoyPulsacionLarga();
  Init_timerJoyRebotesBajada();
  
  //inicializo cola de mesnsajes (desde aquí envío)
  Init_MsgQueue_Joy();

}


// INICIALIZACIÓN DEL HILO DEL JOYSTICK
int Init_ThJoy (void) {

  const osThreadAttr_t ThJoy_attributes = {
    .name = "ThJoy",
    .stack_size = 128U, // <- Nuevo tamaño
  };

  tid_joy = osThreadNew(ThJoy, NULL, &ThJoy_attributes);
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

    statusJoy = osThreadFlagsWait(0x1001, osFlagsWaitAny, osWaitForever);

    //Inicializo timer cada que llegue flag de que ha sido pulsado un botón del joystick
    
    if (statusJoy == 0x1000){ //si es FLANCO de SUBIDA
      statusJoy = 0x0000; //borramos flag
      pin_activo = pin_presionado.pin; //guardamos el pin que ha sido presionado
      osTimerStart(timerJoyRebotes, 50U); //Iniciamos timer para gestionar rebotes
      
    }
    
    if(statusJoy == 0x0001){ //Si es FLANCO de BAJADA
      statusJoy = 0x0000; //borramos flag
      osTimerStart(timerJoyRebotesBajada, 50U); //Iniciamos timer para gestionar rebotes de bajada
    }
  }
}


//Inicialización del timer de rebotes
int Init_timerJoyRebotes (void){

  timerJoyRebotes = osTimerNew(TimerJoyRebotes_Callback, osTimerOnce, NULL, NULL);

  if (timerJoyRebotes != NULL) { // Si se ha creado el timer correctamente
    return 0;
  } else {
    return -1;
  }

  return NULL;

}


void TimerJoyRebotes_Callback(void *arg){                                       //para REBOTES, flanco de subida

  if (HAL_GPIO_ReadPin(pin_presionado.port, pin_presionado.pin)){
    osTimerStart(timerJoyPulsacionLarga, 950U);
  }

}


//Inicialización del timer de rebotes para el flanco de bajada
int Init_timerJoyRebotesBajada (void){

  timerJoyRebotesBajada = osTimerNew(TimerJoyRebotesBajada_Callback, osTimerOnce, NULL, NULL);

  if (timerJoyRebotesBajada != NULL) { // Si se ha creado el timer correctamente
    return 0;
  } else {
    return -1;
  }

  return NULL;

}


void TimerJoyRebotesBajada_Callback(void *arg){                                 //para el timer de BAJADA

  if (HAL_GPIO_ReadPin(pin_presionado.port, pin_presionado.pin)==0){
    if (osTimerIsRunning(timerJoyPulsacionLarga)) {
      osTimerStop(timerJoyPulsacionLarga);
      /* si se ha dejado de pulsar el gesto del joystick, verificamos si el
       * temporizador sigue activo. Si es así se trata de una PULSACIÓN CORTA,
       * por consigueinte detenemos el temporizador de pulsación larga de
       * manera que no saltará la callback correspondiente*/
       
      switch(pin_activo){ //depende del pin presionado enviamos un mensaje u otro
        case GPIO_PIN_11:
          MsgQueue_Joy = 0x02;
          osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
          break;
        case GPIO_PIN_10:
          MsgQueue_Joy = 0x01;
          osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
          break;
        case GPIO_PIN_12:
          MsgQueue_Joy = 0x03;
          osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
          break;
        case GPIO_PIN_14:
          MsgQueue_Joy = 0x04;
          osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
          break;
        case GPIO_PIN_15:
          MsgQueue_Joy = 0x05;
          osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
          break;
        default: break;
      } //fin del switch
      
      //si el timer ya se ha acabado la función callback del timer ya ha sido llamada = PULSACION LARGA
      
    } //fin de ¿el temporizador sigue activo?
    
  }//fin de ¿se ha dejado de pulsar el gesto del joystick?
  
} //fin de la callback


//Timer de 1s para ver si la pulsación larga ha sido llamada
int Init_timerJoyPulsacionLarga (void){

  timerJoyPulsacionLarga = osTimerNew(TimerJoyPulsacionLarga_Callback, osTimerOnce, NULL, NULL);

  if (timerJoyPulsacionLarga != NULL) { // Si se ha creado el timer correctamente
    return 0;
  } else {
    return -1;
  }

  return NULL;

}


//Si vence el timer de 1s = pulsación larga entra a la callback y manda el mensaje:
void TimerJoyPulsacionLarga_Callback(void *arg){

  switch(pin_activo){ //depende del pin presionado enviamos un mensaje u otro
    case GPIO_PIN_11:
      MsgQueue_Joy = 0x20;
      osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
      break;
    case GPIO_PIN_10:
      MsgQueue_Joy = 0x10;
      osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
      break;
    case GPIO_PIN_12:
      MsgQueue_Joy = 0x30;
      osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
      break;
    case GPIO_PIN_14:
      MsgQueue_Joy = 0x40;
      osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
      break;
    case GPIO_PIN_15:
      MsgQueue_Joy = 0x50;
      osMessageQueuePut(mid_MsgQueue_Joy, &MsgQueue_Joy, 0U, 0U);
      break;
    default: break;
  }

}


int Init_MsgQueue_Joy(void){

  mid_MsgQueue_Joy = osMessageQueueNew(16, sizeof(uint8_t), NULL);
  if (mid_MsgQueue_Joy == NULL) {
    return (-1);
  }
  return(0);

}
