/* ===================================================
 *
 * Copyright EKIA Technologies, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * Billetero EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Archivo cabecera para declaración de funciones del billetero
====================================================*/
#ifndef billetero_CONFIG_H
#define billetero_CONFIG_H

#include "cypins.h"
#include "FreeRTOS.h"    
#include "queue.h"  
#include "stdio.h"    
    

/*=====================================================
;comandos del billetero
=====================================================*/ 
#define BILL_RESET                      (0x30) 
#define BILL_SETUP                      (0x31) 
#define BILL_SECURITY                   (0x32)    
#define BILL_POLL                       (0x33) 
#define BILL_TYPE                       (0x34)    
#define BILL_ESCROW                     (0x35)  
#define BILL_STACKER                    (0x36) 
#define BILL_EXPANSION                  (0X37)
#define BILL_EXPANSION_LEVEL1           (0X00) 
#define BILL_EXPANSION_LEVEL2_ENABLE    (0X01)    
#define BILL_EXPANSION_LEVEL2_ID        (0X02)        
#define BILL_EXPANSION_RECYCLER_SETUP   (0x03)  
#define BILL_EXPANSION_RECYCLER_ENABLE  (0x04)
#define BILL_EXPANSION_DISPENSE_STATUS  (0x05) 
#define BILL_EXPANSION_DISPENSE_BILL    (0x06)
#define BILL_EXPANSION_DISPENSE_VALUE   (0x07)  
#define BILL_EXPANSION_PAYOUT_STATUS    (0x08) 
#define BILL_EXPANSION_PAYOUT_POLL      (0x09) 
#define BILL_EXPANSION_PAYOUT_CANCEL    (0x10)
#define BILL_EXPANSION_FTL_REQ_TO_RCV   (0xFA)
#define BILL_EXPANSION_FTL_RETRY        (0xFB)
#define BILL_EXPANSION_FTL_SEND_BLOCK   (0xFC)    
#define BILL_EXPANSION_FTL_OK_TO_SEND   (0xFD)
#define BILL_EXPANSION_FTL_REQ_TO_SEND  (0xFE)
#define BILL_EXPANSION_DIAGNOSTICS      (0xFF)     
    
    
/*=====================================================
;Costantes del billetero
=====================================================*/     
    
#define BILL_STACK                      (0)
#define BILL_RETENIDO                   (1)
#define BILL_RETORNADO                  (2)
#define BILL_RECICLADOR                 (3)
#define BILL_DESHABILITADO_RECHAZADO    (4)  
#define BILL_RECICLADO_MANUAL           (5)
#define BILL_DISPENSADO_MANUAL          (6)
#define BILL_RECICLADOR_A_STACK         (7)  
    
#define LONGITUD_BILL_POLL              (16)
#define LONGITUD_TIPO_BILLETES          (16)    
 
/*=====================================================
;Variables y estructuras comunes
====================================================*/    

CY_PACKED typedef struct {
    char manufactura[3];
    char serie[12];
    uint16 version;
}CY_PACKED_ATTR xbill_id1;

CY_PACKED typedef struct {
    char modelo[12];
    uint32 opcional;
    CYBIT reciclador;
}CY_PACKED_ATTR xbill_id2;

CY_PACKED typedef struct {
    uint8 nivel;
    uint16 pais;
    uint16 factorEscala;
    uint16 factorDecimal;
    uint16 capacidad;
    uint16 nivelSeguridad;
    uint8 escrow;
    uint8 tipo[LONGITUD_TIPO_BILLETES];
}CY_PACKED_ATTR xbill_setup;

CY_PACKED typedef struct {
    CYBIT comando_ok;
    uint8 poll[LONGITUD_BILL_POLL];
}CY_PACKED_ATTR xbill_activity;

CY_PACKED typedef struct {
    CYBIT lleno;
    uint16 estado;
}CY_PACKED_ATTR xbill_state;

CY_PACKED typedef struct {
    uint16 tipo;
}CY_PACKED_ATTR xbill_type_recycler;

CY_PACKED typedef struct {
    uint16 recicladorLleno;
    uint16 cantidadBilletes[16];
}CY_PACKED_ATTR xbill_recycler_status;

CY_PACKED typedef struct {
    uint16 actividad;
}CY_PACKED_ATTR xbill_paypoll;

CY_PACKED typedef struct {
    //char ruteo[20];
    uint8 ruteo;
    uint billete_depositado;
}CY_PACKED_ATTR xbillete_ingresado;

CY_PACKED typedef struct {
    CYBIT comando_ok;
    xbillete_ingresado ingresado;
}CY_PACKED_ATTR xpollBill;

CY_PACKED typedef struct {
    uint8 billetero;
    uint8 billeteroOk;
}CY_PACKED_ATTR xestadosBilletero;


xQueueHandle setupBill;
xQueueHandle identBill1;
xQueueHandle identBill2;
xQueueHandle stateBill;
xQueueHandle typeRecycler;
xQueueHandle stateRecycler;
xQueueHandle payoutRecycler;
xQueueHandle paypollRecycler;
xQueueHandle estadoBilletero;
xQueueHandle actividadBilletero;
xQueueHandle erroresBilletero;
xQueueHandle avisosBilletero;

/*=====================================================
;funciones del billetero
====================================================*/      
uint8 BILL_reset();    
xbill_activity BILL_poll();   
CYBIT BILL_setup();    
CYBIT BILL_security(uint8 Y1, uint8 Y2);    
CYBIT BILL_type(uint16 aceptados, uint16 retenidos); 
CYBIT BILL_escrow(uint8 Y1);    
CYBIT BILL_stacker();    
CYBIT BILL_exp_id_level1();    
CYBIT BILL_exp_id_level2();    
CYBIT BILL_exp_level2_enable(uint32 funcionesOpcionales);    
CYBIT BILL_exp_recycler_setup();    
CYBIT BILL_exp_recycler_enable(char habilitador[19]);    
CYBIT BILL_exp_dispense_status();    
CYBIT BILL_exp_dispense_bill(uint8 tipo, uint16 cantidad);    
CYBIT BILL_exp_dispense_value(uint16 valor);    
CYBIT BILL_payout_status();    
CYBIT BILL_paypoll();
CYBIT BILL_payout_cancel();    
CYBIT bill_start();
CYBIT BILL_Disable();
CYBIT BILL_Enable();

xbillete_ingresado billete_ingreso(uint8 ruteoBillete);

#endif

/* [] END OF FILE */
