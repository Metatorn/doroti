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

#ifndef `$INSTANCE_NAME`CONFIG_H
#define `$INSTANCE_NAME`CONFIG_H
    
#include "cypins.h"
#include "CyLib.h"
#include "stdlib.h"
    
#define `$INSTANCE_NAME`_Stack_Header        0
#define `$INSTANCE_NAME`_Stack_Version       1
#define `$INSTANCE_NAME`_Stack_Length        2
#define `$INSTANCE_NAME`_Stack_Command       3
#define `$INSTANCE_NAME`_Stack_ACK           4
#define `$INSTANCE_NAME`_Stack_Parameter1    5
#define `$INSTANCE_NAME`_Stack_Parameter2    6    
#define `$INSTANCE_NAME`_Stack_CheckSum      7
#define `$INSTANCE_NAME`_Stack_Verification  8    
#define `$INSTANCE_NAME`_Stack_End           9   
    
#define `$INSTANCE_NAME`_EQ_NORMAL      0
#define `$INSTANCE_NAME`_EQ_POP         1
#define `$INSTANCE_NAME`_EQ_ROCK        2
#define `$INSTANCE_NAME`_EQ_JAZZ        3
#define `$INSTANCE_NAME`_EQ_CLASSIC     4
#define `$INSTANCE_NAME`_EQ_BASS        5

#define `$INSTANCE_NAME`_DEVICE_U_DISK  1
#define `$INSTANCE_NAME`_DEVICE_SD      2
#define `$INSTANCE_NAME`_DEVICE_AUX     3
#define `$INSTANCE_NAME`_DEVICE_SLEEP   4
#define `$INSTANCE_NAME`_DEVICE_FLASH   5    
    
#define `$INSTANCE_NAME`_NEXT                0x01
#define `$INSTANCE_NAME`_PREVIOUS            0x02
#define `$INSTANCE_NAME`_SPEC_TRACK          0x03
#define `$INSTANCE_NAME`_INCREASE_VOL        0x04
#define `$INSTANCE_NAME`_DECREASE_VOL        0x05
#define `$INSTANCE_NAME`_SPEC_VOL            0x06
#define `$INSTANCE_NAME`_SPEC_EQ             0x07
#define `$INSTANCE_NAME`_PLAYBACK_MODE       0x08
#define `$INSTANCE_NAME`_PLAYBACK_SOURCE     0x09
#define `$INSTANCE_NAME`_STANDBY             0x0A
#define `$INSTANCE_NAME`_NORMAL_WORK         0x0B
#define `$INSTANCE_NAME`_RESET               0x0C
#define `$INSTANCE_NAME`_PLAYBACK            0x0D
#define `$INSTANCE_NAME`_PAUSE               0x0E
#define `$INSTANCE_NAME`_SPEC_FOLDER         0x0F
#define `$INSTANCE_NAME`_VOLUME_ADJUST       0x10
#define `$INSTANCE_NAME`_REPEAT              0x11
#define `$INSTANCE_NAME`_MP3_FOLDER          0x12
#define `$INSTANCE_NAME`_ADVERTISE           0x13
#define `$INSTANCE_NAME`_PLAY_FOLDER         0x14 
#define `$INSTANCE_NAME`_STOP_ADVERTISE      0x15
#define `$INSTANCE_NAME`_STOP                0x16
#define `$INSTANCE_NAME`_REPEAT_FOLDER       0x17
#define `$INSTANCE_NAME`_RANDOM              0x18
#define `$INSTANCE_NAME`_LOOP                0x19
#define `$INSTANCE_NAME`_DAC                 0x1A    
    
void `$INSTANCE_NAME`_enviarComando(uint8 comando, uint8 datoAlto, uint8 datoBajo);
void `$INSTANCE_NAME`_iniciar();
void `$INSTANCE_NAME`_apagar();
void `$INSTANCE_NAME`_siguiente();
void `$INSTANCE_NAME`_anterior();
void `$INSTANCE_NAME`_reproducir(uint8 numeroArchivo);
void `$INSTANCE_NAME`_volumenMas();
void `$INSTANCE_NAME`_volumenMenos();
void `$INSTANCE_NAME`_definirVolumen(uint8 volumen);
void `$INSTANCE_NAME`_definirEcualizador(uint8 ecualizador);
void `$INSTANCE_NAME`_definirReproduccion(uint8 modo);
void `$INSTANCE_NAME`_definirFuente(uint8 fuente);
void `$INSTANCE_NAME`_sleep();
void `$INSTANCE_NAME`_reset();
void `$INSTANCE_NAME`_normal();
void `$INSTANCE_NAME`_playback();
void `$INSTANCE_NAME`_pausa();
void `$INSTANCE_NAME`_reproducirCarpeta(uint8 carpeta, uint8 archivo);
void `$INSTANCE_NAME`_ajustarSalida(CYBIT habilitado, uint8 ganancia);
void `$INSTANCE_NAME`_activarRepetirTodo();
void `$INSTANCE_NAME`_desactivarRepetirTodo();
void `$INSTANCE_NAME`_reproducirCarpetaMP3(uint8 archivo);
void `$INSTANCE_NAME`_reproducirPropaganda(uint8 archivo);
void `$INSTANCE_NAME`_detenerPropaganda();
void `$INSTANCE_NAME`_reproducirCarpetaGrande(uint8 carpeta, uint16 archivo);
void `$INSTANCE_NAME`_detener();
void `$INSTANCE_NAME`_repetirCarpeta(uint8 carpeta);
void `$INSTANCE_NAME`_aleatorio();
void `$INSTANCE_NAME`_habilitarRepeticion();
void `$INSTANCE_NAME`_deshabilitarRepeticion();
void `$INSTANCE_NAME`_habilitarDAC();
void `$INSTANCE_NAME`_desHabilitarDAC();
CYBIT `$INSTANCE_NAME`_ocupado();
    
#endif    

/* [] END OF FILE */
