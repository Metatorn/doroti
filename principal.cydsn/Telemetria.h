/* ==============================================================
 *
 * Copyright EKIA Technology SAS, 2019
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
;ARCHIVO CABECERA PARA MANEJO DEL SISTEMA GSM/GPRS
===============================================================*/

#ifndef TELEMETRIA_CONFIG_H
#define TELEMETRIA_CONFIG_H

#include "cypins.h"     
#include "tipos.h" 
    
#define PERIODO_PING    (10*60)
#define PERIODO_AT      (30)    
    
#define ORG_ID                  "wqmp0z"   
    
#define TOPICO_COMANDOS         "command/DOROTI-"   
#define TOPICO_GESTION          "cmdManage"
#define TOPICO_RESULTADOS       "cmdGeneral"
#define TOPICO_CHECKON          "checkOn"    

#define TOPICO_CHECKED          "checked"    
//#define TOPICO_PRUEBAS          "iot-2/cmd/pruebas/fmt/json"    
#define TOPICO_INFO_MAQUINA     "iot-2/evt/infoMaquina/fmt/json"
#define TOPICO_RESP_CLAVE       "iot-2/evt/cambioClave/fmt/json"    
//#define TOPICO_CAMBIO_CLAVE     "iot-2/cmd/cambioClave/fmt/json"      
#define TOPICO_AVISOS_BILLETERO "iot-2/evt/avisoBilletero/fmt/json"
#define TOPICO_ERRORES_BILLETERO "iot-2/evt/errorBilletero/fmt/json"
#define TOPICO_AVISOS_MONEDERO  "iot-2/evt/avisoMonedero/fmt/json"
#define TOPICO_ERRORES_MONEDERO "iot-2/evt/errorMonedero/fmt/json"    
#define TOPICO_REGISTRO         "iot-2/evt/registroMaquina/fmt/json"
//#define TOPICO_COMANDOS         "iot-2/cmd/comandosTelemetria/fmt/json" 
   
//#define TOPICO_RESULTADOS       "iot-2/evt/resultadoComandos/fmt/json"     
     
//#define TOPICO_ACTUALIZAR       "iotdm-1/mgmt/initiate/firmware/update"    
//#define TOPICO_DETALLES         "iotdm-1/device/update" 
#define TOPICO_CODIGOS_ERROR    "iotdevice-1/add/diag/errorCodes"    
//#define TOPICO_DESCARGA_SOFT    "iotdm-1/mgmt/initiate/firmware/download"
#define TOPICO_ESTADO_DESCARGA  "iotdevice-1/notify"    
//#define TOPICO_GESTION          "iotdevice-1/mgmt/manage"
#define TOPICO_GESTION_RESP     "iotdm-1/response"
//#define TOPICO_OBSERVACION      "iotdm-1/observe"    
#define TOPICO_REINICIO_FABRICA "iotdm-1/mgmt/initiate/device/factory_reset"      
//#define TOPICO_RESET            "iotdm-1/mgmt/initiate/device/reboot"
#define TOPICO_RESPUESTA        "iotdevice-1/response"  
    
#define COMANDO_CONTROLLED      "controlLed"
#define COMANDO_LEER_HUMEDAD    "leerHumedad"
#define COMANDO_LEER_TEMP       "leerTemp"
#define COMANDO_LEER_CONF_TEMP  "leerConfTemp"    
#define COMANDO_DEFINIR_TEMP    "definirTemp"
#define COMANDO_DEFINIR_CONF_TEMP "definirConfTemp"    
#define COMANDO_DEFINIR_PRODUCTO "definirProducto" 
#define COMANDO_DEFINIR_PROVEEDOR "definirProveedor"     
#define COMANDO_LEER_PRODUCTO   "solicitarProducto" 
#define COMANDO_LEER_BANDEJA    "solicitarBandeja"     
#define COMANDO_REINICIO        "reinicio" 
#define COMANDO_CAMBIAR_CLAVE   "cambiarClave" 
#define COMANDO_ACUMULADO_DIA   "acuDia"
#define COMANDO_ACUMULADO_MES   "acuMes"
#define COMANDO_PARAMETROS_BANDEJAS "parametrosBandejas"
#define COMANDO_AUTORIZAR_PRODUCTO "autorizarProducto"
#define COMANDO_AUTORIZAR_PROVEEDOR "autorizarProveedor"    

//Atributos de respuesta para descarga de firmware    
#define DESCARGA_INMEDIATA      (200)
#define VERSION_DIFIERE         (204)
#define ERROR_DESCARGA          (400)
#define DESCARGA_FALLIDA        (500)
    
//Atributos de respuesta para contabilidad
#define ARCHIVO_NO_EXISTE       (401)
#define INFORMACION_NO_EXISTE   (402)
    
#define TEXT_NO_ARCHIVO         "No existe el archivo solicitado"
#define TEXT_NO_INFO            "No existe el registro solicitado"    

//Atributos de respuesta para solicitud de reinicio    
#define REINICIO_INMEDIATO      (202)   
#define REINICIO_FALLIDO        (500)
#define REINICIO_NO_SOPORTADO   (501)  

//Atributos de respuesta para  mgmt.firmware.state    
#define DESOCUPADO              (0)
#define DESCARGANDO             (1)
#define DESCARGADO              (2)
    
//Atributos de respuesta para mgmt.firmware.updateStatus
#define CORRECTO                (0)
#define EN_CURSO                (1)    
#define FUERA_MEMORIA           (2)
#define CONEXION_PERDIDA        (3)    
#define ERROR_VERIFICACION      (4)
#define IMAGEN_NO_SOPORTADA     (5)
#define URL_NO_VALIDA           (6)  
    
//codigos de error
#define TOPICO_PERDIDO          (0)
#define COMANDO_PERDIDO         (1) 
#define COMANDO_ERROR           (2)     
    
void reinicializar();
void Telemetria_Start(void);
void Telemetria_Stop();
void SIM800L_MQTT_enConexion(); 
void SIM800L_MQTT_enOperacion();
void SIM800L_MQTT_enMensaje(char *Topico, int LongitudTopico, char *Mensaje, int LongitudMensaje);
#endif  
