/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * HMI EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones de la pantalla tactil
====================================================*/
#ifndef `$INSTANCE_NAME`_CONFIG_H
#define `$INSTANCE_NAME`_CONFIG_H

#include "cypins.h" 
#include "RTC_RTC.h"   

/*=====================================================
;Variables del HMI
====================================================*/ 
#define `$INSTANCE_NAME`_LEN_MAX                 (60)
#define `$INSTANCE_NAME`_TASA_BAUDIOS_HMI        (19200) //tasa de comunicacion con la pantalla    
    
/*=====================================================
;Constantes de estado de comunicación
====================================================*/ 
    
#define `$INSTANCE_NAME`_CORRECTO                (100)
#define `$INSTANCE_NAME`_FALLIDO                 (101)  
    
#define `$INSTANCE_NAME`_DATO_NUMERICO           (0x71)
#define `$INSTANCE_NAME`_DATO_TEXTO              (0x70)    
    
/*=====================================================
;Comandos utilizados en la pantallla
====================================================*/    
#define cmd_pantalla            "page "
#define cmd_texto               ".txt=" 
#define cmd_baud                "baud="    
#define cmd_bauds               "bauds=" 
#define cmd_dim                 "dim="   
#define cmd_draw                "thdra="   
#define cmd_draw_color          "thc=" 
#define cmd_calibrar            "touch_j"    
#define cmd_vis                 "vis "
#define cmd_tsw                 "tsw "
#define cmd_get                 "get " 
#define cmd_print_val           "print "  
#define cmd_val                 ".val=" 
#define cmd_decimal             ".vvs1="    
#define cmd_getPagina           "sendme"
#define cmd_lenMax              "txt_maxl="
#define cmd_font                "font="    
#define cmd_comboBox            ".path"    

/*=====================================================
;Constantes de colores de la pantalla
====================================================*/ 
    
#define color_error             "RED" 
#define color_normal            "WHITE"
#define color_alerta	          "YELLOW" 
#define color_anuncio       "BLUE"
#define color_bloqueo           "GREEN"    
    
/*=====================================================
;Constantes de divisores para el reloj del HMI, 
;para modificar la tasa de comunicacion
====================================================*/  
#define `$INSTANCE_NAME`_TMAX_HMI                    (15) 
#define `$INSTANCE_NAME`_BAUDIOS_HMI                 `$INSTANCE_NAME`_TASA_BAUDIOS_HMI
#define `$INSTANCE_NAME`_BAUD_9600                   (192) 
#define `$INSTANCE_NAME`_BAUD_19200                  (96)
#define `$INSTANCE_NAME`_BAUD_38400                  (48)
#define `$INSTANCE_NAME`_BAUD_57600                  (32)
#define `$INSTANCE_NAME`_BAUD_115200                 (16)
    
/*=====================================================
;Variables y estructuras comunes
====================================================*/ 

typedef struct {
    char buffer[`$INSTANCE_NAME`_LEN_MAX];
    char numero[15];
    uint8 objeto;
    uint8 fin;
    uint8 estado;
}bufferEntradaHMI;

/*=====================================================
;funciones del HMI
====================================================*/ 
    
CY_ISR(rxHMI);
void `$INSTANCE_NAME`_Start();
void `$INSTANCE_NAME`_tx(char* comando);
CYBIT `$INSTANCE_NAME`_ventana(uint8 valor);
void `$INSTANCE_NAME`_tx_comando(char* objeto, char* numero, char* parametro, char* valor);

CYBIT `$INSTANCE_NAME`_setBaud_default(int baudios);
CYBIT `$INSTANCE_NAME`_setBaud(int baudios);
void `$INSTANCE_NAME`_brillo(uint8 valor_brillo);
void `$INSTANCE_NAME`_dibujo(uint8 habilitar, char*color);
void `$INSTANCE_NAME`_visualizar(char* objeto, CYBIT visible);
void `$INSTANCE_NAME`_print_value(char* objeto);
void `$INSTANCE_NAME`_touch(char* objeto, char* numero, CYBIT habilitado);
void `$INSTANCE_NAME`_set_value(char* objeto,int valor);
void `$INSTANCE_NAME`_set_float(char* objeto,int valor, int decimales);
void `$INSTANCE_NAME`_get_value(char* objeto);
void `$INSTANCE_NAME`_calibrar();
int `$INSTANCE_NAME`_obtenerPagina();
void `$INSTANCE_NAME`_combo_texto(CYBIT lineaInicial, char* objeto, char* texto);
RTC_RTC_TIME_DATE *`$INSTANCE_NAME`_read_time();
CYBIT `$INSTANCE_NAME`_write_time();

bufferEntradaHMI `$INSTANCE_NAME`_rx_comando();
CYBIT `$INSTANCE_NAME`_leerReinicio();

#endif    
/* [] END OF FILE */
