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
 * ===================================================
;Archivo cabecera para declaración de variables y
;constantes comunes de todos los task del sistema 
====================================================*/
#ifndef TIPOS_CONFIG_H
#define TIPOS_CONFIG_H

#include "EKIA_CONFIG.h"    
#include "FreeRTOS.h" 
#include "semphr.h"    
#include "queue.h"
#include "task.h"
#include "cypins.h"    
#include "FS.h"     

/*=====================================================
;CONSTANTES DEL SISTEMA
====================================================*/  
#define RX_BUF_USB_LENGHT   (64)   
#define TX_BUF_USB_LENGHT   (64)       
#define T_ARRANQUE          TIEMPO_ARRANQUE   
#define MOTORES_DOROTI       (18)    
    
#define BLUETOOTH_LEN_MAX   (70)
#define GSM_LEN_MAX         (200)
#define GSM_CMD_LEN_MAX     (150) 
#define EN_ESPERA           (2)    
#define LISTO               (1)
#define NO_LISTO            (0)  
#define MDB_ESPERA          (20)
#define MDB_NO_DETECT       (21)
#define MDB_DETECT          (22)
#define ACTIVO              (10)
#define NO_ACTIVO           (11)    
    
#define ESPERA_OPERACION    (100)    
#define CANCELAR_VENTA      (101)
#define MODIFICAR_SALDO     (102)
#define RELLENO_MONEDAS     (103)
#define ACTIVAR_SISTEMAS    (104)
#define DESACTIVAR_SISTEMAS (105)
#define DEVOLVER_DINERO     (101)  
    
#define LONGITUD_CLAVE_USUARIO  (14)
#define LONGITUD_IDENTIFICADOR_USUARIO  (10)    
#define LONGITUD_NOMBRE_USUARIO (15)
    
#define ID_FABRICANTE       (20)
#define ID_ADMINISTRADOR    (30)    
    
/*=====================================================
;Estados del sistema para comunicación
====================================================*/    
#define DESCONECTADO        (10)
#define CONECTADO           (11)
#define ESPERA              (12) 
#define DESHABILITADO       (13)    
    
/*=====================================================
;Constantes para indicar notificaciones
====================================================*/
#define NORMAL              (9)    
#define NO_CAMBIO           (10)  
#define ERROR_FUSIBLE       (11)
#define ERROR_MDB           (12)
#define ERROR_TARJETA_POTENCIA (13)    
    
/*=====================================================
;Prioridades de las tareas
====================================================*/    
#define priorLED            (tskIDLE_PRIORITY + 5)
#define priorMDB            (tskIDLE_PRIORITY + 1)
#define priorInterface      (tskIDLE_PRIORITY + 5)
#define priorUSB            (tskIDLE_PRIORITY + 3)  
#define priorEstadoUSB      (tskIDLE_PRIORITY + 3)    
#define priorGeneral        (tskIDLE_PRIORITY + 6) 
#define priorMonitoreoSD    (tskIDLE_PRIORITY + 4)
#define priorWifi           (tskIDLE_PRIORITY + 2)
#define priorTelemetria     (tskIDLE_PRIORITY + 2) 
    
/*=====================================================
;Memoria de las tareas
====================================================*/    
#define memLED              (configMINIMAL_STACK_SIZE)
#define memMDB              (900)
#define memInterface        (1200)
#define memUSB              (600)  
#define memEstadoUSB        (300) 
#define memGeneral          (1000)    
#define memWifi             (800)
#define memTelemetria       (1000)
#define memMonitoreoSD      (1000)    
    
/*=====================================================
;Textos para registros de eventos de la máquina
====================================================*/ 
#define contexto_arranque           (1)    
#define contexto_error_caida        (2)
#define contexto_credito            (3)
#define contexto_devolucion_monedas (4)
#define contexto_venta_ok           (5)
#define contexto_venta_aviso        (6) 
#define contexto_recarga_ok         (7) 
#define contexto_error_comunicacion (8)    
    
#define tipo_anuncio                "Anuncio"
#define tipo_operacion              "Operacion"    
#define tipo_error                  "Error"  
    
#define text_registro_arranque      "Maquina ha sido encendida" 
#define text_registro_billete       "Aceptado billete de " 
#define text_registro_dev_billete   "Devuelto billete de "
#define text_registro_dev_moneda    "Devuelta Moneda de "    
#define text_registro_moneda        "Moneda ingresada de "    
#define text_registro_credito       "Credito Ingresado: "
#define text_registro_credito2      "Credito Final: "    
#define text_registro_devolucion    "Se ha devuelto dinero: "
#define text_registro_error_caida   "El producto no cayo: "
#define text_registro_error_recarga "Fallo recarga a "  
#define text_registro_cancelado     "Venta Cancelada"
#define text_registro_recarga_ok    "Recarga Safistactoria a "    
#define text_registro_venta_ok      "Venta Satisfactoria: " 
#define text_registro_venta_aviso   "Compuerta abierta, venta validada: "    
#define text_registro_puerta_ab     "Se ha abierto la puerta"
#define text_registro_puerta_ce     "Se ha cerrado la puerta"
#define text_registro_comunicacion  "Error de comunicacion tarjeta motores" 
#define text_registro_descanso_in   "Unidad entro a descanso"    
#define text_registro_descanso_out  "Unidad salio de descanso" 
#define text_registro_nfc_ventaOk   "Venta Satisfactoria tarjeta: "    
#define text_registro_nfc_ventaFalla "Fallo venta tarjeta NFC: " 
#define text_registro_nfc_ventaAlert "Compuerta abierta, Venta tarjeta: " 
#define text_registro_nfc_recargaOk "Actualizacion tarjeta "   
#define text_registro_nfc_recargaFalla "Error actualizando tarjeta "
#define text_registro_devolucionManual "Se hizo devolucion manual de dinero: "
#define text_registro_vacioReciclador "Solicitud de vaciado de reciclador"    
    
/*=====================================================
;Solicitudes de operaciones en la memoria externa
====================================================*/     
#define ACTUALIZAR_CONTABILIDAD             (100)   
#define ACTUALIZAR_REGISTRO                 (200) 
#define GUARDAR_DESCARGA                    (400)    

/*=====================================================
;Variables y estructuras comunes
====================================================*/ 

typedef struct {
    char entrada[BLUETOOTH_LEN_MAX];
}xbufferEntradaBluetooth;

typedef struct {
    char entrada[GSM_CMD_LEN_MAX];
}xbufferEntradaGSM;

typedef struct {
    CYBIT finCadena;
    char entrada[RX_BUF_USB_LENGHT];
}xbufferEntradaUSB;

typedef struct {
    uint8 comando;
    int opcional1;
    int opcional2;
}xcomandoUSB;

typedef struct {
    uint8 comando;
    int opcional1;
    int opcional2;
}xcomandoHMI;
    
typedef struct {
    int dato1;
    int dato2;
    int dato3;
    int dato4;
    int dato5;
    int dato6;
    float dato7;
    char dato8[60];
    uint8 dato9;
}xcadenas;    

typedef struct {
    uint8 operacion;
    int valor;
}xresPantalla;

typedef struct {
    char numSerie[17];
    char serie[5];
    //char Dependencia;
    char tipo;
    char fecha[5];
    char version[3];
    char nombre[5];
}xnumSerie;

typedef struct {
    CYBIT activo;
    uint8 nivel;
}xbrillo;

typedef struct {
    char clave[14];
}xclave;

typedef struct {
    CYBIT estadoLector;
}xConfiguracionAccesorios;

typedef struct {
    int numero;
    float precio;
    uint8 cantidad;
    CYBIT habilitado;
    uint8 proveedor;
    uint16 alturaXItem;
}xProducto;

typedef struct {
    CYBIT canastaCerrada;
    CYBIT canastaBloquedada;
    uint8 comando;
}xCanasta;

typedef struct {
    CYBIT activado; 
    CYBIT descanso;
    uint16 gradosC;
}xTemperatura;

typedef struct {
    CYBIT estado; 
    float medida;
}xMedidaDispensador;

typedef struct {
    uint8 numBandejas;
    uint8 estado[NUMERO_BANDEJAS_MAX]; 
    uint8 numMotores[NUMERO_BANDEJAS_MAX];
}xBandejas;

typedef struct {
    uint8 estadoMaquina;
    xnumSerie Serie;
    xbrillo BrilloPantalla;
    xbrillo BrilloMaquina;
    uint16 PesoMinimo;
    float escalaPeso;
    CYBIT bloqueoMaquina;
}xConfiguracion;

typedef struct {
    xTemperatura refrigeracion;
    uint8 HorasDescanso;
    uint8 MinutosDescanso;
    uint16 HisteresisSuper;
    uint16 HisteresisInfer;
}xConfiguracionTemperatura;    

typedef struct {
    CYBIT existenciaTelemetria;
    CYBIT estadoTelemetria;
    uint8 estadoConexionGSM;
}xConfiguracionTelemetria;

typedef struct{
    uint16 tiempoMovimiento;
    uint16 cicloUtil;
}xtiempoComplementario;

typedef struct {
    uint16 tiemposBandeja[NUMERO_BANDEJAS_MAX];
    xtiempoComplementario parametrosMotores;
    xBandejas configBandejas;
}xConfiguracionBandejas;

typedef struct {
    int horas;
    uint8 mediasHoras;
    uint8 minutos;
}xHorasMaquina;

typedef struct {
    int operacion;
    int estado;
    xProducto producto;
}xContabilidad;

typedef struct {
    int operacion;
    char tipo[20];
    char evento[100];
}xEventos;

typedef struct {
    int resultado1;
    int resultado2;
    int resultado3;
}xRespuestasGSM;

typedef struct {
    CYBIT encendido;
    xRespuestasGSM registro;
    xRespuestasGSM GPRS;
    xRespuestasGSM potenciaSenal;
    char fabricante[20];
    char modelo[20];
    char IMEI[20];
    char IMSI[15];
    char operador[50];
    char estado[15];
    char IpLocal[16];
    xRespuestasGSM debug;
}xInformacionGSM;

typedef struct {
    int operacion;
    CYBIT estado;
    int numBytes;
    char datos[200];
    char ruta[50];
}xinfoDescargada;

typedef struct {
    char usuario[LONGITUD_NOMBRE_USUARIO];
    int identificador;
    uint16 producto;
    char clave[LONGITUD_CLAVE_USUARIO];
    uint8 cantidad;
}xAutorizacion;

typedef struct {
    char nombre[LONGITUD_NOMBRE_USUARIO];
    char identificador[LONGITUD_CLAVE_USUARIO];
    char clave[LONGITUD_CLAVE_USUARIO];
    int idHuella;
}xDatosUsuario;

typedef struct {
    char clave[LONGITUD_CLAVE_USUARIO];
    uint8 identificador; 
    uint64 productosAutorizados;
}xProveedor;

CY_PACKED typedef struct {
    int comandos;
    char datos[5][150];
}CY_PACKED_ATTR xComandosSIM;

typedef struct{
    char* stringPointer;
	int code;
}debug_print_data_t;

#endif
/* [] FIN DE ARCHIVO */
