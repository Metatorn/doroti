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
 * ===================================================
;Archivo cabecera para declaración Colas comunes
====================================================*/
#ifndef COLAS_CONFIG_H
#define COLAS_CONFIG_H

#include "EKIA_CONFIG.h"    
#include "FreeRTOS.h" 
#include "semphr.h"  

/*=====================================================
;Declaración de controladores de Queues
====================================================*/  
xQueueHandle cUSB; //Queue para envio de comandos desde puerto USB       
xQueueHandle sistemaStatus;
xQueueHandle estadoFrio;    
xQueueHandle medidas; 
xQueueHandle peso;
xQueueHandle parametros;
xQueueHandle solicitudDispensar;
xQueueHandle respuestaDispensar;
xQueueHandle solicitudApertura;    
xQueueHandle solicitudPeso;     
xQueueHandle cadena;  
xQueueHandle cadena2;    
xQueueHandle configuracionesTemperatura;    
xQueueHandle sensorPuerta1;
xQueueHandle sensorPuerta2;    
    
void colas_Init();   
    
#endif 

/* [] END OF FILE */
