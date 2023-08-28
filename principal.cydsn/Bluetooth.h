/* ==============================================================
 *
 * Copyright EKIA Technologies, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;ARCHIVO CABECERA PARA MANEJO DEL SISTEMA BLUETOOTH
===============================================================*/

#ifndef BLUETOOTH_CONFIG_H
#define BLUETOOTH_CONFIG_H

#include "cypins.h"     
#include "colas.h" 
    
/*=====================================================
;Constantes Bluetooth
====================================================*/  
#define TMAX_Bluetooth                    (50)     
    
void Bluetooth_Start(void);
void Bluetooth_Stop();
    
#endif     
/* [] END OF FILE */
