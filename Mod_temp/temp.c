#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include "temp.h"
#include "Driver_I2C.h"

//Cabeceras y variables del I2C
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1;



void I2C_Callback(uint32_t eventId);
void initI2C(void);

int medirTemp(float *temp_c);

//Cabeceras y variables del hilo 
int Init_ThTemp(void);
void Temp (void *argument);
int Init_MsgQueue_Temp(void);

osThreadId_t tid_Temp;
osMessageQueueId_t mid_MsgQueueTemp;

float t;

//Dirección del sensor en la linea I2C
const uint8_t addr = 0x48;

//Dirección de los registros internos (por si hiciera falta)
const uint8_t reg_conf = 0x01;
const uint8_t reg_temp = 0x00;



//------------------------Inicializacion del módulo-----------------------------
void initModTemp (void){
  initI2C();
  Init_ThTemp();
  Init_MsgQueue_Temp();
}

//******************************************************************************
//-----------------------------------I2C----------------------------------------
//******************************************************************************
void initI2C(void){

  I2Cdrv->Initialize(I2C_Callback);
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  I2Cdrv->Control(ARM_I2C_BUS_SPEED,ARM_I2C_BUS_SPEED_STANDARD);
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR,0);
}


//Función de callback del I2C
void I2C_Callback(uint32_t eventId){
  

  if(eventId & ARM_I2C_EVENT_TRANSFER_DONE){
    osThreadFlagsSet(tid_Temp,0x01);
  }
}

//---------------------------------HILO-----------------------------------------
int Init_ThTemp (void){
    tid_Temp = osThreadNew(Temp, NULL, NULL);
    if (tid_Temp == NULL) {
    return(-1);
  }
    return(0);
}
void Temp (void *argument){

  while(1){
    if(medirTemp(&t) ==0){
      osMessageQueuePut(mid_MsgQueueTemp,&t,0,0);
    }
   osDelay(1000);
  }
}
int medirTemp (float *temp_c){

//Buffer de lectura de temperatura
  uint8_t buf[2];
  //Temperatura
  uint16_t temperatura;


  I2Cdrv->MasterTransmit(addr,&reg_temp,1,true);                                
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);


  I2Cdrv->MasterReceive(addr,buf,2,false);                                      
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  

  // 11 bits en complemento a 2, resolución 0.125 ºC
  temperatura = (int16_t)((buf[0] << 8) | buf[1]);
  temperatura >>= 5;                    // Dejar 11 bits significativos 
  *temp_c = (float)temperatura * 0.125f;                                        //Para redondear en sprintf a 1 decimal ("Temp = %.1f ºC\n", *temp_c);

  return 0;
}
//--------------------------------COLA------------------------------------------
int Init_MsgQueue_Temp(void) {
 
  mid_MsgQueueTemp = osMessageQueueNew(4,sizeof(float), NULL);                              
  if (mid_MsgQueueTemp == NULL) {
    return (-1);
  }
  return(0);
}