/* ===================================================
 *
 * Copyright EKIA Technologies, 2017
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
; CABECERA LIBRERIA PARA MANEJO DEL SENSOR HX711
====================================================*/
#ifndef HX711_CONFIG_H
#define HX711_CONFIG_H
    
#include "tipos.h"
#include "cytypes.h"

#define MUESTRAS            (2.0f)   
#define RESOLUCION          (4194304.0f)    //2^22
#define TOLERANCIA_GRUESA   (10.0f)           //tolerancia en valor entero del peso   
#define TOLERANCIA_FINA     (0.1f)          //tolerancia fina en gramos
#define ESCALA              (2.0f/567.0f)      //factor de escala para el Hx711
#define PESO_REFERENCIA     (1000.0)          //peso de referencia para la calibración automática del sensor de peso
#define PASOS_CALIBRACION   (5)             //cambios en el factor de escala para calibrarlo
#define PESO_MINIMO         (6)       
    
float leerSensorPeso();    
void peso_Start();
void setCeroPeso(); 
void wakeUp_Hx711();
void sleep_Hx711();
void hx711_set_offset(int offset);
uint32 hx711_get_offset();
//void calibrarOffset();
void hx711_set_channel(char canal);
int leerEscalaPeso();
void definirEscalaPeso(float valorEscala);
void definirPesoMinimo(float peso);
float leerPesoMinimo();
//int calibrarPeso();

#endif
/* [] END OF FILE */
