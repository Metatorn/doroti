/* ===================================================
 *
 * Copyright EKIA Technology SAS, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY SAS.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del modulo GSM A6
====================================================*/
#ifndef GSMA6_CONFIG_H
#define GSMA6_CONFIG_H
    
#include "cypins.h"    
#include "EKIA_CONFIG.h"  
#include "colas.h" 
#include "tipos.h"
#include "Telemetria.h"    

#define `$INSTANCE_NAME`_GSM_TIME_OUT    (20)    
#define `$INSTANCE_NAME`_COMANDO_LEN     (150)
#define `$INSTANCE_NAME`_GSM_LEN_MAX     (200)    
#define `$INSTANCE_NAME`_NUM_COMANDOS    (5)    
    
#define `$INSTANCE_NAME`_IPINITIAL       (1)
#define `$INSTANCE_NAME`_IPSTART         (2)
#define `$INSTANCE_NAME`_IPCONFIG        (3)
#define `$INSTANCE_NAME`_IPGPRSACT       (3)
#define `$INSTANCE_NAME`_IPSTATUS        (4)   
#define `$INSTANCE_NAME`_TCPCLOSED       (4)    
#define `$INSTANCE_NAME`_TCPCONNECTING   (5)
#define `$INSTANCE_NAME`_CONNECTOK       (6)
#define `$INSTANCE_NAME`_CONNECTFAIL     (6) 
#define `$INSTANCE_NAME`_PDPDEACT        (6)
    
#define `$INSTANCE_NAME`_OPERTIGO        "Colombia Movil SA " 
    
#define `$INSTANCE_NAME`_APN_TIGO_M2M    "testco.tigo.com" 
#define `$INSTANCE_NAME`_APN_TIGO        "web.colombiamovil.com.co"
#define `$INSTANCE_NAME`_APN_AVANTEL     "lte.avantel.com.co" 
#define `$INSTANCE_NAME`_APN_VIRGIN      "web.vmc.net.co"
#define `$INSTANCE_NAME`_APN_MOVISTAR    "internet.movistar.com.co"
#define `$INSTANCE_NAME`_APN_CLARO       "internet.comcel.com.co"
#define `$INSTANCE_NAME`_APN_UNE         "www.une.net.co"
#define `$INSTANCE_NAME`_APN_ETB         "moviletb.net.co" 
#define `$INSTANCE_NAME`_APN_EXITO       "movilexito.net.co"
#define `$INSTANCE_NAME`_APN_FLASH       "internet.flashmobile.omv"    
    
#define `$INSTANCE_NAME`_BANDA_EGSM_MODE         1
#define `$INSTANCE_NAME`_BANDA_PGSM_MODE         2
#define `$INSTANCE_NAME`_BANDA_DCS_MODE          3
#define `$INSTANCE_NAME`_BANDA_GSM850_MODE       4
#define `$INSTANCE_NAME`_BANDA_PCS_MODE          5
#define `$INSTANCE_NAME`_BANDA_EGSM_DCS_MODE     6
#define `$INSTANCE_NAME`_BANDA_GSM850_PCS_MODE   7
#define `$INSTANCE_NAME`_BANDA_EGSM_PCS_MODE     8
#define `$INSTANCE_NAME`_BANDA_ALL_BAND          9  

#define `$INSTANCE_NAME`_DATA_FIN                0    
#define `$INSTANCE_NAME`_DATA_AVAILABLE          1
#define `$INSTANCE_NAME`_NET_ERROR               61
#define `$INSTANCE_NAME`_DNS_ERROR               62
#define `$INSTANCE_NAME`_CONNECT_ERROR           63
#define `$INSTANCE_NAME`_TIMEOUT                 64
#define `$INSTANCE_NAME`_SERVER_ERROR            65
#define `$INSTANCE_NAME`_NOT_ALLOW               66
#define `$INSTANCE_NAME`_REPLAY_ERROR            70
#define `$INSTANCE_NAME`_USER_ERROR              71
#define `$INSTANCE_NAME`_PASS_ERROR              72
#define `$INSTANCE_NAME`_TYPE_ERROR              73
#define `$INSTANCE_NAME`_REST_ERRROR             74
#define `$INSTANCE_NAME`_PASSIVE_ERROR           75
#define `$INSTANCE_NAME`_ACTIVE_ERROR            76
#define `$INSTANCE_NAME`_OPERATE_ERROR           77
#define `$INSTANCE_NAME`_UPLOAD_ERROR            78    
#define `$INSTANCE_NAME`_DOWNLOAD_ERROR          79
    
/*=====================================================
;Funciones de manejo del SIM800L
====================================================*/     
void `$INSTANCE_NAME`_apagar();
CYBIT `$INSTANCE_NAME`_encender();
int `$INSTANCE_NAME`_recibirArray();
int `$INSTANCE_NAME`_enviarString(char* cadena);
xbufferEntradaGSM `$INSTANCE_NAME`_leerComando(int indice);
xRespuestasGSM `$INSTANCE_NAME`_verificarRegistro();
xRespuestasGSM `$INSTANCE_NAME`_potenciaSenal();
xbufferEntradaGSM `$INSTANCE_NAME`_identFabricante();
xbufferEntradaGSM `$INSTANCE_NAME`_Modelo();
xbufferEntradaGSM `$INSTANCE_NAME`_IMEI();
xbufferEntradaGSM `$INSTANCE_NAME`_IMSI();
xRespuestasGSM `$INSTANCE_NAME`_verificarEstadoGPRS();
xbufferEntradaGSM `$INSTANCE_NAME`_verificarOperador();
uint8 `$INSTANCE_NAME`_verificarEstadoTCP();
CYBIT `$INSTANCE_NAME`_TCPInit();
CYBIT `$INSTANCE_NAME`_Banda(uint8 banda);
CYBIT `$INSTANCE_NAME`_conectarHost(char* Host, char* Port);
CYBIT `$INSTANCE_NAME`_datosaAT();
CYBIT `$INSTANCE_NAME`_ATaDatos();
CYBIT `$INSTANCE_NAME`_conectarFTP(char* servidor, char* usuario, char*clave, char*archivo);
    
#endif    

/* [] END OF FILE */

