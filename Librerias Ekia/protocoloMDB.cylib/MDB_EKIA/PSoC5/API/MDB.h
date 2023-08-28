/* ===================================================
 *
 * Copyright EKIA Technologies, 2017
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MDB EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Archivo cabecera para declaración de funciones del MDB
====================================================*/
#ifndef `$INSTANCE_NAME`_CONFIG_H
#define `$INSTANCE_NAME`_CONFIG_H

#include "cypins.h" 
#include "cyfitter.h"
#include "cytypes.h"
#include "FreeRTOS.h"
#include "queue.h" 
#include "sistema_pago.h"    

/*=====================================================
;Constantes del protocolo MDB
====================================================*/    
#define TMAX                                (SISPAGO_TMAX)//(200)
#define TPERIODO                            (SISPAGO_TPERIODO)    
#define TIME_OUT                            (SISPAGO_TIMEOUT)    
#define MODO_CRITICO                        (1) 
#define MDB_BILL_DEBUG                      (SISPAGO_BILL_DEBUG)
#define MDB_COIN_DEBUG                      (SISPAGO_COIN_DEBUG)      

/*=====================================================
;Comandos y respuestas del protocolo MDB
====================================================*/      
#define `$INSTANCE_NAME`_ACK            (0x00)
#define `$INSTANCE_NAME`_NACK           (0xFF)
#define `$INSTANCE_NAME`_RET            (0xAA)    
#define `$INSTANCE_NAME`_DATO           (0)
#define `$INSTANCE_NAME`_DIRECCION      (1) 
    
xQueueHandle estadoMDB;    
    
void `$INSTANCE_NAME`_Start();
uint8 `$INSTANCE_NAME`_rx(uint8* MDB_receptBuffer);
CY_ISR(`$INSTANCE_NAME`_isr_rxMDB);
void `$INSTANCE_NAME`_tx(uint8 comando, CYBIT modo);
void `$INSTANCE_NAME`_tx_checksum(uint8 check);
void `$INSTANCE_NAME`_tx_ACK();
void `$INSTANCE_NAME`_tx_RET();

#endif  
/* [] END OF FILE */