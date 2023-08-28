/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Archivo cabecera para declaración de Semaforos de FreeRTOS 
====================================================*/
#ifndef SEMAFOROS_CONFIG_H
#define SEMAFOROS_CONFIG_H

#include "EKIA_CONFIG.h"    
#include "FreeRTOS.h" 
#include "semphr.h"    
#include "queue.h"
#include "task.h"
#include "cypins.h"    
#include "FS.h"    

/*=====================================================
;Declaración de controladores de semaforos
====================================================*/

xSemaphoreHandle memoriaOcupada;
xSemaphoreHandle telemetriaOcupada;
xSemaphoreHandle eepromOcupada;
xSemaphoreHandle inicializando;
xSemaphoreHandle busOcupado;
xSemaphoreHandle operacionOcupada;
xSemaphoreHandle horasMaquina;
xSemaphoreHandle telemetriaOcupada;

void semaforos_Init();
    
#endif
/* [] FIN DE ARCHIVO */
