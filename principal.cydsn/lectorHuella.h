/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del bus principal
====================================================*/

#ifndef LECTOR_CONFIG_H
#define LECTOR_CONFIG_H

#include "cypins.h"     
#include "colas.h"   
      
#define FINGER_LONGITUD_BUFFER      12
#define FINGER_TIMEOUT              15    
    
#define FINGER_INICIO1              0x55
#define FINGER_INICIO2              0xAA
#define FINGER_DISPOSITIVO_ID1      0x01
#define FINGER_DISPOSITIVO_ID2      0x00
    
#define FINGER_ACK                  0x30
#define FINGER_NACK                 0x31
    
#define FINGER_CMD_OPEN             0x01
#define FINGER_CMD_CLOSE            0x02
#define FINGER_CMD_USB              0x03
#define FINGER_CMD_BAUDRATE         0x04
#define FINGER_CMD_LED              0x12
#define FINGER_CMD_ENROLLCOUNT      0x20
#define FINGER_CMD_ENROLLED         0x21
#define FINGER_CMD_ENROLLSTART      0x22
#define FINGER_CMD_ENROLL1          0x23
#define FINGER_CMD_ENROLL2          0x24
#define FINGER_CMD_ENROLL3          0x25
#define FINGER_CMD_ISFINGER         0x26
#define FINGER_CMD_DELETEID         0x40
#define FINGER_CMD_DELETEALL        0x41
#define FINGER_CMD_VERIFY           0x50
#define FINGER_CMD_IDENTIFY         0x51
#define FINGER_CMD_VERIFYTEMP       0x52
#define FINGER_CMD_IDENTIFYTEMP     0x53
#define FINGER_CMD_CAPTURE          0x60
#define FINGER_CMD_MAKETEMPLATE     0x61
#define FINGER_CMD_GETIMAGE         0x62
#define FINGER_CMD_GETRAW           0x63
#define FINGER_CMD_GETTEMPLATE      0x70
#define FINGER_CMD_SETTEMPLATE      0x71
#define FINGER_CMD_SETSECURITY      0xF0
#define FINGER_CMD_GETSECURITY      0xF1
#define FINGER_CMD_IDENTIFYTEMP2    0xF4
#define FINGER_CMD_ENTERSTANDBY     0xF9    
    
#define FINGER_ERROR_TIMEOUT        0x1001
#define FINGER_ERROR_BAUDRATE       0x1002
#define FINGER_ERROR_POS            0x1003
#define FINGER_ERROR_NOTUSED        0x1004
#define FINGER_ERROR_ALREADYUSED    0x1005
#define FINGER_ERROR_COMMERR        0x1006
#define FINGER_ERROR_VERIFY         0x1007
#define FINGER_ERROR_IDENTIFY       0x1008
#define FINGER_ERROR_DB_FULL        0x1009
#define FINGER_ERROR_DB_EMPTY       0x100A
#define FINGER_ERROR_TURN_ERR       0x100B
#define FINGER_ERROR_BADFINGER      0x100C
#define FINGER_ERROR_ENROLL         0x100D
#define FINGER_ERROR_NOT_SUPPORT    0x100E
#define FINGER_ERROR_DEV_ERR        0x100F
#define FINGER_ERROR_CANCELED       0x1010
#define FINGER_ERROR_INVALID        0x1011
#define FINGER_ERROR_NOTPRESSED     0x1012
//#define FINGER_ERROR_DUPLICATED    
    
typedef struct {
    CYBIT ack;
    uint32 codigo;
    uint32 id;
    uint8 aux;
}xRespuestaLector;

xSemaphoreHandle lectorOcupado;

void finger_habilitacion();  
CYBIT finger_open();
CYBIT finger_close();
CYBIT finger_SetLED(CYBIT estado);    
CYBIT finger_cambiarBaudios(int baudios);
xRespuestaLector finger_totalHuellas();
CYBIT finger_verificarInscripcion(uint32 ID);
xRespuestaLector finger_iniciarInscripcion(uint32 ID);
xRespuestaLector finger_inscribirPlantilla1();
xRespuestaLector finger_inscribirPlantilla2();
xRespuestaLector finger_inscribirPlantilla3();
CYBIT finger_dedoPresionado();
CYBIT finger_borrarInscripcion(uint32 ID);
CYBIT finger_borrarTodos();
xRespuestaLector finger_verificarEspecifico(uint32 ID);
xRespuestaLector finger_identificarHuella();
xRespuestaLector finger_capturarHuella(CYBIT altaCalidad);
CYBIT finger_definirSeguridad(uint8 nivelSeguridad);
xRespuestaLector finger_obtenerSeguridad();
CYBIT finger_entrarDescanso();
CYBIT finger_salirDescanso();
int finger_leerHuella();
xRespuestaLector finger_registroHuella(uint8 paso, uint8 indice, uint8 offset);
    
#endif 
/* [] END OF FILE */
