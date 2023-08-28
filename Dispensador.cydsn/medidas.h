/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2020
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
;Archivo cabecera para declaración de la tarea de sensado
====================================================*/

#ifndef MEDIDAS_CONFIG_H
#define MEDIDAS_CONFIG_H

#include "cypins.h"     
#include "tipos.h" 
    
#define MUESTRAS_TEMP       (5)
#define PERIODO             (2000)
#define COMPARA             (PERIODO/2)     
    
void mediciones_Init();
    
#endif    

/* [] END OF FILE */
