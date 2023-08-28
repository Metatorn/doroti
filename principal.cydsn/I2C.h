/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del bus principal
====================================================*/

#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

#include "cypins.h"     
#include "colas.h"   
      
#define I2C_TIMEOUT             (500) //tiempo máximo para intentar comunicación  
#define COMANDO_TIMEOUT         (70)
      
/*Errores y estados de la comunicacion*/
#define COMUNICACION_FALLIDA    (10)
#define COMANDO_ERROR           (11)
#define COMANDO_CORRECTO        (12)
#define ERROR_PRODUCTO          (13) 
#define COMANDO_ALERTA          (14) 
#define ERROR_AUTENTICACION     (15)
#define COMANDO_ESPERA          (16)
#define COMANDO_NO_CAYO         (17)    
    
/* Tamaño del buffer y del paquete */
#define LARGO_BUFFER            (30u)    
#define LARGO_PAQUETE           (LARGO_BUFFER)   

void habilitar_busPrincipal();    
CYBIT bus_escribir_Comando(uint8 Direccion, uint8* bufferEscritura);
CYBIT bus_leer_comando(uint8 Direccion, uint8* bufferLectura);
void bus_limpiar_bufferEscritura(uint8* bufferEscritura, uint8 largo);
void bus_limpiar_bufferLectura(uint8* bufferLectura, uint8 largo);
uint8 verificarConexionEsclavo(uint8 direccion);

#endif 
/* [] END OF FILE */
