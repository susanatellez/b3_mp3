#include "cmsis_os2.h"                                                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>

#include "principal.h"                                                          //

/*******************************************************************************
 *                           MODULO PRINCIPAL..................................*
********************************************************************************/

#define MODO_REPOSO       0x01
#define MODO_REPRODUCCION 0x02
#define MODO_HORA         0x03

#define EV_JOY            0x01
#define EV_POT            0x02
#define EV_TEMP           0x03

#define NUM_CANCIONES     3

uint8_t modo_actual;
uint8_t evento = 0X00;

// HILO PRINCIPAL = TEST.C
osThreadId_t tid_principal;
//int Init_ThPrincipal (void);                                                  //en el .H
void ThPrincipal (void *argument);

//COLAS DE OTROS MÓDULOS
extern osMessageQueueId_t mid_MsgQueue_Joy;                                     //faltan los demás
extern osMessageQueueId_t mid_MsgQueue_POT;
extern osMessageQueueId_t mid_MsgQueueTemp;
extern osMessageQueueId_t mid_MsgQueueMp3;                                      //FALTA mod MP3
extern osMessageQueueId_t mid_MsgQueueLCD;
extern osMessageQueueId_t mid_MsgQueueMp3Info;
extern osMessageQueueId_t mid_MsgQueue_RGB;
extern osMessageQueueId_t mid_MsgQueue_PWM;
extern osMessageQueueId_t mid_MsgQueueCom;
//extern osMessageQueueId_t mid_MsgQueueComRx;
//COLAS DEL H-PRINCIPAL ASOCIADAS A LOS MÓDULOS
uint8_t msgJoy = 0x00;
uint8_t msgPot = 0x00;
float msgTemp = 0x00;
uint8_t var = 0;
uint8_t msgRGB;
uint8_t msgPWM;
t_comando msgMP3;                                                               //FALTA mod MP3
MsgLCD_t msgLCD;
t_respuesta msg_respuesta;
t_msgcom msgCom;
//
uint8_t file = 0x00;
uint8_t cancion = 0x00;
static uint16_t tiempo_segundos = 0; // Cuenta total de segundos = tiempo de canción
static uint32_t ultimo_tick = 0;

uint8_t estadoSD = 1;
//Datos obtenidos
uint8_t replyMP3status;
uint8_t replyNumTotalFiles;
uint8_t replyCurrentPlaying;
uint8_t replyNumFolders = 3;
uint8_t replyNumFolders_p;
uint8_t replyCurrentVolume;

uint8_t finishedPlaying = 0;
uint8_t playPause; //play 0 Pause 1
uint8_t msgOrden;
//Segundos del reloj
extern uint32_t tiempo;
uint32_t tiempo_mod;

void actualizarLCD_modoReposo (void);
void actualizarLCD_modoReproduccion (void);
void actualizarLCD_modoHora(void);

int Init_ThPrincipal (void){ //Inicializamos hilo y cola
  
  tid_principal = osThreadNew(ThPrincipal, NULL, NULL);               //aquí le cambiamos el tamaño?
  if (tid_principal == NULL) {
    return(-1);
  }
  
  return(0);
}

void ThPrincipal (void *argument){
  

  modo_actual = MODO_REPOSO;
  
  //Comprobaciones iniciales numFiles y numFolders
  //NUM FILES:
  msgMP3.tipo = 0x01;
  msgMP3.com = 0x48;
  msgMP3.dat1 = 0x00;
  msgMP3.dat2 = 0x00;
  osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0U, 0U);
//  //NUM FOLDERS
//  msgMP3.tipo = 0x01;
//  msgMP3.com = 0x4F;
//  msgMP3.dat1 = 0x00;
//  msgMP3.dat2 = 0x00;
//  osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0U, 0U);
  
  while(1){
    
    if (osMessageQueueGet(mid_MsgQueue_Joy, &msgJoy, NULL, 10) == osOK) {
      evento = EV_JOY;
    }
    // Si no hubo joystick, probar POT
    else if (osMessageQueueGet(mid_MsgQueue_POT, &msgPot, NULL, 10) == osOK) {
      evento = EV_POT;
    }
    // Si tampoco, probar TEMP
    else if (osMessageQueueGet(mid_MsgQueueTemp, &msgTemp, NULL, 10) == osOK) {
      evento = EV_TEMP;
      }
//    else if(osMessageQueueGet(mid_MsgQueueComRx, &msgOrden,NULL,10)== osOK){
//      
//    }

    if(osMessageQueueGet(mid_MsgQueueMp3Info,&msg_respuesta,NULL,10)== osOK){
      switch(msg_respuesta.tipo){
        case 0: //= del tipo asíncrono
          estadoSD = msg_respuesta.info;
          if(estadoSD == 0){ //si no hay tarjeta
            modo_actual = MODO_REPOSO;
            msgRGB = 0x03; //ENVIAR PARPADEO RGB ROJO (RGB_NO_SD)
            osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
            msgPWM = 0x01; //PWM_NO_SD
            osMessageQueuePut(mid_MsgQueue_PWM, &msgPWM, 0U, 0U);
          }else if (estadoSD == 1){ //cuando hay tarjeta
            msgRGB = 0x00; //RGB_OFF
            osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
            msgPWM = 0x00; //PWM_OFF
            osMessageQueuePut(mid_MsgQueue_PWM, &msgPWM, 0U, 0U);
          } else if(estadoSD == 3){ //si se ha acabado la cancion
            finishedPlaying = 1;
            msgRGB = 0x04; //ROSITA //////////////////////////////////////////////
            osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
          }
          break;
        //case1&2 no hacen nada
        case 0x42:
          replyMP3status = msg_respuesta.info;
          break;
        case 0x48:
          replyNumTotalFiles = msg_respuesta.info;
          break;
        case 0x4c:
          replyCurrentPlaying = msg_respuesta.info;
          break;
        case 0x4F:
          replyNumFolders_p = msg_respuesta.info;
          break;
        case 0x43:
          replyCurrentVolume = msg_respuesta.info;
          break;
      }
    } //fin del if con los mensajes asíncronos y sincronos
    
    switch(modo_actual){                                                        //SWITCH PRINCIPAL
    
      case MODO_REPOSO :                                                        //MODO REPOSO
          switch(evento){
          case EV_JOY :
            evento = 0x00; //borramos flag
            if(msgJoy == 0x50 && estadoSD > 1){
              
              //Hacemos el wake up
              msgMP3.tipo = 0x00;
              msgMP3.com = 0x0B;
              msgMP3.dat1 = 0x00;
              msgMP3.dat2 = 0x00;
              osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);

              //Ponemos volumen
              if (msgPot > 30) msgPot = 30;
              msgMP3.tipo = 0x00;
              msgMP3.com = 0x06;
              msgMP3.dat1 = 0x00;
              msgMP3.dat2 = msgPot;
              osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
              osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
              
              //Empezamos a reproducir la primera canción
              msgMP3.tipo = 0x00;
              msgMP3.com = 0x0F;
              msgMP3.dat1 = 0x01;
              msgMP3.dat2 = 0x01;
              osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
              osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
              
              file = msgMP3.dat1;
              cancion = msgMP3.dat2;
              msgRGB = 0x01; //VERDE
              osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
              
              //Cambiamos al siguiente modo
              playPause = 0; //play 0 Pause 1
              tiempo_segundos = 0;
              actualizarLCD_modoReproduccion();
              modo_actual = MODO_REPRODUCCION;
            }
            break;
            
          case EV_TEMP:
          actualizarLCD_modoReposo();
          break;
          
          default: break;
        }
        break;                                                                  //FIN modo reposo
        
      case MODO_REPRODUCCION :                                                  //MODO REPDRODUCCIÓN
        switch(evento){//tipo de evento
          case EV_JOY:
            evento = 0x00;//borramos flag
            switch(msgJoy){
              
              case 0x01:                                                        // UP: siguiente carpeta (1º canción)
                msgMP3.tipo = 0x00;
                msgMP3.com = 0x0F;
                msgMP3.dat1 = file<replyNumFolders ? file+1 : 0x01;
                msgMP3.dat2 = 0x01;
                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              
                msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
                osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
              
                file = msgMP3.dat1;
                cancion = msgMP3.dat2;
                msgRGB = 0x01; //RGB_PLAY
                osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                finishedPlaying = 0;
                tiempo_segundos = 0;
                actualizarLCD_modoReproduccion();
                break;
              
              case 0x02:                                                        //RIGHT: siguiente canción (de la misma carpeta)
                msgMP3.tipo = 0x00;
                msgMP3.com = 0x0F;
                msgMP3.dat1 = file;
                msgMP3.dat2 = (cancion<NUM_CANCIONES) ? cancion+1 : 0x01;
                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              
                msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
                osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
              
                file = msgMP3.dat1;
                cancion = msgMP3.dat2;
                msgRGB = 0x01; //RGB_PLAY
                osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                finishedPlaying = 0;
                tiempo_segundos = 0;
                actualizarLCD_modoReproduccion();
                break;
              
              case 0x03:                                                        //DOWN: carpeta anterior
                msgMP3.tipo = 0x00;
                msgMP3.com = 0x0F;
                msgMP3.dat1 = file>1 ? file-1 : replyNumFolders;
                msgMP3.dat2 = 0x01;
                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
                
                msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
                osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
                
                file = msgMP3.dat1;
                cancion = msgMP3.dat2;
                msgRGB = 0x01; //RGB_PLAY
                osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                finishedPlaying = 0;
                tiempo_segundos = 0;
                actualizarLCD_modoReproduccion();
                break;
              
              case 0x04:                                                        //LEFT: canción anterior
                msgMP3.tipo = 0x00;
                msgMP3.com = 0x0F;
                msgMP3.dat1 = file;
                msgMP3.dat2 = cancion>1 ? cancion-1 : NUM_CANCIONES;
                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              
                msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
                osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
              
                file = msgMP3.dat1;
                cancion = msgMP3.dat2;
                msgRGB = 0x01; //RGB_PLAY
                osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                finishedPlaying = 0;  
                tiempo_segundos = 0;
                actualizarLCD_modoReproduccion();
              break;
              
              case 0x05:                                                        //CENTER (CORTO)
                if (playPause == 0){    //Si esta "Runeando"
                  playPause = 1;
                  msgMP3.tipo = 0x00;
                  msgMP3.com = 0x0E;    //= PAUSE
                  msgMP3.dat1 = 0x00;
                  msgMP3.dat2 = 0x00;
                  osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
                  
                  msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
                  osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
                  
                  msgRGB = 0x02; //AZUL
                  osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                  msgPWM = 0X03; //PWM_PAUSE
                  osMessageQueuePut(mid_MsgQueue_PWM, &msgPWM, 0U, 0U);
                  actualizarLCD_modoReproduccion();
                } else if (playPause == 1){   //si está pausado
                  playPause = 0;
                  msgMP3.tipo = 0x00;
                  msgMP3.com = 0x0D;                                            //= PLAY
                  msgMP3.dat1 = 0x00;
                  msgMP3.dat2 = 0x00;
                  osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
              osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
                  msgRGB = 0x01; //RGB_PLAY
                  osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                  msgPWM = 0X02; //PWM_Play
                  osMessageQueuePut(mid_MsgQueue_PWM, &msgPWM, 0U, 0U);
                  actualizarLCD_modoReproduccion();
                }
                break;
              
//              case 0x20:
//                msgMP3.com = 0x;
//                msgMP3.dat1 = 0x;
//                msgMP3.dat2 = 0x0;
//                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
//                break;
                
//              case 0x40:
//                msgMP3.com = 0x;
//                msgMP3.dat1 = 0x;
//                msgMP3.dat2 = 0x0
//                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
//                break;
              
              case 0x50:
                //Hacemos el sleep
                msgMP3.tipo = 0x00;
                msgMP3.com = 0x0A;
                msgMP3.dat1 = 0x00;
                msgMP3.dat2 = 0x00;
                osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
              
              msgCom.longitud = sprintf(msgCom.mensaje,"%02d:%02d:%02d --> 7E FF 06 %02X 00 %02X %02X EF \r",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60,msgMP3.com, msgMP3.dat1, msgMP3.dat2);
              osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
              
                actualizarLCD_modoHora();
                msgRGB = 0x00; //RGB_OFF
                osMessageQueuePut(mid_MsgQueue_RGB, &msgRGB, 0U, 0U);
                tiempo_mod = tiempo;
                modo_actual = MODO_HORA;
                break;
              
//              default: break;
            }
          break; //del case EV_JOY dentro del case MODO_REPRODUCCION
        
          case EV_POT:
            evento = 0x00;        //borramos flag
            if (msgPot > 30) msgPot = 30;
            msgMP3.tipo = 0x00;
            msgMP3.com = 0x06;    // = SET VOLUME
            msgMP3.dat1 = 0x00;
            msgMP3.dat2 = msgPot;
            osMessageQueuePut(mid_MsgQueueMp3, &msgMP3, 0, 0);
          

          
            actualizarLCD_modoReproduccion();
            break;
          
//          default: break;
        }//fin del swhitch(evento), MODO_REPRODUCCION
        
        break;                                                                  //FIN modo reprodcucion
        
      case MODO_HORA :                                                          //MODO_HORA
        switch(evento){
          uint8_t confirmado=0;
          case EV_JOY:
            evento = 0x00; //borramos flag
            actualizarLCD_modoHora();
            switch(msgJoy){
              case 0x01:
                if(var==0){
              tiempo_mod++;
                }else if(var == 1){
                  tiempo_mod = tiempo_mod + 60;
                }else if(var==2){
                tiempo_mod = tiempo_mod + 3600;
                }
             if (tiempo_mod >= 86400) {
              tiempo_mod -= 86400;
             }
                break;
              case 0x02:
                var = (var  > 0) ? var - 1 : 2;
                break;
              case 0x03:
                if(var==0){
              tiempo_mod = tiempo_mod -1;
                }else if(var == 1){
                  tiempo_mod = tiempo_mod - 60;
                }else if(var==2){
                  tiempo_mod = tiempo_mod - 3600;
                }
              if (tiempo_mod >= 86400) {
              tiempo_mod += 86400;
              }
                break;
              case 0x04:
                var = (var < 2) ? var+1 :0;
                
                break;
              case 0x05:
                confirmado = 1;
                break;
              case 0x50:
                if(confirmado == 1){
                confirmado = 0;
                tiempo = tiempo_mod;
                }
                actualizarLCD_modoReposo();
                modo_actual = MODO_REPOSO;
                break;
            }
            break;
          case EV_TEMP:
            evento = 0x00;
            actualizarLCD_modoHora();
          break;
        break;                                                                  //FIN modo hora
          
//      default:
//        modo_actual = MODO_REPOSO;
//        break;
          
      } //fin switch(evento) Case MODO_HORA
    }                                                                           //FIN del switch principal

    if (modo_actual == MODO_REPRODUCCION && playPause == 0 && finishedPlaying == 0) {
      uint32_t tick_actual = osKernelGetTickCount();                              // Obtenemos el tiempo actual del sistema
      if (tick_actual - ultimo_tick >= 1000) {                                    // Si ha pasado 1 segundo (1000 ticks) desde la última vez
        tiempo_segundos++;                                                      // Sumamos un segundo
        ultimo_tick = tick_actual;                                              // Reseteamos la marca de tiempo
        actualizarLCD_modoReproduccion();                                       // Refrescamos la pantalla
      }
    }


  } //fin del while(1)
} //FIN ThPrincipal

//Definicion/declaracion de funciones

void actualizarLCD_modoReposo (void){
  LCD_limpiarBuffer();
  msgLCD.modo = 1;
  msgLCD.linea = 0;
  sprintf(msgLCD.texto, " SBM 2025   T:%.1fC",msgTemp);
  osMessageQueuePut(mid_MsgQueueLCD, &msgLCD,0,0);
  msgLCD.linea = 2;
  sprintf(msgLCD.texto,"   %02d:%02d:%02d ",tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60);
  osMessageQueuePut(mid_MsgQueueLCD, &msgLCD,0,0);
   msgCom.longitud = sprintf(msgCom.mensaje,"---------------------\r SBM 2025   T:%.1fC\r   %02d:%02d:%02d \r",msgTemp,tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60);
  osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
}

void actualizarLCD_modoReproduccion (void){
  uint8_t min = tiempo_segundos / 60;
  uint8_t sec = tiempo_segundos % 60;
  LCD_limpiarBuffer();
  msgLCD.modo = 2;
  msgLCD.linea = 0;
  msgCom.longitud = sprintf(msgLCD.texto, "F:%02d C:%02d      VOL:%02d", file, cancion, msgPot);
  osMessageQueuePut(mid_MsgQueueLCD, &msgLCD, 0, 0);
  msgLCD.linea = 2;
  msgCom.longitud = sprintf(msgLCD.texto, "      T %02d:%02d  ", min, sec);
  osMessageQueuePut(mid_MsgQueueLCD, &msgLCD, 0, 0);
  msgCom.longitud = sprintf(msgCom.mensaje,"---------------------\rF:%02d C:%02d      VOL:%02d\r      T %02d:%02d  \r",file, cancion, msgPot, min, sec);
  osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
}

void actualizarLCD_modoHora(void){
  LCD_limpiarBuffer();
  msgLCD.modo = 3;
  msgLCD.linea = 0;
  sprintf(msgLCD.texto, "   HORA    T:%.1fC",msgTemp);
  osMessageQueuePut(mid_MsgQueueLCD, &msgLCD,0,0);
  msgLCD.linea = 2;
  if(var == 0 ){
      sprintf(msgLCD.texto,"   %02d:%02d:%02d  ",tiempo_mod / 3600,(tiempo_mod % 3600) / 60,tiempo_mod % 60);
      msgCom.longitud = sprintf(msgCom.mensaje,"---------------------\r    HORA    T:%.1fC    \r    %02d:%02d:%02d\r          --\r",msgTemp,tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60);
      osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
  }else if(var == 1){
      sprintf(msgLCD.texto,"   %02d:%02d:%02d  ",tiempo_mod / 3600,(tiempo_mod % 3600) / 60,tiempo_mod % 60);
    msgCom.longitud = sprintf(msgCom.mensaje,"---------------------\r    HORA    T:%.1fC    \r    %02d:%02d:%02d\r       --   \r",msgTemp,tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60);
      osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
  }else if(var == 2){
      sprintf(msgLCD.texto,"   %02d:%02d:%02d  ",tiempo_mod / 3600,(tiempo_mod % 3600) / 60,tiempo_mod % 60);
    msgCom.longitud = sprintf(msgCom.mensaje,"---------------------\r    HORA    T:%.1fC    \r    %02d:%02d:%02d\r    --      \r",msgTemp,tiempo / 3600,(tiempo % 3600) / 60,tiempo % 60);
      osMessageQueuePut(mid_MsgQueueCom, &msgCom, 0, 0);
  }
  osMessageQueuePut(mid_MsgQueueLCD, &msgLCD,0,0);
  
;
  
}
