/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del bus principal
====================================================*/

#ifndef I2C_CONFIG_H
#define I2C_CONFIG_H

#include "cypins.h"     
#include "tipos.h"          
      
#define DIRECCION               (110) //dirección I2C del módulo dispensador    

/* mascaras de incio y final de un paquete */    
#define PAQUETE_INICIO          '='
#define PAQUETE_FIN             ';'   
    
/* estado de comando valido */    
#define CMD_CORRECTO            'C'
#define CMD_FALLA               'F'
#define CMD_ESPERA              'E'  
#define CMD_ALERTA              'A'
#define CMD_NULO                'N'
#define CMD_NO_CAYO             'M'    
    
/* Tamaño del buffer y del paquete */
#define LARGO_BUFFER            (30u)    
#define LARGO_PAQUETE           (LARGO_BUFFER)   

/* posiciones en el paquete */
#define PAQUETE_INICIO_POS      0u
#define PAQUETE_ESTADO_POS      1u
#define PAQUETE_CMD_POS         PAQUETE_ESTADO_POS     
#define PAQUETE_WR_FIN_POS      (2u)    

/* comandos posibles */    
#define ENCENDER_LED_DEBUG      (1)
#define APAGAR_LED_DEBUG        (2)     
#define DEFINIR_BRILLO          (3)
    
#define LEER_VOLTAJE            (6)
#define LEER_CORRIENTE          (7)  
#define LEER_TEMPERATURA1       (8)
#define LEER_TEMPERATURA2       (9)     

#define LEER_TEMPERATURA_PROM   (10)
#define DEFINIR_TEMPERATURA     (11)  
#define APAGAR_REFRIGERACION    (12)
#define ENCENDER_REFRIGERACION  (13)  
#define LEER_TEMPERATURA_DEFINIDA (14) 
#define DEFINIR_RANGO_SUPERIOR  (15)
#define DEFINIR_RANGO_INFERIOR  (16) 
#define LEER_HUMEDAD1           (17)
#define LEER_HUMEDAD2           (18)
#define LEER_HUMEDAD_PROM       (19)     
    
#define LEER_ESTADO_PUERTA      (20)
#define ABRIR_PUERTA            (21)
#define CERRAR_PUERTA           (22) 
    
#define LEER_ESTADO_CANASTA     (30)
#define LEER_DISTANCIA          (31)    
    
#define MOVER_MOTOR             (40)
#define MODO_PRUEBA             (41)
#define MODO_NORMAL             (42)
#define LEER_MOTOR              (43)
#define ABRIR_SLOT              (44)    
     
#define LEER_ESCALA_PESO        (50)
#define DEFINIR_ESCALA_PESO     (51)    
#define LEER_PESO               (52) 
#define PESO_CERO               (55)     
#define DEFINIR_PESO_MINIMO     (56)
#define LEER_PESO_MINIMO        (57)
    
#define DEFINIR_TIEMPO_BANDEJAS (62)      
    
#define LEER_ESTADO_TARJETA     (60)
#define DEFINIR_ESTADO_TARJETA  (63)  


void I2C_habilitar_busPrincipal(uint8* buffer_Lectura, uint8* bufferEscritura);  
void I2C_limpiarBuffer();
CYBIT I2C_ComandoRecibido(uint8* buffer_Escritura);
    
#endif 
/* [] END OF FILE */
