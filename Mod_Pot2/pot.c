#include "cmsis_os2.h"                                                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>

#include "pot.h"                                                                //
#include "math.h"
/*******************************************************************************
 * MODULO POT
 * ADC (Analog-to-Digital Converter, convertidor analógico-digital)
 * toma una señal eléctrica analógica, en este caso la tensión en el terminal
 * del potenciómetro y la convierte en un número entero digital para que el
 * micro pueda “leer” sensores/potenciómetros que entregan voltaje
 *******************************************************************************/

#define VREF 3.3f                                                               //Nosostros lo hemos conectado al típico
#define RESOLUTION_12B 4096.0f

static GPIO_InitTypeDef GPIO_InitStruct;                                        //
//static void initModPOT (void);                                                //está en el .H

// HILO DEL JOYSTICK
osThreadId_t tid_POT;                                                           //no me va si lo muevo al .H
uint32_t statusQueuePOT = 0x0000;
int Init_ThPOT (void);

static ADC_HandleTypeDef adchandle; // handle inicializado y usado para las operaciones ADC
float value;
float redondeado;
void ThPOT (void *argument);

//Para inicializar el ADC_HandlerTypeDef                                        //estan en el .H
//int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef *ADC_Instance);
//Para leer un canal y devolver el voltaje en float
//float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel);

// QUEUE
osMessageQueueId_t mid_MsgQueue_POT;
uint8_t MsgQueue_POT = 0x00;                                                    //mensaje de metes a la cola
int Init_MsgQueue_POT(void);


void initModPOT (void){

  //ADC1_pins_F429ZI_config(){... funciónd e configuración de los pines
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // Configuración para ADC1_IN10 (PC0)
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  //Inicialización del ADC
  ADC_Init_Single_Conversion(&adchandle, ADC1);

  //Inicialización de la cola
  Init_MsgQueue_POT();

  //Inicialización del hilo
  Init_ThPOT();

}

int Init_ThPOT (void){

  const osThreadAttr_t ThPOT_attributes = {
    .name = "ThPOT",
    .stack_size = 512U, // <- Nuevo tamaño
  };

  tid_POT = osThreadNew(ThPOT, NULL, &ThPOT_attributes);
  if (tid_POT == NULL) {
    return(-1);
  }
  return(0);

}


void ThPOT (void *argument){
  uint8_t msg_ant = 0x00;

  while (1) {
    value = ADC_getVoltage(&adchandle, 10); // El ADC solo conoce canales por eso le pasamos el CN10
    redondeado = ((int)(value*100))/100.0f;
    MsgQueue_POT = (uint8_t)((redondeado-0.15) / (3.26-0.15) * 30.0f);
    if(msg_ant != MsgQueue_POT){
    //En nuestra placa va del 0.14 al 3.29   de manera que se ve 3-99 por eso las cuentas se ven RARUNAS
    statusQueuePOT = osMessageQueuePut(mid_MsgQueue_POT, &MsgQueue_POT, NULL, 10U);
    osDelay(1000);
     msg_ant = MsgQueue_POT;
    }

  }

}


int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef *ADC_Instance) {

  hadc->Instance = ADC_Instance; //ASIGNA LA INSTANCIA FíSICA DEL ADC, para que HAL sepa a qué ADC referirse
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;//Divide la velocidad del reloj del DC, usamos la mitad del reloj periférico
  hadc->Init.Resolution = ADC_RESOLUTION_12B; //Pone resolución a 12 bits (0..4095). Eso implica pasos de Vref/4096 (afecta a la precisión del valor digital)
  hadc->Init.ScanConvMode = DISABLE; //Desactiva el scan (no se recorren múltiples canales automáticamente)
  hadc->Init.ContinuousConvMode = DISABLE; //hace una conversión cada vez que se pide (modo single)
  hadc->Init.DiscontinuousConvMode = DISABLE; //se usa si ScanConvMode está activado y se quiere convertir en grupos, no aplicable aquí
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; //No hay disparo externo por flanco, se usará software para iniciar la conversión
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START; //Indica el origen del disparo, la conversión se iniciará por software
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT; //Alineación de datos a la derecha (valor crudo en 12 bits almacenado en 16 bits)
  hadc->Init.NbrOfConversion = 1; //Número de conversiones en la secuencia = una sola medición por disparo = solo quiero medir un canal cada vez
  //DMA: Direct Memory Acces, mueve datos automáticamente, sin usar la CPU. Cuando el ADC termina una conversión, genera un número.y este número hay que guardarlo en memoria
  hadc->Init.DMAContinuousRequests = DISABLE;//con DMA es como su tuvieses un ayudante que te guarda los valores, sin que la CPU haga nada
  //lo deberíamos habilitar si quisiesemos muestrear rápido, muchas mediciones por segundo, si quisiese liberar la CPU para otras cosas...
  hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV; //Indica cuando se considera final de conversión (EOC) = cuándo puedes leer el valor
  //SINGLE_CONV significa que EOC se produce por cada conversión.

  if (HAL_ADC_Init(hadc) != HAL_OK) {
      return -1;
  }
  return 0;

}


float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel) {
    ADC_ChannelConfTypeDef sConfig = {0}; //Estructura local para la configuración del canal ADC; inicializada a 0
    HAL_StatusTypeDef status;
    uint32_t raw = 0; //valor devuelto por el ADC
    float voltage = 0;//valor convertio a voltios

    sConfig.Channel = Channel;//canal por el que queremos leer
    sConfig.Rank = 1; //Posición en la secuencia de conversiones, aquí la primera y única
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;//cuánto tiempo el ADC “muestra” la señal antes de convertir

    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) { //configura el canal en el ADC
      return -1;
    }

    HAL_ADC_Start(hadc); //Inicia conversión por software

    do
        status = HAL_ADC_PollForConversion(hadc, 0); 
    while (status != HAL_OK);

    //EESTO ES LO IMPORTANTE
    raw = HAL_ADC_GetValue(hadc); //Lee el resultado del ADC
    voltage = raw * VREF / RESOLUTION_12B; //lo convertimos a voltios

    return voltage;//devolvemos el valor en voltios
}

int Init_MsgQueue_POT(void){
  mid_MsgQueue_POT = osMessageQueueNew(4, sizeof(uint8_t), NULL);              
  if (mid_MsgQueue_POT == NULL) {
    return (-1);
  }
  return(0);
}
