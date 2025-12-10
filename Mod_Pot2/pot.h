#ifndef __POT_H
#define __POT_H

//Para que funciona he tenido que añadir en "Manage Run-Time Enviroment"
//en Device => STM32Cube HAL => ADC
#include "stm32f4xx_hal.h"                                                      //para definir ADC_HandleTypeDef etc etc

void initModPOT (void);

#endif
