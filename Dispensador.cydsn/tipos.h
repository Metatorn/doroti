/* ===================================================
 *
 * Copyright EKIA Technology S.A.S., 2020
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de constantes del dispensador
====================================================*/
#ifndef TIPOS_CONFIG_H
#define TIPOS_CONFIG_H

#include "EKIA_CONFIG.h"    
#include "cypins.h"  
#include "LED.h"
    
#define LED_ON              LED_Write(0);    
#define LED_OFF             LED_Write(1);        
    
#define DISPENSADOR_NO_LISTO    (100)
#define DISPENSADOR_ESPERA      (101) 
    
#define EMERGENCIA              (2)      
    
/*=====================================================
;Estados del sistema para comunicación
====================================================*/    
#define DESCONECTADO        (10)
#define CONECTADO           (11)
#define ESPERA              (12) 
#define DESHABILITADO       (13)    
#define RX_BUF_USB_LENGHT   (64)   
#define TX_BUF_USB_LENGHT   (64)    
       
/*=====================================================
;Prioridades de las tareas
====================================================*/ 
#define priorMonitor            (tskIDLE_PRIORITY + 4)    
#define priorInterpretador      (tskIDLE_PRIORITY + 5) 
#define priorMediciones         (tskIDLE_PRIORITY + 3)
#define priorControl            (tskIDLE_PRIORITY + 4) 
#define priorUSB                (tskIDLE_PRIORITY + 2)  
#define priorEstadoUSB          (tskIDLE_PRIORITY + 2)     
    
/*=====================================================
;Memoria de las tareas
====================================================*/    
//#define memLED              (configMINIMAL_STACK_SIZE)
#define memInterpretador        (1000)  
#define memMediciones           (1000) 
#define memMonitor              (1000)   
#define memControl              (1000)
#define memUSB                  (800)  
#define memEstadoUSB            (500)    
    
/*=====================================================
;Variables y estructuras comunes
====================================================*/  
    
#define SET_CERO_PESO               (10)   

#define T_MOVER_MOTOR           (1500)
#define T_MOVIMIENTO            (4300-T_MOVER_MOTOR)
#define T_ESPERA_CAIDA          (80) //tiempo para detectar la caida de un objeto  
#define NUM_INTENTOS            (1)     
   
CY_PACKED typedef struct {
    float temperatura;
    float humedad;
}CY_PACKED_ATTR xAmbiente;

typedef struct {
    xAmbiente ambiente1;
    xAmbiente ambiente2;
    xAmbiente ambienteProm;
    float peso;
    float distancia[SENSORES_ULTRASONIDO];
    float voltaje;
    float corriente;
}xMedidas;

typedef struct {
    int tiempoMovimientoMax;
    int tiempoBandeja[NUMERO_BANDEJAS];
    int cicloUtil;
    float pesoMinimo;
    int escala;
}xConfiguraciones;
    
typedef struct {
    int dato1;
    int dato2;
    int dato3;
    int dato4;
    int dato5;
    int dato6;
    float dato7;
    char dato8[60];
    uint8 dato9;
}xcadenas;  

typedef struct {
    CYBIT finCadena;
    char entrada[RX_BUF_USB_LENGHT];
}xbufferEntradaUSB;

typedef struct {
    uint8 comando;
    int opcional1;
    int opcional2;
}xcomandoUSB;

typedef struct {
    CYBIT activado; 
    float gradosC;
    float HisteresisSuper;
    float HisteresisInfer;
}xConfiguracionTemperatura; 


#endif
/* [] FIN DE ARCHIVO */
