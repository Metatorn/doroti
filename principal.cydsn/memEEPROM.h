/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;Archivo cabecera para declaración de funciones de la memoria EEPROM
====================================================*/
#ifndef MEMORIAEEPROM_CONFIG_H
#define MEMORIAEEPROM_CONFIG_H

#include "cypins.h"
#include "tipos.h" 
#include "EEPROM.h"
#include "EKIA_CONFIG.h"    

#define FILAS_DE_MOTORES                NUMERO_BANDEJAS_MAX
#define COLUMNAS_DE_MOTORES             NUMERO_PRODUCTOS_BANDEJA_MAX 
#define NUMERO_MOTORES_FILA             (COLUMNAS_DE_MOTORES)
#define LONGITUD_PRODUCTOS              (6)    
#define BYTES_PRODUCTOS                 FILAS_DE_MOTORES*NUMERO_MOTORES_FILA*LONGITUD_PRODUCTOS  
    
#define BYTES_REGISTRO                  (6)
#define LONGITUD_SERIE                  (16)
#define LONGITUD_CLAVE                  (14)
#define LONGITUD_HUELLA_USUARIO         (14) 
#define LONGITUD_HORAS                  (3)    
#define LONGITUD_TEMPERATURA            (2)   
#define LONGITUD_PESO                   (2)
#define LONGITUD_BANDERAS               (2)  //57  
#define LONGITUD_TIEMPOS                (18) //
#define LONGITUD_BANDEJAS               (6)  
#define LONGITUD_BILLETES_ACEPTADOS     (2)
#define LONGITUD_BILLETES_RETENIDOS     (2)
#define LONGITUD_BILLETES_RECICLADOS    (2)    
#define LONGITUD_MONEDAS_ACEPTADAS      (2)   
#define LONGITUD_HISTER_SUPER           (2)   
#define LONGITUD_HISTER_INFER           (2) //SE ALMACENA EN 16 BITS PARA CONTENER 2 DECIMAS DE PRECISION
#define LONGITUD_HORAS_DESCANSO         (1)
#define LONGITUD_MINUTOS_DESCANSO       (1)   //76
#define LONGITUD_PESO_MINIMO            (2)
    
#define LONG_NOMBRE_RED                 (20)
#define LONG_TIPO_CLAVE                 (1)
#define LONG_CLAVE_RED                  (20)
#define LONG_TIPO_DIRECCION             (1)
#define LONG_NUMERO_IP                  (4)
#define LONG_MASCARA_SUBRED             (4)
#define LONGITUD_PARAMETROS_WIFI        (LONG_NOMBRE_RED+LONG_TIPO_CLAVE+LONG_CLAVE_RED+LONG_TIPO_DIRECCION+LONG_NUMERO_IP+LONG_MASCARA_SUBRED)

#define LONG_USUARIO_CLAVE              (14)
#define LONG_USUARIO_NOMBRE             (15)    
#define LONG_USUARIO_IDENTI             (10)   
#define LONGITUD_PARAMETROS_USUARIO     (39)    
    
#define FACTOR                          (100.0f)  
    
#define MASCARA_DEVOLUCION_AUTO         (0xFE)
#define MASCARA_ESTADO_TELEMETRIA       (0xFD) 
#define MASCARA_EXISTENCIA_TELEMETRIA   (0xFB)
#define MASCARA_BLOQUEO                 (0xF7)   
#define MASCARA_CONFIRMACION            (0xDF) 
#define MASCARA_RECICLADOR              (0x7F)    
    
#define DIRECCION_PROGRAMA              (0x400087FF)   
#define DIRECCION_BASE                  (0)
#define DIRECCION_CONTRA_ADMIN          (DIRECCION_BASE+LONGITUD_SERIE)
#define DIRECCION_HORAS                 (DIRECCION_CONTRA_ADMIN+LONGITUD_CLAVE)
#define DIRECCION_CONTRA_TESOR          (DIRECCION_HORAS+LONGITUD_HORAS)
#define DIRECCION_CONTRA_OPER           (DIRECCION_CONTRA_TESOR+LONGITUD_CLAVE) 
#define DIRECCION_TEMPERATURA           (DIRECCION_CONTRA_OPER+LONGITUD_CLAVE)
#define DIRECCION_ESCALA_PESO           (DIRECCION_TEMPERATURA+LONGITUD_TEMPERATURA) 
#define DIRECCION_ESCALA_TEMP           (DIRECCION_ESCALA_PESO+LONGITUD_PESO)
#define DIRECCION_BRILLO_PANTALLA       (DIRECCION_ESCALA_TEMP+1)
#define DIRECCION_BRILLO_MAQUINA        (DIRECCION_BRILLO_PANTALLA+1)
#define DIRECCION_BANDERAS              (DIRECCION_BRILLO_MAQUINA+1)  
#define DIRECCION_TELEMETRIA            (DIRECCION_BANDERAS)
#define DIRECCION_TIEMPOS               (DIRECCION_BANDERAS+LONGITUD_BANDERAS) 
#define DIRECCION_BANDEJAS              (DIRECCION_TIEMPOS+LONGITUD_TIEMPOS)
#define DIRECCION_BILLETES_ACEPTADOS    (DIRECCION_BANDEJAS+LONGITUD_BANDEJAS)
#define DIRECCION_BILLETES_RETENIDOS    (DIRECCION_BILLETES_ACEPTADOS+LONGITUD_BILLETES_ACEPTADOS)
#define DIRECCION_MONEDAS_ACEPTADAS     (DIRECCION_BILLETES_RETENIDOS+LONGITUD_BILLETES_RETENIDOS)
#define DIRECCION_BILLETES_RECICLADOS   (DIRECCION_MONEDAS_ACEPTADAS+LONGITUD_MONEDAS_ACEPTADAS)    
#define DIRECCION_HISTER_SUPER          (DIRECCION_BILLETES_RECICLADOS+LONGITUD_BILLETES_RECICLADOS)
#define DIRECCION_HISTER_INFER          (DIRECCION_HISTER_SUPER+LONGITUD_HISTER_SUPER)
#define DIRECCION_HORAS_DESCANSO        (DIRECCION_HISTER_INFER+LONGITUD_HISTER_INFER)
#define DIRECCION_MINUTOS_DESCANSO      (DIRECCION_HORAS_DESCANSO+LONGITUD_HORAS_DESCANSO)
#define DIRECCION_PESO_MINIMO           (DIRECCION_MINUTOS_DESCANSO+LONGITUD_MINUTOS_DESCANSO)    
#define DIRECCION_PARAMETROS_WIFI       (DIRECCION_PESO_MINIMO+LONGITUD_PESO_MINIMO)    

#define DIRECCION_BASE_PRODUCTO         (400)
#define DIRECCION_BASE_USUARIOS         (700)
  
#define ERROR_ESCRITURA                 (0)
#define SERIE_GUARDADA_CORRECTAMENTE    (1)
#define SERIE_NO_PUEDE_GUARDARSE        (2)    

void habilitarEEPROM();
void deshabilitarEEPROM();
int obtenerDireccion(int numeroProducto);
xnumSerie leerSerie();
uint8 escribirSerie();
uint8 actualizarSerie(char* version, char* fecha, char* numero);
xbrillo leerBrilloPantalla();
CYBIT escribirBrilloPantalla(xbrillo brillo);
xProducto leerProducto(int numeroProducto);
CYBIT escribirProducto(xProducto producto);
xclave EEPROM_leerClaveAdmin();
CYBIT EEPROM_escribirClaveAdmin(xclave clave);
xclave EEPROM_leerClaveTesorero();
CYBIT EEPROM_escribirClaveTesorero(xclave clave);
xclave EEPROM_leerClaveOperador();
CYBIT EEPROM_escribirClaveOperador(xclave clave);
xbrillo leerBrilloMaquina();
CYBIT escribirBrilloMaquina(xbrillo brillo);
float leerEscalaPeso();
CYBIT escribirEscalaPeso(float escala);
float leerEscalaTemperatura();
CYBIT escribirEscalaTemperatura(float escala);
xTemperatura leerRefrigeracion();
CYBIT escribirRefrigeracion(xTemperatura refrigeracion);
xHorasMaquina leerHoras();
CYBIT escribirHoras(uint8 mediasHoras);
CYBIT escribirNumeroHoras(uint32 horas);
CYBIT leerEstadoDevolucion();
CYBIT escribirEstadoDevolucion(CYBIT devolucionAuto);
CYBIT leerEstadoTelemetria();
CYBIT escribirEstadoTelemetria(CYBIT activa);
CYBIT leerExistenciaTelemetria();
CYBIT escribirExistenciaTelemetria(CYBIT existencia);
CYBIT EEPROM_leerEstadoConfirmacion();
CYBIT EEPROM_escribirEstadoConfirmacion(CYBIT habilitado);
xtiempoComplementario EEPROM_leerTiempos(uint16 tiempos[]);
CYBIT EEPROM_escribirTiempos(uint16 tiempos[], uint16 tiempoMovimiento, uint16 cicloUtil);
xBandejas EEPROM_leerBandejas();
CYBIT EEPROM_escribirBandejas(xBandejas configBandejas);
uint16 EEPROM_leerBilletesAceptados();
CYBIT EEPROM_escribirBilletesAceptados(uint16 billetes);
uint16 EEPROM_leerBilletesRetenidos();
CYBIT EEPROM_escribirBilletesRetenidos(uint16 billetes);
uint16 EEPROM_leerBilletesReciclados();
CYBIT EEPROM_escribirBilletesReciclados(uint16 billetes);
CYBIT EEPROM_leerHabilitacionReciclador();
CYBIT EEPROM_escribirHabilitacionReciclador(CYBIT habilitado);
CYBIT EEPROM_escribirHisterSuper(uint16 gradosHisterSuper);
uint16 EEPROM_leerHisterSuper();
CYBIT EEPROM_escribirHisterInfer(uint16 gradosHisterInfer);
uint16 EEPROM_leerHisterInfer();
CYBIT EEPROM_escribirHorasDescanso(uint8 HorasDescanso);
uint8 EEPROM_leerHorasDescanso();
CYBIT EEPROM_escribirMinutosDescanso(uint8 MinutosDescanso);
uint8 EEPROM_leerMinutosDescanso();
CYBIT EEPROM_escribirPesoMinimo(uint16 PesoMinimo);
uint16 EEPROM_leerPesoMinimo();
xDatosUsuario EEPROM_leerCedulaUsuario(uint8 identificador);
CYBIT EEPROM_escribirCedulaUsuario(char* clave, uint8 identificador);
xDatosUsuario EEPROM_leerNombreUsuario(uint8 identificador);
CYBIT EEPROM_escribirNombreUsuario(char* nombre, uint8 identificador);
CYBIT EEPROM_leerBloqueo();
CYBIT EEPROM_escribirBloqueo(CYBIT habilitado);

#endif   
/* [] END OF FILE */
