#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

//static GPIO_InitTypeDef GPIO_InitStruct;                            //hay que borrarlo del .c
void initModJoy (void);                                                  //este también

//extern osThreadId_t tid_joy;                                       //este no lo sé ahora mismo jajaj

typedef struct {
    uint16_t pin;
    GPIO_TypeDef *port;
} mygpio_pin;
extern mygpio_pin pin_presionado;



#endif
