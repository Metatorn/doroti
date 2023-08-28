/* ===================================================
 *
 * Copyright EKIA Technologies, 2022
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del SIM800L
====================================================*/
#ifndef SIM800L_CONFIG_H
#define SIM800L_CONFIG_H
    
#include "cypins.h"    
#include "EKIA_CONFIG.h"  
#include "tipos.h"
#include "Telemetria.h" 
    
#define `$INSTANCE_NAME`_TIMEOUT_CONST         10
#define `$INSTANCE_NAME`_WAIT_RESPONSE   10000       

#define `$INSTANCE_NAME`_GSM_TIME_OUT    (20)    
#define `$INSTANCE_NAME`_COMANDO_LEN     (150)
#define `$INSTANCE_NAME`_GSM_LEN_MAX     (500)    
#define `$INSTANCE_NAME`_NUM_COMANDOS    (5)    
  
enum{    
    `$INSTANCE_NAME`_IPINITIAL = 1,
    `$INSTANCE_NAME`_IPSTART,
    `$INSTANCE_NAME`_IPCONFIG,
    `$INSTANCE_NAME`_IPGPRSACT,
    `$INSTANCE_NAME`_IPSTATUS,
    `$INSTANCE_NAME`_TCPCLOSED,
    `$INSTANCE_NAME`_TCPCONNECTING,
    `$INSTANCE_NAME`_CONNECTOK,
    `$INSTANCE_NAME`_CONNECTFAIL,
    `$INSTANCE_NAME`_PDPDEACT
}`$INSTANCE_NAME`_IP_STATUSES;
    
enum{
    `$INSTANCE_NAME`_OPER_TIGO = 1,
    `$INSTANCE_NAME`_OPER_TIGO_M2M, //2
    `$INSTANCE_NAME`_OPER_AVANTEL,  //3
    `$INSTANCE_NAME`_OPER_VIRGIN,   //4
    `$INSTANCE_NAME`_OPER_MOVISTAR, //5
    `$INSTANCE_NAME`_OPER_CLARO,    //6
    `$INSTANCE_NAME`_OPER_UNE,      //7
    `$INSTANCE_NAME`_OPER_ETB,      //8
    `$INSTANCE_NAME`_OPER_EXITO,    //9
    `$INSTANCE_NAME`_OPER_FLASH,    //10
    `$INSTANCE_NAME`_OPER_WOM       //11
}`$INSTANCE_NAME`_OPERATORS;   
    
#define `$INSTANCE_NAME`_APN_TIGO_M2M    "m2mco.tigo.com" 
#define `$INSTANCE_NAME`_APN_TIGO        "web.colombiamovil.com.co"
#define `$INSTANCE_NAME`_APN_AVANTEL     "lte.avantel.com.co" 
#define `$INSTANCE_NAME`_APN_VIRGIN      "web.vmc.net.co"
#define `$INSTANCE_NAME`_APN_MOVISTAR    "internet.movistar.com.co"
#define `$INSTANCE_NAME`_APN_CLARO       "internet.comcel.com.co"
#define `$INSTANCE_NAME`_APN_UNE         "www.une.net.co"
#define `$INSTANCE_NAME`_APN_ETB         "moviletb.net.co" 
#define `$INSTANCE_NAME`_APN_EXITO       "movilexito.net.co"
#define `$INSTANCE_NAME`_APN_FLASH       "internet.flashmobile.omv" 
#define `$INSTANCE_NAME`_APN_WOM         "internet.wom.co"     
    
enum{
    `$INSTANCE_NAME`_BANDA_EGSM_MODE = 1,
    `$INSTANCE_NAME`_BANDA_PGSM_MODE,
    `$INSTANCE_NAME`_BANDA_DCS_MODE,
    `$INSTANCE_NAME`_BANDA_GSM850_MODE,
    `$INSTANCE_NAME`_BANDA_PCS_MODE,
    `$INSTANCE_NAME`_BANDA_EGSM_DCS_MODE,
    `$INSTANCE_NAME`_BANDA_GSM850_PCS_MODE,
    `$INSTANCE_NAME`_BANDA_EGSM_PCS_MODE,
    `$INSTANCE_NAME`_BANDA_ALL_BAND
}`$INSTANCE_NAME`_BANDS;

enum{
    `$INSTANCE_NAME`_DATA_FIN = 0,  
    `$INSTANCE_NAME`_DATA_AVAILABLE,
    `$INSTANCE_NAME`_NET_ERROR = 61,
    `$INSTANCE_NAME`_DNS_ERROR,
    `$INSTANCE_NAME`_CONNECT_ERROR,
    `$INSTANCE_NAME`_TIMEOUT,
    `$INSTANCE_NAME`_SERVER_ERROR,
    `$INSTANCE_NAME`_NOT_ALLOW,
    `$INSTANCE_NAME`_REPLAY_ERROR = 70,
    `$INSTANCE_NAME`_USER_ERROR,
    `$INSTANCE_NAME`_PASS_ERROR,
    `$INSTANCE_NAME`_TYPE_ERROR,
    `$INSTANCE_NAME`_REST_ERRROR,
    `$INSTANCE_NAME`_PASSIVE_ERROR,
    `$INSTANCE_NAME`_ACTIVE_ERROR,
    `$INSTANCE_NAME`_OPERATE_ERROR,
    `$INSTANCE_NAME`_UPLOAD_ERROR,
    `$INSTANCE_NAME`_DOWNLOAD_ERROR
}`$INSTANCE_NAME`_ERRORS;
    
/*=====================================================
;Funciones de manejo del SIM800L
====================================================*/     
void DEBUG_SIM800L(char* debugString);
void `$INSTANCE_NAME`_clearBuffer(char* buffer, int size);
uint8 `$INSTANCE_NAME`_receiveData(char* bufferRx);
uint8 `$INSTANCE_NAME`_check();
uint8 `$INSTANCE_NAME`_shutdown();
uint8 `$INSTANCE_NAME`_start();

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
CYBIT `$INSTANCE_NAME`_TCPInit(uint16 operador);
CYBIT `$INSTANCE_NAME`_Banda(uint8 banda);
CYBIT `$INSTANCE_NAME`_conectarHost(char* Host, char* Port);
CYBIT `$INSTANCE_NAME`_datosaAT();
CYBIT `$INSTANCE_NAME`_ATaDatos();
CYBIT `$INSTANCE_NAME`_conectarFTP(char* servidor, char* usuario, char*clave, char*archivo);
    
#endif    

/* [] END OF FILE */

