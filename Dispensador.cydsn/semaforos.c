/* ===================================================
 *
 * Copyright EKIA Technology S.A.S., 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * AGUATERO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA PARA LA CREACION DE LOS SEMAFOROS UTILIZADOS 
====================================================*/
#include "FreeRTOS.h"
#include "semphr.h" 
#include "cypins.h"

#include "semaforos.h"


/*Esta Funcion crea los semaforos necesarios*/

void semaforos_Init(){
    pesoOcupado = xSemaphoreCreateMutex();
    pruebaDesactivada = xSemaphoreCreateMutex();
}

/* [] END OF FILE */
