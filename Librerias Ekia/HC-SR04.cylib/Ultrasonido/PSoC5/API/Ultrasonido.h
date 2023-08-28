/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * ULTRASONIDO_EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones propias
;de EKIA que comandan el sensor de distancia ultrasonico
====================================================*/
#ifndef `$INSTANCE_NAME`_CONFIG_H
#define `$INSTANCE_NAME`_CONFIG_H

#include "CyLib.h"  

/* Definiciones de constantes */

float `$INSTANCE_NAME`_medirDistancia(uint8 preescaler); 
void `$INSTANCE_NAME`_Start();
void `$INSTANCE_NAME`_Stop();
   
#endif    
/* [] END OF FILE */
