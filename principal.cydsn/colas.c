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
 * ===================================================
;PROGRAMA PARA LA CREACION DE LOS QUEUES UTILIZADOS 
====================================================*/
#include "FreeRTOS.h"
#include "semphr.h" 
#include "queue.h"
#include "colas.h"
#include "cypins.h"

#include "MDB_Tipos.h"
#include "MDB_monedero.h"
#include "MDB_billetero.h"
#include "tipos.h"


/*Esta Funcion genera las colas necesarias de comunicación entre tareas
y para el intercambio de informaciónC*/

void colas_Init(){
    debugMessageQ = xQueueCreate(20, sizeof(debug_print_data_t));
    cUSB = xQueueCreate(10, sizeof(int));
    funcion = xQueueCreate(1, sizeof(xcomandoUSB));
    funcionI2C = xQueueCreate(1, sizeof(xcomandoUSB));
    comandosSIM = xQueueCreate(3,sizeof(xComandosSIM));
    //minutosFuncionando = xQueueCreate(1, sizeof(uint8));
    cadena = xQueueCreate(1, sizeof(xcadenas));
    cadena2 = xQueueCreate(10, sizeof(xcadenas));
    notificaciones = xQueueCreate(1, sizeof(uint8));
    xQueueReset(notificaciones);
    comandosHMI = xQueueCreate(4,sizeof(xcomandoHMI));
    xQueueReset(comandosHMI);
    //productosConfigurados = xQueueCreate(1,sizeof(xproductosMaquina));
    datosGSM = xQueueCreate(1, sizeof(xInformacionGSM));
    xQueueReset(datosGSM);
   
    
    estadoRTOS = xQueueCreate(1,sizeof(uint8));
    estadoMemoriaSD = xQueueCreate(1,sizeof(uint8));
    estadoConfiguraciones = xQueueCreate(1,sizeof(uint8));
    
    respuestaPantalla = xQueueCreate(1,sizeof(xresPantalla));
    configuracionesMaquina = xQueueCreate(1,sizeof(xConfiguracion));
    xQueueReset(configuracionesMaquina);
    configuracionesRemotas = xQueueCreate(1,sizeof(xConfiguracionTelemetria));
    xQueueReset(configuracionesRemotas);
    configuracionesBandejas = xQueueCreate(1,sizeof(xConfiguracionBandejas));
    xQueueReset(configuracionesBandejas);
    configuracionesAccesorios = xQueueCreate(1,sizeof(xConfiguracionAccesorios));
    xQueueReset(configuracionesAccesorios);
    actualizarInformacion = xQueueCreate(10,sizeof(xContabilidad));
    xQueueReset(actualizarInformacion);
    actualizarRegistro = xQueueCreate(5,sizeof(xEventos));
    xQueueReset(actualizarRegistro);
    enviarRegistro = xQueueCreate(5,sizeof(xEventos));
    xQueueReset(enviarRegistro);
    configuracionesTemperatura = xQueueCreate(1,sizeof(xConfiguracionTemperatura));
    xQueueReset(configuracionesTemperatura);
    
    autorizaciones = xQueueCreate(3,sizeof(xAutorizacion));
    xQueueReset(autorizaciones);
    autorizacionProveedor = xQueueCreate(3,sizeof(xProveedor));
    xQueueReset(autorizacionProveedor);
    
    datosDescargados = xQueueCreate(3,sizeof(xinfoDescargada));
    //salidaTelemetria;
    //entradaTelemetria;
}
/* [] FIN DEL ARCHIVO */
