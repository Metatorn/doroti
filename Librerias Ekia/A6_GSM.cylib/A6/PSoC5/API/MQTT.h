/* ===================================================
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
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del protocolo MQTT
====================================================*/

#ifndef `$INSTANCE_NAME`_MQTT_CONFIG_H
#define `$INSTANCE_NAME`_MQTT_CONFIG_H

#include "cypins.h"     
#include "colas.h" 
#include "stdio.h"
#include "tipos.h"    
#include "Telemetria.h"    
           
// QoS value bit 2 bit 1 Description
//   0       0       0   At most once    Fire and Forget         <=1
//   1       0       1   At least once   Acknowledged delivery   >=1
//   2       1       0   Exactly once    Assured delivery        =1
//   3       1       1   Reserved          
#define `$INSTANCE_NAME`_MQTT_TIEMPO_VIDA                 (PERIODO_PING+900)
#define `$INSTANCE_NAME`_MQTT_INTENTOS_PING               (5)    
    
#define `$INSTANCE_NAME`_MQTT_DESCONECTADO                0
#define `$INSTANCE_NAME`_MQTT_CONECTADO                   1 
    
#define `$INSTANCE_NAME`_MQTT_MASCARA_NOMBRE_USUARIO      (128)
#define `$INSTANCE_NAME`_MQTT_MASCARA_CLAVE               (64)
#define `$INSTANCE_NAME`_MQTT_MASCARA_WILL_MANTENIMIENTO  (32)  
//#define MASCARA_WILL_QoS        (24)  
#define `$INSTANCE_NAME`_MQTT_MASCARA_WILL                (4)    
#define `$INSTANCE_NAME`_MQTT_MASCARA_SESION_LIMPIA       (2)
    
#define `$INSTANCE_NAME`_MQTT_MASCARA_DUP            (8)      //envio duplicado, solo aplica para QoS>0
#define `$INSTANCE_NAME`_MQTT_MASCARA_QoS            (6)      //calidad del servicio    
#define `$INSTANCE_NAME`_MQTT_MASCARA_MANTENIMIENTO  (1)         
    
#define `$INSTANCE_NAME`_MQTT_VERSION_PROTOCOLO      (3) 
#define `$INSTANCE_NAME`_MQTT_ESCALA_WILL_QoS        (8)
#define `$INSTANCE_NAME`_MQTT_ESCALA_QoS             (2)      //(()&QoS)/QoS_Scale
    
#define `$INSTANCE_NAME`_MQTT_LEN_MAX        (300) 
#define `$INSTANCE_NAME`_MQTT_TOPIC_LEN_MAX  (50)
#define `$INSTANCE_NAME`_MQTT_MENS_LEN_MAX   (250)    
#define `$INSTANCE_NAME`_MQTT_NO_ACK         (0xFF)    
    
#define `$INSTANCE_NAME`_MQTT_CONNECT        1       //Solicitud de cliente para conectar a broker       Cliente -> Servidor    
#define `$INSTANCE_NAME`_MQTT_CONNACK        2       //Conexión con ACK                                  Cliente <- Servidor
#define `$INSTANCE_NAME`_MQTT_PUBLISH        3       //Publicar Mensaje                                  Cliente -> Servidor 
#define `$INSTANCE_NAME`_MQTT_PUBACK         4       //Publicar con ACK                                  Cliente <-> Servidor
#define `$INSTANCE_NAME`_MQTT_PUBREC         5       //Publicación recibida (entrega asegurada parte 1)  Cliente <-> Servidor
#define `$INSTANCE_NAME`_MQTT_PUBREL         6       //Liberar Publicación (entrega asegurada parte 2)   Cliente <-> Servidor
#define `$INSTANCE_NAME`_MQTT_PUBCOMP        7       //Publicación completa (entrega asegurada parte 3)  Cliente <-> Servidor
#define `$INSTANCE_NAME`_MQTT_SUBSCRIBE      8       //Solicitud de suscripción                          Cliente     Servidor
#define `$INSTANCE_NAME`_MQTT_SUBACK         9       //Suscribir con ACK                                 Cliente <- Servidor
#define `$INSTANCE_NAME`_MQTT_UNSUBSCRIBE    10      //Solicitud para darse de baja de una suscripcion   Cliente 
#define `$INSTANCE_NAME`_MQTT_UNSUBACK       11      //Desuscribirse con ACK                             Cliente <- Servidor
#define `$INSTANCE_NAME`_MQTT_PINGREQ        12      //Solicitud de PING                                 Cliente -> Servidor     
#define `$INSTANCE_NAME`_MQTT_PINGRESP       13      //Respuesta de PING                                 Cliente    Servidor        
#define `$INSTANCE_NAME`_MQTT_DISCONNECT     14      //Cliente desconectandose                           Cliente -> Servidor   
/*=====================================================
;Funciones del protocolo MQTT
====================================================*/

void `$INSTANCE_NAME`_MQTT_recibir();    
void `$INSTANCE_NAME`_MQTT_enviarLargo(int longitud);
void `$INSTANCE_NAME`_MQTT_enviarCadenaUTF(char *string);
void `$INSTANCE_NAME`_MQTT_conectarBroker(char* protocolo, char* ID_Cliente, char* nombreUsuario, char* clave, uint8 sesionLimpia, uint8 QoS, uint8 mantenimiento, char *Topico, char *Mensaje);
void `$INSTANCE_NAME`_MQTT_desconectar();
CYBIT `$INSTANCE_NAME`_MQTT_ping();
int `$INSTANCE_NAME`_MQTT_generarIDMensaje();
CYBIT `$INSTANCE_NAME`_MQTT_leerEstado();
void `$INSTANCE_NAME`_MQTT_publicar(uint8 DUP, uint8 Qos, uint8 Mantenimiento, uint ID_Mensaje, char *Topico, char *Mensaje);
void `$INSTANCE_NAME`_MQTT_publicarACK(uint ID_Mensaje);
void `$INSTANCE_NAME`_MQTT_publicarREC(uint ID_Mensaje);
void `$INSTANCE_NAME`_MQTT_publicarREL(uint8 DUP, uint ID_Mensaje);
void `$INSTANCE_NAME`_MQTT_publicarCOMP(uint ID_Mensaje);
void `$INSTANCE_NAME`_MQTT_suscribir(uint8 DUP, uint8 Qos, uint ID_Mensaje, char *Topico);
void `$INSTANCE_NAME`_MQTT_desvincular(uint8 DUP, uint ID_Mensaje, char *Topico);

#endif  

/* [] END OF FILE */
