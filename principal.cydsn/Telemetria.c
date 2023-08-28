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
 * ============================== ================================
;FUNCIONES PARA MANEJO DEL SISTEMA TELEMETRIA
===============================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cyapicallbacks.h"

#include "EKIA_CONFIG.h"
#include "Telemetria.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"
#include "LED_DEBUG.h"
#include "memEEPROM.h"
#include "MDB.h"
#include "MDB_Tipos.h"
#include "dispensador.h"
#include "memoriaExt.h"
#ifdef MODULO_SIM800L
    #include "SIM800L_SIM.h"
    #include "SIM800L_SIM_REG.h"
    #include "SIM800L_MQTT.h"
    #include "SIM800L.h"
    #include "SIM_PinReset.h"
#endif    

#include "BuzzerPWM.h"

#define periodo 20000
#define periodo2 30000
#define periodo3 50000
#define compar 150

TaskHandle_t xTelemetria = NULL;
 
void actualizarMetadatos(){
    xConfiguracion parametros;
    char metadatos[200];
    xQueuePeek(configuracionesMaquina,(void*)&parametros,10);
    sprintf(metadatos,"{\"d\":{\"supports\":{\"deviceActions\": 1, \"firmwareActions\": 1},\"deviceInfo\":{\"serialNumber\":\"%s\",\"fwVersion\":\"%s\",\"hwVersion\":\"%s\"}},\"reqId\":\"%s\",\"deviceId\":\"DOROTI-%s\"}",
        parametros.Serie.numSerie,
        VERSION_SOFTWARE,
        parametros.Serie.version,
        parametros.Serie.serie,
        NUMERO_DE_SERIE
    );    
    SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_GESTION,metadatos);
}
/*******************************************************************************
* en recepción de mensaje del broker
********************************************************************************
*
* Tarea que realiza:
*  Funcion que se ejecuta siempre que se reciba un mensaje del broker
*
*******************************************************************************/
void publicarMensajeOk(int messageId){
    char string[100];
    xConfiguracion parametros;
    xQueuePeek(configuracionesMaquina,(void*)&parametros,10);
    sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"message\":\"%s\",\"deviceId\":\"DOROTI-%s\"}}",
            messageId,
            200,
            "OK",
            parametros.Serie.serie
        );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
}

void publicarMensajeError(uint8 tipo, int messageId){
    char string[100];
    xConfiguracion parametros;
    xQueuePeek(configuracionesMaquina,(void*)&parametros,10);
    switch (tipo){
        case TOPICO_PERDIDO:
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"message\":\"%s\",\"deviceId\":\"DOROTI-%s\"}}",
                messageId,
                4,
                "ERROR",
                parametros.Serie.serie
            );
        break;
        case COMANDO_PERDIDO:
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"message\":\"%s\",\"deviceId\":\"DOROTI-%s\"}}",
                messageId,
                5,
                "ERROR",
                parametros.Serie.serie
            );
        break;
    }
    SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);

}

void SIM800L_MQTT_enMensaje(char *Topico, int LongitudTopico, char *Mensaje, int LongitudMensaje){
    CYBIT comando_ok = pdFALSE;
    const char delimitadores[8] = "{}:,[]\"";
    char* token = 0;
    int numero = 0, mensajeId = 0, columna=0, bandeja=1;
    char string[200];
    char usuario[30];
    char topicoEntrada[30]; 
    xConfiguracionTelemetria parametros;
    xMedidaDispensador temperaturaLeida, humedadLeida;
    xConfiguracion configuraciones;
    xConfiguracionTemperatura configuracionTemp;
    xConfiguracionBandejas confBandejas;
    xAutorizacion autorizacion;
    xProveedor proveedor;
    xProducto producto;
    xConfiguracion parametrosMaquina;
    
    xQueuePeek(configuracionesMaquina,(void*)&parametrosMaquina,10);
    xQueuePeek(configuracionesRemotas,(void*)&parametros,1);
    
    sprintf(topicoEntrada,"%s%s",TOPICO_COMANDOS,parametrosMaquina.Serie.serie);    
    if(!strcmp(Topico,topicoEntrada)){
        token = strtok(Mensaje, delimitadores); //{\"messageId:\"
        token = strtok(NULL, delimitadores);
        mensajeId = atoi(token);
        token = strtok(NULL, delimitadores); //{\"command:\"
        token = strtok(NULL, delimitadores);// \"%s\"
        if(!strcmp(token,COMANDO_CONTROLLED)){
            token = strtok(NULL, delimitadores); // ,\"status\":
            token = strtok(NULL, delimitadores); // \"%d\"
            LED_DEBUG_Write(atoi(token));
            publicarMensajeOk(mensajeId);
        }
        else if (!strcmp(token,COMANDO_LEER_HUMEDAD)){
            while(!xSemaphoreTake(busOcupado,10));
            humedadLeida = Dispensador_LeerHumedad(3);
            xSemaphoreGive(busOcupado);
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"humedadActual\":\"%.3f\",\"deviceId\":\"DOROTI-%s\"}}",
                mensajeId,
                200,
                humedadLeida.medida,
                parametrosMaquina.Serie.serie
            );
            SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
        }
        else if (!strcmp(token,COMANDO_LEER_TEMP)){
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,1);
            while(!xSemaphoreTake(busOcupado,10));
            temperaturaLeida = Dispensador_LeerTemperatura(3);
            xSemaphoreGive(busOcupado);
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"temperaturaActual\":\"%.3f\",\"temperaturaDefinida\":\"%.3f\",\"estado\":\"%d\",\"descanso\":\"%d\",\"deviceId\":\"DOROTI-%s\"}}",
                mensajeId,
                200,
                temperaturaLeida.medida,
                configuracionTemp.refrigeracion.gradosC/100.0f,
                configuracionTemp.refrigeracion.activado,
                configuracionTemp.refrigeracion.descanso,
                parametrosMaquina.Serie.serie
            );
            SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
        }
        else if (!strcmp(token,COMANDO_LEER_CONF_TEMP)){
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,1);
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"horasDescanso\":\"%d\",\"minutosDescanso\":\"%d\",\"histeresisSuperior\":\"%d\",\"histeresisInferior\":\"%d\",\"deviceId\":\"DOROTI-%s\"}}",
                mensajeId,
                200,
                configuracionTemp.HorasDescanso,
                configuracionTemp.MinutosDescanso,
                configuracionTemp.HisteresisSuper,
                configuracionTemp.HisteresisInfer,
                parametrosMaquina.Serie.serie
            );
            SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
        }
        else if (!strcmp(token,COMANDO_DEFINIR_TEMP)){
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            token = strtok(NULL, delimitadores); // ,\"estado\":
            token = strtok(NULL, delimitadores); // \"%d\"
            configuracionTemp.refrigeracion.activado = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"grados\":
            token = strtok(NULL, delimitadores); // \"%d\"
            configuracionTemp.refrigeracion.gradosC = atoi(token);
            xQueueOverwrite(configuracionesMaquina,(void*)&configuraciones);
            while(!xSemaphoreTake(memoriaOcupada,100));
            escribirRefrigeracion(configuracionTemp.refrigeracion);
            xSemaphoreGive(memoriaOcupada);
            while(!xSemaphoreTake(busOcupado,10));
            Dispensador_definir_temperatura(configuracionTemp.refrigeracion.gradosC);
            vTaskDelay(pdMS_TO_TICKS(10));
            xSemaphoreGive(busOcupado);
            publicarMensajeOk(mensajeId);
        }
        else if (!strcmp(token,COMANDO_DEFINIR_CONF_TEMP)){
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            token = strtok(NULL, delimitadores); // ,\"estado\":
            token = strtok(NULL, delimitadores); // \"%d\"
            configuracionTemp.refrigeracion.activado = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"grados\":
            token = strtok(NULL, delimitadores); // \"%d\"
            configuracionTemp.refrigeracion.gradosC = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"grados\":
            token = strtok(NULL, delimitadores); // \"%d\"
            configuracionTemp.refrigeracion.gradosC = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"grados\":
            token = strtok(NULL, delimitadores); // \"%d\"
            configuracionTemp.refrigeracion.gradosC = atoi(token);
            xQueueOverwrite(configuracionesMaquina,(void*)&configuraciones);
            while(!xSemaphoreTake(memoriaOcupada,100));
            escribirRefrigeracion(configuracionTemp.refrigeracion);
            xSemaphoreGive(memoriaOcupada);
            while(!xSemaphoreTake(busOcupado,10));
            Dispensador_definir_temperatura(configuracionTemp.refrigeracion.gradosC);
            vTaskDelay(pdMS_TO_TICKS(10));
            xSemaphoreGive(busOcupado);
            publicarMensajeOk(mensajeId);
        }
        else if (!strcmp(token,COMANDO_DEFINIR_PRODUCTO)){
            token = strtok(NULL, delimitadores); // ,\"producto\":
            token = strtok(NULL, delimitadores); // \"%d\"
            producto.numero = atoi(token);
            while(!xSemaphoreTake(memoriaOcupada,100));
            producto = leerProducto(producto.numero);
            xSemaphoreGive(memoriaOcupada);
            token = strtok(NULL, delimitadores); // ,\"estado\":
            token = strtok(NULL, delimitadores); // \"%d\"
            producto.habilitado = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"precio\":
            token = strtok(NULL, delimitadores); // \"%d\"
            producto.precio = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"cantidad\":
            token = strtok(NULL, delimitadores); // \"%d\"
            producto.cantidad = atoi(token);
            while(!xSemaphoreTake(memoriaOcupada,100));
            escribirProducto(producto);
            xSemaphoreGive(memoriaOcupada);
            publicarMensajeOk(mensajeId);
        }
        else if (!strcmp(token,COMANDO_DEFINIR_PROVEEDOR)){
            token = strtok(NULL, delimitadores); // ,\"proveedor\":
            for(numero=0;numero<(NUMERO_PRODUCTOS_BANDEJA_MAX*NUMERO_BANDEJAS_MAX);numero++){
                comando_ok = pdTRUE;
                token = strtok(NULL, delimitadores); // \"%d\"
                //string[numero] = atoi(token);
                while(!xSemaphoreTake(memoriaOcupada,100));
                producto = leerProducto(bandeja*100+columna);
                producto.proveedor = atoi(token);
                if(!escribirProducto(producto)){
                    comando_ok = pdFALSE;
                }
                xSemaphoreGive(memoriaOcupada);
                columna++;
                if(columna>=NUMERO_PRODUCTOS_BANDEJA_MAX){
                    columna=0;
                    bandeja++;
                }
                if(bandeja>NUMERO_BANDEJAS_MAX){
                    bandeja=1;
                }
            }
            if(comando_ok){
                publicarMensajeOk(mensajeId);
            }
            else{
                publicarMensajeError(COMANDO_ERROR,mensajeId);
            }
        }
        else if (!strcmp(token,COMANDO_LEER_PRODUCTO)){
            token = strtok(NULL, delimitadores); // ,\"producto\":
            token = strtok(NULL, delimitadores); // \"%d\"
            producto.numero = atoi(token);
            while(!xSemaphoreTake(memoriaOcupada,100));
            producto = leerProducto(producto.numero);
            xSemaphoreGive(memoriaOcupada);
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"producto\":\"%d\",\"precio\":\"%f\",\"cantidad\":\"%d\",\"habilitado\":\"%d\",\"altura\":\"%hu\",\"deviceId\":\"DOROTI-%s\"}}",
                mensajeId,
                200,
                producto.numero,
                producto.precio,
                producto.cantidad,
                producto.habilitado,
                producto.alturaXItem,
                parametrosMaquina.Serie.serie
            );
            SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
        }
        else if (!strcmp(token,COMANDO_LEER_BANDEJA)){
            uint8 bandeja=0;
            uint8 indice=0;
            char cadenaBandeja[60];
            char cadenatemp[10];
            token = strtok(NULL, delimitadores); // ,\"bandeja\":
            token = strtok(NULL, delimitadores); // \"%d\"
            bandeja = atoi(token);
            xQueuePeek(configuracionesBandejas,(void*)&confBandejas,1);
            if(confBandejas.configBandejas.numBandejas>=bandeja){
                while(!xSemaphoreTake(memoriaOcupada,100));
                strcpy(cadenaBandeja," ");
                for(indice=0;indice<confBandejas.configBandejas.numMotores[bandeja-1];indice++){
                    producto.numero = (bandeja*100)+indice;
                    producto = leerProducto(producto.numero);
                    sprintf(cadenatemp,"\"%d\":\"%d\",",producto.numero,producto.cantidad);
                    strcat(cadenaBandeja,cadenatemp);
                }
                indice=0;
                while (cadenaBandeja[++indice] != 0);
                cadenaBandeja[indice-1]=' ';
                xSemaphoreGive(memoriaOcupada);
                sprintf(string,"{\"messageId\":%d,\"d\":{\"code\":%d,\"deviceId\":\"DOROTI-%s\",\"bandeja\":{%s}}}",
                    mensajeId,
                    200,
                    parametrosMaquina.Serie.serie,
                    cadenaBandeja
                );
                SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
            }
            else{
                publicarMensajeError(COMANDO_PERDIDO, mensajeId);
            }
        }
        else if (!strcmp(token,COMANDO_REINICIO)){
            publicarMensajeOk(mensajeId);
            vTaskDelay(pdMS_TO_TICKS(500));
            CySoftwareReset();
        }
        else if (!strcmp(token,COMANDO_CAMBIAR_CLAVE)){
            xclave clave;
            token = strtok(NULL, delimitadores); // ,\"tipo\":
            token = strtok(NULL, delimitadores); // \"%s\"
            sprintf(usuario,"%s",token);
            token = strtok(NULL, delimitadores); // ,\"clave\":
            token = strtok(NULL, delimitadores); // \"%s\"
            while(!xSemaphoreTake(eepromOcupada,1));
            sprintf(clave.clave,"%s",token);
            if(!strcmp(usuario,"administrador")){
                EEPROM_escribirClaveAdmin(clave);
                publicarMensajeOk(mensajeId);
            }
            else if(!strcmp(usuario,"operador")){
                EEPROM_escribirClaveOperador(clave);
                publicarMensajeOk(mensajeId);
            }
            else if(!strcmp(usuario,"tesorero")){
                EEPROM_escribirClaveTesorero(clave);
                publicarMensajeOk(mensajeId);
            }
            else{
                publicarMensajeError(COMANDO_PERDIDO, mensajeId);
            }
            xSemaphoreGive(eepromOcupada);
        }
        else if (!strcmp(token,COMANDO_PARAMETROS_BANDEJAS)){
            xQueuePeek(configuracionesBandejas,(void*)&confBandejas,1);
            sprintf(string,"{\"d\":{\"messageId\":%d,\"code\":%d,\"numBandejas\":%d,\"motores1\":\"%d\",\"motores2\":\"%d\",\"motores3\":\"%d\",\"motores4\":\"%d\",\"motores5\":\"%d\",\"motores6\":\"%d\",\"deviceId\":\"DOROTI-%s\"}}",
                mensajeId,
                200,
                confBandejas.configBandejas.numBandejas,
                confBandejas.configBandejas.numMotores[0],
                confBandejas.configBandejas.numMotores[1],
                confBandejas.configBandejas.numMotores[2],
                confBandejas.configBandejas.numMotores[3],
                confBandejas.configBandejas.numMotores[4],
                confBandejas.configBandejas.numMotores[5],
                parametrosMaquina.Serie.serie
            );
            SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,string);
        }
        else if (!strcmp(token,COMANDO_AUTORIZAR_PRODUCTO)){
            LED_DEBUG_Write(1);
            vTaskDelay(pdMS_TO_TICKS(300));
            LED_DEBUG_Write(0);
            vTaskDelay(pdMS_TO_TICKS(300));
            token = strtok(NULL, delimitadores); // ,\"usuario\":
            token = strtok(NULL, delimitadores); // \"%s\"
            sprintf(autorizacion.usuario,"%s",token);
            token = strtok(NULL, delimitadores); // ,\"identificador\":
            token = strtok(NULL, delimitadores); // \"%d\"
            autorizacion.identificador = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"producto\":
            token = strtok(NULL, delimitadores); // \"%d\"
            autorizacion.producto = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"cantidad\":
            token = strtok(NULL, delimitadores); // \"%d\"
            autorizacion.cantidad = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"clave\":
            token = strtok(NULL, delimitadores); // \"%s\"
            sprintf(autorizacion.clave,"%s",token);
            xQueueSendToBack(autorizaciones,(void*)&autorizacion,10);
            //while(!xSemaphoreTake(memoriaOcupada,100));
            //escribirRefrigeracion(configuracionTemp.refrigeracion);
            //xSemaphoreGive(memoriaOcupada);
            /*while(!xSemaphoreTake(busOcupado,10));
            Dispensador_dispensar(autorizacion.producto);
            vTaskDelay(pdMS_TO_TICKS(10));
            xSemaphoreGive(busOcupado);*/
            publicarMensajeOk(mensajeId);
        }
        else if (!strcmp(token,COMANDO_AUTORIZAR_PROVEEDOR)){
            token = strtok(NULL, delimitadores); // ,\"identificador\":
            token = strtok(NULL, delimitadores); // \"%d\"
            proveedor.identificador = atoi(token);
            token = strtok(NULL, delimitadores); // ,\"clave\":
            token = strtok(NULL, delimitadores); // \"%s\"
            sprintf(proveedor.clave,"%s",token);
            token = strtok(NULL, delimitadores); // ,\"productos\":
            token = strtok(NULL, delimitadores); // \"%d\"
            proveedor.productosAutorizados = atoi(token);
            xQueueSendToBack(autorizacionProveedor,(void*)&proveedor,10);
            publicarMensajeOk(mensajeId);
        }
        else{
            publicarMensajeError(COMANDO_PERDIDO,mensajeId);
        }
    }
    else if(!strcmp(Topico,TOPICO_CHECKON)){
        token = strtok(Mensaje, delimitadores);
        token = strtok(NULL, delimitadores);
        sprintf(string,"{\"deviceId\":\"DOROTI-%s\",\"code\":\"%d\"}",
            parametrosMaquina.Serie.serie,
            200
        );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_CHECKED,string);
    }
    /*else if(!strcmp(Topico,TOPICO_RESET)){
        token = strtok(Mensaje, delimitadores);
        token = strtok(NULL, delimitadores);
        vTaskDelay(pdMS_TO_TICKS(2000));
        sprintf(string,"{\"rc\": %d,\"reqId\":\"%s\",\"deviceId\":\"DOROTI-%s\"}",
            REINICIO_INMEDIATO,
            token,
            parametrosMaquina.Serie.serie
        );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESPUESTA,string);
        CySoftwareReset();
    }*/
    /*else if(!strcmp(Topico,TOPICO_PRUEBAS)){
        //MQTT_suscribir(0,0,MQTT_generarIDMensaje(),TOPICO_GENERAR_ERROR);
    }*/
    /*else if(!strcmp(Topico,TOPICO_CAMBIO_CLAVE)){
        xclave clave;
        sprintf(string,"{\"d\":{\"Clave\":\"Clave Reseteada\",\"deviceId\":\"DOROTI-%s\"}}",
            parametrosMaquina.Serie.serie
        );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESP_CLAVE,string);
        while(!xSemaphoreTake(eepromOcupada,1));
        sprintf(clave.clave,"ekiaadmin");
        EEPROM_escribirClaveAdmin(clave);
        xSemaphoreGive(eepromOcupada);
        BuzzerPWM_WritePeriod(periodo2);
        BuzzerPWM_WriteCompare(periodo2-10);
        vTaskDelay(pdMS_TO_TICKS(3000));
        BuzzerPWM_WritePeriod(1);
        BuzzerPWM_WriteCompare(1/compar);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }*/
    /*else if(!strcmp(Topico,TOPICO_DETALLES)){
        char* reqId;
        int version=0;
        LED_DEBUG_Write(1);
        token = strtok(Mensaje, delimitadores);
        reqId = strtok(NULL, delimitadores);
        do{
            token = strtok(NULL, delimitadores);
        }while(strcmp(token,"version")!=0);
        token = strtok(NULL, delimitadores);
        if(!strcmp(token,VERSION_SOFTWARE)){
            version=ERROR_DESCARGA;
        }
        else{
            version=VERSION_DIFIERE;
        }
        sprintf(string,"{\"rc\": %d,\"reqId\":\"%s\",\"deviceId\":\"DOROTI-%s\"}",
            version,
            reqId,
            parametrosMaquina.Serie.serie
        );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESPUESTA,string);
    }*/
    else{
        publicarMensajeError(TOPICO_PERDIDO, mensajeId);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

/*******************************************************************************
* en operacion
********************************************************************************
*
* Tarea que realiza:
*  Funcion que se ejecuta siempre que se haya establecido una conexion y se encuentre
*  activa
*
*******************************************************************************/
void SIM800L_MQTT_enOperacion(){ 
    char mensaje[500];
    uint16 actividad;
    xEventos evento;
    if(xQueueReceive(avisosBilletero,(void*)&actividad,100)){
        sprintf(mensaje,
                "{\"d\":{\"Billetero\":{\"AvisoBilletero\":%d},\"deviceId\":\"DOROTI-%s\"}}",
                actividad,
                NUMERO_DE_SERIE
            );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_AVISOS_BILLETERO,mensaje);
    }
    if(xQueueReceive(erroresBilletero,(void*)&actividad,100)){//se recibió un mensaje de solicitud puntoRed?
        sprintf(mensaje,
                "{\"d\":{\"Billetero\":{\"ErrorBilletero\":%d},\"deviceId\":\"DOROTI-%s\"}}",
                actividad,
                NUMERO_DE_SERIE
            );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_ERRORES_BILLETERO,mensaje);
    }
    if(xQueueReceive(enviarRegistro,(void*)&evento,100)){//se recibió un mensaje de solicitud puntoRed?
        sprintf(mensaje,
                "{\"d\":{\"Registro\":{\"%s\":\"%s\"},\"deviceId\":\"DOROTI-%s\"}}",
                evento.tipo,
                evento.evento,
                NUMERO_DE_SERIE
            );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_REGISTRO,mensaje);
    }
    if(xQueueReceive(avisosMonedero,(void*)&actividad,100)){
        sprintf(mensaje,
                "{\"d\":{\"Monedero\":{\"AvisoMonedero\":%d},\"deviceId\":\"DOROTI-%s\"}}",
                actividad,
                NUMERO_DE_SERIE
            );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_AVISOS_MONEDERO,mensaje);
    }
    if(xQueueReceive(erroresMonedero,(void*)&actividad,100)){//se recibió un mensaje de solicitud puntoRed?
        sprintf(mensaje,
                "{\"d\":{\"Monedero\":{\"ErrorMonedero\":%d},\"deviceId\":\"DOROTI-%s\"}}",
                actividad,
                NUMERO_DE_SERIE
            );
        SIM800L_MQTT_publicar(0,0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_ERRORES_MONEDERO,mensaje);
    }
}

/*******************************************************************************
* en conexion con broker
********************************************************************************
*
* Tarea que realiza:
*  Funcion que se ejecuta siempre que se entable una conexión con el broker remoto
*
*******************************************************************************/
void SIM800L_MQTT_enConexion(){ 
    xConfiguracionTelemetria parametros;
    xConfiguracion paramMaquina;
    char topic[30], mensaje[100];
    xQueuePeek(configuracionesRemotas,(void*)&parametros,1);
    xQueuePeek(configuracionesMaquina,(void*)&paramMaquina,1);
    SIM800L_MQTT_suscribir(0,2,SIM800L_MQTT_generarIDMensaje(),TOPICO_GESTION_RESP);
    actualizarMetadatos();
    sprintf(mensaje,"{\"d\":{\"Estado Conexion\":\"%s se ha conectado correctamente\"}}",paramMaquina.Serie.serie);
    SIM800L_MQTT_publicar(0,2,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESULTADOS,mensaje);
    //SIM800L_MQTT_publicar(0,2,0,SIM800L_MQTT_generarIDMensaje(),"test","{\"d\":{\"Estado Conexion\":\"Se ha conectado correctamente\"}}");
    //SIM800L_MQT T_suscribir(0,2,SIM800L_MQTT_generarIDMensaje(),TOPICO_RESET);
    //SIM800L_MQTT_suscribir(0,0,SIM800L_MQTT_generarIDMensaje(),TOPICO_PRUEBAS);
    sprintf(topic,"%s%s",TOPICO_COMANDOS,paramMaquina.Serie.serie);
    SIM800L_MQTT_suscribir(0,0,SIM800L_MQTT_generarIDMensaje(),topic);
    SIM800L_MQTT_suscribir(0,1,SIM800L_MQTT_generarIDMensaje(),TOPICO_CHECKON);
    
    if(parametros.estadoConexionGSM==DESCONECTADO){
        parametros.estadoConexionGSM = CONECTADO;
        xQueueOverwrite(configuracionesRemotas,(void*)&parametros);
    }
}

/*******************************************************************************
* configurarGSM
********************************************************************************
*
* Tarea que realiza:
*  Funcion que realiza una primera configuración del GSM recien es encendido
*  Obtiene también algunos parámetros propios del dispositivo
*
*******************************************************************************/
CYBIT confGSM(){
    CYBIT comando_ok = pdFALSE;
    xInformacionGSM infoGSM = {pdFALSE,
        0,0,0,0,0,0,0,0,0,
        "","","","","","",""
    };
    xbufferEntradaGSM datosEntrada;
    xRespuestasGSM respuesta;
    uint8 paso = 0;
    int timeOut = 0;
    do{
        comando_ok = SIM800L_start();
        if(comando_ok == pdFALSE){
            reinicializar();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }while(comando_ok == pdFALSE);
    comando_ok=pdFALSE;
    while(!comando_ok){
        //SIM_ClearRxBuffer();
        switch (paso){
            case 0:
                if(SIM800L_Banda(SIM800L_BANDA_GSM850_MODE)){
                    paso++;
                }
            break;
            case 1:
                datosEntrada = SIM800L_identFabricante();
                strcpy(infoGSM.fabricante,datosEntrada.entrada);
                paso++;
            break;
            case 2:
                datosEntrada = SIM800L_Modelo();
                strcpy(infoGSM.modelo,datosEntrada.entrada);
                paso++;
            break;       
            case 3:
                infoGSM.potenciaSenal= SIM800L_potenciaSenal();
                paso++;
            break; 
            case 4:
                datosEntrada = SIM800L_IMEI();
                strcpy(infoGSM.IMEI,datosEntrada.entrada);
                paso++;
            break;  
            case 5:
                datosEntrada = SIM800L_IMSI();
                strcpy(infoGSM.IMSI,datosEntrada.entrada);
                paso++;
            break;     
            case 6:
                respuesta = SIM800L_verificarRegistro();
                infoGSM.registro = respuesta;
                vTaskDelay(pdMS_TO_TICKS(1000));
                timeOut++;
                if(((respuesta.resultado2 == 1)||(respuesta.resultado2 == 5))||(timeOut>=SIM800L_GSM_TIME_OUT)){
                    comando_ok = pdTRUE;
                    infoGSM.encendido = pdTRUE;
                }
            break;
        }
    }
    if(timeOut>=SIM800L_GSM_TIME_OUT){
        infoGSM.encendido = pdFALSE;
        comando_ok = pdFALSE;
    }
    xQueueOverwrite(datosGSM,&infoGSM);
    return comando_ok;
}

/*******************************************************************************
* conectarBroker
********************************************************************************
*
* Tarea que realiza:
*  Funcion para iniciar la conexion al broker remoto
*
*******************************************************************************/
void conectarBroker(){
    xConfiguracion parametros;
    char string[100],clave[20];
    xQueuePeek(configuracionesMaquina,(void*)&parametros,100);
    //sprintf(string,"d:%s:EKIA_DOROTI:DOROTI-%s",ORG_ID,parametros.Serie.serie);
    sprintf(string,"DOROTI-%s",parametros.Serie.serie);
    sprintf(clave,"EKIADOROTI%s",parametros.Serie.serie);
    //if(SIM800L_conectarHost("wqmp0z.messaging.internetofthings.ibmcloud.com","1883")){
    if(SIM800L_conectarHost("134.209.171.145","1883")){    
        //SIM800L_MQTT_conectarBroker("MQIsdp","ipta","","",1,0,0,"","");
        SIM800L_MQTT_conectarBroker("MQIsdp",string,"use-token-auth",clave,1,2,0,"","");
        //SIM800L_MQTT_conectarBroker("MQIsdp","d:wqmp0z:EKIA_DOROTI:DOROTI-A001","use-token-auth","EKIADOROTIA001",1,0,0,"","");
        while(SIM800L_MQTT_leerEstado() != SIM800L_MQTT_CONECTADO){
            SIM800L_MQTT_recibir();
        }
    }
}

/*******************************************************************************
* reinicializar
********************************************************************************
*
* Tarea que realiza:
*  Funcion que resetea los parametros y desconecta comunicaciones activas
*
*******************************************************************************/

void reinicializar(){
    SIM800L_MQTT_desconectar();
    SIM800L_SIM_REG_Write(SIM800L_SIM_REG_Read()&0x02);
    vTaskDelay(pdMS_TO_TICKS(800));
    SIM800L_SIM_REG_Write(SIM800L_SIM_REG_Read()|0x01);
    xQueueReset(datosGSM);
    /*while(!confGSM()){
        SIM800L_apagar();
        vTaskDelay(pdMS_TO_TICKS(500));
    }*/
}

/*******************************************************************************
* TAREA DE TELEMETRIA
********************************************************************************
*
* Tarea que realiza:
*  Programa principal de telemetría
*
*******************************************************************************/
void Telemetria(void* pvParameters){ 
    (void)pvParameters;
    xInformacionGSM infoGSM;
    xConfiguracionTelemetria parametros;
    CYBIT primeraVez = pdTRUE;
    CYBIT reinicio = pdTRUE;
    int tiempoVida = 0, tiempoVer = 0;
    SIM800L_SIM_REG_Write(SIM800L_SIM_REG_Read()&0x02);
    vTaskDelay(pdMS_TO_TICKS(500));
    SIM800L_SIM_REG_Write(SIM800L_SIM_REG_Read()|0x01);
    while(!confGSM()){
        SIM800L_shutdown();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelay(pdMS_TO_TICKS(500));
    xQueuePeek(datosGSM, &infoGSM,1);
    BuzzerPWM_Start();
    BuzzerPWM_WritePeriod(1);
    BuzzerPWM_WriteCompare(1/compar);
    for(;;)
    {
        //verifique si el modo de puente de comandos AT se encuentra activo
        //if(!modoPuenteAT()){;
        if(SIM800L_MQTT_leerEstado()==SIM800L_MQTT_DESCONECTADO){
            xQueuePeek(configuracionesRemotas,(void*)&parametros,1);
            parametros.estadoConexionGSM = DESCONECTADO;
            xQueueOverwrite(configuracionesRemotas,(void*)&parametros);
            //verifique el registro en red
            infoGSM.registro = SIM800L_verificarRegistro();
            //se encuentra registrado?
            if((infoGSM.registro.resultado2==1)||(infoGSM.registro.resultado2==5)){
                //si lo está, termina el reinicio de parametros y pide el operador una vez
                reinicio = pdFALSE;
                //actualiza el estado de la señal y del GPRS
                infoGSM.potenciaSenal = SIM800L_potenciaSenal();
                infoGSM.GPRS = SIM800L_verificarEstadoGPRS();
            }
            else{
                //si no lo está, indica que debe reiniciar la conexión
                reinicio = pdTRUE;
                primeraVez= pdTRUE;
                confGSM();
            }
            //si ya se encuentra registrado solicita el nombre de operador
            if(primeraVez && !reinicio){
                strcpy(infoGSM.IMSI,SIM800L_IMSI().entrada);
                vTaskDelay(pdMS_TO_TICKS(100));
                strcpy(infoGSM.operador,SIM800L_verificarOperador().entrada);
                xQueueOverwrite(datosGSM,(void*)&infoGSM);
                if(SIM800L_TCPInit(1)){
                    primeraVez = pdFALSE;
                    conectarBroker();
                }
                else{
                    reinicializar();
                    reinicio = pdTRUE;
                    primeraVez= pdTRUE;
                }
                xQueuePeek(datosGSM,&infoGSM,10);
            }
        }
        else{
            xQueuePeek(datosGSM,&infoGSM,10);
            if(tiempoVida >= PERIODO_PING){
                BuzzerPWM_WritePeriod(periodo);
                BuzzerPWM_WriteCompare(periodo/compar);
                vTaskDelay(pdMS_TO_TICKS(1000));
                BuzzerPWM_WritePeriod(1);
                BuzzerPWM_WriteCompare(1/compar);
                vTaskDelay(pdMS_TO_TICKS(1000));
                if(!SIM800L_MQTT_ping()){
                    reinicializar();
                    reinicio = pdTRUE;
                    primeraVez= pdTRUE;
                }
                tiempoVida = 0; 
            }
            SIM800L_MQTT_recibir();
            SIM800L_MQTT_enOperacion();
            if(tiempoVer >= PERIODO_AT){
                if(xSemaphoreTake(telemetriaOcupada,1)){
                    if(SIM800L_datosaAT()){
                        //LED_DEBUG_Write(1);
                    }
                    //verifica estado de registro en red
                    infoGSM.registro = SIM800L_verificarRegistro();
                    if(!(infoGSM.registro.resultado2==1)&&!(infoGSM.registro.resultado2==5)){
                        reinicializar();
                        reinicio = pdTRUE;
                        primeraVez= pdTRUE;
                    }
                    xQueueOverwrite(datosGSM,(void*)&infoGSM);
                    //if(SIM800L_conectarFTP("ekiavending.com","IPTA","ekiaipta01","principal.hex")){
                       // LED_DEBUG_Write(1);
                    //}
                    xQueuePeek(datosGSM,&infoGSM,10);
                    //actualiza potencia de la señal
                    infoGSM.potenciaSenal = SIM800L_potenciaSenal();
                    //verifica estado de la conexión GPRS
                    infoGSM.GPRS = SIM800L_verificarEstadoGPRS();
                    //hace una prueba de conexion FTP
                    if(infoGSM.GPRS.resultado1!=1){
                        reinicializar();
                        reinicio = pdTRUE;
                        primeraVez= pdTRUE;
                    }
                    if(!SIM800L_ATaDatos()){
                        reinicializar();
                        reinicio = pdTRUE;
                        primeraVez= pdTRUE;
                    }
                    tiempoVer = 0;
                    xSemaphoreGive(telemetriaOcupada);
                }
            }
            tiempoVida++;
            tiempoVer++;
        }
        xQueueOverwrite(datosGSM,(void*)&infoGSM);
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}

/*******************************************************************************
* ELIMINACION DE LA TAREA DE TELEMETRIA
********************************************************************************
*
* Tarea que realiza:
*  Detiene los perifericos relacionados con el SIM800, elimina la tarea y 
*  libera la memoria ocupada por la telemetria. 
*
*******************************************************************************/
void Telemetria_Stop(){
    SIM800L_MQTT_desconectar();
    SIM800L_shutdown();
    xQueueReset(datosGSM);
    xInformacionGSM infoGSM;
    infoGSM.encendido = pdFALSE;
    xQueueOverwrite(datosGSM,&infoGSM);
    vTaskDelete(xTelemetria);
}

/*******************************************************************************
* DECLARACIÓN DEL TASK DE COMUNICACION GSM/GPRS
********************************************************************************
*
* Tarea que realiza:
*  Crea el task para control del sistema de comunicación GSM
*
*******************************************************************************/
void Telemetria_Start(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(Telemetria, //puntero de la función que crea la tarea (nombre de la tarea)
                   "Telemetria", //nombre textual de la tarea, solo útil para depuración
                   memTelemetria, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorTelemetria, //prioridad de la tarea
                   &xTelemetria)!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}
/* [] END OF FILE */
