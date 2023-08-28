/* ==============================================================
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
 * ==============================================================
;FUNCIONES PARA MANEJO DEL GSM A6
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

 
uint8 bufferEntrada[`$INSTANCE_NAME`_GSM_LEN_MAX];
char comandoRecibido[`$INSTANCE_NAME`_NUM_COMANDOS][`$INSTANCE_NAME`_COMANDO_LEN];

/*******************************************************************************
* limpiarBuffer
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia el buffer de recepción temporal
*
*******************************************************************************/
static void limpiarBuffer(){
    uint8 i;
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

/*******************************************************************************
* verificar
********************************************************************************
*
* Tarea que realiza:
*  Funcion que verifica el buffer y extrae los comandos necesarios
*
*******************************************************************************/
int verificar(){
    //xPuenteSerial puenteGSM;
    //puenteGSM.modoOperacion=1;
    int canTokens = 0;
    const char delimitadores[2] = "\r\n";
    char* token;
    uint8 i = 0;
    limpiarComando();
    `$INSTANCE_NAME``[SIM]`ClearRxBuffer();
    //obtiene el primer token
    token = strtok((char*)bufferEntrada, delimitadores);
    //obtiene el resto de token
    while( token != NULL ) {
        strcpy(comandoRecibido[i],token);
        //strcpy(puenteGSM.comandoEntrada.entrada,comandoRecibido[i]);
        //xQueueSendToBack(puenteSerialEntradaGSM,&puenteGSM,200);
        token = strtok(NULL, delimitadores);
        i++;
        canTokens++;
    }
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
    uint8 i;
    vTaskDelay(pdMS_TO_TICKS(100));
    limpiarBuffer();
    if(`$INSTANCE_NAME``[SIM]`GetRxBufferSize()>0)
    {
        for(i=0;i<`$INSTANCE_NAME``[SIM]`GetRxBufferSize();i++)
        {
            bufferEntrada[i]=`$INSTANCE_NAME``[SIM]`rxBuffer[i];
        }
    }
    return verificar();
}

/*******************************************************************************
* limpiarTodo
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia todos los buffer involucrados en el manejo del A6
*
*******************************************************************************/
void limpiarTodo(){
    `$INSTANCE_NAME``[SIM]`ClearRxBuffer();
    limpiarBuffer();
    limpiarComando();
}

/*******************************************************************************
* enviarString
********************************************************************************
*
* Tarea que realiza:
*  Envia cadenas de caracteres por el puerto del A6
*
*******************************************************************************/
int `$INSTANCE_NAME`_enviarString(char* cadena){
    `$INSTANCE_NAME``[SIM]`PutString(cadena);
    vTaskDelay(pdMS_TO_TICKS(10));
    return recibirArray();
}

/*******************************************************************************
* leerComando
********************************************************************************
*
* Tarea que realiza:
*  Devuelve el contenido del comando recibido apuntado por el valor indice
*
*******************************************************************************/
xbufferEntradaGSM `$INSTANCE_NAME`_leerComando(int indice){
    xbufferEntradaGSM comando;
    strcpy(comando.entrada,comandoRecibido[indice]);
    return comando; 
}

/*******************************************************************************
* apagarA6
********************************************************************************
*
* Tarea que realiza:
*  Apagar el módulo A6
*
*******************************************************************************/
void `$INSTANCE_NAME`_apagar(){
    `$INSTANCE_NAME``[SIM]`PutString("AT+CPOF\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    recibirArray();
    `$INSTANCE_NAME``[SIM_REG]`Write(0x00);
    `$INSTANCE_NAME``[SIM]`Stop();
}

/*******************************************************************************
* encenderA6
********************************************************************************
*
* Tarea que realiza:
*  Enciende el módulo A6
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_encender(){
    CYBIT comando_ok=pdFALSE;
    `$INSTANCE_NAME``[SIM]`Start();
    `$INSTANCE_NAME``[SIM_REG]`Write(0x03);
    vTaskDelay(pdMS_TO_TICKS(3000));
    `$INSTANCE_NAME``[SIM]`PutString("AT\r\n");
    recibirArray();
    if(!strcmp(comandoRecibido[1],"OK")){
        comando_ok = pdTRUE;
    }
    return comando_ok;
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CREG?\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CGMI\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CGMM\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+EGMR=2,7\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CIMI\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CGATT?\r\n");
    recibirArray();
    sscanf(comandoRecibido[1],"+CGATT:%d",&respuesta.resultado1);
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CSQ\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+COPS?\r\n");
    vTaskDelay(pdMS_TO_TICKS(1000));
    recibirArray();
    
    puntero = strrchr(comandoRecibido[1],ch);
    *puntero = 0;
    puntero = strchr(comandoRecibido[1],ch);
    strcpy(respuesta.entrada,puntero+1);
    
    vTaskDelay(pdMS_TO_TICKS(500));
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CIPSTATUS\r\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    recibirArray();
    puntero = strchr(comandoRecibido[1],ch);
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
    `$INSTANCE_NAME``[SIM]`PutString("AT+CIFSR\r\n");
    recibirArray();
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
    `$INSTANCE_NAME``[SIM]`PutString(string);
    recibirArray();
    if(!strcmp(comandoRecibido[1],"OK")){
        comando_ok = pdTRUE;
    }
    vTaskDelay(pdMS_TO_TICKS(500));
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
CYBIT `$INSTANCE_NAME`_TCPInit(){
    //xbufferEntradaGSM bufferEntrada;
    xRespuestasGSM respuesta;
    xInformacionGSM infoGSM;
    uint8 estadoModem = 0, estadoTCP=0;
    CYBIT comando_ok = pdFALSE, fin = pdFALSE;
    char comando[200];
    int timeOut = 0;
    
    xQueuePeek(datosGSM,&infoGSM,10);
    while(!fin){
        vTaskDelay(pdMS_TO_TICKS(1));
        switch(estadoModem){
            //Habilitar eco en el modem
            case 0:{
                `$INSTANCE_NAME``[SIM]`PutString("ATE1\r\n");
                vTaskDelay(pdMS_TO_TICKS(10));
                recibirArray();
                timeOut++;
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoModem++;
                }
                if(timeOut>=`$INSTANCE_NAME`_GSM_TIME_OUT){
                    fin = pdTRUE;
                }
            break;}
            //verificar registro en el operador    
            case 1:{
                respuesta = `$INSTANCE_NAME`_verificarRegistro();
                if((respuesta.resultado2 == 1)||(respuesta.resultado2 == 5)){
                    //configura modo de IP única
                    `$INSTANCE_NAME``[SIM]`PutString("AT+CIPMUX=0\r\n");
                    vTaskDelay(pdMS_TO_TICKS(200));
                    //configura el modo de aplicación TCP en modo transparente
                    `$INSTANCE_NAME``[SIM]`PutString("AT+CIPMODE=1\r\n");
                    vTaskDelay(pdMS_TO_TICKS(200));
                    limpiarTodo();
                    estadoModem++;
                }else{
                    fin = pdTRUE;
                }
            break;} 
            //habilitar enlace  GPRS    
            case 2:{
                `$INSTANCE_NAME``[SIM]`PutString("AT+CGATT=1\r\n");
                vTaskDelay(pdMS_TO_TICKS(200));
                recibirArray();
                if(!strcmp(comandoRecibido[1],"OK")){
                    estadoModem++;
                }
                limpiarTodo();
            break;}
            //solicita el estado de TCP/IP y toma decisiones de cada uno    
            case 3:{
                estadoTCP = `$INSTANCE_NAME`_verificarEstadoTCP();
                switch(estadoTCP){
                    //si pide la ip inicial configura el APN
                    case `$INSTANCE_NAME`_IPINITIAL:{
                        //si es TIGO configura el APN de Tigo
                        //if(strcmp(infoGSM.operador,OPERTIGO)){
                            sprintf(comando,"AT+CSTT=\"%s\"\r\n",`$INSTANCE_NAME`_APN_AVANTEL);
                            `$INSTANCE_NAME``[SIM]`PutString(comando);
                            vTaskDelay(pdMS_TO_TICKS(200));
                            limpiarTodo();
                        //}
                    break;}
                    //brinda la conexión con el GPRS    
                    case `$INSTANCE_NAME`_IPSTART:{
                        `$INSTANCE_NAME``[SIM]`PutString("AT+CIICR\r\n");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                        limpiarTodo();
                    break;}
                    //obtiene la ip local configurada por el operador    
                    case `$INSTANCE_NAME`_IPGPRSACT:{
                        xQueuePeek(datosGSM,&infoGSM,10);
                        strcpy(infoGSM.IpLocal,`$INSTANCE_NAME`_IpLocal().entrada);
                        xQueueOverwrite(datosGSM,&infoGSM);
                    break; }
                    //si está todo correcto, termina la preparación
                    case `$INSTANCE_NAME`_IPSTATUS:{
                        limpiarTodo();
                        estadoModem++;
                    break;}
                    default:{
                        `$INSTANCE_NAME``[SIM]`PutString("AT+CIPSHUT\r\n");
                        limpiarTodo();
                        fin = pdTRUE;
                    break;  }  
                }
            break;}
            //se configura el bearer para aplicaciones IP
            case 4:{
                fin = pdTRUE;
                comando_ok = pdTRUE;
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
        switch(estadoTCP){
            case `$INSTANCE_NAME`_IPSTATUS:
                sprintf(comando,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",Host,Port);
                `$INSTANCE_NAME``[SIM]`PutString(comando);
                recibirArray();
                vTaskDelay(pdMS_TO_TICKS(200));
                if(!strcmp(comandoRecibido[1],"OK")){
                    vTaskDelay(pdMS_TO_TICKS(400));
                    //LED_DEBUG_Write(1);
                }
            break;
            //si se ha iniciado el servicio TCP correctamente espera a que conecte al broker remoto    
            case `$INSTANCE_NAME`_TCPCONNECTING:
                while(!fin){
                    if((!strcmp(comandoRecibido[0],"CONNECT"))||(!strcmp(comandoRecibido[3],"CONNECT"))){
                        xQueuePeek(datosGSM,&infoGSM,10);
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
                    if(timeOut>=`$INSTANCE_NAME`_GSM_TIME_OUT){
                        fin = pdTRUE;
                    }
                }
            break;   
            default:
                `$INSTANCE_NAME``[SIM]`PutString("AT+CIPSHUT\r\n");
                limpiarTodo();
                fin = pdTRUE;
            break;    
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
    `$INSTANCE_NAME``[SIM]`PutString("+++");
    vTaskDelay(pdMS_TO_TICKS(1000));
    recibirArray();
    if(!strcmp(comandoRecibido[1],"OK")){
        comando_ok = pdTRUE;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
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


/* [] END OF FILE */
