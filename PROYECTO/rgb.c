#include "cmsis_os2.h"                                                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "rgb.h"                                                                //

/*******************************************************************************
 *      MODULO PWM OUTPUT: Módulo encargado de generar una señal PWM
********************************************************************************/

//#define RGB_OFF   0x00 //no lucen
//#define RGB_PLAY  0x01 //VERDE
//#define RGB_PAUSE 0x02 //AZUL
//#define RGB_NO_SD 0x03 //ROJO
//#define RGB_NO_SD 0x04 //ROSA RGB_CANCION_ACADABDA

static GPIO_InitTypeDef GPIO_InitStruct;                                        //
//void initModRGB (void);                                                       //está en el .H


// HILO DEL PWM
osThreadId_t tid_RGB;
uint32_t statusRGB = 0x0000;
int Init_ThRGB (void);
void ThRGB (void *argument);

// COLA DE MENSAJES
osMessageQueueId_t mid_MsgQueue_RGB;
uint8_t MsgQueue_RGB = 0x00;                                                    //mensaje que sacas de la cola
osStatus_t statusQueueRGB;                                                      //para ver que coge el mensaje correcto
//uint8_t estado_actual = 0;
int Init_MsgQueue_RGB(void);


void initModRGB (void){

  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;                //
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;                                   //
  GPIO_InitStruct.Pull = GPIO_PULLUP;                                           //
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;                                 //

  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);                                       //

  //Inicializamos los leds RGB apagados
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                          //R
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                          //G
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                          //B

  Init_MsgQueue_RGB();
  Init_ThRGB();

}


int Init_ThRGB (void){

  const osThreadAttr_t ThRGB_attributes = {
    .name = "ThRGB",
    .stack_size = 256U,
  };

  tid_RGB = osThreadNew(ThRGB, NULL, &ThRGB_attributes);
  if (tid_RGB == NULL) {
    return(-1);
  }
  return(0);

}


void ThRGB (void *argument){

  while(1){

    statusQueueRGB = osMessageQueueGet(mid_MsgQueue_RGB, &MsgQueue_RGB, NULL, 10U);

//    if(statusQueueRGB == osOK){
//      switch(MsgQueue_RGB){
//        case 0x00: //APAGADOS
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
//          break;
//        case 0x01: //ROJO
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
//          break;
//        case 0x02: //AMARILLO
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
//          break;
//        case 0x03: //VERDE
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
//          break;
//        case 0x04: //AZUL
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);                  //B
//          break;
//        case 0x05: //CIAN
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);                  //B
//          break;
//        case 0x06: //ROSITA
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);                  //B
//          break;
//        case 0x07: //BLANCO
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);                  //R
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);                  //G
//          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);                  //B
//          break;
//        default: break;
//      }
//    }

//    if(statusQueueRGB == osOK){
//      estado_actual = MsgQueue_RGB;
//      if (estado_actual == 0) HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_SET);
//    }
      switch(MsgQueue_RGB){
        case 0x00: //APAGADOS RGB_OFF
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
          break;
        case 0x01: //VERDE RGB_PLAY
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);                                 //G
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
          osDelay(250); //4Hz
          break;
        case 0x02: //AZUL RGB_PAUSE
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);                  //R
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_11);                                 //B
          osDelay(1000); //1Hz
          break;
        case 0x03: //ROJO RGB_NO_SD
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);                                 //R
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);                  //B
          osDelay(200); //5Hz
          break;
        case 0x04: //ROSA RGB_CANCION_ACADABDA
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);                  //R
          HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);                  //G
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_11);                  //B
          osDelay(200); //5Hz
          break;
        case 0x05:                                                              //ESTAS 5 LÍNEAS ///////////////////////////////////////////
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);                  //R
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);                  //G
          HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_11);                  //B
          break;
        default: break;
      }


  }

}

int Init_MsgQueue_RGB(void){
    mid_MsgQueue_RGB = osMessageQueueNew(4, sizeof(uint8_t), NULL);
  if (mid_MsgQueue_RGB == NULL) {
    return (-1);
  }
  return(0);
}