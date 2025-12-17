#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include "lcd.h"
#include "Driver_SPI.h" 
#include "Arial12x12.h"
/*------------------------------------------------------------------------------
-------------------------------- MODULO LCD ------------------------------------
------------------------------------------------------------------------------*/

//Driver SPI
extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

//Prototipos hilo
int Init_ThLCD (void);
void LCD (void *argument);
int Init_MsgQueue_LCD(void);
//Prototipos hardware
void LCD_reset(void);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char cmd) ;
void LCD_init (void);
void LCD_update (void);
void symbolToLocalBuffer(uint8_t line,uint8_t symbol);
void LCD_writeLine(uint8_t line, const char* texto);
void LCD_llenarBuffer(void) ;

void LCD_Callback (uint32_t event);

//Variables
static unsigned char buffer[512];
MsgLCD_t mensaje;
static uint16_t index;
extern uint8_t var;
osThreadId_t tid_LCD;
osMessageQueueId_t mid_MsgQueueLCD;

//Función de inicialización principal
void initModLCD (void){
  //Inicialización del hardware del LCD
  LCD_reset();
  LCD_init();
  LCD_limpiarBuffer();
  //Inicialización del hilo gestor de mensajes
  Init_ThLCD();
  Init_MsgQueue_LCD();
}
//FUNCIÓNES RELATIVAS A LOS HILOS

int Init_ThLCD (void) {
  const osThreadAttr_t ThLCD_attributes = {
    .name = "ThLCD", //Poner este campo si no no funciona de manera correcta
    .stack_size = 512U,
  };
  tid_LCD = osThreadNew(LCD, NULL, &ThLCD_attributes);
  if (tid_LCD == NULL) {
    return(-1);
  }
 
  return(0);
}
void LCD (void *argument){
//  MsgLCD_t mensaje;
  while(1){
      if(osMessageQueueGet
        
      
      (mid_MsgQueueLCD,&mensaje,NULL,10U)==osOK){
      LCD_writeLine(mensaje.linea, mensaje.texto);
      if(mensaje.linea == 2){
        rayita(var);
      }
      LCD_update();
     }
   }
 }


int Init_MsgQueue_LCD(void) {
 
  mid_MsgQueueLCD = osMessageQueueNew(4,sizeof(MsgLCD_t) , NULL);//MIRAR  BIEN TAMAÑOS
  if (mid_MsgQueueLCD == NULL) {
    return (-1);
  }
  return(0);
}
//FUNCIONES DE CONFIGURACIÓN HARWARE DEL LCD
void LCD_reset(void){

  GPIO_InitTypeDef GPIO_InitStruct;

  SPIdrv->Initialize(LCD_Callback);
  SPIdrv->PowerControl(ARM_POWER_FULL);

  SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000); //

  SPIdrv->Control (ARM_SPI_CONTROL_SS , ARM_SPI_SS_INACTIVE );

  //RESET PA6
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  //A0 PF13
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  //CS PD14
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  // SECUENCIA DE RESET
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  HAL_Delay(2);
}

void LCD_wr_data(unsigned char data) { 
  // Seleccionar CS = 0;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,GPIO_PIN_RESET);
  // Seleccionar A0 = 1; 
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
  // Escribir un dato (data) usando la función SPIDrv->Send(…); 
  SPIdrv->Send(&data,1);
  // Esperar a que se libere el bus SPI; 
  //while(SPIdrv->GetStatus().busy);  
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);  
  // Seleccionar CS = 1;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,GPIO_PIN_SET);
}

void LCD_wr_cmd(unsigned char cmd) { 
  // Seleccionar CS = 0; 
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,GPIO_PIN_RESET);
  // Seleccionar A0 = 0;
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
  // Escribir un comando (cmd) usando la función SPIDrv->Send(…); 
  SPIdrv->Send(&cmd,1);
  // Esperar a que se libere el bus SPI; 
  //while(SPIdrv->GetStatus().busy);
  osThreadFlagsWait(0x01, osFlagsWaitAny,osWaitForever);
  // Seleccionar CS = 1; } 
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,GPIO_PIN_SET);
}

void LCD_init (void){

  LCD_wr_cmd(0xAE);	//Display off
  LCD_wr_cmd(0xA2);	//Vpol = 1/9
  LCD_wr_cmd(0xA0);	//Direccionamiento de RAM normal
  LCD_wr_cmd(0xC8);	//Scan de salidas COM es el normal
  LCD_wr_cmd(0x22);	//Relacion Rint = 2
  LCD_wr_cmd(0x2F);	//Power on
  LCD_wr_cmd(0x40); //El display empieza en la linea 0
  LCD_wr_cmd(0xAF); //Display ON
  LCD_wr_cmd(0x81);//Contraste
  LCD_wr_cmd(0x0F);//rellenar con contraste al gusto
  LCD_wr_cmd(0xA4); //Display all points normal
  LCD_wr_cmd(0xA6); //LCD display normal
 }


void LCD_update (void){

  int i;
  LCD_wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0     
  LCD_wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0     
  LCD_wr_cmd(0xB0);      // Página 0 

  for(i=0;i<128;i++){
    LCD_wr_data(buffer[i]);
  }
  LCD_wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0     
  LCD_wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0     
  LCD_wr_cmd(0xB1);      // Página 1 

  for(i=128;i<256;i++){
    LCD_wr_data(buffer[i]);
  }
  LCD_wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0     
  LCD_wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0     
  LCD_wr_cmd(0xB2);      // Página 2 

  for(i=256;i<384;i++){
    LCD_wr_data(buffer[i]);
  }

  LCD_wr_cmd(0x00);      // 4 bits de la parte baja de la dirección a 0     
  LCD_wr_cmd(0x10);      // 4 bits de la parte alta de la dirección a 0     
  LCD_wr_cmd(0xB3);      // Página 3 

  for(i=384;i<512;i++){
    LCD_wr_data(buffer[i]);
  }
}

void symbolToLocalBuffer(uint8_t line,uint8_t symbol){
  uint8_t i, value1, value2;
  uint16_t offset=0;

  offset=25*(symbol - ' ');

  for(i=0; i<12 ; i++){
    value1=Arial12x12[offset+i*2+1];
    value2=Arial12x12[offset+i*2+2];
    buffer[i+index+(128*line)] = value1;
    buffer[i+128+index+(128*line)] = value2;
  }
  index = index + Arial12x12[offset];
}
void LCD_writeLine(uint8_t line, const char* texto){

  uint8_t i;
  index = 0;
  
  for(i=0; texto[i] != '\0'; i++){
    symbolToLocalBuffer(line,texto[i]);	
  }
}
/*
void LCD_llenarBuffer(void) {
    
    for(int i = 0; i < 512; i++) {
        buffer[i] = 0xFF;
    }
}
*/
void LCD_limpiarBuffer(void) {
    
    for(int i = 0; i < 512; i++) {
        buffer[i] = 0x00;
    }
}

void LCD_Callback (uint32_t event){
  if(event & ARM_SPI_EVENT_TRANSFER_COMPLETE){
  osThreadFlagsSet(tid_LCD,0x01);
  }
}

void rayita (uint8_t unidad){

  if(mensaje.modo == 3){
    if(unidad == 0){ //segundos
      for(int i =  434; i< 444; i++){
        buffer[i] = 0x02;
      }
    }else if(unidad == 1){
      for(int i =  420; i< 430; i++){
        buffer[i] = 0x02;
      }
    }else if(unidad == 2){
        for(int i =  407; i< 417; i++){
          buffer[i] = 0x02;
      }
    }
  } //fin if mensaje.como

}




