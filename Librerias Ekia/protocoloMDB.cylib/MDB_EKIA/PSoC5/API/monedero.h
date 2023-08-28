/* ===================================================
 *
 * Copyright EKIA Technologies, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * Monedero EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Archivo cabecera para declaración de funciones del monedero 
====================================================*/
#ifndef monedero_CONFIG_H
#define monedero_CONFIG_H

#include "cypins.h"
#include "FreeRTOS.h"    
#include "queue.h"  
#include "stdio.h"    

/*=====================================================
;comandos del monedero
=====================================================*/ 
#define COIN_RESET                      (0x08) //
#define COIN_SETUP                      (0x09) //
#define COIN_TUBE_STATUS                (0x0A) //   
#define COIN_POLL                       (0x0B) //
#define COIN_TYPE                       (0x0C) //   
#define COIN_DISPENSE                   (0x0D) // 
#define COIN_EXPANSION                  (0x0F) //
#define COIN_EXPANSION_ID               (0x00) // 
#define COIN_EXPANSION_FEATURE          (0x01) //     
#define COIN_EXPANSION_PAYOUT           (0x02)   
#define COIN_EXPANSION_PAYOUT_STATUS    (0x03)     
#define COIN_EXPANSION_PAYOUT_POLL      (0x04)      
#define COIN_EXPANSION_SEND_DIAGNOS     (0x05) //
#define COIN_EXPANSION_FILL_REPORT      (0x06)
#define COIN_EXPANSION_PAYOUT_REPORT    (0x07)
#define COIN_EXPANSION_FTL_REQ_TO_RCV   (0xFA)
#define COIN_EXPANSION_FTL_RETRY        (0xFB)
#define COIN_EXPANSION_FTL_SEND_BLOCK   (0xFC)    
#define COIN_EXPANSION_FTL_OK_TO_SEND   (0xFD)
#define COIN_EXPANSION_FTL_REQ_TO_SEND  (0xFE)
#define COIN_EXPANSION_DIAGNOSTICS      (0xFF)    

/*=====================================================
;Costantes del monedero
====================================================*/       
#define TIPO_0                          (0)
#define TIPO_1                          (1)    
#define TIPO_2                          (2)
#define TIPO_3                          (3)
#define TIPO_4                          (4)
#define TIPO_5                          (5)    
#define TIPO_6                          (6)
#define TIPO_7                          (7)
#define TIPO_8                          (8)
#define TIPO_9                          (9)    
#define TIPO_10                         (10)
#define TIPO_11                         (11)
#define TIPO_12                         (12)
#define TIPO_13                         (13)    
#define TIPO_14                         (14)
#define TIPO_15                         (15)    
    
#define MONEDA_DESCONOCIDA              (0xFF)   
    
#define MONEDA_CASHBOX                  (0)
#define MONEDA_TUBOS                    (1)
#define MONEDA_NO_USADA                 (2)
#define MONEDA_RECHAZADA                (3) 
#define MONEDA_ESPERA                   (100)    
    
#define LONGITUD_COIN_POLL              (16)  
#define LONGITUD_TIPO_MONEDAS           (16)
#define LONGITUD_TUBOS                  (16)     
          
/*=====================================================
;Variables y estructuras comunes
====================================================*/    

CY_PACKED typedef struct {
    uint16 ruteo;
    uint8 tipo[LONGITUD_TIPO_MONEDAS];
    uint8 nivel;
    uint16 pais;
    uint8 factorEscala;
    uint8 decimales;
}CY_PACKED_ATTR xcoin_setup;

CY_PACKED typedef struct {
    uint8 principal;
    uint8 secundario;
}CY_PACKED_ATTR xcoin_state;

CY_PACKED typedef struct {
    uint16 tuboLleno;
    uint16 tuboEstado[LONGITUD_TUBOS];
}CY_PACKED_ATTR xcoin_tube;

CY_PACKED typedef struct {
    char manufactura[3];
    char serie[12];
    char modelo[12];
    uint16 version;
    uint32 opcional;
}CY_PACKED_ATTR xcoin_id;

CY_PACKED typedef struct {
    uint8 poll[LONGITUD_COIN_POLL];
}CY_PACKED_ATTR xcoin_activity;

CY_PACKED typedef struct {
    uint monto;
    uint monto_almacenado;
}CY_PACKED_ATTR xmonedas_devueltas;

CY_PACKED typedef struct {
    char ruteo[20]; 
    int ruta;
    uint moneda_depositada;
    uint cantidad_almacenada;
}CY_PACKED_ATTR xmoneda_ingresada;

CY_PACKED typedef struct {
    CYBIT comando_ok;
    xmonedas_devueltas devueltas;
    xmoneda_ingresada ingresadas;
    CYBIT cancelar;
}CY_PACKED_ATTR xpollCoin; 

CY_PACKED typedef struct {
    uint8 monedero;
    uint8 monederoOk;
}CY_PACKED_ATTR xestadosMonedero;


xQueueHandle setupCoin;
xQueueHandle identCoin;
xQueueHandle stateCoin;
xQueueHandle tubeCoin;
xQueueHandle estadoMonedero;
xQueueHandle actividadMonedero;
xQueueHandle erroresMonedero;
xQueueHandle avisosMonedero;
    
/*=====================================================
;funciones del monedero
====================================================*/        
    
CYBIT coin_start();
uint8 COIN_reset();
uint8 COIN_poll(xcoin_activity* activity);
CYBIT COIN_setup();
CYBIT COIN_tube_status();
CYBIT COIN_dispense(uint8 tipo, uint8 cantidad);
CYBIT COIN_exp_id();
CYBIT COIN_exp_feature_en(uint32 funcionesOpcionales);
CYBIT COIN_exp_diagnostic_status();
CYBIT COIN_exp_tube_status();
CYBIT COIN_type(uint16 aceptados, uint16 dispensadoManual);
uint8 COIN_exp_payout(uint8 Y1);
CYBIT COIN_exp_paypoll();
CYBIT COIN_exp_paypoll_status();
CYBIT COIN_Disable();
CYBIT COIN_Enable();
xmonedas_devueltas moneda_dispensa(uint8 manejoMonedas, uint8 cantidad);
xmoneda_ingresada moneda_ingreso(uint8 manejoMonedas, uint8 cantidad);

#endif

/* [] END OF FILE */
