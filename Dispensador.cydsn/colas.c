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
;PROGRAMA PARA LA CREACION DE LAS COLAS UTILIZADAS 
====================================================*/
#include "FreeRTOS.h"
#include "queue.h" 
#include "cypins.h"

#include "colas.h"
#include "tipos.h"

void colas_Init(){
    cUSB = xQueueCreate(10, sizeof(int));
    sistemaStatus = xQueueCreate(1, sizeof(int));
    estadoFrio  = xQueueCreate(1,sizeof(uint8));
    medidas = xQueueCreate(1,sizeof(xMedidas));
    peso = xQueueCreate(1,sizeof(float));
    parametros = xQueueCreate(1,sizeof(xConfiguraciones));
    solicitudDispensar = xQueueCreate(1,sizeof(uint8));
    respuestaDispensar = xQueueCreate(1,sizeof(uint8));
    solicitudApertura = xQueueCreate(1,sizeof(uint8));
    solicitudPeso = xQueueCreate(1,sizeof(uint8));
    cadena = xQueueCreate(1, sizeof(xcadenas));
    cadena2 = xQueueCreate(10, sizeof(xcadenas));
    configuracionesTemperatura = xQueueCreate(1,sizeof(xConfiguracionTemperatura));
    xQueueReset(configuracionesTemperatura);
    sensorPuerta1 = xQueueCreate(1, sizeof(uint8));
    sensorPuerta2 = xQueueCreate(1, sizeof(uint8));
}
