/* ==============================================================
 *
 * Copyright EKIA Technology SAS, 2022
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;FUNCIONES PARA MANEJO DEL SIM800L
===============================================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cyapicallbacks.h"

#include "`$INSTANCE_NAME`_SIM.h"
#include "colas.h"
#include "`$INSTANCE_NAME`_SIM_REG.h"
#include "`$INSTANCE_NAME`.h"
#include "tipos.h"
#include "Telemetria.h"
#include "memoriaExt.h"
#include "semaforos.h"
#include "LED_DEBUG.h"

 
uint8 bufferEntrada[`$INSTANCE_NAME`_GSM_LEN_MAX];
char comandoRecibido[`$INSTANCE_NAME`_NUM_COMANDOS][`$INSTANCE_NAME`_COMANDO_LEN];

void DEBUG_SIM800L(char* debugString){
    char string[250];
    sprintf(string,"Response = SIM800L: %s\r\n",debugString);
    DEBUG(string);
}

void _SIM800L_Print(const char *ptr){
    `$INSTANCE_NAME``[SIM]`PutString(ptr);
}

void _SIM800L_PutChar(const char ptr){
    `$INSTANCE_NAME``[SIM]`PutChar(ptr);
}

uint8 _SIM800L_GetChar(){
    return `$INSTANCE_NAME``[SIM]`GetChar();
}

void `$INSTANCE_NAME`_clearBuffer(char* buffer, int size){
    int index = 0;
    for(index=0; index<size; index++){
        buffer[index]=0;
    }
}

/*********************************************************
Receive Data: fill the bufferRx with the data received by the SIM800
**********************************************************/
uint8 `$INSTANCE_NAME`_receiveData(char* bufferRx){
    uint8 indexBuffer=0, valid=0;
    uint timeout=0;
    vTaskDelay(pdMS_TO_TICKS(10));
    while(!valid){
        while((`$INSTANCE_NAME``[SIM]`GetRxBufferSize()>0))
        {
            valid = pdTRUE;
            bufferRx[indexBuffer] = `$INSTANCE_NAME``[SIM]`GetChar();
            indexBuffer++;
        }
        timeout++;
        if(timeout>=`$INSTANCE_NAME`_TIMEOUT_CONST){
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    bufferRx[indexBuffer]=0;
    return valid;
}

uint8 _SIM800L_waitResponse(void) {
    const char* stringsResponses[] = {"ERROR", "OK", "ready", "FAIL", "no change", "Linked", "Unlink", ">"};
    char received;
    char response = 0;
    uint8_t continue_loop = 1;
    uint8 index, character = 0;
    uint8 timeout=0;
    while (continue_loop) {
        if(`$INSTANCE_NAME``[SIM]`GetRxBufferSize()>0){
            received = _`$INSTANCE_NAME`_GetChar();
        }
        else{
            timeout++;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        for (index = 0; index < 8; index++) {
            if (stringsResponses[index][character] == received) {
                character++;
                if (character == strlen(stringsResponses[index])) {
                    response = index;
                    continue_loop = 0;
                    `$INSTANCE_NAME``[SIM]`ClearRxBuffer();
                }
            }
        }
        if(timeout>=`$INSTANCE_NAME`_TIMEOUT_CONST){
            continue_loop = 0;
        }
    }
    return response;
}

/*******************************************************************************
* limpiarBuffer
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia el buffer de recepción temporal
*
*******************************************************************************/
static void limpiarBuffer(){
    uint i;
    for(i=0; i<(sizeof bufferEntrada); i++){
        bufferEntrada[i]=0;
    } 
}

/*******************************************************************************
* limpiarComando
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia los buffers de comandos
*
*******************************************************************************/
static void limpiarComando(){
    uint8 i,j;
    for(i=0; i<`$INSTANCE_NAME`_NUM_COMANDOS; i++){
        for(j=0; j<`$INSTANCE_NAME`_COMANDO_LEN; j++){
            comandoRecibido[i][j]=0;
        }
    } 
}

int verificar(){
    //xPuenteSerial puenteGSM;
    //puenteGSM.modoOperacion=1;
    xComandosSIM comandos;
    int canTokens = 0;
    const char delimitadores[3] = "\r\n";
    char* token;
    uint8 i = 0;
    limpiarComando();
    //`$INSTANCE_NAME``[SIM]`ClearRxBuffer();
    //obtiene el primer token
    token = strtok((char*)bufferEntrada, delimitadores);
    //obtiene el resto de token
    while( token != NULL ) {
        strcpy(comandoRecibido[i],token);
        strcpy(comandos.datos[i],token);
        //strcpy(puenteGSM.comandoEntrada.entrada,comandoRecibido[i]);
        //xQueueSendToBack(puenteSerialEntradaGSM,&puenteGSM,200);
        DEBUG(token);
        token = strtok(NULL, delimitadores);
        i++;
        canTokens++;
        if(canTokens>=`$INSTANCE_NAME`_NUM_COMANDOS){
            DEBUG(token);
            break;
        }
    }
    comandos.comandos=canTokens;
    xQueueSendToBack(comandosSIM,&comandos,100);
    return canTokens;
}

/*******************************************************************************
* recibirArray
********************************************************************************
*
* Tarea que realiza:
*  Funcion que copia el buffer del puerto serie al buffer temporal para obtener
*  los comandos y respuestas que hayan sido recibidos
*
*******************************************************************************/
int recibirArray(){
    uint i=0;
    vTaskDelay(pdMS_TO_TICKS(100));
    limpiarBuffer();
    while(`$INSTANCE_NAME``[SIM]`GetRxBufferSize()>0)
    {
        bufferEntrada[i]=`$INSTANCE_NAME``[SIM]`GetChar();
        CyDelay(5);
        i++;
    }
    return verificar();
}

/*******************************************************************************
* enviarComando
********************************************************************************
*
* Tarea que realiza:
*  Envia comandos AT por el puerto del SIM800L
*
*******************************************************************************/
int `$INSTANCE_NAME`_enviarComando(char* comando){
    limpiarBuffer();
    `$INSTANCE_NAME``[SIM]`PutString(comando);
    vTaskDelay(pdMS_TO_TICKS(200));
    return recibirArray();
}

/*******************************************************************************
* limpiarTodo
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia todos los buffer involucrados en el manejo del SIM800L
*
*******************************************************************************/
void limpiarTodo(){
    `$INSTANCE_NAME``[SIM]`ClearRxBuffer();
    limpiarBuffer();
    limpiarComando();
}

/*********************************************************
Check ESP: must return OK if the ESP is attached 
**********************************************************/
uint8 `$INSTANCE_NAME`_check(){
    _SIM800L_Print("AT\r\n");
    return _SIM800L_waitResponse();
}

/*******************************************************************************
* shutdown SIM800L
********************************************************************************
*
* Task:
*  Shutdown the SIM800L Module
*
*******************************************************************************/
uint8 `$INSTANCE_NAME`_shutdown(){
    _SIM800L_Print("AT+CPOWD=1\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    `$INSTANCE_NAME``[SIM_REG]`Write(0x00);
    `$INSTANCE_NAME``[SIM]`Stop();
    return _SIM800L_waitResponse();
}


/*******************************************************************************
* power on SIM800L
********************************************************************************
*
* Task:
*  Power up the SIM800L Module
*
*******************************************************************************/
uint8 `$INSTANCE_NAME`_start(){
    `$INSTANCE_NAME``[SIM]`Start();
    `$INSTANCE_NAME``[SIM_REG]`Write(0x03);
    vTaskDelay(pdMS_TO_TICKS(3000));
    return `$INSTANCE_NAME`_check();
}

/*******************************************************************************
* verificarRegistro
********************************************************************************
*
* Tarea que realiza:
*  verifica que el modulo se encuentre registrado con el operador
*
*******************************************************************************/
xRespuestasGSM `$INSTANCE_NAME`_verificarRegistro(){
    xRespuestasGSM respuesta;
    _SIM800L_Print("AT+CREG?\r\n");
    sscanf(comandoRecibido[1],"+CREG: %d,%d",&respuesta.resultado1,&respuesta.resultado2);
    vTaskDelay(pdMS_TO_TICKS(2));
    return respuesta;
}

/*******************************************************************************
* identificación de fabricante
********************************************************************************
*
* Tarea que realiza:
*  solicita la información de fabricante del módulo
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_identFabricante(){
    xbufferEntradaGSM bufferEntrada;
    `$INSTANCE_NAME`_enviarComando("AT+CGMI\r\n");
    sscanf(comandoRecibido[1],"%s",bufferEntrada.entrada);
    return bufferEntrada;
}

/*******************************************************************************
* identificación de Modelo
********************************************************************************
*
* Tarea que realiza:
*  solicita la información de modelo
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_Modelo(){
    xbufferEntradaGSM bufferEntrada;
    `$INSTANCE_NAME`_enviarComando("ATI\r\n");
    sscanf(comandoRecibido[1],"%s",bufferEntrada.entrada);
    return bufferEntrada;
}

/*******************************************************************************
* identificación de IMEI
********************************************************************************
*
* Tarea que realiza:
*  solicita la información de modelo
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_IMEI(){
    xbufferEntradaGSM bufferEntrada;
    `$INSTANCE_NAME`_enviarComando("AT+CGSN\r\n");
    sscanf(comandoRecibido[1],"%s",bufferEntrada.entrada);
    return bufferEntrada;
}

/*******************************************************************************
* identificación de IMSI (international mobile subscriber identity)
********************************************************************************
*
* Tarea que realiza:
*  solicita la identificación individual de la SIM que esté introducida
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_IMSI(){
    xbufferEntradaGSM bufferEntrada;
    `$INSTANCE_NAME`_enviarComando("AT+CIMI\r\n");
    sscanf(comandoRecibido[1],"%s",bufferEntrada.entrada);
    return bufferEntrada;
}

/*******************************************************************************
* verificarEstadoGPRS
********************************************************************************
*
* Tarea que realiza:
*  verifica que en que estado se encuentra el servicio GPRS
*
*******************************************************************************/
xRespuestasGSM `$INSTANCE_NAME`_verificarEstadoGPRS(){
    xRespuestasGSM respuesta;
    `$INSTANCE_NAME`_enviarComando("AT+CGATT?\r\n");
    sscanf(comandoRecibido[1],"+CGATT: %d",&respuesta.resultado1);
    vTaskDelay(pdMS_TO_TICKS(2));
    return respuesta;
}

/*******************************************************************************
* potencia señal
********************************************************************************
*
* Tarea que realiza:
*  adquiere el valor de potencia de señal
*
*******************************************************************************/
xRespuestasGSM `$INSTANCE_NAME`_potenciaSenal(){
    xRespuestasGSM respuesta;
    `$INSTANCE_NAME`_enviarComando("AT+CSQ\r\n");
    sscanf(comandoRecibido[1],"+CSQ: %d,%d",&respuesta.resultado1,&respuesta.resultado2);
    vTaskDelay(pdMS_TO_TICKS(2));
    return respuesta;
}

/*******************************************************************************
* verificarOperador
********************************************************************************
*
* Tarea que realiza:
*  obtiene el estado y el nombre del operador
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_verificarOperador(){
    xbufferEntradaGSM respuesta;
    char* puntero;
    char ch = '\"';
    limpiarBuffer();
    `$INSTANCE_NAME``[SIM]`PutString("AT+COPS?\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    recibirArray();
    
    puntero = strrchr(comandoRecibido[1],ch);
    *puntero = 0;
    puntero = strchr(comandoRecibido[1],ch);
    strcpy(respuesta.entrada,puntero+1);
    return respuesta;
}

/*******************************************************************************
* verificarEstadoTCP
********************************************************************************
*
* Tarea que realiza:
*  obtiene el estado de la conexión TCP
*
*******************************************************************************/
uint8 `$INSTANCE_NAME`_verificarEstadoTCP(){
    uint8 estado=0;
    char* puntero;
    char ch = ' ';
    xInformacionGSM infoGSM;
    
    xQueuePeek(datosGSM,&infoGSM,10);
    limpiarBuffer();
    `$INSTANCE_NAME``[SIM]`PutString("AT+CIPSTATUS\r\n");
    vTaskDelay(pdMS_TO_TICKS(500));
    recibirArray();
    puntero = strchr(comandoRecibido[2],ch);
    if(!strcmp(puntero+1,"IP INITIAL")){
        estado=`$INSTANCE_NAME`_IPINITIAL;
    }
    else if(!strcmp(puntero+1,"IP START")){
        estado=`$INSTANCE_NAME`_IPSTART;
    }
    else if(!strcmp(puntero+1,"IP CONFIG")){
        estado=`$INSTANCE_NAME`_IPCONFIG;
    }
    else if(!strcmp(puntero+1,"IP GPRSACT")){
        estado=`$INSTANCE_NAME`_IPGPRSACT;
    } 
    else if(!strcmp(puntero+1,"IP STATUS")){
        estado=`$INSTANCE_NAME`_IPSTATUS;
    }
    else if(!strcmp(puntero+1,"TCP CLOSED")){
        estado=`$INSTANCE_NAME`_TCPCLOSED;
    }
    else if(!strcmp(puntero+1,"TCP CONNECTING")){
        estado=`$INSTANCE_NAME`_TCPCONNECTING;
    }
    else if(!strcmp(puntero+1,"CONNECT OK")){
        estado=`$INSTANCE_NAME`_CONNECTOK;
    }
    else if(!strcmp(puntero+1,"CONNECT FAIL")){
        estado=`$INSTANCE_NAME`_CONNECTFAIL;
    }
    else if(!strcmp(puntero+1,"PDP DEACT")){
        estado=`$INSTANCE_NAME`_PDPDEACT;
    }
    strcpy(infoGSM.estado,puntero+1);
    xQueueOverwrite(datosGSM,&infoGSM);
    return estado;
}

/*******************************************************************************
* identificación de ip de GPRS
********************************************************************************
* 
* Tarea que realiza:
*  solicita la ip que tiene configurada para la conexion GPRS (local)
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_IpLocal(){
    xbufferEntradaGSM bufferEntrada;
    `$INSTANCE_NAME`_enviarComando("AT+CIFSR\r\n");
    strcpy(bufferEntrada.entrada,comandoRecibido[1]);
    return bufferEntrada;
}

/*******************************************************************************
* definicion de bandas
********************************************************************************
* 
* Tarea que realiza:
*  configura la banda con la que trabaja el SIM800L
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_Banda(uint8 banda){
    CYBIT comando_ok = pdFALSE;
    char string[30];
    char cadena[20];
    switch(banda){
        case `$INSTANCE_NAME`_BANDA_EGSM_MODE:
            strcpy(cadena,"EGSM_MODE");
        break;
        case `$INSTANCE_NAME`_BANDA_EGSM_DCS_MODE:
            strcpy(cadena,"EGSM_DCS_MODE");
        break;
        case `$INSTANCE_NAME`_BANDA_EGSM_PCS_MODE:
            strcpy(cadena,"EGSM_PCS_MODE");
        break;
        case `$INSTANCE_NAME`_BANDA_GSM850_MODE:
            strcpy(cadena,"GSM850_MODE");
        break;
        case `$INSTANCE_NAME`_BANDA_GSM850_PCS_MODE:
            strcpy(cadena,"GSM850_PCS_MODE");
        break; 
        case `$INSTANCE_NAME`_BANDA_PCS_MODE:
            strcpy(cadena,"PCS_MODE");
        break; 
        case `$INSTANCE_NAME`_BANDA_PGSM_MODE:
            strcpy(cadena,"PGSM_MODE");
        break; 
        case `$INSTANCE_NAME`_BANDA_ALL_BAND:
            strcpy(cadena,"ALL_BAND");
        break;     
    }
    sprintf(string,"AT+CBAND=\"%s\"\r\n",cadena);
    `$INSTANCE_NAME`_enviarComando(string);
    if(!strcmp(comandoRecibido[1],"OK")){
        comando_ok = pdTRUE;
    }
    return comando_ok;
}

/*******************************************************************************
* iniciar conexión TCP/IP
********************************************************************************
*
* Tarea que realiza:
*  rutina para iniciar el protocolo TCP
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_TCPInit(uint16 operador){
    //xbufferEntradaGSM bufferEntrada;
    xRespuestasGSM respuesta;
    xInformacionGSM infoGSM;
    uint8 estadoModem = 0, estadoTCP=0;
    CYBIT comando_ok = pdFALSE, fin = pdFALSE;
    char comando[200];
    int timeOut = 0;
    char *apn;
    
    DEBUG("Inf = MODEM: Init TCP connection\r\n");
    xQueuePeek(datosGSM,&infoGSM,10);
    switch(operador){
        case(`$INSTANCE_NAME`_OPER_TIGO):{
            apn=`$INSTANCE_NAME`_APN_TIGO;
        break;}
        case(`$INSTANCE_NAME`_OPER_TIGO_M2M):{
            apn=`$INSTANCE_NAME`_APN_TIGO_M2M;
        break;}
        case(`$INSTANCE_NAME`_OPER_AVANTEL):{
            apn=`$INSTANCE_NAME`_APN_AVANTEL;
        break;}
        case(`$INSTANCE_NAME`_OPER_VIRGIN):{
            apn=`$INSTANCE_NAME`_APN_VIRGIN;
        break;}
        case(`$INSTANCE_NAME`_OPER_MOVISTAR):{
            apn=`$INSTANCE_NAME`_APN_MOVISTAR;
        break;}
        case(`$INSTANCE_NAME`_OPER_CLARO):{
            apn=`$INSTANCE_NAME`_APN_CLARO;
        break;}
        case(`$INSTANCE_NAME`_OPER_UNE):{
            apn=`$INSTANCE_NAME`_APN_UNE;
        break;}
        case(`$INSTANCE_NAME`_OPER_ETB):{
            apn=`$INSTANCE_NAME`_APN_ETB;
        break;}
        case(`$INSTANCE_NAME`_OPER_EXITO):{
            apn=`$INSTANCE_NAME`_APN_EXITO;
        break;}
        case(`$INSTANCE_NAME`_OPER_FLASH):{
            apn=`$INSTANCE_NAME`_APN_FLASH;
        break;}
        default:{
            apn=`$INSTANCE_NAME`_APN_AVANTEL;
        break;}
    }
    while(!fin){
        vTaskDelay(pdMS_TO_TICKS(1));
        switch(estadoModem){
            //Habilitar eco en el modem
            case 0:{
                `$INSTANCE_NAME`_enviarComando("ATE1\r\n");
                timeOut++;
                xQueuePeek(datosGSM,&infoGSM,10);
                infoGSM.debug.resultado1 = 1;
                if(!strcmp(comandoRecibido[1],"OK")){
                    DEBUG("Inf = MODEM: ECO Active\r\n");
                    xQueuePeek(datosGSM,&infoGSM,10);
                    infoGSM.debug.resultado2 = 1;
                    estadoModem++;
                }
                if(timeOut>=`$INSTANCE_NAME`_GSM_TIME_OUT){
                    DEBUG("Inf = MODEM: Error in eco activation\r\n");
                    infoGSM.debug.resultado2 = 0;
                    fin = pdTRUE;
                }
                xQueueOverwrite(datosGSM,&infoGSM);
            break;}
            //verificar registro en el operador    
            case 1:{
                xQueuePeek(datosGSM,&infoGSM,10);
                infoGSM.debug.resultado1 = 2;
                respuesta = `$INSTANCE_NAME`_verificarRegistro();
                if((respuesta.resultado2 == 1)||(respuesta.resultado2 == 5)){
                    //configura modo de IP única
                    `$INSTANCE_NAME`_enviarComando("AT+CIPMUX=0\r\n");
                    //configura el modo de aplicación TCP en modo transparente
                    `$INSTANCE_NAME`_enviarComando("AT+CIPMODE=1\r\n");
                    estadoModem++;
                    infoGSM.debug.resultado2 = 1;
                    DEBUG("Inf = MODEM: Connection bridge stablish\r\n");
                }else{
                    DEBUG("Inf = MODEM: Error stablishing bridge\r\n");
                    infoGSM.debug.resultado2 = 0;
                    fin = pdTRUE;
                }
                xQueueOverwrite(datosGSM,&infoGSM);
            break;} 
            //habilitar enlace  GPRS    
            case 2:{
                xQueuePeek(datosGSM,&infoGSM,10);
                infoGSM.debug.resultado1 = 3;
                DEBUG("Inf = MODEM: Activating GRPS\r\n");
                `$INSTANCE_NAME`_enviarComando("AT+CGATT=1\r\n");
                estadoModem++;
                timeOut=0;
                infoGSM.debug.resultado2 = 1;
                xQueueOverwrite(datosGSM,&infoGSM);
            break;}
            //solicita el estado de TCP/IP y toma decisiones de cada uno    
            case 3:{
                xQueuePeek(datosGSM,&infoGSM,10);
                infoGSM.debug.resultado1 = 4;
                DEBUG("Inf = MODEM: Activating TCP\r\n");
                estadoTCP = `$INSTANCE_NAME`_verificarEstadoTCP();
                timeOut++;
                if(timeOut>=2000){
                    DEBUG("Inf = MODEM: Error activating TCP\r\n");
                    infoGSM.debug.resultado2 = 0;
                    xQueueOverwrite(datosGSM,&infoGSM);
                    fin = pdTRUE;
                }
                switch(estadoTCP){
                    //si pide la ip inicial configura el APN
                    case `$INSTANCE_NAME`_IPINITIAL:{
                        //if(strcmp(infoGSM.operador,OPERTIGO)){
                            DEBUG("Inf = MODEM: Complementing status...\r\n");
                            sprintf(comando,"AT+CSTT=\"%s\"\r\n",apn);
                            `$INSTANCE_NAME`_enviarComando(comando);
                            //vTaskDelay(pdMS_TO_TICKS(200));
                        //}
                    break;}
                    //brinda la conexión con el GPRS    
                    case `$INSTANCE_NAME`_IPSTART:{
                        DEBUG("Inf = MODEM: GPRS connected\r\n");
                        `$INSTANCE_NAME`_enviarComando("AT+CIICR\r\n");
                        //vTaskDelay(pdMS_TO_TICKS(500));
                    break;}
                    //obtiene la ip local configurada por el operador    
                    case `$INSTANCE_NAME`_IPGPRSACT:{
                        DEBUG("Inf = MODEM: Obtaining IP\r\n");
                        xQueuePeek(datosGSM,&infoGSM,10);
                        strcpy(infoGSM.IpLocal,`$INSTANCE_NAME`_IpLocal().entrada);
                        xQueueOverwrite(datosGSM,&infoGSM);
                    break; }
                    //si está todo correcto, termina la preparación
                    case `$INSTANCE_NAME`_IPSTATUS:{
                        DEBUG("Inf = MODEM: Finishing set of connection\r\n");
                        limpiarTodo();
                        estadoModem++;
                    break;}
                    case `$INSTANCE_NAME`_TCPCONNECTING:{
                        DEBUG("Inf = MODEM: Connecting TCP\r\n");
                        limpiarBuffer();
                        //limpiarTodo();
                        estadoModem++;
                    break;}
                    case `$INSTANCE_NAME`_CONNECTOK:{
                        DEBUG("Inf = MODEM: Connection OK\r\n");
                        limpiarBuffer();
                        //limpiarTodo();
                        estadoModem++;
                    break;}
                    case `$INSTANCE_NAME`_PDPDEACT:{
                        DEBUG("Err = MODEM: Connection Deactivated\r\n");
                        `$INSTANCE_NAME`_enviarComando("AT+CIPSHUT\r\n");
                        fin = pdTRUE;
                    break;}
                    case `$INSTANCE_NAME`_CONNECTFAIL:{
                        DEBUG("Err = MODEM: Connection Failed\r\n");
                        `$INSTANCE_NAME`_enviarComando("AT+CIPSHUT\r\n");
                        fin = pdTRUE;
                    break;}
                    case `$INSTANCE_NAME`_TCPCLOSED:{
                        DEBUG("Err = MODEM: TCP connection closed\r\n");
                        `$INSTANCE_NAME`_enviarComando("AT+CIPSHUT\r\n");
                        fin = pdTRUE;
                    break;}
                }
            break;}
            //se configura el bearer para aplicaciones IP
            case 4:{
                DEBUG("Inf = MODEM: Bearer setting IP connection\r\n");
                xQueuePeek(datosGSM,&infoGSM,10);
                infoGSM.debug.resultado1 = 5;
                `$INSTANCE_NAME`_enviarComando("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
                sprintf(comando,"AT+SAPBR=3,1,\"APN\",\"%s\"\r\n",apn);
                `$INSTANCE_NAME`_enviarComando(comando);
                `$INSTANCE_NAME`_enviarComando("AT+SAPBR=1,1\r\n");
                fin = pdTRUE;
                comando_ok = pdTRUE;
                infoGSM.debug.resultado2 = 1;
                xQueueOverwrite(datosGSM,&infoGSM);
            break;}
        } 
    }
    return comando_ok;
}

/*******************************************************************************
* conectarse a un Host Remoto
********************************************************************************
*
* Tarea que realiza:
*  esta función debe utilzarce solo cuando exista un enlace creado por TCP,
*  permite conectarse a una página o servidor remoto
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_conectarHost(char* Host, char* Port){
    xInformacionGSM infoGSM;
    CYBIT comando_ok = pdFALSE, fin = pdFALSE;
    uint8_t estadoTCP;
    char comando[200];
    int timeOut = 0;
    
    //intenta conectarse al broker remoto 
    while(!fin){
        estadoTCP = `$INSTANCE_NAME`_verificarEstadoTCP();
        timeOut++;
        if(timeOut>=1000){
            fin = pdTRUE;
        }
        switch(estadoTCP){
            case `$INSTANCE_NAME`_IPSTATUS:
                DEBUG("Inf = MODEM: Trying to connect to remote broker\r\n");
                sprintf(comando,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",Host,Port);
                `$INSTANCE_NAME`_enviarComando(comando);
                if(!strcmp(comandoRecibido[1],"OK")){
                    //LED_DEBUG_Write(1);
                }
            break;
            //si se ha iniciado el servicio TCP correctamente espera a que conecte al broker remoto    
            case `$INSTANCE_NAME`_TCPCONNECTING:
                while(!fin){
                    limpiarBuffer();
                    if((!strcmp(comandoRecibido[0],"CONNECT"))||(!strcmp(comandoRecibido[3],"CONNECT"))){
                        xQueuePeek(datosGSM,&infoGSM,10);
                        DEBUG("Inf = MODEM: Connected to server\r\n");
                        strcpy(infoGSM.estado,"CONECTADO");
                        xQueueOverwrite(datosGSM,&infoGSM);
                        //SIM_PutString("AT+CIPSEND\r\n");
                        //recibirArray();
                        comando_ok = pdTRUE;
                        fin = pdTRUE;
                    }
                    recibirArray();
                    vTaskDelay(pdMS_TO_TICKS(200));
                    timeOut++;
                    if(timeOut>=100){
                        fin = pdTRUE;
                    }
                }
            break;   
            case `$INSTANCE_NAME`_PDPDEACT:{
                `$INSTANCE_NAME`_enviarComando("AT+CIPSHUT\r\n");
                fin = pdTRUE;
            break;}
            case `$INSTANCE_NAME`_CONNECTFAIL:{
                `$INSTANCE_NAME`_enviarComando("AT+CIPSHUT\r\n");
                fin = pdTRUE;
            break;}
            case `$INSTANCE_NAME`_TCPCLOSED:{
                `$INSTANCE_NAME`_enviarComando("AT+CIPSHUT\r\n");
                fin = pdTRUE;
            break;}  
        }
    }
    return comando_ok;
}

/*******************************************************************************
* cambiar de modo datos a modo comando AT
********************************************************************************
*
* Tarea que realiza:
*  cambia a comandos AT sin desconectar del host TCP
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_datosaAT(){
    CYBIT comando_ok = pdFALSE;
    vTaskDelay(pdMS_TO_TICKS(1000));
    limpiarTodo();
    `$INSTANCE_NAME``[SIM]`PutString("+++");
    vTaskDelay(pdMS_TO_TICKS(1000));
    recibirArray();
    if(!strcmp(comandoRecibido[1],"OK")){
        comando_ok = pdTRUE;
    }
    return comando_ok;
}

/*******************************************************************************
* cambiar de modo comandos AT a modo datos
********************************************************************************
*
* Tarea que realiza:
*  cambia a modo de datos desde modo comandos AT
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_ATaDatos(){
    uint8 i=0,j=0;
    int tiempo=0;
    CYBIT comando_ok = pdFALSE, recibido = pdFALSE;
    limpiarTodo();
    `$INSTANCE_NAME``[SIM]`PutString("ATO\r\n");
    //recibirArray();
    vTaskDelay(pdMS_TO_TICKS(200));
    while((tiempo<`$INSTANCE_NAME`_GSM_TIME_OUT)&&(!recibido)&&(`$INSTANCE_NAME``[SIM]`GetRxBufferSize()>0))
    {
        bufferEntrada[i]=`$INSTANCE_NAME``[SIM]`GetChar();
        if(bufferEntrada[i]==0x0A){
            tiempo = 0;
            sprintf(comandoRecibido[j],"%s",bufferEntrada);
            i=0;
            if(!strcmp(comandoRecibido[j],"CONNECT\r\n")){
                recibido=pdTRUE;
                comando_ok = pdTRUE;
            }
            else if(!strcmp(comandoRecibido[j],"ERROR\r\n")){
                recibido=pdTRUE;
                comando_ok = pdFALSE;
            }
            limpiarBuffer();
            j++;
        }
        else{
            i++;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
        tiempo++;
    }
    return comando_ok;
}

/*******************************************************************************
* conectarse a un servidor FTP Remoto
********************************************************************************
*
* Tarea que realiza:
*  esta función permite conectar a un servidor FTP para descarga de archivos
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_conectarFTP(char* servidor, char* usuario, char*clave, char*archivo){
    CYBIT comando_ok = pdFALSE, fin = pdFALSE, recepcion = pdFALSE;
    uint8_t estadoFTP = 1, cantToken = 0;
    char comando[200];
    int timeOut=0, token=0, acumulado=0;
    xRespuestasGSM respuesta;
    xinfoDescargada datos;
    xInformacionGSM infoGSM;
    //intenta conectarse al servidor remoto 
    xQueuePeek(datosGSM,(void*)&infoGSM,10);
    while(!fin){
        switch(estadoFTP){
            //define parametros para conexion FTP
            case 1:{
                `$INSTANCE_NAME`_enviarComando("AT+FTPCID=1\r\n");
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
                else{
                    estadoFTP = 0;
                }
            break;}
            case 2:{
                sprintf(comando,"AT+FTPSERV=\"%s\"\r\n",servidor);
                `$INSTANCE_NAME`_enviarComando(comando);
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
            break;}
            case 3:{
                sprintf(comando,"AT+FTPUN=\"%s\"\r\n",usuario);
                `$INSTANCE_NAME`_enviarComando(comando);
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
            break;}
            case 4:{
                sprintf(comando,"AT+FTPPW=\"%s\"\r\n",clave);
                `$INSTANCE_NAME`_enviarComando(comando);
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
            break;}
            case 5:{
                sprintf(comando,"AT+FTPGETNAME=\"%s\"\r\n",archivo);
                `$INSTANCE_NAME`_enviarComando(comando);
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
            break;}
            case 6:{
                `$INSTANCE_NAME`_enviarComando("AT+FTPGETPATH=\"/\"\r\n");
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
            break;} 
            case 7:{
                `$INSTANCE_NAME`_enviarComando("AT+FTPGET=1\r\n");
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoFTP++;
                }
            break;}
            case 8:{//espera a que se reciban datos validos
                limpiarBuffer();
                cantToken = recibirArray();
                sscanf(comandoRecibido[0],"+FTPGET: %d,%d",&respuesta.resultado1,&respuesta.resultado2);
                if(respuesta.resultado1==1){
                    switch(respuesta.resultado2){
                        case `$INSTANCE_NAME`_DATA_AVAILABLE:
                            estadoFTP++;
                            vTaskDelay(pdMS_TO_TICKS(100));
                        break;
                        case `$INSTANCE_NAME`_DATA_FIN:
                            comando_ok = pdTRUE;
                            fin = pdTRUE;
                        break;   
                        default:
                            comando_ok = respuesta.resultado2;
                            fin = pdTRUE;
                        break;    
                    }
                }
                if(respuesta.resultado1==2){
                    estadoFTP++;
                    goto recibirdatos;
                }
            break;}
            case 9:{
                cantToken = `$INSTANCE_NAME`_enviarComando("AT+FTPGET=2,140\r\n");
                sscanf(comandoRecibido[1],"+FTPGET: %d,%d",&respuesta.resultado1,&respuesta.resultado2);
                if(respuesta.resultado1==2){
                    recibirdatos: strcpy(datos.ruta,archivo);
                    datos.estado = pdTRUE;
                    datos.numBytes = respuesta.resultado2;
                    acumulado += datos.numBytes;
                    respuesta.resultado1 = 0;
                    respuesta.resultado2 = 0;
                    infoGSM.debug.resultado2 = acumulado;
                    infoGSM.debug.resultado1 = datos.numBytes;
                    xQueueOverwrite(datosGSM,(void*)&infoGSM);
                    if(datos.numBytes>0){
                        strcpy(datos.datos,"");
                        for(token=2;token<cantToken;token++){
                            datos.operacion = GUARDAR_DESCARGA;
                            comandoRecibido[token][strlen(comandoRecibido[token])]=0;
                            if(strcmp(comandoRecibido[token],"OK")){
                                if(comandoRecibido[token][0]==':'){
                                    strcat(datos.datos,"\r\n");
                                }
                                strcat(datos.datos,comandoRecibido[token]);
                            }
                            else{
                                while(!xQueueSend(datosDescargados,(void*)&datos,10));
                                break;
                            }
                        }
                    }
                    /*else{
                        comando_ok = pdTRUE;
                        fin = pdTRUE;
                        datos.estado = pdFALSE;
                        while(!xQueueSendToBack(datosDescargados,(void*)&datos,10));
                    }*/
                }
                else if(respuesta.resultado1==1){
                    infoGSM.debug.resultado1 = respuesta.resultado1;
                    infoGSM.debug.resultado2 = respuesta.resultado2;
                    xQueueOverwrite(datosGSM,(void*)&infoGSM);
                    if(respuesta.resultado2 == `$INSTANCE_NAME`_DATA_FIN){
                        comando_ok = pdTRUE;
                        fin = pdTRUE;
                        datos.estado = pdFALSE;
                        while(!xQueueSendToBack(datosDescargados,(void*)&datos,10));
                    }
                }
                else if(!strcmp(comandoRecibido[1],"ERROR")){
                    comando_ok = pdFALSE;
                    fin = pdTRUE;
                    datos.estado = pdFALSE;
                    while(!xQueueSendToBack(datosDescargados,(void*)&datos,10));
                }
            break;}
            default:
                fin = pdTRUE;
            break;    
        }
    }
    return comando_ok;
}

/* [] END OF FILE */
