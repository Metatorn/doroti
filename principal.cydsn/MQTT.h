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

#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "cypins.h"     
#include "colas.h" 
#include "stdio.h"
#include "Telemetria.h"    
           
// QoS value bit 2 bit 1 Description
//   0       0       0   At most once    Fire and Forget         <=1
//   1       0       1   At least once   Acknowledged delivery   >=1
//   2       1       0   Exactly once    Assured delivery        =1
//   3       1       1   Reserved          
#define TIEMPO_VIDA                 (PERIODO_PING+900)
#define INTENTOS_PING               (5)    
    
#define MQTT_DESCONECTADO           0
#define MQTT_CONECTADO              1 
    
#define MASCARA_NOMBRE_USUARIO      (128)
#define MASCARA_CLAVE               (64)
#define MASCARA_WILL_MANTENIMIENTO  (32)  
//#define MASCARA_WILL_QoS        (24)  
#define MASCARA_WILL                (4)    
#define MASCARA_SESION_LIMPIA       (2)
    
#define MASCARA_DUP                 (8)      //envio duplicado, solo aplica para QoS>0
#define MASCARA_QoS                 (6)      //calidad del servicio    
#define MASCARA_MANTENIMIENTO       (1)         
    
#define VERSION_PROTOCOLO           (3) 
#define ESCALA_WILL_QoS             (8)
#define ESCALA_QoS                  (2)      //(()&QoS)/QoS_Scale
    
#define MQTT_LEN_MAX                (300) 
#define MQTT_TOPIC_LEN_MAX          (50)
#define MQTT_MENS_LEN_MAX           (250)    
#define NO_ACK                      (0xFF)    
    
#define CONNECT                     1       //Solicitud de cliente para conectar a broker       Cliente -> Servidor    
#define CONNACK                     2       //Conexión con ACK                                  Cliente <- Servidor
#define PUBLISH                     3       //Publicar Mensaje                                  Cliente -> Servidor 
#define PUBACK                      4       //Publicar con ACK                                  Cliente <-> Servidor
#define PUBREC                      5       //Publicación recibida (entrega asegurada parte 1)  Cliente <-> Servidor
#define PUBREL                      6       //Liberar Publicación (entrega asegurada parte 2)   Cliente <-> Servidor
#define PUBCOMP                     7       //Publicación completa (entrega asegurada parte 3)  Cliente <-> Servidor
#define SUBSCRIBE                   8       //Solicitud de suscripción                          Cliente     Servidor
#define SUBACK                      9       //Suscribir con ACK                                 Cliente <- Servidor
#define UNSUBSCRIBE                 10      //Solicitud para darse de baja de una suscripcion   Cliente 
#define UNSUBACK                    11      //Desuscribirse con ACK                             Cliente <- Servidor
#define PINGREQ                     12      //Solicitud de PING                                 Cliente -> Servidor     
#define PINGRESP                    13      //Respuesta de PING                                 Cliente    Servidor        
#define DISCONNECT                  14      //Cliente desconectandose                           Cliente -> Servidor   
/*=====================================================
;Funciones del protocolo MQTT
====================================================*/

void MQTT_recibir();    
void MQTT_enviarLargo(int longitud);
void MQTT_enviarCadenaUTF(char *string);
void MQTT_conectarBroker(char* protocolo, char* ID_Cliente, char* nombreUsuario, char* clave, uint8 sesionLimpia, uint8 QoS, uint8 mantenimiento, char *Topico, char *Mensaje);
void MQTT_desconectar();
CYBIT MQTT_ping();
int MQTT_generarIDMensaje();
CYBIT MQTT_leerEstado();
void MQTT_publicar(uint8 DUP, uint8 Qos, uint8 Mantenimiento, uint ID_Mensaje, char *Topico, char *Mensaje);
void MQTT_publicarACK(uint ID_Mensaje);
void MQTT_publicarREC(uint ID_Mensaje);
void MQTT_publicarREL(uint8 DUP, uint ID_Mensaje);
void MQTT_publicarCOMP(uint ID_Mensaje);
void MQTT_suscribir(uint8 DUP, uint8 Qos, uint ID_Mensaje, char *Topico);
void MQTT_desvincular(uint8 DUP, uint ID_Mensaje, char *Topico);

#endif  

/* [] END OF FILE */
