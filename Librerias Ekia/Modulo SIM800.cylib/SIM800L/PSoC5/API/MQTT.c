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
;FUNCIONES PROPIAS DEL PROTOCOLO `$INSTANCE_NAME`_MQTT
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
#include "`$INSTANCE_NAME`_MQTT.h"
#include "Telemetria.h"
#include "tipos.h"

//#include "LED_DEBUG.h"

CYBIT banderaNombreUsuario = pdFALSE;
CYBIT banderaClave = pdFALSE;
CYBIT banderaWill = pdFALSE;
uint8 `$INSTANCE_NAME`_MQTT_Conectado = `$INSTANCE_NAME`_MQTT_DESCONECTADO;
long TimeOut = `$INSTANCE_NAME`_MQTT_TIEMPO_VIDA;   //tiempo de desconexion de 3 minutos
int conexionConACK = `$INSTANCE_NAME`_MQTT_NO_ACK;

uint8 `$INSTANCE_NAME`_MQTT_bufferEntrada[`$INSTANCE_NAME`_MQTT_LEN_MAX];
uint8 `$INSTANCE_NAME`_MQTT_topico[`$INSTANCE_NAME`_MQTT_TOPIC_LEN_MAX];
uint8 `$INSTANCE_NAME`_MQTT_Mensaje[`$INSTANCE_NAME`_MQTT_MENS_LEN_MAX];

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
static void `$INSTANCE_NAME`_MQTT_limpiarBuffer(){
    uint i;
    for(i=0; i<(sizeof `$INSTANCE_NAME`_MQTT_bufferEntrada); i++){
        `$INSTANCE_NAME`_MQTT_bufferEntrada[i]=0;
    }
    for(i=0; i<(sizeof `$INSTANCE_NAME`_MQTT_topico); i++){
        `$INSTANCE_NAME`_MQTT_topico[i]=0;
    }
    for(i=0; i<(sizeof `$INSTANCE_NAME`_MQTT_Mensaje); i++){
        `$INSTANCE_NAME`_MQTT_Mensaje[i]=0;
    }
}

/*******************************************************************************
* recibir datos `$INSTANCE_NAME`_MQTT
********************************************************************************
*
* Tarea que realiza:
*  recibe e interpreta datos con el protocolo `$INSTANCE_NAME`_MQTT
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_recibir(){
    CYBIT siguienteByteLongitud = pdTRUE, fin = pdFALSE;
    uint8 byteEntrada, byteCabecera;
    uint8 tipoMensajeRecibido, /*DUP,*/ QoS, mantenimiento;
    uint32 `$INSTANCE_NAME`_MQTT_Largo = 0, multiplicador = 1, `$INSTANCE_NAME`_MQTT_LargoTemp = 0;
    uint32 indice = 0, i = 0;
    uint32 InicioMensaje = 0;
    int indicePublicar = 0;
    int ID_Mensaje = 0;
    int contador = 0;
    
    vTaskDelay(pdMS_TO_TICKS(50));
    byteCabecera = `$INSTANCE_NAME``[SIM]`GetChar();
    tipoMensajeRecibido = (byteCabecera/16)&0x0F;
    /*DUP = (byteCabecera & MASCARA_DUP)/MASCARA_DUP;*/
    QoS = (byteCabecera & `$INSTANCE_NAME`_MQTT_MASCARA_QoS)/`$INSTANCE_NAME`_MQTT_ESCALA_QoS;
    mantenimiento = byteCabecera & `$INSTANCE_NAME`_MQTT_MASCARA_MANTENIMIENTO;
    if((tipoMensajeRecibido >= `$INSTANCE_NAME`_MQTT_CONNECT)&&(tipoMensajeRecibido <= `$INSTANCE_NAME`_MQTT_DISCONNECT)){
        `$INSTANCE_NAME`_MQTT_Largo = 0;
        vTaskDelay(pdMS_TO_TICKS(5));
        while(siguienteByteLongitud && !fin){
            if(`$INSTANCE_NAME``[SIM]`GetRxBufferSize()>0){
                byteEntrada = `$INSTANCE_NAME``[SIM]`GetChar();
                if(((byteCabecera == 'C') && (byteEntrada == 'L') && (`$INSTANCE_NAME`_MQTT_Largo==0))
                    ||((byteCabecera == '+') && (byteEntrada == 'P') && (`$INSTANCE_NAME`_MQTT_Largo==0))
                ){
                    fin = pdTRUE;
                    indice = 0;
                    `$INSTANCE_NAME`_MQTT_bufferEntrada[indice] = byteCabecera;
                    indice++;
                    `$INSTANCE_NAME`_MQTT_bufferEntrada[indice] = byteEntrada; 
                    indice++;
                    siguienteByteLongitud = pdFALSE;
                    `$INSTANCE_NAME`_MQTT_Conectado = `$INSTANCE_NAME`_MQTT_DESCONECTADO;
                }
                else{
                    if((byteEntrada & 128) != 128){
                        siguienteByteLongitud = pdFALSE;
                    }
                    `$INSTANCE_NAME`_MQTT_Largo += (byteEntrada & 127) * multiplicador;
                    multiplicador *= 128;
                }
            }
            else{
                if(contador>=(60000)){
                    fin = pdTRUE;
                    `$INSTANCE_NAME`_MQTT_Conectado = `$INSTANCE_NAME`_MQTT_DESCONECTADO;
                    break;
                }
                contador++;
                CyDelay(1);
            }
        }
        
        if(!fin){
            indice = 0;
            `$INSTANCE_NAME`_MQTT_LargoTemp = `$INSTANCE_NAME`_MQTT_Largo;
            while((`$INSTANCE_NAME`_MQTT_LargoTemp > 0) && ((`$INSTANCE_NAME``[SIM]`GetRxBufferSize() > 0))){
                `$INSTANCE_NAME`_MQTT_bufferEntrada[indice]=`$INSTANCE_NAME``[SIM]`GetChar();
                `$INSTANCE_NAME`_MQTT_LargoTemp--;
                vTaskDelay(pdMS_TO_TICKS(2));
                indice++;
            }
            switch(tipoMensajeRecibido){
                case `$INSTANCE_NAME`_MQTT_CONNACK:{
                    conexionConACK = (`$INSTANCE_NAME`_MQTT_bufferEntrada[0]<<8)|`$INSTANCE_NAME`_MQTT_bufferEntrada[1];
                    if(conexionConACK == 0){
                        `$INSTANCE_NAME`_MQTT_Conectado = `$INSTANCE_NAME`_MQTT_CONECTADO;
                        `$INSTANCE_NAME`_MQTT_enConexion();
                    }
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PUBLISH:{
                    Longitud_Topico = (`$INSTANCE_NAME`_MQTT_bufferEntrada[0]<<8)|`$INSTANCE_NAME`_MQTT_bufferEntrada[1];
                    indicePublicar = 0;
                    for(i=2; i < (Longitud_Topico + 2); i++){
                        `$INSTANCE_NAME`_MQTT_topico[indicePublicar] = `$INSTANCE_NAME`_MQTT_bufferEntrada[i];
                        indicePublicar++;
                    }
                    `$INSTANCE_NAME`_MQTT_topico[indicePublicar] = 0;
                    Longitud_Topico = indicePublicar;
                    
                    indicePublicar = 0;
                    InicioMensaje = Longitud_Topico + 2UL;
                    ID_Mensaje = 0;
                    
                    if(QoS !=0){
                        InicioMensaje +=2;
                        ID_Mensaje = (`$INSTANCE_NAME`_MQTT_bufferEntrada[Longitud_Topico + 2UL]<<8) | `$INSTANCE_NAME`_MQTT_bufferEntrada[Longitud_Topico + 3UL];
                    }
                    for(i=InicioMensaje; i<`$INSTANCE_NAME`_MQTT_Largo; i++){
                        `$INSTANCE_NAME`_MQTT_Mensaje[indicePublicar] = `$INSTANCE_NAME`_MQTT_bufferEntrada[i];
                        indicePublicar++;
                    }
                    `$INSTANCE_NAME`_MQTT_Mensaje[indicePublicar] = 0;
                    Longitud_Mensaje = indicePublicar;
                    
                    if(QoS == 1){
                        `$INSTANCE_NAME`_MQTT_publicarACK(ID_Mensaje);
                    }
                    else if(QoS == 2){
                        `$INSTANCE_NAME`_MQTT_publicarREC(ID_Mensaje);
                    }
                    `$INSTANCE_NAME`_MQTT_enMensaje((char*)`$INSTANCE_NAME`_MQTT_topico, Longitud_Topico, (char*)`$INSTANCE_NAME`_MQTT_Mensaje, Longitud_Mensaje);
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PUBREL:{    
                    `$INSTANCE_NAME`_MQTT_publicarCOMP((`$INSTANCE_NAME`_MQTT_bufferEntrada[0]<<8)|`$INSTANCE_NAME`_MQTT_bufferEntrada[1]);
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PUBREC:{ 
                    `$INSTANCE_NAME`_MQTT_publicarREL(0,(`$INSTANCE_NAME`_MQTT_bufferEntrada[0]<<8)|`$INSTANCE_NAME`_MQTT_bufferEntrada[1]);
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PUBACK:{
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PUBCOMP:{
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_SUBACK:{
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PINGREQ:{
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_PINGRESP:{
                    conexionviva = pdTRUE;
                    break;
                }
                case `$INSTANCE_NAME`_MQTT_DISCONNECT:{
                    break;
                }
            }
        }
        `$INSTANCE_NAME`_MQTT_limpiarBuffer();
    }
}

/*******************************************************************************
* enviar largo de cadena
********************************************************************************
*
* Tarea que realiza:
*  Funcion que convierte el largo del mensaje al formato requerido por el protocolo
*  `$INSTANCE_NAME`_MQTT a la hora de enviar un mensaje
*
* Parametros:
*   longitud: Corresponde a la longitud calculada del mensaje `$INSTANCE_NAME`_MQTT
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_enviarLargo(int longitud){
    CYBIT  banderaLargo = pdFALSE;
    while (!banderaLargo){
        if ((longitud / 128) > 0)
        {
            `$INSTANCE_NAME``[SIM]`PutChar(((longitud % 128) + 128));
            longitud /= 128;
        }
        else
        {
            banderaLargo = pdTRUE;
            `$INSTANCE_NAME``[SIM]`PutChar(longitud);
        }
    }
}

/*******************************************************************************
* enviar cadena formateada
********************************************************************************
*
* Tarea que realiza:
*  Funcion que envia la longitud de una cadena junto con el mensaje en el formato
*  requerido por el protocolo `$INSTANCE_NAME`_MQTT
*
* Parametros:
*   cadena: Corresponde al mensaje que desea enviarse
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(char *string){
  int longitudLocal = strlen(string);
  `$INSTANCE_NAME``[SIM]`PutChar(longitudLocal / 256);
  `$INSTANCE_NAME``[SIM]`PutChar(longitudLocal % 256);
  `$INSTANCE_NAME``[SIM]`PutString(string);
}

/*******************************************************************************
* generar identificador de mensaje
********************************************************************************
*
* Tarea que realiza:
*  Funcion que genera el identificador de mensaje necesario para algunas funciones
*
*******************************************************************************/
int `$INSTANCE_NAME`_MQTT_generarIDMensaje(){
    if (identificadorMensaje < 65535){
        identificadorMensaje++;
    }
    else{
        identificadorMensaje = 0;
    }
    return identificadorMensaje;
}

/*******************************************************************************
* conectar a un broker remoto utilizando el protocolo `$INSTANCE_NAME`_MQTT
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
void `$INSTANCE_NAME`_MQTT_conectarBroker(char* protocolo, char* ID_Cliente, char* nombreUsuario, char* clave, uint8 sesionLimpia, uint8 QoS, uint8 mantenimiento, char *Topico, char *Mensaje){
    int longitudLocal = 0;
    banderaNombreUsuario = pdFALSE;
    banderaClave = pdFALSE;
    banderaWill = pdFALSE;
    
    //envia la cabecera del paquete
    `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_CONNECT*16);
    
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
    `$INSTANCE_NAME`_MQTT_enviarLargo(longitudLocal);
    //envia el protocolo
    `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(protocolo);
    //envia la version del protocolo
    `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_VERSION_PROTOCOLO);
    //envia la mascara de banderas y mensajes
    `$INSTANCE_NAME``[SIM]`PutChar((banderaNombreUsuario*`$INSTANCE_NAME`_MQTT_MASCARA_NOMBRE_USUARIO +
                    banderaClave*`$INSTANCE_NAME`_MQTT_MASCARA_CLAVE+
                    mantenimiento*`$INSTANCE_NAME`_MQTT_MASCARA_WILL_MANTENIMIENTO+
                    QoS*`$INSTANCE_NAME`_MQTT_ESCALA_WILL_QoS+
                    banderaWill*`$INSTANCE_NAME`_MQTT_MASCARA_WILL+
                    sesionLimpia*`$INSTANCE_NAME`_MQTT_MASCARA_SESION_LIMPIA
                ));
    //envia el tiempo de vida del paquete
    `$INSTANCE_NAME``[SIM]`PutChar(TimeOut / 256);
    `$INSTANCE_NAME``[SIM]`PutChar(TimeOut % 256);
    //envia el identificador de dispositivo o cliente
    `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(ID_Cliente);
    if(banderaWill){
        //envia el tópico y el mensaje
        `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(Topico);
        `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(Mensaje);
    }
    if(banderaNombreUsuario){
        //envia el nombre de usuario y la contraseña
        `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(nombreUsuario);
        if(banderaClave){
            `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(clave);
        }
    }
}

/*******************************************************************************
* leer estado de conexion `$INSTANCE_NAME`_MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion para obtener el estado de la bandera de conexion `$INSTANCE_NAME`_MQTT
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_MQTT_leerEstado(){
    return `$INSTANCE_NAME`_MQTT_Conectado;
}

/*******************************************************************************
* publicar topico `$INSTANCE_NAME`_MQTT
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
void `$INSTANCE_NAME`_MQTT_publicar(uint8 DUP, uint8 Qos, uint8 Mantenimiento, uint ID_Mensaje, char *Topico, char *Mensaje){
    int longitudLocal = 0;
    
    `$INSTANCE_NAME``[SIM]`PutChar((`$INSTANCE_NAME`_MQTT_PUBLISH<<4)+(DUP*`$INSTANCE_NAME`_MQTT_MASCARA_DUP)+(Qos*`$INSTANCE_NAME`_MQTT_ESCALA_QoS)+Mantenimiento);
    longitudLocal = (2 + strlen(Topico));
    if (Qos > 0){
        longitudLocal += 2;
    }
    longitudLocal += strlen(Mensaje);
    `$INSTANCE_NAME`_MQTT_enviarLargo(longitudLocal);
    `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(Topico);
    if (Qos > 0){
        `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
        `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
    }
    `$INSTANCE_NAME``[SIM]`PutString(Mensaje);
}

/*******************************************************************************
* publicar con ACK
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un ACK
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_publicarACK(uint ID_Mensaje){
    `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_PUBACK<<4);
    `$INSTANCE_NAME`_MQTT_enviarLargo(2);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* publicar REC
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un RECIBIDO
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_publicarREC(uint ID_Mensaje){
    `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_PUBREC<<4);
    `$INSTANCE_NAME`_MQTT_enviarLargo(2);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* publicar REL
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un RELEASE (liberar)
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_publicarREL(uint8 DUP, uint ID_Mensaje){
    `$INSTANCE_NAME``[SIM]`PutChar((`$INSTANCE_NAME`_MQTT_PUBREL<<4) + (DUP*`$INSTANCE_NAME`_MQTT_MASCARA_DUP) + `$INSTANCE_NAME`_MQTT_ESCALA_QoS);
    `$INSTANCE_NAME`_MQTT_enviarLargo(2);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* publicar COMP
********************************************************************************
*
* Tarea que realiza:
*  Funcion para publicar un COMPLETO
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_publicarCOMP(uint ID_Mensaje){
    `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_PUBCOMP<<4);
    `$INSTANCE_NAME`_MQTT_enviarLargo(2);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
}

/*******************************************************************************
* suscribir a topico `$INSTANCE_NAME`_MQTT
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
void `$INSTANCE_NAME`_MQTT_suscribir(uint8 DUP, uint8 Qos, uint ID_Mensaje, char *Topico){
    int longitudLocal = 0;
    
    `$INSTANCE_NAME``[SIM]`PutChar((`$INSTANCE_NAME`_MQTT_SUBSCRIBE<<4)+(DUP*`$INSTANCE_NAME`_MQTT_MASCARA_DUP)+(`$INSTANCE_NAME`_MQTT_ESCALA_QoS));
    longitudLocal = 2 + (2 + strlen(Topico)) + 1;
    `$INSTANCE_NAME`_MQTT_enviarLargo(longitudLocal);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
    `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(Topico);
    `$INSTANCE_NAME``[SIM]`PutChar(Qos);
    //vTaskDelay(pdMS_TO_TICKS(10));
}

/*******************************************************************************
* desvincular de topico `$INSTANCE_NAME`_MQTT
********************************************************************************
*
* Tarea que realiza:
*  Funcion para desvincularse de una suscripción a un topico
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_desvincular(uint8 DUP, uint ID_Mensaje, char *Topico){
    int longitudLocal = 0;
    
    `$INSTANCE_NAME``[SIM]`PutChar((`$INSTANCE_NAME`_MQTT_UNSUBSCRIBE<<4)+(DUP*`$INSTANCE_NAME`_MQTT_MASCARA_DUP)+(`$INSTANCE_NAME`_MQTT_ESCALA_QoS));
    longitudLocal = (2 + strlen(Topico)) + 2;
    `$INSTANCE_NAME`_MQTT_enviarLargo(longitudLocal);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje >> 8);
    `$INSTANCE_NAME``[SIM]`PutChar(ID_Mensaje % 256);
    `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(Topico);
}

/*******************************************************************************
* desconectar de broker
********************************************************************************
*
* Tarea que realiza:
*  Funcion para desconectarse del broker remoto
*
*******************************************************************************/
void `$INSTANCE_NAME`_MQTT_desconectar(){
    `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_DISCONNECT<<4);
    `$INSTANCE_NAME`_MQTT_enviarLargo(0);
    `$INSTANCE_NAME`_MQTT_Conectado = `$INSTANCE_NAME`_MQTT_DESCONECTADO;
    return;
}

/*******************************************************************************
* ping al servidor `$INSTANCE_NAME`_MQTT
********************************************************************************
*
* Tarea que realiza:
*  mantiene viva la conexión con el broker remoto
*
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_MQTT_ping(){
    uint8 contador = 0;
    uint16 i = 0;
    conexionviva = pdFALSE;
    do{
        `$INSTANCE_NAME``[SIM]`PutChar(`$INSTANCE_NAME`_MQTT_PINGREQ*16);
        `$INSTANCE_NAME`_MQTT_enviarLargo(0);
        vTaskDelay(pdMS_TO_TICKS(2000));
        for(i=0;i<100;i++){
            `$INSTANCE_NAME`_MQTT_recibir();
            if(conexionviva){
                i=1000;
            }
        }
        contador++;
    }while((!conexionviva)&&(contador<`$INSTANCE_NAME`_MQTT_INTENTOS_PING));
    if(!conexionviva){
        `$INSTANCE_NAME`_MQTT_desconectar();
    }
    return conexionviva;
}
/* [] END OF FILE */

