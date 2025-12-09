#include "cmsis_os2.h"                                                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "PWM.h"                                                                //

/*******************************************************************************
 *      MODULO PWM OUTPUT: Módulo encargado de generar una señal PWM
********************************************************************************/

static GPIO_InitTypeDef GPIO_InitStruct;                                        //están en
//void initModPWM (void);                                                  //es como el initPIN_OUTPUT


// HILO DEL PWM
osThreadId_t tid_PWM;
uint32_t statusPWM = 0x0000;
int Init_ThPWM (void);
void ThPWM (void *argument);

// TIMER
TIM_HandleTypeDef tim1;                                                         //
static void InitTimer1(void);                                                   //CONFIGURACIO E INICIALIZACION TIMER

// COLA DE MENSAJES
/* Thread a la espera de cola de mensajes que indique que hay que 
 * generarar una señal que permite obtener distintos “bips” en la tarjeta 
 * de aplicaciones*/
osMessageQueueId_t mid_MsgQueue_PWM;
uint8_t MsgQueue_PWM = 0x00;                                                    //mensaje de metes a la cola
osStatus_t statusQueuePWM;                                                      //para ver que coge el mensaje correcto
int Init_MsgQueue_PWM(void);

//CONFIGURACIÓN DEL PIN e INICALI
void initModPWM (void){

  //Habilito reloj, configuro pin PE9 y lo inicializao
  
  __HAL_RCC_GPIOE_CLK_ENABLE();                                                 //Habilitamos reloj del puerto al que "pertenece" el pin
  
  GPIO_InitStruct.Pin = GPIO_PIN_9;                                             //Configuramos el pin PE9
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                                       //como salida digital de tipo push.pull
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;                                    //en modo función alternativa
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;//
  
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);                                       //inicializamos el pin pb11 con la configuracion descrita

  //Inicializo la cola
  Init_MsgQueue_PWM();

  //Inicializo el timer
  InitTimer1();

  //Inicializo el hilo del PWM
  Init_ThPWM();
  


}


int Init_ThPWM (void){

//  const osThreadAttr_t ThPWM_attributes = {
//    .name = "ThPWM",
//    .stack_size = 512U, // <- Nuevo tamaño
//  };

  tid_PWM = osThreadNew(ThPWM, NULL, NULL);
  if (tid_PWM == NULL) {
    return(-1);
  }
  return(0);
}


void ThPWM (void *argument){

  while(1){

    statusQueuePWM = osMessageQueueGet(mid_MsgQueue_PWM, &MsgQueue_PWM, NULL, 10U);

    if(statusQueuePWM == osOK){
      switch(MsgQueue_PWM){       //20 000 / period = kHz (arriba y abajo)
        case 0x00:
          __HAL_TIM_SET_COMPARE(&tim1,TIM_CHANNEL_1,0);
//          HAL_TIM_OC_Stop(&tim1, TIM_CHANNEL_1);
        break;
        case 0x01:
          __HAL_TIM_SET_COMPARE(&tim1,TIM_CHANNEL_1,400);
//          HAL_TIM_OC_Stop(&tim1, TIM_CHANNEL_1);
//          tim1.Init.Period = 4;
//          HAL_TIM_OC_Init(&tim1);
//          HAL_TIM_OC_Start(&tim1, TIM_CHANNEL_1); 
          break;
        case 0x10:
          __HAL_TIM_SET_COMPARE(&tim1,TIM_CHANNEL_1,600);
//          HAL_TIM_OC_Stop(&tim1, TIM_CHANNEL_1);
//          tim1.Init.Period = 9;
//          HAL_TIM_OC_Init(&tim1);
//          HAL_TIM_OC_Start(&tim1, TIM_CHANNEL_1); 
          break;
        case 0x11:
          __HAL_TIM_SET_COMPARE(&tim1,TIM_CHANNEL_1,800);
//          HAL_TIM_OC_Stop(&tim1, TIM_CHANNEL_1);
//          tim1.Init.Period = 19;
//          HAL_TIM_OC_Init(&tim1);
//          HAL_TIM_OC_Start(&tim1, TIM_CHANNEL_1); 
          break;
        default: break;
      }
    }

  }
}


static void InitTimer1(void){
  
  TIM_OC_InitTypeDef TIM_Channel_InitStruct;                                    //
  
  __HAL_RCC_TIM1_CLK_ENABLE();                                                  //Habilitamos reloj del periférico del tIM2
  
  tim1.Instance = TIM1;                                                         //=¿con qué timer trabajaremos?
  tim1.Init.Prescaler = 83;                                                    //(TIM1 clk = ) 168 MHz/ 840 =  Hz
  tim1.Init.Period = 999;                                                        // 200 000 / 100 = 1kHz (arriba y abajo)
  tim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  HAL_TIM_PWM_Init(&tim1);                                                      //para mayor resolución te renta preescaler de 0 = 1 (pwm, input..)
  
  TIM_Channel_InitStruct.OCMode = TIM_OCMODE_PWM1;                              //cada vez que el timer alcanza el valor de comparación, la salida del pin cambia de estado
  TIM_Channel_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;                      //
  TIM_Channel_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;                       //el modo rapido no es necesario para el toggle
  TIM_Channel_InitStruct.Pulse = 0; 
  HAL_TIM_PWM_ConfigChannel(&tim1, &TIM_Channel_InitStruct, TIM_CHANNEL_1);      //aplicamos configuracion al canal 1 del TIM1
  
  HAL_TIM_PWM_Start(&tim1, TIM_CHANNEL_1);
  
  
  
}

int Init_MsgQueue_PWM(void){
  mid_MsgQueue_PWM = osMessageQueueNew(4, sizeof(uint8_t), NULL);              
  if (mid_MsgQueue_PWM == NULL) {
    return (-1);
  }
  return(0);
}
