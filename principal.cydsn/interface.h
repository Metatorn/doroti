/* ===================================================
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
 *
 * ===================================================
;Archivo cabecera para declaración de funciones del dimmer
====================================================*/
#ifndef INTERFACE_CONFIG_H
#define INTERFACE_CONFIG_H
    
#include "EKIA_CONFIG.h"               
    
/*=====================================================
;Comandos y respuestas de la pantalla tactil
====================================================*/  
#define T_NOTIFICACIONES            TIEMPO_NOTIFICACION
#define T_ESPERA                    TIEMPO_ESPERA 
#define TIEMPO_REFRESCO             100    
    
#define NUM_CELDAS_CONTABILIDAD     (13)    
#define NUM_DIAS_CONT_SEMANAL       (7) 
    
#define SALTOS                      (5)    
  
enum xPantallas{
    PANTALLA_BIENVENIDA =10,
    PANTALLA_CLAVE,
    PANTALLA_CONFIGURACION,
    PANTALLA_CONF_MEMORIA,
    PANTALLA_CONFIG_PRODUCTO,
    PANTALLA_ESPACIO,
    PANTALLA_MENU,
    PANTALLA_PRINCIPAL,
    PANTALLA_DISPENSAR_MANUAL,
    PANTALLA_REGISTRO,   
    PANTALLA_MENU_MOTORES,
    PANTALLA_CAMBIAR_CLAVE,
    PANTALLA_ILUMINACION,
    PANTALLA_BRILLO_PANTALLA,
    PANTALLA_LUZ_MAQUINA,
    PANTALLA_FECHAYHORA,
    PANTALLA_TEMPERATURA,
    PANTALLA_FORMATEAR,
    PANTALLA_COPIAR,
    PANTALLA_RESTAURAR,
    PANTALLA_SIMULACION_VENTA,
    PANTALLA_MENU_CONECTIVIDAD,
    PANTALLA_TELEMETRIA,
    PANTALLA_PESO, 
    PANTALLA_DIAGNOSTICO,  
    PANTALLA_CAMBIAR_SERIE,
    PANTALLA_INFORMACION,
    PANTALLA_SISTEMAS_PAGO,
    PANTALLA_INFO_GSM,
    PANTALLA_CONF_CONECTIVIDAD,
    PANTALLA_BILLETERO,
    PANTALLA_CONFIG_BANDEJAS,
    PANTALLA_CONFIRMAR_CLAVE,
    PANTALLA_CONFIRMAR_FORMATEO,
    PANTALLA_CONFIRMAR_COPIA,
    PANTALLA_CONFIRMAR_REST,
    PANTALLA_CONFIRMAR_PRODUCTO,
    PANTALLA_CONFIRMAR_BANDEJA,
    PANTALLA_CANCEL_PRODUCTO,
    PANTALLA_MENU_OPERATIVO,
    PANTALLA_CONFIRMAR_SALIDA,
    PANTALLA_MONEDERO,
    PANTALLA_BILLETES_ACEPTADOS,
    PANTALLA_BILLETES_RETENIDOS,
    PANTALLA_BILLETERO_INFO,
    PANTALLA_BILLETERO_ESTADO,
    PANTALLA_MENU_CONFGSM,
    PANTALLA_RECICLADOR, 
    PANTALLA_BILLETES_RECICLADOS,
    PANTALLA_MONEDAS_ACEPTADAS,
    PANTALLA_MONEDERO_INFO,
    PANTALLA_MONEDERO_ESTADO,
    PANTALLA_CONFIGURACION_TEMP,
    PANTALLA_CONFIG_USUARIOS,
    PANTALLA_MENU_HUELLAS,
    PANTALLA_BORRAR_HUELLA,
    PANTALLA_BORRAR_HUELLA_ID,
    PANTALLA_CONFIR_BORRADO_HUELLA,
    PANTALLA_CLAVE_AUTORIZACION,
    PANTALLA_INGRESO_PRECIO,
    PANTALLA_INGRESO_ALTURA,  
    PANTALLA_INGRESO_CANTIDAD,
    PANTALLA_INGRESO_NOMBRE,
    PANTALLA_INGRESO_CEDULA,
    PANTALLA_SALVAPANTALLAS,
    
    PANTALLA_CONTABILIDAD,
    PANTALLA_MENU_CONT_EST,
    PANTALLA_MENU_CONTABILIDAD,
    PANTALLA_MENU_ESTADISTICA,
    PANTALLA_RTOS,
    PANTALLA_MENU_REGISTRO, 
}ventana;

#define label_actualizar_clave1 "GUARDANDO LOS"
#define label_actualizar_clave2 "CAMBIOS..."     
#define label_actualizar_clave3 "ERROR AL"
#define label_actualizar_clave4 "CAMBIAR CLAVE"    
#define label_actualizar_clave5 "CLAVE GUARDADA"
#define label_actualizar_clave6 "CORRECTAMENTE"
#define label_actualizar_clave7 "FALLO, INTENTE"
#define label_actualizar_clave8 "NUEVAMENTE" 
#define label_activi_billete1   "Validador Ocupado"
#define label_activi_billete2   "Billete Recibido"
#define label_activi_billete3   "Billete Retornado"
#define label_activi_billete4   "Validador Desactivado"
#define label_activi_billete5   "Billete Desconocido"
#define label_activi_billete6   "Dispensador Iniciando"
#define label_activi_billete7   "Esperando Retiro"    
#define label_activi_moneda1    "Solicitud Devolucion"    
#define label_adver_canasta1    "DISCULPE CIERRE "
#define label_adver_canasta2    "LA COMPUERTA"
#define label_adver_canasta3    "INFERIOR"    
#define label_adver_canasta4    "Por favor cierre"
#define label_adver_canasta5    "para continuar"
#define label_bluetooth1        "Bluetooth"
#define label_bluetooth2        "Encendido:"  
#define label_bluetooth3        "Configurar"
#define label_bluetooth4        "Informacion" 
#define label_borrarHuella1     "Borrar ID:"    
#define label_brillo_pantalla1  "BRILLO PANTALLA" 
#define label_brillo_pantalla2  "Brillo Automatico:"  
#define label_brillo_pantalla3  "Nivel de Brillo:" 
#define label_billetero1        "BILLETERO"    
#define label_billetero2        "Billetes Aceptados"
#define label_billetero3        "Billetes Retenidos"  
#define label_billetero4        "Informacion"
#define label_billetero5        "Estado Billetero" 
#define label_billetero6        "Reciclador"     
#define label_billeteAceptado1  "ACEPTADOS"
#define label_billeteReciclado1 "RECICLADOS"     
#define label_billeteRetenido1  "RETENIDOS"     
#define label_cancelar1         "DESEA DEVOLVER"  
#define label_cancelar2         "SU DINERO?"
#define label_cancelarMenu1     "DESEA SALIR"
#define label_cancelarMenu2     "DEL MENU?" 
#define label_cancelarConfProd1 "DESEA SALIR SIN"
#define label_cancelarConfProd2 "GUARDAR CAMBIOS?"
#define label_clave1            "Cambio Claves"
#define label_clave2            "Cambiar Clave"
#define label_clave3            "CAMBIAR CLAVES" 
#define label_claveAdmin        "Clave Admin"    
#define label_claveOper         "Clave Operador" 
#define label_claveTesorero     "Clave Tesorero"      
#define label_cambio_clave1     "ATENCION"    
#define label_cambio_clave2     "CLAVE"  
#define label_cambio_clave3     "REINICIADA"
#define label_cambio_clave4     "presione la pantalla"
#define label_cambio_clave5     "para continuar"    
#define label_compra_error1     "DISCULPENOS"    
#define label_compra_error2     "PRODUCTO NO"  
#define label_compra_error3     "DISPONIBLE"
#define label_compra_error4     "presione la pantalla"
#define label_compra_error5     "para continuar" 
#define label_conf_APN          "DEFINIR APN" 
#define label_conf_borraHuella1 "DESEA BORRAR" 
#define label_conf_borraHuella2 "HUELLA ID " 
#define label_conf_borraHuella3 "ID BORRADO" 
#define label_conf_borraHuella4 "CORRECTAMENTE"
#define label_conf_borraHuella5 "ERROR, HUELLA" 
#define label_conf_borraHuella6 "NO REGISTRADA"    
#define label_conf_clave1       "DESEA CAMBIAR"
#define label_conf_clave2       "SU CLAVE?" 
#define label_conf_compra       "CONFIRMAR COMPRA"
#define label_conf_copiar1      "DESEA COPIAR"  
#define label_conf_copiar2      "LOS PARAMETROS?"      
#define label_conf_format1      "DESEA FORMATEAR"
#define label_conf_format2      "LA MEMORIA?"
#define label_conf_GSM1         "CONFIGURAR GSM"
#define label_conf_GSM2         "Definir APN"
#define label_conf_guardar1     "DESEA GUARDAR"
#define label_conf_guardar2     "LOS CAMBIOS?"
#define label_conf_guardar3     "PRODUCTOS"
#define label_conf_guardar4     "ACTUALIZADOS"
#define label_conf_guardar5     "CORRECTAMENTE"        
#define label_conf_guardar6     "FALLO, INTENTE"
#define label_conf_guardar7     "NUEVAMENTE"
#define label_conf_guardar8     "GUARDANDO"
#define label_conf_guardar9     "PRODUCTOS"
#define label_conf_guardar10    "ESPERE..." 
#define label_conf_guardar11    "ERROR AL"
#define label_conf_guardar12    "ACTUALIZAR"
#define label_conf_guardar13    "PRODUCTOS"
#define label_conf_guardar14    "BANDEJAS"
#define label_conf_guardar15    "ACTUALIZADAS"
#define label_conf_guardar16    "CORRECTAMENTE"  
#define label_conf_guardar17    "ERROR AL"
#define label_conf_guardar18    "ACTUALIZAR"
#define label_conf_guardar19    "BANDEJAS"    
#define label_conf_redes1       "Config. Redes"
#define label_conf_redes2       "SIM800 instalado:"    
#define label_conf_parametros1  "DESEA RESTAURAR"    
#define label_conf_parametros2  "LOS PARAMETROS?"
#define label_conf_NFC1         "Configurar NFC"
#define label_conf_NFC2         "NFC Habilitado:"  
#define label_conf_NFC3         "Registro Tarjeta"
#define label_conf_NFC4         "Informacion Tarjeta"
#define label_conf_NFC5         "Recarga Tarjeta"    
#define label_cont_diaria1      "CONTABILIDAD DIARIA" 
#define label_cont_diaria2      "Prod"     
#define label_cont_diaria3      "Usuario" 
#define label_cont_diaria4      "Hora"     
#define label_cont_semanal1     "CONTABILIDAD SEMANAL"
#define label_cont_semanal2     "D/M"    
#define label_cont_semanal3     "Entregas"
#define label_cont_semanal4     "Cant"    
#define label_cont_mensual1     "CONTABILIDAD MENSUAL"
#define label_cont_mensual2     "Dia" 
#define label_cont_mensual3     "Entregas" 
#define label_cont_mensual4     "Cant"     
#define label_cont_anual1       "CONTABILIDAD ANUAL" 
#define label_cont_anual2       "Mes" 
#define label_cont_anual3       "Entregas" 
#define label_cont_anual4       "Cant"     
#define label_copia_parametros1 "REALIZANDO"    
#define label_copia_parametros2 "UNA COPIA DE"      
#define label_copia_parametros3 "PARAMETROS..."
#define label_copia_parametros4 "PARAMETROS"    
#define label_copia_parametros5 "COPIADOS"      
#define label_copia_parametros6 "CORRECTAMENTE"
#define label_copia_parametros7 "guardado en carpeta"    
#define label_copia_parametros8 "CONFIGURACION"   
#define label_copia_parametros9 "ERROR AL"    
#define label_copia_parametros10 "COPIAR LOS"      
#define label_copia_parametros11 "PARAMETROS" 
#define label_copia_parametros12 "FALLO, INTENTE"
#define label_copia_parametros13 "NUEVAMENTE"
#define label_devolucionDinero1  "DEVOLVIENDO"    
#define label_devolucionDinero2  "DINERO"
#define label_devolucionDinero3  ""  
#define label_devolucionDinero4  "Presione la pantalla" 
#define label_devolucionDinero5  "para cancelar"
#define label_devolucionDinero6  "para salir"    
#define label_devolucionDinero7  "DEVOLUCION"    
#define label_devolucionDinero8  "FINALIZADA"    
#define label_devolucion_manual1 "DEVOLUCION"
#define label_devolucion_manual2 "REGISTRADA"
#define label_devolucion_manual3 "CORRECTAMENTE" 
#define label_devolucion_manual4 "NO SE PUDO"
#define label_devolucion_manual5 "REGISTRAR"
#define label_devolucion_manual6 "DEVOLUCION"      
#define label_diagnostico1      "DIAGNOSTICO"
#define label_diagnostico2      "Sensor de Peso"  
#define label_diagnostico3      "Sistema Operativo" 
#define label_diagnostico4      "Estado de Circuitos" 
#define label_diagnostico5      "Calibrar Pantalla"     
#define label_entrega1          "ENTREGANDO"
#define label_entrega2          "PRODUCTO POR" 
#define label_entrega3          "FAVOR ESPERE"    
#define label_entrega4          "NO abra la compuerta"
#define label_entrega5          "inferior, gracias"
#define label_entregado1        "GRACIAS POR"
#define label_entregado2        "SU COMPRA"   
#define label_entregado3        "Presione la pantalla"
#define label_entregado4        "para continuar"   
#define label_error1            ""
#define label_error2            "ERROR!!" 
#define label_error_cambio1     "LO SENTIMOS, NO"
#define label_error_cambio2     "TENEMOS CAMBIO" 
#define label_error_cambio3     "SUFICIENTE"
#define label_error_cambio4     "Ingrese un billete"  
#define label_error_cambio5     "de menor valor" 
#define label_error_entrega1    "LO SENTIMOS"
#define label_error_entrega2    "HA OCURRIDO"
#define label_error_entrega3    "UN ERROR"    
#define label_error_entrega4    "seleccione otro"
#define label_error_entrega5    "producto, gracias" 
#define label_error_entrega6    "Intenta nuevamente"    
#define label_error_Maquina1    "DISCULPE" 
#define label_error_Maquina2    "FUERA DE"
#define label_error_Maquina3    "SERVICIO"
#define label_error_Maquina4    "Servicio No Disponible" 
#define label_error_recarga1    "DISCULPE"  
#define label_error_recarga2    "RECARGA"  
#define label_error_recarga3    "RECHAZADA"  
#define label_error_recarga4    "Error:"      
#define label_espacio1          "Espacio en memoria:" 
#define label_espera_NFC1       "POR FAVOR" 
#define label_espera_NFC2       "ACERQUE SU" 
#define label_espera_NFC3       "TARJETA "
#define label_espera_NFC4       "Se devolvera" 
#define label_espera_NFC5       "su saldo inicial"
#define label_espera_NFC6       "GRACIAS!"
#define label_espera_NFC7       "POR FAVOR"
#define label_espera_NFC8       "ESPERE" 
#define label_espera_NFC9       "Saldo: %d" 
#define label_espera_NFC10      "Tarjeta: %s" 
#define label_espera_NFC11      "NO TIENE"
#define label_espera_NFC12      "SALDO"
#define label_espera_NFC13      "SUFICIENTE"    
#define label_estadoBilletero1  "ESTADO BILLETERO"
#define label_estadoBilletero2  "Descripcion"
#define label_estadoBilletero3  "Valor"
#define label_estadoBilletero4  " Cajon Lleno"
#define label_estadoBilletero5  " Cantidad Billetes"
#define label_estadoBilletero6  " Actividad"    
#define label_estadoBilletero7  " Codigo Errores"    
#define label_estadoBilletero8  " Causa"
#define label_estadoBilletero9  " Cant. en Reciclador"
#define label_estadoMonedero1   "ESTADO MONEDERO"
#define label_estadoMonedero2   "Descripcion"
#define label_estadoMonedero3   "Valor"
#define label_estadoMonedero4   " Cantidad Monedas"
#define label_estadoMonedero5   " Total"    
#define label_estadoMonedero6   " Actividad"    
#define label_estadoMonedero7   " Codigo Errores"    
#define label_estadoMonedero8   " Causa"   
#define label_estado_dispensador1 "Calibrando, espere..."  
#define label_estado_dispensador2 "Sin tarjeta motores" 
#define label_formato1          "MEMORIA" 
#define label_formato2          "FORMATEADA"    
#define label_formato3          "CORRECTAMENTE" 
#define label_formato4          "ERROR AL"    
#define label_formato5          "FORMATEAR LA"    
#define label_formato6          "MEMORIA SD"    
#define label_formato7          "FORMATEANDO"
#define label_formato8          "MEMORIA"
#define label_formato9          "ESPERE..."
#define label_formato10         "FALLO, INTENTE"
#define label_formato11         "NUEVAMENTE" 
#define label_habilitado1       "INICIANDO EL"
#define label_habilitado2       "SISTEMA INTENTA" 
#define label_habilitado3       "MAS TARDE" 
#define label_habilitado4       "Vuelve a intentar" 
#define label_habilitado5       "en unos minutos"    
#define label_informacion1      "INFORMACION" 
#define label_informacion2      "Descripcion" 
#define label_informacion3      "Valor"  
#define label_informacion4      " Nombre"
#define label_informacion5      " Serie"   
#define label_informacion6      " Version Software" 
#define label_informacion7      " Version Maquina"  
#define label_informacion8      " Capacidad" 
#define label_informacion9      " Fecha Fabricacion"    
#define label_informacion10     " Horas de Trabajo" 
#define label_informacion11     " Version Pantalla"     
#define label_infoBilletero1    "INFORMACION BILLETERO"
#define label_infoBilletero2    "Descripcion"
#define label_infoBilletero3    "Valor"
#define label_infoBilletero4    " Nivel"  
#define label_infoBilletero5    " Pais" 
#define label_infoBilletero6    " Factor Escala"
#define label_infoBilletero7    " Factor Decimal"
#define label_infoBilletero8    " Capacidad"
#define label_infoBilletero9    " Retencion"
#define label_infoBilletero10   " Serial"
#define label_infoBilletero11   " Modelo" 
#define label_infoBilletero12   " Version"   
#define label_infoBilletero13   " Soporta FTL"  
#define label_infoBilletero14   " Reciclador"
#define label_infoBilletero15   " Reciclar billetes"      
#define label_infoBluetooth1    "INFORMACION BLUETOOTH"
#define label_infoBluetooth2    "Descripcion"
#define label_infoBluetooth3    "Valor"  
#define label_infoBluetooth4    " Nombre"
#define label_infoBluetooth5    " Direccion"
#define label_infoBluetooth6    " Version"  
#define label_infoBluetooth7    " Rol" 
#define label_infoBluetooth8    "Esclavo"
#define label_infoBluetooth9    "Maestro"
#define label_infoBluetooth10   "Ciclo Esclavo"  
#define label_infoBluetooth11   "Error" 
#define label_infoBluetooth12   " Velocidad" 
#define label_infoBluetooth13   " PIN"  
#define label_infoBluetooth14   " Bits Parada" 
#define label_infoBluetooth15   " Paridad"   
#define label_infoBluetooth16   " Clase"
#define label_infoBluetooth17   "NO" 
#define label_infoBluetooth18   "ODD"   
#define label_infoBluetooth19   "EVEN" 
#define label_infoGSM1          "INFORMACION TELEMETRIA"
#define label_infoGSM2          "Descripcion"
#define label_infoGSM3          "Valor"
#define label_infoGSM4          " Fabricante" 
#define label_infoGSM5          " Registro en red"  
#define label_infoGSM6          " Potencia senal"
#define label_infoGSM7          " Modelo" 
#define label_infoGSM8          " IMEI"    
#define label_infoGSM9          " IMSI DE SIM"
#define label_infoGSM10         " Estado GPRS" 
#define label_infoGSM11         " Operador" 
#define label_infoGSM12         " Estado Conexion"   
#define label_infoGSM13         " IP Local"
#define label_infoGSM14         " APN"    
#define label_infoGPRS0         "Desactivado" 
#define label_infoGPRS1         "Enlazado"
#define label_infoGPRS2         "Desconocido"
#define label_infoMifare1       "INFORMACION TARJETA"
#define label_infoMifare2       "Descripcion"
#define label_infoMifare3       "Valor"
#define label_infoMifare4       " UID"
#define label_infoMifare5       " Sens"
#define label_infoMifare6       " Sel" 
#define label_infoMifare7       " Identificacion" 
#define label_infoMifare8       " Codigo" 
#define label_infoMifare9       " Credito"  
#define label_infoMonedero1     "INFORMACION MONEDERO"
#define label_infoMonedero2     "Descripcion"
#define label_infoMonedero3     "Valor"
#define label_infoMonedero4     " Nivel"  
#define label_infoMonedero5     " Pais" 
#define label_infoMonedero6     " Factor Escala"
#define label_infoMonedero7     " Factor Decimal"
#define label_infoMonedero8     " Serial"
#define label_infoMonedero9     " Modelo" 
#define label_infoMonedero10    " Version"   
#define label_infoMonedero11    " Soporta FTL"  
#define label_infoMonedero12    " Devolucion automatica"    
#define label_infoReg0          "Desconectado"
#define label_infoReg1          "Conectado"
#define label_infoReg2          "Buscando"    
#define label_infoReg3          "Denegado"
#define label_infoReg4          "Desconocido"
#define label_infoReg5          "Roaming"
#define label_ingresoAltura     "Altura (mm)" 
#define label_ingresoCantidad   "Cantidad" 
#define label_ingresoPrecio     "Precio"
#define label_ingresoNombre     "Nombre"
#define label_ingresoCedula     "Cedula"    
#define label_lecturaHuella1    "Huella encontrada"
#define label_lecturaHuella2    "con identificador"    
#define label_lecturaHuella3    "Huella no esta" 
#define label_lecturaHuella4    "registrada" 
#define label_lecturaHuella5    "Ningun dedo"  
#define label_lecturaHuella6    "ha sido detectado"     
#define label_luz_maquina1      "LUZ DE PUERTA" 
#define label_luz_maquina2      "Luz Activa:"  
#define label_luz_maquina3      "Nivel de Luz:"
#define label_memoria1          "Tarjeta de Memoria"   
#define label_memoria2          "Formatear Memoria"
#define label_memoria3          "Espacio Utilizado"  
#define label_memoria4          "Copiar Configuracion"  
#define label_memoria5          "Restaurar Config"
#define label_menu1             "MENU FABRICANTE"  
#define label_menu2             "MENU ADMINISTRADOR"
#define label_menu3             "MENU OPERADOR" 
#define label_menu4             "MENU TESORERO"     
#define label_menu_conect1      "Conectividad" 
#define label_menu_conect2      "Telemetria"
#define label_menu_conect3      "Red EKIA" 
#define label_menu_conect4      "Configurar Redes"     
#define label_menu_cont_est1    "INFO VENTAS"
#define label_menu_cont_est2    "Ver contabilidad"
#define label_menu_cont_est3    "Ver registro"    
#define label_menu_contabilidad1 "CONTABILIDAD"
#define label_menu_contabilidad2 "Diaria"
#define label_menu_contabilidad3 "Semanal"    
#define label_menu_contabilidad4 "Mensual" 
#define label_menu_contabilidad5 "Anual"  
#define label_menu_estadistica1 "ESTADISTICAS"
#define label_menu_estadistica2 "Grafica diaria" 
#define label_menu_estadistica3 "Grafica semanal"     
#define label_menu_estadistica4 "Grafica mensual" 
#define label_menu_estadistica5 "Grafica anual" 
#define label_menu_iluminacion1 "ILUMINACION"
#define label_menu_iluminacion2 "Brillo de Pantalla"
#define label_menu_iluminacion3 "Luz de Puerta"
#define label_menu_usuarios1    "Usuarios"
#define label_menu_usuarios2    "Registrar Usuario" 
#define label_menu_usuarios3    "Verificar Usuario"     
#define label_menu_usuarios4    "Borrar Usuario"   
#define label_menu_usuarios5    "Administrar Huella" 
#define label_menu_huella1      "Huella Dactilar"    
#define label_menu_huella2      "Registro Usuario"
#define label_menu_huella3      "Leer Huella"
#define label_menu_huella4      "Verificar Huella"
#define label_menu_huella5      "Borrar Huella" 
#define label_menu_huella6      "Registro Administrador"    
#define label_modo_trabajo1     "Modo de Trabajo"
#define label_modo_trabajo2     "Punto Red"
#define label_monedero1         "MONEDERO"    
#define label_monedero2         "Monedas Aceptadas" 
#define label_monedero3         "Informacion"
#define label_monedero4         "Estado Monedero"   
#define label_motores1          "Productos y Motores" 
#define label_motores2          "Definir Productos"     
#define label_motores3          "Test de motores"  
#define label_motores4          "Simulacion de venta" 
#define label_motores5          "Gestion Usuarios"     
#define label_motores6          "Configurar Bandejas"     
#define label_movimiento_motor1 "Prueba de motores"
#define label_movimiento_motor2 "Test Completo" 
#define label_movimiento_motor3 "Test Individual:"
#define label_noDisponible1     "DISCULPE"
#define label_noDisponible2     "COMPONENTE"
#define label_noDisponible3     "NO INSTALADO"
#define label_noDisponible4     "Presione la pantalla"
#define label_noDisponible5     "para continuar"    
#define label_nota_monedero     "No hay monedero detectado"
#define label_nota_billetero    "No hay billetero detectado" 
#define label_nota_mdb          "No hay modulos de pago detectados" 
#define label_nota_memoria      "No hay memoria insertada"
#define label_nota_memoriaSD1   "DISCULPE NO" 
#define label_nota_memoriaSD2   "HAY UNA MEMORIA" 
#define label_nota_memoriaSD3   "SD INSERTADA"  
#define label_nota_telemetria1  "DISCULPE, NO" 
#define label_nota_telemetria2  "HAY TELEMETRIA" 
#define label_nota_telemetria3  "INSTALADA" 
#define label_pagos1            "Ingrese su Dinero"  
#define label_pais1             "Colombia"     
#define label_principal1        "BIENVENIDO"
#define label_principal2        "INGRESE"
#define label_principal3        "SU DINERO"  
#define label_principal4        "Credito: %d"
#define label_principal5        "Producto:"
#define label_principal6        "" 
#define label_puntoRed1         "Punto Red"
#define label_puntoRed2         "Habilitado:"    
#define label_producto          "PRODUCTO: %d" 
#define label_recarga1          "REALIZANDO"
#define label_recarga2          "RECARGA POR" 
#define label_recarga3          "FAVOR ESPERE" 
#define label_recargado1        "RECARGA"
#define label_recargado2        "EXITOSA"   
#define label_recargado3        ""
#define label_recargado4        "Identificador:" 
#define label_recargado5        "No.: " 
#define label_reciclador1       "RECICLADOR"
#define label_reciclador2       "Habilitado" 
#define label_reciclador3       "Vaciar Reciclador"
#define label_reciclador4       "Billetes Reciclados"  
#define label_registro1         "ACTUALIZANDO"
#define label_registro2         "TARJETA"
#define label_registro3         "ESPERE..."
#define label_registro4         "TARJETA"
#define label_registro5         "ACTUALIZADA"
#define label_registro6         "CORRECTAMENTE" 
#define label_registro7         "ERROR"
#define label_registro8         "ACTUALIZANDO"
#define label_registro9         "TARJETA" 
#define label_registro_devol    "DEVOLUCION"
#define label_registroHuella1   "Registrando Huella"  
#define label_registroHuella2   "Coloque el dedo"  
#define label_registroHuella3   "por favor"
#define label_registroHuella4   "Levante el dedo" 
#define label_registroHuella5   "Nuevamente"  
#define label_registroHuella6   "una vez mas" 
#define label_registroHuella7   "Error registrando"
#define label_registroHuella8   "la huella con" 
#define label_registroHuella9   "Huella registrada" 
#define label_registroHuella10  "correctamente con"  
#define label_registroUsuario1 "Escribir Nombre"
#define label_registroUsuario2 "Escribir Identificacion"
#define label_registroUsuario3 "Registrar Huella"    
#define label_rest_parametros1  "RESTAURANDO"    
#define label_rest_parametros2  "LOS PARAMETROS"
#define label_rest_parametros3  "DESDE LA SD..."  
#define label_rest_parametros4  "PARAMETROS"    
#define label_rest_parametros5  "RESTAURADOS"
#define label_rest_parametros6  "CORRECTAMENTE" 
#define label_rest_parametros7  "Parametros guardados"
#define label_rest_parametros8  "en memoria interna"
#define label_rest_parametros9  "ERROR AL" 
#define label_rest_parametros10 "RESTAURAR LOS"
#define label_rest_parametros11 "PARAMETROS"  
#define label_rest_parametros12 "FALLO, INTENTE"
#define label_rest_parametros13 "NUEVAMENTE" 
#define label_rest_parametros14 "Verifique el archivo"
#define label_rest_parametros15 "de configuracion"
#define label_saldo_error1      "DISCULPE, SALDO" 
#define label_saldo_error2      "INSUFICIENTE"
#define label_saldo_error3      "Falta: "    
#define label_saldo_error4      "ingrese mas dinero"
#define label_saldo_error5      "e intente nuevamente"  
#define label_Serie1            "Cambiar Serie"  
#define label_servicios1        "SERVICIO NO"
#define label_servicios2        "DISPONIBLE"
#define label_servicios3        "ESPERALO!"
#define label_servicios4        "Pronto podras pagar"
#define label_servicios5        "tus recibos aqui"  
#define label_servicios6        "Aqui Podras recargar"
#define label_servicios7        "tu celular muy pronto"     
#define label_simulacion1       "SIMULACION VENTA" 
#define label_simulacion2       "Producto:"
#define label_simulacion3       "SIMULANDO" 
#define label_simulacion4       "ENTREGA DE"
#define label_sistemas_pago1    "Sistemas de Pago"   
#define label_sistemas_pago2    "Reiniciar Sistemas"  
#define label_sistemas_pago3    "Billetero"
#define label_sistemas_pago4    "Monedero"     
#define label_sistemaOperativo1 "MEMORIA LIBRE"  
#define label_sistemaOperativo2 "Descripcion"  
#define label_sistemaOperativo3 "Valor"      
#define label_telemetria1       "Telemetria"
#define label_telemetria2       "Activada:" 
#define label_telemetria3       "Configurar"
#define label_telemetria4       "Informacion"    
#define label_test_completo0    "TEST MOTORES"    
#define label_test_completo1    "MOVIENDO"
#define label_test_completo2    "MOTOR"  
#define label_test_completo3    "TEST MOTORES"  
#define label_test_completo4    "FINALIZADO" 
#define label_test_completo5    "Presione la pantalla" 
#define label_test_completo6    "para cancelar"    
#define label_vacio             " "                           

#define imagenNull              "20"    
#define imagenEsperaON          "0"
#define imagenEsperaOFF         "50"     
#define imagenOk                "35"
#define imagenAlerta            "34"
#define imagenError             "36" 
#define imagenPuerta            "6"  
    
#define boton1                      (1)
#define boton2                      (2)
#define boton3                      (3)
#define boton4                      (4)    
#define boton5                      (5)    
#define botonMover                  (6)  
#define botonVolver                 (7) 
    
#define CONTABILIDAD_DIARIA         (1)
#define CONTABILIDAD_SEMANAL        (2)
#define CONTABILIDAD_MENSUAL        (3)
#define CONTABILIDAD_ANUAL          (4)   

#define FABRICANTE                  (5)
#define ADMINISTRADOR               (4)
#define PROVEEDOR                   (3)
#define CLIENTE                     (2)    
#define TEMPORAL                    (1)     
    
#define VENTANA_BIENVENIDA      (0)
#define VENTANA_PRINCIPAL       (1)
#define VENTANA_CLAVE           (2)
#define VENTANA_MENU            (3)
#define VENTANA_CONFIRMACION    (4)    
#define VENTANA_CONFIGURACION   (5)
#define VENTANA_CONF_GENERAL    (6)    
#define VENTANA_NOTIFICACIONES  (7)
#define VENTANA_ALFANUMERICO    (8)    
#define VENTANA_INFORMACION     (9)    
#define VENTANA_BANDEJAS        (10)    
#define VENTANA_PRODUCTOS       (11)   
#define VENTANA_SALVAPANTALLAS  (12)
#define VENTANA_NUMERICO        (13)
#define VENTANA_REGISTRO        (14)    
#define VENTANA_TEMPERATURA     (15) 
#define VENTANA_CONFIGURTEMP    (16)
#define VENTANA_PESO            (17)   
#define VENTANA_FECHA_HORA      (18)
    
#define VENTANA_CONTABILIDAD    (20)
#define VENTANA_SELECCION       (19)
#define VENTANA_SISTEMA_OPER    (13)    
       
    
/*=====================================================
;Funciones de la interface de usuario
====================================================*/     
void interface_Init();
    
#endif
/* [] FIN DE ARCHIVO */