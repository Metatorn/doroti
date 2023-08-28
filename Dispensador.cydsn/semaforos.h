/* ===================================================
 *
 * Copyright EKIA Technology S.A.S., 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Archivo cabecera para declaración Semaforos comunes
====================================================*/
#ifndef SEMAFORO_CONFIG_H
#define SEMAFORO_CONFIG_H

#include "EKIA_CONFIG.h"    
#include "FreeRTOS.h" 
#include "semphr.h"  

/*=====================================================
;Declaración de controladores de semaforos
====================================================*/
xSemaphoreHandle pesoOcupado;
xSemaphoreHandle pruebaDesactivada;    
    
void semaforos_Init();    
    
#endif 

/* [] END OF FILE */
