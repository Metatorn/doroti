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
;Archivo de configuracion, tarjeta de potencia EKIA AMIGO
====================================================*/
#ifndef EKIA_CONFIG_H
#define EKIA_CONFIG_H
  
#define VERSION 			(1.4)
#define TIEMPO_ARRANQUE                 (1000)  //tiempo de espera para estabilización de fuentes

#define FRECUENCIA                      (78000000)//frecuencia de trabajo en Hz del microcontrolador
#define FRECUENCIA_TICK                 (1000)
#define NUMERO_PRODUCTOS_BANDEJA        (6)
#define NUMERO_BANDEJAS                 (6)    
#define SENSORES_ULTRASONIDO            (NUMERO_PRODUCTOS_BANDEJA*NUMERO_BANDEJAS) 
#define NUMERO_SENSORES                 (1)  
#define TIEMPO_ARRANQUE_SUAVE           (160)    
     
#define PESO_MINIMO         (6)    
#define HISTERESIS_SUPERIOR             (2.0f)  
#define HISTERESIS_INFERIOR             (1.0f) 
#define TEMPERATURA_DEFAULT             (8.0f)    
    
#endif   
/* [] END OF FILE */
