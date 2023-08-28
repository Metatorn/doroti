/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA PARA LA CREACION DE LOS SEMAFOROS UTILIZADOS 
====================================================*/
#include "FreeRTOS.h"
#include "semphr.h" 
#include "queue.h"
#include "semaforos.h"
#include "cypins.h"

#include "MDB_Tipos.h"
#include "MDB_monedero.h"
#include "MDB_billetero.h"

/*Esta Funcion genera las colas necesarias de comunicación entre tareas
y para el intercambio de informaciónC*/

void semaforos_Init(){
    memoriaOcupada = xSemaphoreCreateMutex();
    telemetriaOcupada = xSemaphoreCreateMutex();
    //rellenoMonedas = xSemaphoreCreateMutex();
    eepromOcupada = xSemaphoreCreateMutex();
    inicializando = xSemaphoreCreateMutex();
    busOcupado = xSemaphoreCreateMutex();
    operacionOcupada = xSemaphoreCreateMutex();
    horasMaquina = xSemaphoreCreateBinary();
    telemetriaOcupada = xSemaphoreCreateMutex();
}
/* [] FIN DEL ARCHIVO */
