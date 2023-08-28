/* ==============================================================
 *
 * Copyright EKIA Technologies, 2017
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;FUNCIONES PROPIAS DEL PROTOCOLO MQTT
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

#include "SIM.h"
#include "colas.h"
#include "SIM_REG.h"
#include "SIM_PinReset.h"
#include "SIM800L.h"
#include "MQTT.h"
#include "Telemetria.h"

#include "LED_DEBUG.h"

CYBIT banderaNombreUsuario = pdFALSE;
CYBIT banderaClave = pdFALSE;
CYBIT banderaWill = pdFALSE;
uint8 MQTT_Conectado = MQTT_DESCONECTADO;
long TimeOut = TIEMPO_VIDA;   //tiempo de desconexion de 3 minutos
int conexionConACK = NO_ACK;

uint8 MQTT_bufferEntrada[MQTT_LEN_MAX];
uint8 MQTT_topico[MQTT_TOPIC_LEN_MAX];
uint8 MQTT_Mensaje[MQTT_MENS_LEN_MAX];

uint32 Longitud_Topico = 0;
uint32 Longitud_Mensaje = 0;

int identificadorMensaje = 0;
uint8 conexionviva = pdTRUE;

/*******************************************************************************
* limpiarBuffer
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia el buffer de recepción temporal
*
*******************************************************************************/
static void MQTT_limpiarBuffer(){
    uint i;
    for(i=0; i<(sizeof MQTT_bufferEntrada); i++){
        MQTT_bufferEntrada[i]=0;
    } 
}

/*******************************************************************************
* recibir datos MQTT
********************************************************************************
*
* Tarea que realiza:
*  recibe e interpreta datos con el protocolo MQTT
*
*******************************************************************************/
void MQTT_recibir(){
    CYBIT siguienteByteLongitud = pdTRUE, fin = pdFALSE;
    uint8 byteEntrada, byteCabecera;
    uint8 tipoMensajeRecibido, /*DUP,*/ QoS, mantenimiento;
    uint32 MQTT_Largo = 0, multiplicador = 1, MQTT_LargoTemp = 0;
    uint32 indice = 0, i = 0;
    uint32 InicioMensaje = 0;
    int indicePublicar = 0;
    int ID_Mensaje = 0;
    
    MQTT_limpiarBuffer();
    vTaskDelay(pdMS_TO_TICKS(50));
    byteCabecera = SIM_GetChar();
    tipoMensajeRecibido = (byteCabecera/16)&0x0F;
    /*DUP = (byteCabecera & MASCARA_DUP)/MASCARA_DUP;*/
    QoS = (byteCabecera & MASCARA_QoS)/ESCALA_QoS;
    mantenimiento = byteCabecera & MASCARA_MANTENIMIENTO;
    if((tipoMensajeRecibido >= CONNECT)&&(tipoMensajeRecibido <= DISCONNECT)){
        MQTT_Largo = 0;
        vTaskDelay(pdMS_TO_TICKS(5));
        while(siguienteByteLongitud && !fin){
            if(SIM_GetRxBufferSize()>0){
                byteEntrada = SIM_GetChar();
                if(((byteCabecera == 'C') && (byteEntrada == 'L') && (MQTT_Largo==0))
                    ||((byteCabecera == '+') && (byteEntrada == 'P') && (MQTT_Largo==0))
                ){
                    fin = pdTRUE;
                    indice = 0;
                    MQTT_bufferEntrada[indice] = byteCabecera;
                    indice++;
                    MQTT_bufferEntrada[indice] = byteEntrada; 
                    indice++;
                    siguienteByteLongitud = pdFALSE;
                    MQTT_Conectado = DESCONECTADO;
                }
                else{
                    if((byteEntrada & 128) != 128){
                        siguienteByteLongitud = pdFALSE;
                    }
                    MQTT_Largo += (byteEntrada & 127) * multiplicador;
                    multiplicador *= 128;
                }
            }
        }
        
        if(!fin){
            indice = 0;
            MQTT_LargoTemp = MQTT_Largo;
            while((MQTT_LargoTemp > 0) && ((SIM_GetRxBufferSize() > 0))){
                MQTT_bufferEntrada[indice]=SIM_GetChar();
                MQTT_LargoTemp--;
                vTaskDelay(pdMS_TO_TICKS(2));
                indice++;
            }
            switch(tipoMensajeRecibido){
                case CONNACK:{
                    conexionConACK = (MQTT_bufferEntrada[0]<<8)|MQTT_bufferEntrada[1];
                    if(conexionConACK == 0){
                        MQTT_Conectado = MQTT_CONECTADO;
                        MQTT_enConexion();
                    }
                    break;
                }
                case PUBLISH:{
                    Longitud_Topico = (MQTT_bufferEntrada[0]<<8)|MQTT_bufferEntrada[1];
                    indicePublicar = 0;
                    for(i=2; i < (Longitud_Topico + 2); i++){
                        MQTT_topico[indicePublicar] = MQTT_bufferEntrada[i];
                        indicePublicar++;
                    }
                    MQTT_topico[indicePublicar] = 0;
                    Longitud_Topico = indicePublicar;
                    
                    indicePublicar = 0;
                    InicioMensaje = Longitud_Topico + 2UL;
                    ID_Mensaje = 0;
                    
                    if(QoS !=0){
                        InicioMensaje +=2;
                        ID_Mensaje = (MQTT_bufferEntrada[Longitud_Topico + 2UL]<<8) | MQTT_bufferEntrada[Longitud_Topico + 3UL];
                    }
                    for(i=InicioMensaje; i<MQTT_Largo; i++){
                        MQTT_Mensaje[indicePublicar] = MQTT_bufferEntrada[i];
                        indicePublicar++;
                    }
                    MQTT_Mensaje[indicePublicar] = 0;
                    Longitud_Mensaje = indicePublicar;
                    
                    if(QoS == 1){
                        MQTT_publicarACK(ID_Mensaje);
                    }
                    else if(QoS == 2){
                        MQTT_publicarREC(ID_Mensaje);
                    }
                    MQTT_enMensaje((char*)MQTT_topico, Longitud_Topico, (char*)MQTT_Mensaje, Longitud_Mensaje);
                    break;
                }
                case PUBREL:{    
                    MQTT_publicarCOMP((MQTT_bufferEntrada[0]<<8)|MQTT_bufferEntrada[1]);
                    break;
                }
                case PUBREC:{ 
                    MQTT_publicarREL(0,(MQTT_bufferEntrada[0]<<8)|MQTT_bufferEntrada[1]);
                    break;
                }
                case PUBACK:{
                    break;
                }
                case PUBCOMP:{
                    break;
                }
                case SUBACK:{
                    break;
                }
                case PINGREQ:{
                    break;
                }
                case PINGRESP:{
                    conexionviva = pdTRUE;
                    break;
                }
                case DISCONNECT:{
                    break;
                }
            }
        }
    }
}

/*******************************************************************************
* enviar largo de cadena
********************************************************************************
*
* Tarea que realiza:
*  Funcion que convierte el largo del mensaje al formato requerido por el protocolo
*  MQTT a la hora de enviar un mensaje
*
* Parametros:
*   longitud: Corresponde a la longitud calculada del mensaje MQTT
*
*******************************************************************************/
void MQTT_enviarLargo(int longitud){
    CYBIT  banderaLargo = pdFALSE;
    while (!banderaLargo){
        if ((longitud / 128) > 0)
        {
            SIM_PutChar(((longitud % 128) + 128));
            longitud /= 128;
        }
        else
        {
            banderaLargo = pdTRUE;
            SIM_PutChar(longitud);
        }
    }
}

/*******************************************************************************
* enviar cadena formateada
********************************************************************************
*
* Tarea que realiza:
*  Funcion que envia la longitud de una cadena junto con el mensaje en el formato
*  requerido por el protocolo MQTT
*
* Parametros:
*   cadena: Corresponde al mensaje que desea enviarse
*
*******************************************************************************/
void MQTT_enviarCadenaUTF(char *string){
  int longitudLocal = strlen(string);
  SIM_PutChar(longitudLocal / 256);
  SIM_PutChar(longitudLocal % 256);
  SIM_PutString(string);
}

/*******************************************************************************
* generar identificador de mensaje
********************************************************************************
*
* Tarea que realiza:
*  Funcion que genera el identificador de mensaje necesario para algunas funciones
*
*******************************************************************************/
int MQTT_generarIDMensaje(){
    if (identificadorMensaje < 65535){
        identificadorMensaje++;
    }
    else{
        identificadorMensaje = 0;
    }
    return identificadorMensaje;
}

/*******************************************************************************
* conectar a un broker remoto utilizando el protocolo MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion que crea la comunicación con un broker remoto y mantiene el enlace
*
* Parametros:
*   organizacion:   Nombre de la organización, identificador unico del servidor.
*   ID_Cliente:     Una cadena de identifica al cliente en el servidor, debe ser único
*                   entre todos los clientes que se conecten al servidor, debe ser mayor
*                   que 0 y menor de 24.
*   nombreUsuario:  el nombre de usuario es solicitado de forma opcional, en cuyo 
*                   caso corresponde al identificador de usuario y que suele ser utilizado
*                   a modo de autenticación.
*   clave:          cadena que corresponde a la contraseña de usuario y que es utilizada
*                   para autenticar al usuario, este valor es opcional.
*   sesionLimpia:   indica si la conexión será de tipo limpia o no, si es en (0) entonces
*                   el servidor almacenará las suscripciones del cliente luego de su desconexión.
*                   si es (1) el servidor descartará cualquier información anterior sobre el cliente
*                   y tratará la conexión como limpia (cleanSession)
*   QoS:            Calidad del servicio del mensaje
*   mantenimiento:  Indica el tipo de mantenimiento, si está en (1) indica al servidor si debe mantener el mensaje
*   Topico:         Dirección del tópico que desee conectar y en el cual se publicaran mensajes en caso de una
*                   desconexión involuntaria por parte del cliente.
*   Mensaje:        Mensaje que será enviado al tópico antes configurado en caso de una desconexión
*                   involuntaria por parte del cliente.
*
*******************************************************************************/
void MQTT_conectarBroker(char* protocolo, char* ID_Cliente, char* nombreUsuario, char* clave, uint8 sesionLimpia, uint8 QoS, uint8 mantenimiento, char *Topico, char *Mensaje){
    int longitudLocal = 0;
    banderaNombreUsuario = pdFALSE;
    banderaClave = pdFALSE;
    banderaWill = pdFALSE;
    
    //envia la cabecera del paquete
    SIM_PutChar(CONNECT*16);
    
    //verifica si hay un nombre de usuario, en cuyo caso habilita la bandera correspondiente
    if(strcmp(nombreUsuario,"")){
        banderaNombreUsuario = pdTRUE; 
    }
     //verifica si hay una contraseña, en cuyo caso habilita la bandera correspondiente
    if(strcmp(clave,"")){
        banderaClave = pdTRUE; 
    }
    //verifica si existe un tópico y mensaje configurados y habilita la bandera correspondiente
    if((strcmp(Topico,""))&&(strcmp(Mensaje,""))){
        banderaWill = pdTRUE;
    }
    
    //calcula la longitud del mensaje en función de las banderas que se encuentren activas
    longitudLocal = (2+strlen(protocolo))+1+3+(2+ strlen(ID_Cliente));
    if(banderaWill){
        longitudLocal += 2+strlen(Topico)+strlen(Mensaje);
    }
    if(banderaNombreUsuario){
        longitudLocal += 2+strlen(nombreUsuario);
        if(banderaClave){
            longitudLocal += 2+strlen(clave);
        }
    }
    //envia la longitud del mensaje
    MQTT_enviarLargo(longitudLocal);
    //envia el protocolo
    MQTT_enviarCadenaUTF(protocolo);
    //envia la version del protocolo
    SIM_PutChar(VERSION_PROTOCOLO);
    //envia la mascara de banderas y mensajes
    SIM_PutChar((banderaNombreUsuario*MASCARA_NOMBRE_USUARIO +
                    banderaClave*MASCARA_CLAVE+
                    mantenimiento*MASCARA_WILL_MANTENIMIENTO+
                    QoS*ESCALA_WILL_QoS+
                    banderaWill*MASCARA_WILL+
                    sesionLimpia*MASCARA_SESION_LIMPIA
                ));
    //envia el tiempo de vida del paquete
    SIM_PutChar(TimeOut / 256);
    SIM_PutChar(TimeOut % 256);
    //envia el identificador de dispositivo o cliente
    MQTT_enviarCadenaUTF(ID_Cliente);
    if(banderaWill){
        //envia el tópico y el mensaje
        MQTT_enviarCadenaUTF(Topico);
        MQTT_enviarCadenaUTF(Mensaje);
    }
    if(banderaNombreUsuario){
        //envia el nombre de usuario y la contraseña
        MQTT_enviarCadenaUTF(nombreUsuario);
        if(banderaClave){
            MQTT_enviarCadenaUTF(clave);
        }
    }
}

/*******************************************************************************
* leer estado de conexion MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion para obtener el estado de la bandera de conexion MQTT
*
*******************************************************************************/
CYBIT MQTT_leerEstado(){
    return MQTT_Conectado;
}

/*******************************************************************************
* publicar topico MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un topico
*
* Parametros:
*   DUP:    Esta bandera se pone en 1 cuando el cliente o el servidor intenta reentregar
*           un mensaje PUBLISH, esto aplica para mensajes donde el valor de Qos es mayor a 0
*   Qos:    Nivel de QoS que el cliente quiere para enviar mensajes
*   Mantenimiento: si la bandera de mantenimiento está en 1, el servidor deberia mantener el
*           mensaje después de que haya sido entregado a todos los suscriptores.
*           Cuando una nueva suscripción es establecida en un topico, el ultimo mensaje retenido
*           en ese topico es enviado al nuevo suscriptor
*   ID_Mensaje: Campo del identificador de mensaje, se usa solo en mensajes donde el nivel
*           de Qos es mayor a 0 (mensaje suscribir tiene Qos = 1)
*   Topico: Nombre del topico en el que se va a publicar
*   Mensaje: Mensaje que será publicado en el topico
*
*******************************************************************************/
void MQTT_publicar(uint8 DUP, uint8 Qos, uint8 Mantenimiento, uint ID_Mensaje, char *Topico, char *Mensaje){
    int longitudLocal = 0;
    
    SIM_PutChar((PUBLISH<<4)+(DUP*MASCARA_DUP)+(Qos*ESCALA_QoS)+Mantenimiento);
    longitudLocal = (2 + strlen(Topico));
    if (Qos > 0){
        longitudLocal += 2;
    }
    longitudLocal += strlen(Mensaje);
    MQTT_enviarLargo(longitudLocal);
    MQTT_enviarCadenaUTF(Topico);
    if (Qos > 0){
        SIM_PutChar(ID_Mensaje >> 8);
        SIM_PutChar(ID_Mensaje % 256);
    }
    SIM_PutString(Mensaje);
}

/*******************************************************************************
* publicar con ACK
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un ACK
*
*******************************************************************************/
void MQTT_publicarACK(uint ID_Mensaje){
    SIM_PutChar(PUBACK<<4);
    MQTT_enviarLargo(2);
    SIM_PutChar(ID_Mensaje >> 8);
    SIM_PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* publicar REC
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un RECIBIDO
*
*******************************************************************************/
void MQTT_publicarREC(uint ID_Mensaje){
    SIM_PutChar(PUBREC<<4);
    MQTT_enviarLargo(2);
    SIM_PutChar(ID_Mensaje >> 8);
    SIM_PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* publicar REL
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un RELEASE (liberar)
*
*******************************************************************************/
void MQTT_publicarREL(uint8 DUP, uint ID_Mensaje){
    SIM_PutChar((PUBREL<<4) + (DUP*MASCARA_DUP) + ESCALA_QoS);
    MQTT_enviarLargo(2);
    SIM_PutChar(ID_Mensaje >> 8);
    SIM_PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* publicar COMP
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un COMPLETO
*
*******************************************************************************/
void MQTT_publicarCOMP(uint ID_Mensaje){
    SIM_PutChar(PUBCOMP<<4);
    MQTT_enviarLargo(2);
    SIM_PutChar(ID_Mensaje >> 8);
    SIM_PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* suscribir a topico MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion para suscribirse a un topico
*
* Parametros:
*   DUP:    Esta bandera se pone en 1 cuando el cliente o el servidor intenta reentregar
*           un mensaje SUBSCRIBE, esto aplica para mensajes donde el valor de Qos es mayor a 0
*   Qos:    Nivel de QoS que el cliente quiere para recibir mensajes
*   ID_Mensaje: Campo del identificador de mensaje, se usa solo en mensajes donde el nivel
*           de Qos es mayor a 0 (mensaje suscribir tiene Qos = 1)
*   Topico: Nombre del topico al que se quiere suscribir
*
*******************************************************************************/
void MQTT_suscribir(uint8 DUP, uint8 Qos, uint ID_Mensaje, char *Topico){
    int longitudLocal = 0;
    
    SIM_PutChar((SUBSCRIBE<<4)+(DUP*MASCARA_DUP)+(ESCALA_QoS));
    longitudLocal = 2 + (2 + strlen(Topico)) + 1;
    MQTT_enviarLargo(longitudLocal);
    SIM_PutChar(ID_Mensaje >> 8);
    SIM_PutChar(ID_Mensaje % 256);
    MQTT_enviarCadenaUTF(Topico);
    SIM_PutChar(Qos);
    //vTaskDelay(pdMS_TO_TICKS(10));
}

/*******************************************************************************
* desvincular de topico MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion para desvincularse de una suscripción a un topico
*
*******************************************************************************/
void MQTT_desvincular(uint8 DUP, uint ID_Mensaje, char *Topico){
    int longitudLocal = 0;
    
    SIM_PutChar((UNSUBSCRIBE<<4)+(DUP*MASCARA_DUP)+(ESCALA_QoS));
    longitudLocal = (2 + strlen(Topico)) + 2;
    MQTT_enviarLargo(longitudLocal);
    SIM_PutChar(ID_Mensaje >> 8);
    SIM_PutChar(ID_Mensaje % 256);
    MQTT_enviarCadenaUTF(Topico);
}

/*******************************************************************************
* desconectar de broker
********************************************************************************
*
* Tarea que realiza:
*  Funcion para desconectarse del broker remoto
*
*******************************************************************************/
void MQTT_desconectar(){
    SIM_PutChar(DISCONNECT<<4);
    MQTT_enviarLargo(0);
    MQTT_Conectado = MQTT_DESCONECTADO;
    return;
}

/*******************************************************************************
* ping al servidor MQTT
********************************************************************************
*
* Tarea que realiza:
*  mantiene viva la conexión con el broker remoto
*
*******************************************************************************/
CYBIT MQTT_ping(){
    uint8 contador = 0;
    uint16 i = 0;
    conexionviva = pdFALSE;
    do{
        SIM_PutChar(PINGREQ*16);
        MQTT_enviarLargo(0);
        vTaskDelay(pdMS_TO_TICKS(2000));
        for(i=0;i<100;i++){
            MQTT_recibir();
            if(conexionviva){
                i=1000;
            }
        }
        contador++;
    }while((!conexionviva)&&(contador<INTENTOS_PING));
    if(!conexionviva){
        MQTT_desconectar();
    }
    return conexionviva;
}
/* [] END OF FILE */
