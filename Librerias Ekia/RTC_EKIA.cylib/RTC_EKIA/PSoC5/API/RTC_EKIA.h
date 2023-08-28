/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * RTC_EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones propias
;de EKIA que complementan el componente RTC de Cypress
;archivo basado en la libreria RelativetoFixed.h
====================================================*/
#ifndef `$INSTANCE_NAME`_CONFIG_H
#define `$INSTANCE_NAME`_CONFIG_H

#include "CyLib.h"
#include "`$INSTANCE_NAME`_RTC.h"    

/* Definiciones de mes */
#define `$INSTANCE_NAME`_ENERO           (1u)
#define `$INSTANCE_NAME`_FEBRERO         (2u)
#define `$INSTANCE_NAME`_MARZO           (3u)
#define `$INSTANCE_NAME`_ABRIL           (4u)
#define `$INSTANCE_NAME`_MAYO            (5u)
#define `$INSTANCE_NAME`_JUNIO           (6u)
#define `$INSTANCE_NAME`_JULIO           (7u)
#define `$INSTANCE_NAME`_AGOSTO          (8u)
#define `$INSTANCE_NAME`_SEPTIEMBRE      (9u)
#define `$INSTANCE_NAME`_OCTUBRE         (10u)
#define `$INSTANCE_NAME`_NOVIEMBRE       (11u)    
#define `$INSTANCE_NAME`_DICIEMBRE       (12u)

/* Definiciones de numero de dias en el mes */
#define `$INSTANCE_NAME`_DIAS_EN_ENERO           (31u)
#define `$INSTANCE_NAME`_DIAS_EN_FEBRERO         (28u)
#define `$INSTANCE_NAME`_DIAS_EN_MARZO           (31u)
#define `$INSTANCE_NAME`_DIAS_EN_ABRIL           (30u)
#define `$INSTANCE_NAME`_DIAS_EN_MAYO            (31u)
#define `$INSTANCE_NAME`_DIAS_EN_JUNIO           (30u)
#define `$INSTANCE_NAME`_DIAS_EN_JULIO           (31u)
#define `$INSTANCE_NAME`_DIAS_EN_AGOSTO          (31u)
#define `$INSTANCE_NAME`_DIAS_EN_SEPTIEMBRE      (30u)
#define `$INSTANCE_NAME`_DIAS_EN_OCTUBRE         (31u)
#define `$INSTANCE_NAME`_DIAS_EN_NOVIEMBRE       (30u)
#define `$INSTANCE_NAME`_DIAS_EN_DICIEMBRE       (31u)    
    
#define `$INSTANCE_NAME`_MESES_POR_ANO           (12u)

uint32 `$INSTANCE_NAME`_bisiesto(uint32 year); 
uint8 `$INSTANCE_NAME`_DiasEnMes(uint32 month, uint32 year);
`$INSTANCE_NAME``[RTC]`TIME_DATE `$INSTANCE_NAME`_fechaRelativa(`$INSTANCE_NAME``[RTC]`TIME_DATE fecha, int dias, int mes, int year);
    
#endif    
/* [] END OF FILE */
