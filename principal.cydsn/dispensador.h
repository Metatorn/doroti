/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2020
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
;Archivo cabecera para declaración de funciones del dispensador
====================================================*/

#ifndef DISPENS_CONFIG_H
#define DISPENS_CONFIG_H

#include "cypins.h"     
#include "tipos.h"   
      

#define DIRECCION_DISPENSADOR   (110) //dirección I2C del módulo dispensador 
 
#define DISPENSADOR_NO_LISTO    (100)
#define DISPENSADOR_ESPERA      (101)     
 
#define EMERGENCIA              (2)     

/* mascaras de incio y final de un paquete */    
#define PAQUETE_INICIO          '='
#define PAQUETE_FIN             ';'  
    
//#define DISPENSADOR_NO_LISTO    (10)
//#define DISPENSADOR_ESPERA      (11) 
    
/* estado de comando valido */    
#define CMD_CORRECTO            'C'
#define CMD_FALLA               'F'
#define CMD_NO_CAYO             'M'    
#define CMD_ESPERA              'E'  
#define CMD_ALERTA              'A'
#define CMD_NULO                'N'    
    
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
#define LEER_CORRIENTE          (7)// 
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
    
#define LEER_ESTADO_CANASTA     (30)//
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

uint8 Dispensador_LED_DEBUG_write(CYBIT valor);
uint8 Dispensador_definir_brillo(uint8 valor);
xMedidaDispensador Dispensador_LeerPeso();
uint8 Dispensador_definir_escala_peso(float escala);
float Dispensador_leer_escala_peso();
uint8 Dispensador_setCero_peso();
xMedidaDispensador Dispensador_LeerVoltaje();
xMedidaDispensador Dispensador_LeerTemperatura(uint8 sensor);
float Dispensador_LeerTemperaturaDefinida();
xMedidaDispensador Dispensador_LeerHumedad(uint8 sensor);
xMedidaDispensador Dispensador_LeerDistancia(uint8 sensor);
CYBIT Dispensador_definir_Tiempos(uint16 tiempos[], uint16 tiempoMovimiento, uint16 cicloUtil);
uint8 Dispensador_dispensar(uint16 producto);
uint8 Dispensador_mover_motor(uint8 motor);
uint8 Dispensador_leer_motor();
uint8 Dispensador_abrirProducto(uint16 producto);
uint8 Dispensador_abrir_slot(uint8 slot);
uint8 Dispensador_Modo_Prueba(CYBIT valor);
uint8 Dispensador_definir_peso_minimo(uint16 peso);
uint16 Dispensador_leer_peso_minimo();
uint8 Dispensador_definir_temperatura(uint16 temperatura);
uint8 Dispensador_definir_rango_superior(uint16 rango_sup);
uint8 Dispensador_definir_rango_inferior(uint16 rango_inf);
uint8 Dispensador_Refrigerador_write(CYBIT valor);
uint8 Dispensador_leer_puerta();
uint8 Dispensador_abrir_puerta(CYBIT valor);
xCanasta Dispensador_leer_canasta();
uint8 Dispensador_definir_estado(uint8 estado);
uint8 Dispensador_leer_estado();

#endif 
/* [] END OF FILE */
