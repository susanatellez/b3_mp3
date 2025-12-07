#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include "temp.h"
#include "Driver_I2C.h"

//Cabeceras y variables del I2C
extern ARM_DRIVER_I2C Driver_I2C1;
static ARM_DRIVER_I2C *I2Cdrv = &Driver_I2C1; //puntero que apunta al driver I2C1

void I2C_Callback(uint32_t eventId);
void initI2C(void);

int medirTemp(float *temp_c);

//Cabeceras y variables del hilo y la cola
int Init_ThTemp(void);
void Temp (void *argument);
int Init_MsgQueue_Temp(void);

osThreadId_t tid_Temp;
osMessageQueueId_t mid_MsgQueueTemp;

float t; //variable guarda temperatura

//Dirección del sensor en la linea I2C A2,A1 y A0 = GND = 0 
const uint8_t addr = 0x48; //dirección I2C del sensor de temperatura

//Dirección de los registros internos (por si hiciera falta)
const uint8_t reg_conf = 0x01;
const uint8_t reg_temp = 0x00; //Dirección del registro de temperatura dentro del sensor



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
  //Inicializa la unidad I2C del microcontrolador y registra la función I2C_Callback
  //para ser llamada cuando ocurran eventos asíncronos (como el fin de una transferencia).
  I2Cdrv->Initialize(I2C_Callback);
  //Enciende el periférico I2C (lo habilita) y habilita los recursos asociados
  I2Cdrv->PowerControl(ARM_POWER_FULL);
  //Configura velocidad del bus al estándar que es 100kHz
  I2Cdrv->Control(ARM_I2C_BUS_SPEED,ARM_I2C_BUS_SPEED_STANDARD);
  //Medida de robustez
  //intentar liberar o restablecer el bus I2C si está bloqueado por un dispositivo esclavo
  I2Cdrv->Control(ARM_I2C_BUS_CLEAR,0);
}


//Función de callback del I2C
void I2C_Callback(uint32_t eventId){
  
  if(eventId & ARM_I2C_EVENT_TRANSFER_DONE){ //si la transferencia ha sido exitosa
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
    if(medirTemp(&t) ==0){ //si ha medido la temperatura con éxito
      osMessageQueuePut(mid_MsgQueueTemp,&t,0,0);
    }
   osDelay(1000);
  }
}

int medirTemp (float *temp_c){

  //Buffer de lectura de temperatura,
  uint8_t buf[2]; //aquí se almacenarán los datos que vienen directamente del I2C
  int16_t temperatura; //reconstruir los datos de la Temperatura dados por el I2C


  // => Lee el registro 0x00 (que es el de la temperatura)
  I2Cdrv->MasterTransmit(addr,&reg_temp,1,true);
  //(direccionSensor, datoAenviar, tamañoDatos, noCondicionStop)
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);


  // => Envíame los 2 bytes de datos que acabo de decirte que leas
  I2Cdrv->MasterReceive(addr,buf,2,false);
  //(deDndSeRecibenLosDatos, dndSeAlmacenaranLosDatos, tamañoDatosArecibir, condicionStopActivada)
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);

  //La Condicion de Stop libera el bus I2C de manera que en la primera línea crucial
  //no tenerlo activado porque prepara el bus para la fase de lectura inmediata

  //osThreadFlagsWait es para que el hilo se bloquee, espere pasivamente, sin consumir recursos de la CPU

  // Medida que entrega: 11 bits en complemento a 2, resolución 0.125 ºC
  temperatura = ((buf[0] << 8) | buf[1]); //Invierte los bytes recibidos, buf[0] mas significativos, buf[1] menos, los junta en 16 bits
  temperatura >>= 5;                      // Dejar 11 bits significativos (visto en el datasheet)
  *temp_c = temperatura * 0.125f;         //Para redondear en sprintf a 1 decimal ("Temp = %.1f ºC\n", *temp_c);

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