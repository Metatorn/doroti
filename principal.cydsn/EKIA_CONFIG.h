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
;Archivo de configuracion, maquina vending EKIA ASM
====================================================*/
#ifndef EKIA_CONFIG_H
#define EKIA_CONFIG_H

#define VERSION_SOFTWARE                ("2.0")    
//#define DEPENDENCIA                     ("V")   //division de EKIA Vending Technology    
#define TIPO_MAQUINA                    ("D")   //tipo: D(Dr. DOROTI)
//#define SIZE                            ("D")   //tamaño: P(Small), C(compact), S(slim), E(estandar), M(mega)     
#define VERSION                         ("01")
#define NUMERO_DE_SERIE                 ("A001")    
#define FECHA                           ("0220")//fecha de fabricacion MMAA    
#define NUMERO_BANDEJAS_MAX             (6)     //numero de filas por maquina (max 6)    
#define NUMERO_PRODUCTOS_BANDEJA_MAX    (6)    //numero de columnas por maquina (max 6))  
  
#define TIEMPO_ARRANQUE                 (1000)  //tiempo de espera para estabilización de fuentes

#define FRECUENCIA                      (75997000)//frecuencia de trabajo en Hz del microcontrolador
#define FRECUENCIA_TICK                 (1000)
#define TIEMPO_NOTIFICACION             (200)
#define TIEMPO_ESPERA                   (200) 
#define TIEMPO_RETORNO                  (40)    
#define HORAS_DESCANSO_COMPRESOR        (8)
#define TIEMPO_DESCANSO_REFRIGERACION   (10) 
#define RANGO_SUPERIOR                  (200)
#define RANGO_INFERIOR                  (10)
#define PESO_MINIMO                     (0) //1.5gr
#define ESCALA_PESO                     (1.0f/567.0f)     
    
#define MAX_BANDEJAS                    (6) 
#define MAX_PRODUCTOS_BANDEJA           (6)   
    
#define INTENTOS_CREDITO                (500)
    
#endif   
/* [] END OF FILE */
