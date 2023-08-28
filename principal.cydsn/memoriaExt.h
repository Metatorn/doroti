
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
;Archivo cabecera para declaración de funciones de la memoria SD
====================================================*/
#ifndef MEMORIASD_CONFIG_H
#define MEMORIASD_CONFIG_H

#include "cypins.h"
#include "tipos.h"    
#include "memoriaSD.h"
#include "FS.h"
#include "RTC_RTC.h"    
    
#define freq_1_Hz               (100000/1)
#define freq_10_Hz              (100000/10)  
#define freq_20_Hz              (100000/20)  
#define freq_50_Hz              (100000/50)  
#define freq_100_Hz             (100000/100)     
    
#define pName_Dir_Conf          "\\CONFIGURACION"  
#define pName_Dir_Cont          "\\CONTABILIDAD"
#define pName_Dir_Cont_Dia      "\\CONTABILIDAD\\DIARIO"
#define pName_Dir_Cont_Mes      "\\CONTABILIDAD\\MENSUAL"
#define pName_Dir_Cont_Anual    "\\CONTABILIDAD\\ANUAL"   
#define pName_Dir_Registro      "\\REGISTRO"    
    
#define ruta_configuracion      "\\CONFIGURACION\\PARAMETROS.csv"
#define ruta_productos          "\\CONFIGURACION\\PRODUCTOS.csv"
#define ruta_credito            "\\CONFIGURACION\\credito.csv"    
    
#define label_serie             "Num. Serie:"
#define label_brilloAuto        "Brillo Automatico:"
#define label_brilloNivel       "Nivel Brillo:"
#define label_ilumiAct          "Iluminacion Activa:"
#define label_ilumiNivel        "Nivel Iluminacion:" 
#define label_refriAct          "Refrigeracion Activa:"
#define label_temp              "Temperatura deseada:" 
#define label_escalaPeso        "Escala de peso:"  
#define label_escalaTemp        "Escala de Temperatura:"    
    
typedef struct { 
    int filaLeida;
    char lineaLeida[100];
    long punteroByte;
    CYBIT finArchivo;
    FS_FILE *archivo;
}xLecturaSD;

typedef struct {
    int fila;
    char celda1[40];
    char celda2[40];
    char celda3[40];
    char celda4[40];
    char celda5[40];
    char celda6[40];
    char celda7[40];
    char celda8[40];
}xCeldas;

typedef struct {
    xCeldas celdaPorFila;
    int filasDisponibles;
    long puntero;
}xCeldasArchivo; 

typedef struct {
    float capacidad;
    float utilizado;
    char prefijo[6];
}xEspacioMemoria;

CYBIT habilitarSD();   
CYBIT deshabilitarSD();
xEspacioMemoria espacioSD();
CYBIT formatearSD();
xLecturaSD leerArchivoSD(FS_FILE *archivo);
xCeldas separarCeldasSD(xLecturaSD filaLeida);
CYBIT restaurarConfiguracionSD();
CYBIT copiarConfiguracionSD();
CYBIT restaurarProductosSD();
CYBIT copiarProductosSD(char* ruta);
CYBIT escribirContabilidadSD(xProducto productoSolicitado);
CYBIT escribirContabilidadDiaSD(xProducto productoSolicitado, RTC_RTC_TIME_DATE fechaHora);
xCeldasArchivo leerContabilidadDiariaSD(int venta, RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura);
CYBIT escribirContabilidadMensualSD(xProducto productoSolicitado, RTC_RTC_TIME_DATE fechaHora);
xCeldasArchivo leerContabilidadMensualSD(RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura);
CYBIT escribirContabilidadAnualSD(xProducto productoSolicitado, RTC_RTC_TIME_DATE fechaHora);
xCeldasArchivo leerContabilidadAnualSD(RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura);
CYBIT escribirRegistroSD(xEventos informacion);
xCeldasArchivo leerRegistroSD(int registro, RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura);
CYBIT guardarArchivoDescargaSD();

#endif   
/* [] END OF FILE */
