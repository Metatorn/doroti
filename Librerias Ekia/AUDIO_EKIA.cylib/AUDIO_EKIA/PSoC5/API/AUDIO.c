/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Archivo cabecera para manejo del reproductor DFPlayer
====================================================*/

#include "CyLib.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "`$INSTANCE_NAME`.h"
#include "`$INSTANCE_NAME`_busy.h"
#include "`$INSTANCE_NAME`_DFP.h"
#include "stdio.h"
#include "stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

uint8 `$INSTANCE_NAME`_bufferSalida[10] = {0x7E, 0xFF, 06, 00, 00, 00, 00, 00, 00, 0xEF}; 

void `$INSTANCE_NAME`_resetBuffer(){
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Command]=0x00;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_ACK]=0x00;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Parameter1]=0x00;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Parameter2]=0x00;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_CheckSum]=0x00;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Verification]=0x00;
}

void `$INSTANCE_NAME`_enviarComando(uint8 comando, uint8 datoAlto, uint8 datoBajo){
    uint16_t sum = 0;
    int i=0;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Command]=comando;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Parameter1]=datoAlto;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Parameter2]=datoBajo;
    for (i=`$INSTANCE_NAME`_Stack_Version; i<`$INSTANCE_NAME`_Stack_CheckSum; i++) {
        sum += `$INSTANCE_NAME`_bufferSalida[i];
    }
    sum = -sum;
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_CheckSum]=(uint8)(sum>>8);
    `$INSTANCE_NAME`_bufferSalida[`$INSTANCE_NAME`_Stack_Verification]=(uint8)(sum);
    taskENTER_CRITICAL();
    `$INSTANCE_NAME``[DFP]`PutArray(`$INSTANCE_NAME`_bufferSalida,10);
    taskEXIT_CRITICAL();
}

void `$INSTANCE_NAME`_iniciar(){
    `$INSTANCE_NAME``[DFP]`Start();
}

void `$INSTANCE_NAME`_apagar(){
    `$INSTANCE_NAME``[DFP]`Stop();
}

void `$INSTANCE_NAME`_siguiente(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_NEXT, 0, 0);
}

void `$INSTANCE_NAME`_anterior(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_PREVIOUS, 0 ,0);
}

void `$INSTANCE_NAME`_reproducir(uint8 numeroArchivo){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_SPEC_TRACK, 0, numeroArchivo);
}

void `$INSTANCE_NAME`_volumenMas(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_INCREASE_VOL, 0, 0);
}

void `$INSTANCE_NAME`_volumenMenos(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_DECREASE_VOL, 0, 0);
}

void `$INSTANCE_NAME`_definirVolumen(uint8 volumen){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_SPEC_VOL, 0, volumen);
}

void `$INSTANCE_NAME`_definirEcualizador(uint8 ecualizador){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_SPEC_EQ, 0, ecualizador);
}

void `$INSTANCE_NAME`_definirReproduccion(uint8 modo){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_PLAYBACK_MODE, 0, modo);
}

void `$INSTANCE_NAME`_definirFuente(uint8 fuente){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_PLAYBACK_SOURCE, 0, fuente);
    CyDelay(200);
}

void `$INSTANCE_NAME`_sleep(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_STANDBY, 0, 0);
}

void `$INSTANCE_NAME`_reset(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_RESET, 0, 0);
}

void `$INSTANCE_NAME`_normal(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_NORMAL_WORK, 0, 0);
}

void `$INSTANCE_NAME`_playback(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_PLAYBACK, 0, 0);
}

void `$INSTANCE_NAME`_pausa(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_PAUSE, 0, 0);
}

void `$INSTANCE_NAME`_reproducirCarpeta(uint8 carpeta, uint8 archivo){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_SPEC_FOLDER, carpeta, archivo);
}

void `$INSTANCE_NAME`_ajustarSalida(CYBIT habilitado, uint8 ganancia){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_VOLUME_ADJUST, ganancia, habilitado);
}

void `$INSTANCE_NAME`_activarRepetirTodo(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_REPEAT, 0, 1);
}

void `$INSTANCE_NAME`_desactivarRepetirTodo(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_REPEAT, 0, 0);
}

void `$INSTANCE_NAME`_reproducirCarpetaMP3(uint8 archivo){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_MP3_FOLDER, 0, archivo);
}

void `$INSTANCE_NAME`_reproducirPropaganda(uint8 archivo){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_ADVERTISE, 0, archivo);
}

void `$INSTANCE_NAME`_detenerPropaganda(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_STOP_ADVERTISE, 0, 0);
}

/*void `$INSTANCE_NAME`_reproducirCarpetaGrande(uint8 carpeta, uint16 archivo){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_PLAY_FOLDER, ((carpeta<<12)|archivo), 0);
}*/

void `$INSTANCE_NAME`detener(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_STOP, 0, 0);
}

void `$INSTANCE_NAME`repetirCarpeta(int carpeta){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_REPEAT_FOLDER, 0, carpeta);
}

void `$INSTANCE_NAME`aleatorio(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_RANDOM, 0, 0);
}

void `$INSTANCE_NAME`habilitarRepeticion(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_LOOP, 0, 0);
}

void `$INSTANCE_NAME`deshabilitarRepeticion(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_LOOP, 0, 1);
}

void `$INSTANCE_NAME`habilitarDAC(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_DAC, 0, 0);
}

void `$INSTANCE_NAME`desHabilitarDAC(){
    `$INSTANCE_NAME`_enviarComando(`$INSTANCE_NAME`_DAC, 0, 1);
}

CYBIT `$INSTANCE_NAME`_ocupado(){
    return `$INSTANCE_NAME``[busy]`Read();
}
/* [] END OF FILE */
