/* ===================================================
 *
 * Copyright EKIA Technologies, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MDB 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
*/

#ifndef `$INSTANCE_NAME`_TIPOS_CONFIG_H
#define `$INSTANCE_NAME`_TIPOS_CONFIG_H

#include "cypins.h"
#include "`$INSTANCE_NAME`_monedero.h"
#include "`$INSTANCE_NAME`_billetero.h"
#include "FreeRTOS.h"    
#include "queue.h"     
#include "stdio.h"    

    
/*=====================================================
;COMANDOS PROPIOS DE LOS DISPOSITIVOS MDB
====================================================*/ 
#define `$INSTANCE_NAME`_LEN_MAX                          (36)
    
#define EKIA_ACK                                          (10)
#define EKIA_NACK                                         (11)    
#define EKIA_JUST_RESET                                   (12)
#define EKIA_MULTI_RESPONSE                               (13)    
    
#define DIR_MONEDERO                                      (0x08)     
#define DIR_BILLETERO                                     (0x30)       
#define DIR_CASHLESS                                      (0x10)     
#define TMAX_BILL                                         (15)   
#define TMAX_COIN                                         (15)
#define MASCARA_BILLETES                                  (0xFF)     
    
#define `$INSTANCE_NAME`_EKIA_RESETEO                     (13)
#define `$INSTANCE_NAME`_EKIA_OK                          (14)
#define `$INSTANCE_NAME`_EKIA_DISPENSE                    (15) 
#define `$INSTANCE_NAME`_EKIA_GUARDAR_BILL                (16)
#define `$INSTANCE_NAME`_EKIA_DEVOLVER_BILL               (17)
#define `$INSTANCE_NAME`_EKIA_ENABLE_BILL                 (18) 
#define `$INSTANCE_NAME`_EKIA_DISABLE_BILL                (19)
#define `$INSTANCE_NAME`_EKIA_ENABLE_COIN                 (20) 
#define `$INSTANCE_NAME`_EKIA_DISABLE_COIN                (21)    
#define `$INSTANCE_NAME`_EKIA_TIPO_BILLETE                (22) 
#define `$INSTANCE_NAME`_EKIA_VACIAR_RECICLADOR           (23) 
#define `$INSTANCE_NAME`_EKIA_ESTADO_TUBOS                (24) 
#define `$INSTANCE_NAME`_EKIA_ESTADO_TAMBOR               (25)     
    
/*=====================================================
;Variables y estructuras comunes
====================================================*/      
    
typedef struct {
    uint8 datosEntrada[`$INSTANCE_NAME`_LEN_MAX];
    uint8 ack;
    uint8 indice;
    uint8 checksum;
    uint8 finalizado;
}xbufferMDB;    

typedef struct {
    uint saldo;
    xbillete_ingresado depositado;
}xcredito;

typedef struct {
    uint16 billetesAceptados;
    uint16 billetesTrabajo;
    uint16 billetesReciclados;
    CYBIT recicladorHabilitado;
    uint16 factorEscalaBillete;
    uint16 factorDecimalBillete;
}xSistemasPago;

xQueueHandle bufferMDB;
xQueueHandle bufferRxMDB;
xQueueHandle creditoValido;
xQueueHandle payoutCoin;
xQueueHandle paypollCoin;
xQueueHandle configuracionSistemasPago;



#endif