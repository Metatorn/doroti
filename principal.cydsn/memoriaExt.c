/* ==============================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;FUNCIONES PROPIAS DE LA MEMORIA SD
===============================================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "FS.h"
#include "memoriaExt.h"
#include "memoriaSD.h"
#include "Clock_lento.h"
#include "LED_DEBUG.h"
#include "memEEPROM.h"
#include "RTC_RTC.h"
#include "RTC_EKIA.h"

/*******************************************************************************
* habilitarSD
********************************************************************************
* Función para habilitar el sistema de archivos
*******************************************************************************/
CYBIT habilitarSD(){
    CYBIT comando_ok=pdFALSE;
    FS_Init();
    FS_FAT_SupportLFN();
    FS_ConfigUpdateDirOnWrite(pdTRUE);
    Clock_lento_Enable();
    return comando_ok;
}

/*******************************************************************************
* deshabilitarSD
********************************************************************************
* Función para deshabilitar el sistema de archivos
*******************************************************************************/
CYBIT deshabilitarSD(){
    CYBIT comando_ok=pdFALSE;
    FS_DeInit();
    Clock_lento_Disable();
    return comando_ok;
}

/*******************************************************************************
* obtenerMarcaTiempoSD
********************************************************************************
* Función para obtener el timeStam basado en la estructura de tiempo del RTC
*******************************************************************************/
uint32 obtenerMarcaTiempoSD(){
    taskENTER_CRITICAL();
    uint32 timeStamp = 0;
    timeStamp = (
                    (RTC_RTC_ReadSecond()/2)|
                    (RTC_RTC_ReadMinute()<<5)|
                    (RTC_RTC_ReadHour()<<11)|
                    (RTC_RTC_ReadDayOfMonth()<<16)|
                    (RTC_RTC_ReadMonth()<<21)|
                    ((RTC_RTC_ReadYear()-1980))<<25
                );
    taskEXIT_CRITICAL();
    return timeStamp;
}

/*******************************************************************************
* espacioSD
********************************************************************************
* Función para medir el espacio libre de la memoria SD
*******************************************************************************/ 
xEspacioMemoria espacioSD(){
    xEspacioMemoria espacio;
    espacio.capacidad = FS_GetVolumeSizeKB("memoriaVending")/1000.0f;
    stpcpy(espacio.prefijo,"MB");
    espacio.utilizado = espacio.capacidad - (FS_GetVolumeFreeSpaceKB("memoriaVending")/1000.0f);
    return espacio;
}

/*******************************************************************************
* formatearSD
********************************************************************************
* Función para formatear la memoria SD y dejarla con carpetas y archivos básicos
* para la correcta operación de la máquina
*******************************************************************************/
CYBIT formatearSD(){
    CYBIT comando_ok=pdFALSE;
    FS_FILE *archivoProducto;
    uint32 timeStamp = 0;
    char string[50];
    Clock_lento_SetDividerRegister(freq_20_Hz,0);
    if(FS_GetVolumeSizeKB("memoriaVending")>0){
        if(!FS_FormatSD("memoriaVending")){
            if(!FS_MkDir(pName_Dir_Conf)){
                if(!FS_MkDir(pName_Dir_Cont)){
                    if(!FS_MkDir(pName_Dir_Cont_Dia)){
                        if(!FS_MkDir(pName_Dir_Cont_Mes)){
                            if(!FS_MkDir(pName_Dir_Cont_Anual)){
                                if(!FS_MkDir(pName_Dir_Registro)){
                                    comando_ok=pdTRUE;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if(comando_ok==pdTRUE){
        comando_ok = pdFALSE;
        timeStamp = obtenerMarcaTiempoSD();
        if(!FS_SetFileTime(pName_Dir_Conf,timeStamp)){
            if(!FS_SetFileTime(pName_Dir_Cont,timeStamp)){
                if(!FS_SetFileTime(pName_Dir_Cont_Dia,timeStamp)){
                    if(!FS_SetFileTime(pName_Dir_Cont_Mes,timeStamp)){
                        if(!FS_SetFileTime(pName_Dir_Cont_Anual,timeStamp)){
                            if(!FS_SetFileTime(pName_Dir_Registro,timeStamp)){
                                comando_ok=pdTRUE;          
                            }
                        }
                    }
                }
            }
        }
    }
    archivoProducto = FS_FOpen(ruta_productos,"w");
    if(archivoProducto != NULL){
        sprintf(string,"Fila;Producto;Precio;Cantidad;Habilitado\r\n");
        FS_Write(archivoProducto,string,strlen(string));
        sprintf(string,"FIN;;;;\r\n");
        FS_Write(archivoProducto,string,strlen(string));
        FS_FClose(archivoProducto);
        if(FS_SetFileTime(ruta_productos,timeStamp)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok=pdFALSE;
    }
    if(!copiarConfiguracionSD()){
        comando_ok=pdFALSE;
    }
    else{
        if(FS_SetFileTime(ruta_configuracion,timeStamp)){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* leerArchivoSD
********************************************************************************
* Función que permite leer un archivo de la SD en formato específico, retorna
* una linea delimitada por un retorno de carro.
* usa como parámetros de entrada el archivo que se quiere leer y el puntero para
* saber desde que byte debe ser leido el archivo.
* Retorna la información completa de la linea y el puntero donde finalice
*******************************************************************************/
xLecturaSD leerArchivoSD(FS_FILE *archivo){
    xLecturaSD buffer;
    char bufferLectura[sizeof(buffer.lineaLeida)];
    char temp[50];
    int indice = 0;
    long puntero = 0;
    long comparador = 0;
    uint8 i = 0, j = 0;
    
    //limpie el buffer de la linea leida
    for (j=0;j<sizeof(buffer.lineaLeida);j++){
        buffer.lineaLeida[j] = 0;
    }
    for (j=0;j<50;j++){
        temp[j] = 0;
    }
    //ubica el puntero tomando como referencia el ultimo puntero leido
    puntero = FS_FTell(archivo); 
    //realiza la lectura del archivo desde la ultima posición leida
    FS_Read(archivo, bufferLectura,sizeof(bufferLectura));
    //ubique el final de la linea
    while(bufferLectura[indice]!='\r'){
        indice++;
    }
    indice+=2; //correr en dos posiciones extra, para eliminar el retorno de carro en la siguiente lectura
    //ubique el puntero en la ubicación justo despues del retorno de linea
    puntero += indice;
    FS_FSeek(archivo,puntero,FS_SEEK_SET);
    
    //verifica si es el final del archivo
    comparador = FS_FTell(archivo);
    FS_FSeek(archivo,0,FS_FILE_END);    
    if(FS_FTell(archivo)==comparador){
        buffer.finArchivo = pdTRUE;
    }
    else{
        buffer.finArchivo = pdFALSE;
    }
    FS_FSeek(archivo,puntero,FS_SEEK_SET);
    buffer.punteroByte = puntero;
    //determine cual ha sido el numero de linea leido
    while(bufferLectura[i]!=';'){
        temp[i]=bufferLectura[i];
        i++;
    }
    i++; //correr en una posición extra, para eliminar el ;
    buffer.filaLeida = atoi(temp);
    //sscanf(temp,"%d",&buffer.filaLeida);

    //copie el buffer leido hasta el retorno de carro en el buffer de linea
    for(j=0;j<(indice-i-1);j++){
        buffer.lineaLeida[j]=bufferLectura[i+j];
    }
    buffer.archivo = archivo;
    return buffer;
}

/*******************************************************************************
* separarCeldas
********************************************************************************
* Función que permite obtener el contenido de una celda en particular
*******************************************************************************/
xCeldas separarCeldasSD(xLecturaSD filaLeida){
    xCeldas celdas;
    char celdaTemp[sizeof(celdas.celda1)];
    int numCelda = 0;
    int i=0;
    uint8 j=0;
    CYBIT finFila = pdFALSE;
    celdas.fila = filaLeida.filaLeida;
    while(!finFila){
        //borrar la celda temporal antes de leer una nueva
        for(j=0;j<sizeof(celdaTemp);j++){
            celdaTemp[j]=0;
        }
        j = 0;
        while((filaLeida.lineaLeida[i]!=';')&&(filaLeida.lineaLeida[i]!='\r')){
            celdaTemp[j]=filaLeida.lineaLeida[i];
            j++;
            i++;
        }
        switch (numCelda){
            case 0:
                numCelda++;
                strcpy(celdas.celda1,celdaTemp);
            break;
            case 1:
                numCelda++;
                strcpy(celdas.celda2,celdaTemp);
            break;
            case 2:
                numCelda++;
                strcpy(celdas.celda3,celdaTemp);
            break;
            case 3:
                numCelda++;
                strcpy(celdas.celda4,celdaTemp);
            break;
            case 4:
                numCelda++;
                strcpy(celdas.celda5,celdaTemp);
            break;
            case 5:
                numCelda++;
                strcpy(celdas.celda6,celdaTemp);
            break;
            case 6:
                numCelda++;
                strcpy(celdas.celda7,celdaTemp);
            break;
            case 7:
                numCelda++;
                strcpy(celdas.celda8,celdaTemp);
            break;
            default:
                finFila = pdTRUE;
            break;    
        }
        if(filaLeida.lineaLeida[i]!='\r'){
            i++;
        }
    }
    return celdas;
}

/*******************************************************************************
* recuperarConfiguracion
********************************************************************************
* Función para copiar la configuración de máquina desde la SD
*******************************************************************************/
CYBIT restaurarConfiguracionSD(){
    CYBIT comando_ok=pdTRUE;
    CYBIT finArchivo = pdFALSE;
    xLecturaSD parametroLeido;
    FS_FILE *archivoConfiguracion;
    xConfiguracion parametrosMaquina;
    xConfiguracionTemperatura parametrosTemperatura;
    xCeldas celdas;
    int valor;
    float valorF;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    
    xQueuePeek(configuracionesMaquina,&parametrosMaquina,10);
    xQueuePeek(configuracionesTemperatura, &parametrosTemperatura, 10);
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    archivoConfiguracion = FS_FOpen(ruta_configuracion,"r");
    if(archivoConfiguracion != NULL){
        while(!finArchivo){
            //realice una lectura del archivo CSV
            parametroLeido = leerArchivoSD(archivoConfiguracion);
            archivoConfiguracion = parametroLeido.archivo;
            celdas = separarCeldasSD(parametroLeido);
            switch(parametroLeido.filaLeida){
                case 0: //cabecera
                    //no haga nada, cabecera de archivo
                break;
                case 1: //serial
                    //no haga nada, el serial no puede ser modificado
                    if(strcmp(celdas.celda1,label_serie)){
                        comando_ok = pdFALSE;
                    }
                break;
                case 2: //brillo automatico
                    if(strcmp(celdas.celda1,label_brilloAuto)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        sscanf(celdas.celda2,"%d",&valor);
                        parametrosMaquina.BrilloPantalla.activo=valor;
                    }
                break;
                case 3: //nivel de brillo de pantalla
                    if(strcmp(celdas.celda1,label_brilloNivel)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        sscanf(celdas.celda2,"%d",&valor);
                        parametrosMaquina.BrilloPantalla.nivel=valor;
                    }
                break;
                case 4: //iluminación activada
                    if(strcmp(celdas.celda1,label_ilumiAct)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        sscanf(celdas.celda2,"%d",&valor);
                        parametrosMaquina.BrilloMaquina.activo=valor;
                    }
                break;
                case 5: //nivel de iluminación
                    if(strcmp(celdas.celda1,label_ilumiNivel)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        sscanf(celdas.celda2,"%d",&valor);
                        parametrosMaquina.BrilloMaquina.nivel=valor;
                    }
                break;
                case 6: //refrigeracion activada
                    if(strcmp(celdas.celda1,label_refriAct)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        sscanf(celdas.celda2,"%d",&valor);
                        parametrosTemperatura.refrigeracion.activado=valor;
                    }
                break;
                case 7: //temperatura deseada
                    if(strcmp(celdas.celda1,label_temp)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        valorF = atof(celdas.celda2);
                        //sscanf(celdas.celda2,"%d",&valorF);
                        parametrosTemperatura.refrigeracion.gradosC=valorF*100.0f;
                    }
                break;
                case 8: //escala de peso
                    if(strcmp(celdas.celda1,label_escalaPeso)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        valorF = atof(celdas.celda2);
                        parametrosMaquina.escalaPeso=valorF;
                    }
                break;
                case 9: //escala de temperatura
                    if(strcmp(celdas.celda1,label_escalaTemp)){
                        comando_ok = pdFALSE;
                    }
                    else{
                        valorF = atof(celdas.celda2);
                       //parametrosTemperatura.escalaTemperatura=valorF;
                    }
                    finArchivo = pdTRUE;
                break;  
                default:
                    //finArchivo = TRUE;
                break;
            }
        }
        FS_FClose(archivoConfiguracion);
        if(FS_SetFileTimeEx(ruta_configuracion,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    if(comando_ok){
        comando_ok = pdFALSE;
        xQueueOverwrite(configuracionesMaquina,&parametrosMaquina);
        xQueueOverwrite(configuracionesTemperatura,&parametrosTemperatura);
        while(!xSemaphoreTake(eepromOcupada,1));
        if(escribirBrilloPantalla(parametrosMaquina.BrilloPantalla)){
            if(escribirBrilloMaquina(parametrosMaquina.BrilloMaquina)){
                if(escribirRefrigeracion(parametrosTemperatura.refrigeracion)){
                    if(escribirEscalaPeso(parametrosMaquina.escalaPeso)){
                        //if(escribirEscalaTemperatura(parametrosTemperatura.escalaTemperatura)){
                            comando_ok = pdTRUE; 
                        //}
                    }
                }
            }
        }
        
        xSemaphoreGive(eepromOcupada);
    }
    return comando_ok;
}

/*******************************************************************************
* copiarConfiguracion
********************************************************************************
* Función para copiar la configuración de máquina a la memoria externa
*******************************************************************************/
CYBIT copiarConfiguracionSD(){
    FS_FILE *archivoConfiguracion;
    CYBIT comando_ok=pdTRUE;
    xConfiguracion parametrosMaquina;
    xConfiguracionTemperatura parametrosTemperatura;
    char string[50];
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    xQueuePeek(configuracionesMaquina,&parametrosMaquina,10);
    xQueuePeek(configuracionesTemperatura,&parametrosTemperatura,10);
    Clock_lento_SetDividerRegister(freq_10_Hz,0);    
    int fila=1;
    
    //verifica si hay un archivo existente
    archivoConfiguracion = FS_FOpen(ruta_configuracion,"r");
    if(archivoConfiguracion != NULL){
        //si existe, guarde la fecha de creación
        FS_FClose(archivoConfiguracion);
        FS_GetFileTime(ruta_configuracion,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
    }
    archivoConfiguracion = FS_FOpen(ruta_configuracion,"w");
    if(archivoConfiguracion != NULL){
        sprintf(string,"Fila;Parametro;valor\r\n");
        FS_Write(archivoConfiguracion,string,strlen(string));
        //sprintf(string,"1;Num. Serie:;\%s\r\n",parametrosMaquina.Serie.numSerie);
        sprintf(string,"%d;Num. Serie:;",fila);
        FS_Write(archivoConfiguracion,string,strlen(string));
        FS_Write(archivoConfiguracion,parametrosMaquina.Serie.numSerie,sizeof(parametrosMaquina.Serie.numSerie));
        sprintf(string,"\r\n");
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Brillo Automatico:;\%d\r\n",fila,parametrosMaquina.BrilloPantalla.activo);
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Nivel Brillo:;\%d\r\n",fila,parametrosMaquina.BrilloPantalla.nivel);
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Iluminacion Activa:;\%d\r\n",fila,parametrosMaquina.BrilloMaquina.activo);
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Nivel Iluminacion:;\%d\r\n",fila,parametrosMaquina.BrilloMaquina.nivel);
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Refrigeracion Activa:;\%d\r\n",fila,parametrosTemperatura.refrigeracion.activado);
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Temperatura deseada:;\%.2f\r\n",fila,parametrosTemperatura.refrigeracion.gradosC/100.0f);
        FS_Write(archivoConfiguracion,string,strlen(string));
        fila++;
        sprintf(string,"%d;Escala de peso:;\%f\r\n",fila,parametrosMaquina.escalaPeso);
        FS_Write(archivoConfiguracion,string,strlen(string));
        //sprintf(string,"%d;Escala de Temperatura:;\%f\r\n",fila,parametrosTemperatura.escalaTemperatura);
        //FS_Write(archivoConfiguracion,string,strlen(string));
        //fila++;
        FS_Write(archivoConfiguracion,string,strlen(string));
        FS_FClose(archivoConfiguracion);
        if(FS_SetFileTimeEx(ruta_configuracion,fechaCreacion,FS_FILETIME_CREATE)||
            FS_SetFileTimeEx(ruta_configuracion,timeStamp,FS_FILETIME_MODIFY)||
            FS_SetFileTimeEx(ruta_configuracion,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* recuperarProductos
********************************************************************************
* Función para recuperar la configuración de productos desde la SD
*******************************************************************************/
CYBIT restaurarProductosSD(){
    CYBIT comando_ok=pdTRUE;
    CYBIT finArchivo = pdFALSE;
    xLecturaSD productoLeido;
    FS_FILE *archivoProductos;
    xCeldas celdas;
    xProducto producto;
    int valor;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    archivoProductos = FS_FOpen(ruta_productos,"r");
    if(archivoProductos != NULL){
        productoLeido = leerArchivoSD(archivoProductos); //lee la cabecera del archivo
        archivoProductos = productoLeido.archivo;
        while(!finArchivo){
            //realice una lectura del archivo CSV
            productoLeido = leerArchivoSD(archivoProductos); //lee toda la fila de un producto
            archivoProductos = productoLeido.archivo;
            celdas = separarCeldasSD(productoLeido);
            sscanf(celdas.celda1,"%d",&valor);
            producto.numero = atoi(celdas.celda1);
            sscanf(celdas.celda2,"$%d",&valor);
            producto.precio = valor;
            sscanf(celdas.celda3,"%d",&valor);
            producto.cantidad = valor;
            if(!strcmp(celdas.celda4,"SI")){
                producto.habilitado = pdTRUE;
            }
            else{
                producto.habilitado = pdFALSE;
            }
            while(!xSemaphoreTake(eepromOcupada,1));
            if(!escribirProducto(producto)){
                comando_ok = pdFALSE; 
            }
            xSemaphoreGive(eepromOcupada);
            if(productoLeido.finArchivo==pdTRUE){
                finArchivo=pdTRUE;
            }
        }
        FS_FClose(archivoProductos);
        if(FS_SetFileTimeEx(ruta_productos,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* copiarProductos
********************************************************************************
* Función para copiar la configuración de productos de la máquina a la memoria externa
*******************************************************************************/
CYBIT copiarProductosSD(char* ruta){
    int fila=1;
    uint8 num = 0;
    uint16 indiceProducto = 100;
    uint16 centena = 100; 
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    FS_FILE *archivoProducto;
    CYBIT comando_ok=pdTRUE, volcado=pdFALSE;
    xProducto producto;
    char string[50];
    char estado[3];
    xConfiguracion parametros;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesMaquina, &parametros, 10);
    xQueuePeek(configuracionesBandejas, &bandejas,10);
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    //verifica si hay un archivo existente
    archivoProducto = FS_FOpen(ruta,"r");
    if(archivoProducto != NULL){
        //si existe, guarde la fecha de creación
        FS_FClose(archivoProducto);
        FS_GetFileTime(ruta,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
    }
    archivoProducto = FS_FOpen(ruta,"w");
    if(archivoProducto != NULL){
        sprintf(string,"Fila;Producto;Precio;Cantidad;Habilitado\r\n");
        FS_Write(archivoProducto,string,strlen(string));
        while(!volcado){
            if(xSemaphoreTake(eepromOcupada,( TickType_t )100)){
                producto = leerProducto(indiceProducto);
                if(producto.habilitado==pdTRUE){
                    stpcpy(estado,"SI");
                }
                else{
                    stpcpy(estado,"NO");
                }
                sprintf(string,"%d;%d;$%.0f;%d;%s\r\n",
                    fila,
                    indiceProducto,
                    producto.precio,
                    producto.cantidad,
                    estado
                );
                FS_Write(archivoProducto,string,strlen(string));
                num++;
                indiceProducto++;
                fila++;
                xSemaphoreGive(eepromOcupada);
            }
            if(indiceProducto>=(MOTORES_DOROTI+centena)){
                centena+=100;
                indiceProducto = centena;
            }
            if(num>=(bandejas.configBandejas.numBandejas*MOTORES_DOROTI)){
                volcado = pdTRUE;
            }
        }
        FS_FClose(archivoProducto);
        if(FS_SetFileTimeEx(ruta_productos,fechaCreacion,FS_FILETIME_CREATE)||
            FS_SetFileTimeEx(ruta_productos,timeStamp,FS_FILETIME_MODIFY)||
            FS_SetFileTimeEx(ruta_productos,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}


/*******************************************************************************
* escribirContabilidad
********************************************************************************
* Función para actualizar la contabilidad de la máquina
*******************************************************************************/
CYBIT escribirContabilidadSD(xProducto productoSolicitado){
    CYBIT comando_ok = pdFALSE;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHora = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    /***********************************
    * Escribir contabilidad diaria
    ***********************************/
    if(productoSolicitado.numero!=0){
        if(escribirContabilidadDiaSD(productoSolicitado,fechaHora)){
            if(escribirContabilidadMensualSD(productoSolicitado,fechaHora)){
                if(escribirContabilidadAnualSD(productoSolicitado,fechaHora)){
                    comando_ok = pdTRUE;
                }
            }
        }
    }
    return comando_ok;   
}    
    
/*******************************************************************************
* Contabilidad Diaria 
********************************************************************************
* Funciones para actualizar y obtener la contabilidad dia a dia de la máquina
*******************************************************************************/    
CYBIT escribirContabilidadDiaSD(xProducto productoSolicitado, RTC_RTC_TIME_DATE fechaHora){ 
    CYBIT comando_ok = pdTRUE;
    xLecturaSD contabilidadLeida;
    CYBIT finArchivo = pdFALSE;
    char rutaContabilidad[50];
    char string[100];
    FS_FILE *archivoContabilidad;
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    int precio = 0;
    
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    contabilidadLeida.filaLeida=0;
    
    sprintf(rutaContabilidad,"%s\\%d_%d_%d.csv",
        pName_Dir_Cont_Dia,
        fechaHora.Year,
        fechaHora.Month,
        fechaHora.DayOfMonth
    );
    
    //verifica si hay un archivo existente
    archivoContabilidad = FS_FOpen(rutaContabilidad,"r");
    if(archivoContabilidad != NULL){
        //si existe, guarde la fecha de creación
        while(!finArchivo){
            contabilidadLeida = leerArchivoSD(archivoContabilidad); //lee toda la fila de un producto
            archivoContabilidad = contabilidadLeida.archivo;
            if(contabilidadLeida.finArchivo==pdTRUE){
                finArchivo=pdTRUE;
            }
        }   
        FS_FClose(archivoContabilidad);
        FS_GetFileTime(rutaContabilidad,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
    }
    archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
    if(archivoContabilidad != NULL){
        if(fechaCreacion == timeStamp){
            sprintf(string,"Fila;Producto;Precio;Hora\r\n");
            FS_Write(archivoContabilidad,string,strlen(string));
            contabilidadLeida.filaLeida = 0;
        }
        contabilidadLeida.filaLeida++;
        precio = productoSolicitado.precio;
        sprintf(string,"%d;%d;$%d;%d:%d:%d\r\n",
            contabilidadLeida.filaLeida,
            productoSolicitado.numero,
            precio,
            fechaHora.Hour,
            fechaHora.Min,
            fechaHora.Sec
        );
        FS_Write(archivoContabilidad,string,strlen(string));
        FS_FClose(archivoContabilidad);
        if(FS_SetFileTimeEx(rutaContabilidad,fechaCreacion,FS_FILETIME_CREATE)||
            FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_MODIFY)||
            FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

xCeldasArchivo leerContabilidadDiariaSD(int venta, RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura){
    xCeldasArchivo celdas;
    xLecturaSD fechaLeida;
    FS_FILE *archivoContabilidad;
    char rutaContabilidad[50];
    uint32 timeStamp = obtenerMarcaTiempoSD();
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    sprintf(rutaContabilidad,"%s\\%d_%d_%d.csv",
        pName_Dir_Cont_Dia,
        fechaHora.Year,
        fechaHora.Month,
        fechaHora.DayOfMonth
    );
    archivoContabilidad = FS_FOpen(rutaContabilidad,"r");
    if(archivoContabilidad != NULL){
        if(primeraLectura==pdTRUE){
            do{ //indique cuantas filas hay disponibles para mostrar
                fechaLeida = leerArchivoSD(archivoContabilidad);
                archivoContabilidad = fechaLeida.archivo;
            }while(fechaLeida.finArchivo!=pdTRUE);
            celdas.filasDisponibles = fechaLeida.filaLeida;
        }
        else{
            //ubique el cursor en el principio del documento
            FS_FSeek(archivoContabilidad,puntero,FS_SEEK_SET); 
            do{ //busque el primer registro que concuerde con la venta
                fechaLeida = leerArchivoSD(archivoContabilidad);
                archivoContabilidad = fechaLeida.archivo;
                celdas.celdaPorFila = separarCeldasSD(fechaLeida);
            }while(fechaLeida.filaLeida!=venta);
        }
        celdas.puntero = FS_FTell(archivoContabilidad)-1;
        FS_FClose(archivoContabilidad);
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_ACCESS);
    }
    else{
        celdas.filasDisponibles=0;
    }
    return celdas;
}


/*******************************************************************************
* Contabilidad Mensual
********************************************************************************
* Función para actualizar y leer la contabilidad mensual de la máquina
*******************************************************************************/
CYBIT escribirContabilidadMensualSD(xProducto productoSolicitado, RTC_RTC_TIME_DATE fechaHora){
    CYBIT comando_ok = pdTRUE, primeraVez=pdFALSE;
    char rutaContabilidad[50];
    char mes[15];
    char string[100];
    FS_FILE *archivoContabilidad;
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    RTC_RTC_TIME_DATE fechaDia = fechaHora;
    RTC_RTC_TIME_DATE fechaInicial = fechaHora;
    uint8 dia=0;
    uint8 diasGuardados = 0;
    long puntero = 0;
    int ventas = 0, indice = 0, precio = 0, registros = 0;
    xCeldasArchivo contabilidadLeida;
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    //encuentre el valor en texto del mes solicitado
    switch(fechaHora.Month){
        case(1):
            stpcpy(mes,"enero");
        break;
        case(2):
            stpcpy(mes,"febrero");
        break;
        case(3):
            stpcpy(mes,"marzo");
        break;
        case(4):
            stpcpy(mes,"abril");
        break;
        case(5):
            stpcpy(mes,"mayo");
        break;
        case(6):
            stpcpy(mes,"junio");
        break;
        case(7):
            stpcpy(mes,"julio");
        break;
        case(8):
            stpcpy(mes,"agosto");
        break;
        case(9):
            stpcpy(mes,"septiembre");
        break;
        case(10):
            stpcpy(mes,"octubre");
        break;
        case(11):
            stpcpy(mes,"noviembre");
        break;
        case(12):
            stpcpy(mes,"diciembre");
        break;
        default:
            stpcpy(mes,"no_name");
        break;    
    }
    
    //encuentre la ruta del archivo basado en la fecha solicitada
    sprintf(rutaContabilidad,"%s\\%s_%d.csv",
        pName_Dir_Cont_Mes,
        mes,
        fechaHora.Year
    );
    /***********************************
    * Escribir contabilidad mensual
    ***********************************/
    
    //verifica si hay un archivo existente
    archivoContabilidad = FS_FOpen(rutaContabilidad,"r");
    if(archivoContabilidad != NULL){
        //si existe, guarde la fecha de creación
        FS_FClose(archivoContabilidad);
        FS_GetFileTime(rutaContabilidad,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
        primeraVez=pdTRUE;
    }
    
    //obtenga el numero de registros almacenados disponibles
    fechaInicial.DayOfMonth = 1;
    contabilidadLeida = leerContabilidadMensualSD(fechaInicial,puntero,1);
    diasGuardados = contabilidadLeida.filasDisponibles;
    
    
    //verifica si existen registros de dias futuros al solicitado, en cuyo caso hay un error
    //vuelva a generar el archivo de contabilidad mensual en ese caso
    if(diasGuardados>fechaHora.DayOfMonth){
        fechaCreacion = timeStamp;
        primeraVez=pdTRUE;
        diasGuardados=0;
    }
    
    //verifique si hay menos dias guardados que el dia solicitado, en cuyo caso
    //deberá condensar la información de ese dia para actualizar el archivo mensual
    if(diasGuardados<fechaHora.DayOfMonth){
        //verifica si entre el dia solicitado, y el ultimo dia guardado hay registros
        if((fechaHora.DayOfMonth-diasGuardados)>1){
            //si faltan dias entre el dia solicitado y el dia guardado, reeescriba todo el archivo
            fechaCreacion=timeStamp;
            primeraVez = pdTRUE;
            diasGuardados = 0;
        }
        
        //genere el encabezado si es la primera vez que se escribe
        if((fechaCreacion==timeStamp)&&(primeraVez==pdTRUE)){
            archivoContabilidad = FS_FOpen(rutaContabilidad,"w");
            if(archivoContabilidad != NULL){
                sprintf(string,"Dia;Ventas;Cantidad\r\n");
                FS_Write(archivoContabilidad,string,strlen(string));
                FS_FClose(archivoContabilidad);
                
                //solicite la información de contabilidad de cada dia y condense
                for(dia=1;dia<=fechaHora.DayOfMonth;dia++){
                    puntero=0;
                    //actualice el dia para leer el archivo de contabilidad diaria correspondiente
                    fechaDia.DayOfMonth=dia;
                    contabilidadLeida = leerContabilidadDiariaSD(1,fechaDia,puntero,pdTRUE);
                    registros = contabilidadLeida.filasDisponibles;
                    ventas = 0;
                    for(indice=1;indice<=registros;indice++){
                        contabilidadLeida = leerContabilidadDiariaSD(indice,fechaDia,puntero,pdFALSE);
                        puntero = contabilidadLeida.puntero;
                        sscanf(contabilidadLeida.celdaPorFila.celda2,"$%d",&precio);
                        ventas += precio;
                    }
                    //escriba el archivo de contabilidad mensual con el dato de ventas diarias
                    archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
                    if(archivoContabilidad != NULL){
                        sprintf(string,"%d;$%d;%d\r\n",
                            dia,
                            ventas,
                            registros
                        );
                        FS_Write(archivoContabilidad,string,strlen(string));
                        FS_FClose(archivoContabilidad);
                    }
                    else{
                        comando_ok = pdFALSE;
                    }
                }
            }
            else{
                comando_ok = pdFALSE;
            }
        }
        else{
            puntero=0;
            //obtenga el numero de ventas registradas en el dia
            contabilidadLeida = leerContabilidadDiariaSD(1,fechaDia,puntero,pdTRUE);
            registros = contabilidadLeida.filasDisponibles;
            ventas = 0;
            //lea el archivo de contabilidad diaria completo y calcule el total de dinero
            for(indice=1;indice<=registros;indice++){
                contabilidadLeida = leerContabilidadDiariaSD(indice,fechaDia,puntero,pdFALSE);
                puntero = contabilidadLeida.puntero;
                sscanf(contabilidadLeida.celdaPorFila.celda2,"$%d",&precio);
                ventas += precio;
            }
            
            //escriba el archivo de contabilidad mensual con el dato de ventas diarias
            archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
            if(archivoContabilidad != NULL){
                sprintf(string,"%d;$%d;%d\r\n",
                    fechaDia.DayOfMonth,
                    ventas,
                    registros
                );
                FS_Write(archivoContabilidad,string,strlen(string));
                FS_FClose(archivoContabilidad);
            }
            else{
                comando_ok = pdFALSE;
            }
        }
    }
    else{
        //si hay información, actualice el dia en curso con la nueva venta generada
        fechaHora.DayOfMonth--;
        //ubique el principio de la linea que debe ser actualizada
        contabilidadLeida = leerContabilidadMensualSD(fechaHora,0,0);
        puntero = contabilidadLeida.puntero;
        //ahora lea la fila solicitada que debe ser actualizada
        fechaHora.DayOfMonth++;
        contabilidadLeida = leerContabilidadMensualSD(fechaHora,puntero,0);
        sscanf(contabilidadLeida.celdaPorFila.celda1,"$%d",&ventas);
        sscanf(contabilidadLeida.celdaPorFila.celda2,"%d",&registros);
        ventas += productoSolicitado.precio;
        registros++;
        
        archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
        if(archivoContabilidad != NULL){
            sprintf(string,"%d;$%d;%d\r\n",
                fechaHora.DayOfMonth,
                ventas,
                registros
            );
            FS_FSeek(archivoContabilidad,puntero+1,FS_SEEK_SET);
            FS_Write(archivoContabilidad,string,strlen(string));
            FS_FClose(archivoContabilidad);
        }
        else{
            comando_ok = pdFALSE;
        }
    }
    if(FS_SetFileTimeEx(rutaContabilidad,fechaCreacion,FS_FILETIME_CREATE)||
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_MODIFY)||
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_ACCESS)){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}

xCeldasArchivo leerContabilidadMensualSD(RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura){
    xCeldasArchivo celdas;
    xLecturaSD diaLeido;
    FS_FILE *archivoContabilidad;
    char rutaContabilidad[50];
    char mes[15];
    uint32 timeStamp = obtenerMarcaTiempoSD();
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    switch(fechaHora.Month){
        case(1):
            stpcpy(mes,"enero");
        break;
        case(2):
            stpcpy(mes,"febrero");
        break;
        case(3):
            stpcpy(mes,"marzo");
        break;
        case(4):
            stpcpy(mes,"abril");
        break;
        case(5):
            stpcpy(mes,"mayo");
        break;
        case(6):
            stpcpy(mes,"junio");
        break;
        case(7):
            stpcpy(mes,"julio");
        break;
        case(8):
            stpcpy(mes,"agosto");
        break;
        case(9):
            stpcpy(mes,"septiembre");
        break;
        case(10):
            stpcpy(mes,"octubre");
        break;
        case(11):
            stpcpy(mes,"noviembre");
        break;
        case(12):
            stpcpy(mes,"diciembre");
        break;
        default:
            stpcpy(mes,"no_name");
        break;    
    }
    
    sprintf(rutaContabilidad,"%s\\%s_%d.csv",
        pName_Dir_Cont_Mes,
        mes,
        fechaHora.Year
    );
    
    archivoContabilidad = FS_FOpen(rutaContabilidad,"r");
    if(archivoContabilidad != NULL){
        if(primeraLectura==pdTRUE){
            do{ //indique cuantas filas hay disponibles para mostrar
                diaLeido = leerArchivoSD(archivoContabilidad);
                archivoContabilidad = diaLeido.archivo;
            }while(diaLeido.finArchivo!=pdTRUE);
            celdas.filasDisponibles = diaLeido.filaLeida;
        }
        else{
            //ubique el cursor en el principio del documento
            FS_FSeek(archivoContabilidad,puntero,FS_SEEK_SET); 
            do{ //busque el primer registro que concuerde con el dia solicitado
                diaLeido = leerArchivoSD(archivoContabilidad);
                archivoContabilidad = diaLeido.archivo;
                celdas.celdaPorFila = separarCeldasSD(diaLeido);
            }while(diaLeido.filaLeida!=fechaHora.DayOfMonth);
        }
        celdas.puntero = FS_FTell(archivoContabilidad)-1;
        FS_FClose(archivoContabilidad);
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_ACCESS);
    }
    else{
        celdas.filasDisponibles=0;
    }
    return celdas;
}


/*******************************************************************************
* Contabilidad Anual
********************************************************************************
* Función para actualizar y leer la contabilidad mensual de la máquina
*******************************************************************************/
CYBIT escribirContabilidadAnualSD(xProducto productoSolicitado, RTC_RTC_TIME_DATE fechaHora){
    CYBIT comando_ok = pdTRUE, primeraVez=pdFALSE;
    char rutaContabilidad[100];
    char string[100];
    FS_FILE *archivoContabilidad;
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    RTC_RTC_TIME_DATE fechaMes = fechaHora;
    RTC_RTC_TIME_DATE fechaInicial = fechaHora;
    uint8 mes=0;
    uint8 mesesGuardados = 0;
    long puntero = 0;
    long ventas = 0, indice = 0, precio = 0, registros = 0;
    long cantidad = 0, cantidadPorMes = 0;
    xCeldasArchivo contabilidadLeida;
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    //encuentre la ruta del archivo basado en el año solicitado
    sprintf(rutaContabilidad,"%s\\%d.csv",
        pName_Dir_Cont_Anual,
        fechaHora.Year
    );
    /***********************************
    * Escribir contabilidad anual
    ***********************************/
    
    //verifica si hay un archivo existente
    archivoContabilidad = FS_FOpen(rutaContabilidad,"r");
    if(archivoContabilidad != NULL){
        //si existe, guarde la fecha de creación
        FS_FClose(archivoContabilidad);
        FS_GetFileTime(rutaContabilidad,&fechaCreacion);
        
        //obtenga el numero de registros almacenados disponibles
    fechaInicial.DayOfMonth=1;
    fechaInicial.Month=1;
    contabilidadLeida = leerContabilidadAnualSD(fechaInicial,puntero,pdTRUE);
    mesesGuardados = contabilidadLeida.filasDisponibles;
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
        primeraVez=pdTRUE;
    }
    
    //obtenga el numero de registros almacenados disponibles
    fechaInicial.DayOfMonth=1;
    fechaInicial.Month=1;
    contabilidadLeida = leerContabilidadAnualSD(fechaInicial,puntero,pdTRUE);
    mesesGuardados = contabilidadLeida.filasDisponibles;
    
    //verifique si existe la información de los meses anteriores al solicitado
    //de lo contrario creela condensando la información por mes
    if(mesesGuardados>fechaHora.Month){
        fechaCreacion = timeStamp;
        primeraVez=pdTRUE;
        mesesGuardados=0;
    }
    
    if(mesesGuardados<fechaHora.Month){
        //verifica si entre el mes solicitado, y el ultimo mes guardado hay registros
        if((fechaHora.Month-mesesGuardados)>1){
            //si faltan meses entre el mes solicitado y el ultimo mes guardado, reeescriba todo el archivo
            fechaCreacion=timeStamp;
            primeraVez = pdTRUE;
            mesesGuardados = 0; 
        }
        
        //genere el encabezado si es la primera vez que se escribe
        if((fechaCreacion==timeStamp)&&(primeraVez==pdTRUE)){
            archivoContabilidad = FS_FOpen(rutaContabilidad,"w");
            if(archivoContabilidad != NULL){
                sprintf(string,"Mes;Ventas;Cantidad\r\n");
                FS_Write(archivoContabilidad,string,strlen(string));
                FS_FClose(archivoContabilidad);
                
                //solicite la información de contabilidad de cada mes y condense
                for(mes=1;mes<=fechaHora.Month;mes++){
                    puntero=0;
                    //actualice el mes para leer el archivo de contabilidad mensual correspondiente
                    fechaMes.Month=mes;
                    //obtenga el numero de dias registrados en el mes
                    contabilidadLeida = leerContabilidadMensualSD(fechaMes,puntero,pdTRUE);
                    registros = contabilidadLeida.filasDisponibles;
                    ventas = 0;
                    cantidad = 0;
                    //lea el archivo de contabilidad mensual completo y calcule el total de dinero
                    for(indice=1;indice<=registros;indice++){
                        fechaMes.DayOfMonth=indice;
                        contabilidadLeida = leerContabilidadMensualSD(fechaMes,puntero,pdFALSE);
                        puntero = contabilidadLeida.puntero;
                        sscanf(contabilidadLeida.celdaPorFila.celda1,"$%ld",&precio);
                        sscanf(contabilidadLeida.celdaPorFila.celda2,"%ld",&cantidadPorMes);
                        ventas += precio;
                        cantidad += cantidadPorMes;
                    }
                    //escriba el archivo de contabilidad anual con el dato de ventas mensuales
                    archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
                    if(archivoContabilidad != NULL){
                        sprintf(string,"%d;$%ld;%ld\r\n",
                            mes,
                            ventas,
                            cantidad
                        );
                        FS_Write(archivoContabilidad,string,strlen(string));
                        FS_FClose(archivoContabilidad);
                    }
                    else{
                        comando_ok = pdFALSE;
                    }
                }
            }
            else{
                comando_ok = pdFALSE;
            }
        }
        else{
            puntero=0;
            //obtenga el numero de dias registrados en el mes
            contabilidadLeida = leerContabilidadMensualSD(fechaMes,puntero,pdTRUE);
            registros = contabilidadLeida.filasDisponibles;
            ventas = 0;
            cantidad = 0;
            //lea el archivo de contabilidad mensual completo y calcule el total de dinero
            for(indice=1;indice<=registros;indice++){
                fechaMes.DayOfMonth=indice;
                contabilidadLeida = leerContabilidadMensualSD(fechaMes,puntero,pdFALSE);
                puntero = contabilidadLeida.puntero;
                sscanf(contabilidadLeida.celdaPorFila.celda1,"$%ld",&precio);
                sscanf(contabilidadLeida.celdaPorFila.celda2,"%ld",&cantidadPorMes);
                ventas += precio;
                cantidad += cantidadPorMes;
            }
            
            //escriba el archivo de contabilidad anual con el dato de ventas mensuales
            archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
            if(archivoContabilidad != NULL){
                sprintf(string,"%d;$%ld;%ld\r\n",
                    fechaMes.DayOfMonth,
                    ventas,
                    cantidad
                );
                FS_Write(archivoContabilidad,string,strlen(string));
                FS_FClose(archivoContabilidad);
            }
            else{
                comando_ok = pdFALSE;
            }
        }
    }
    else{
        //si hay información, actualice el mes en curso con la nueva venta generada
        fechaHora.Month--;
        //ubique el principio de la linea que debe ser actualizada
        contabilidadLeida = leerContabilidadAnualSD(fechaHora,0,0);
        puntero = contabilidadLeida.puntero;
        //ahora lea la fila solicitada que debe ser actualizada
        fechaHora.Month++;
        contabilidadLeida = leerContabilidadAnualSD(fechaHora,puntero,0);
        sscanf(contabilidadLeida.celdaPorFila.celda1,"$%ld",&ventas);
        sscanf(contabilidadLeida.celdaPorFila.celda2,"%ld",&cantidad);
        ventas += productoSolicitado.precio;
        cantidad++;
        
        archivoContabilidad = FS_FOpen(rutaContabilidad,"a");
        if(archivoContabilidad != NULL){
            sprintf(string,"%d;$%ld;%ld\r\n",
                fechaHora.Month,
                ventas,
                cantidad
            );
            FS_FSeek(archivoContabilidad,puntero+1,FS_SEEK_SET);
            FS_Write(archivoContabilidad,string,strlen(string));
            FS_FClose(archivoContabilidad);
        }
        else{
            comando_ok = pdFALSE;
        }
    }
    if(FS_SetFileTimeEx(rutaContabilidad,fechaCreacion,FS_FILETIME_CREATE)||
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_MODIFY)||
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_ACCESS)){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}

xCeldasArchivo leerContabilidadAnualSD(RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura){
    xCeldasArchivo celdas;
    xLecturaSD mesLeido;
    FS_FILE *archivoContabilidad;
    char rutaContabilidad[50];
    uint32 timeStamp = obtenerMarcaTiempoSD();
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    sprintf(rutaContabilidad,"%s\\%d.csv",
        pName_Dir_Cont_Anual,
        fechaHora.Year
    );
    
    archivoContabilidad = FS_FOpen(rutaContabilidad,"r");
    if(archivoContabilidad != NULL){
        if(primeraLectura==pdTRUE){
            do{ //indique cuantas filas hay disponibles para mostrar
                mesLeido = leerArchivoSD(archivoContabilidad);
                archivoContabilidad = mesLeido.archivo;
            }while(mesLeido.finArchivo!=pdTRUE);
            celdas.filasDisponibles = mesLeido.filaLeida;
        }
        else{
            //ubique el cursor en el principio del documento
            FS_FSeek(archivoContabilidad,puntero,FS_SEEK_SET); 
            do{ //busque el primer registro que concuerde con el mes solicitado
                mesLeido = leerArchivoSD(archivoContabilidad);
                archivoContabilidad = mesLeido.archivo;
                celdas.celdaPorFila = separarCeldasSD(mesLeido);
            }while(mesLeido.filaLeida!=fechaHora.Month);
        }
        celdas.puntero = FS_FTell(archivoContabilidad)-1;
        FS_FClose(archivoContabilidad);
        FS_SetFileTimeEx(rutaContabilidad,timeStamp,FS_FILETIME_ACCESS);
    }
    else{
        celdas.filasDisponibles=0;
    }
    return celdas;
}

/*******************************************************************************
* Registro diario
********************************************************************************
* Funciones para actualizar y obtener toda la información de trabajo con la máquina
*******************************************************************************/    
CYBIT escribirRegistroSD(xEventos informacion){ 
    CYBIT comando_ok = pdTRUE;
    xLecturaSD registroLeido;
    CYBIT finArchivo = pdFALSE;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHora = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    char rutaRegistro[50];
    char string[100];
    FS_FILE *archivoRegistro;
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();

    registroLeido.filaLeida = 0;
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    sprintf(rutaRegistro,"%s\\%d_%d_%d.csv",
        pName_Dir_Registro,
        fechaHora.Year,
        fechaHora.Month,
        fechaHora.DayOfMonth
    );
    
    //verifica si hay un archivo existente
    archivoRegistro = FS_FOpen(rutaRegistro,"r");
    if(archivoRegistro != NULL){
        //si existe, guarde la fecha de creación
        while(!finArchivo){
            registroLeido = leerArchivoSD(archivoRegistro); //lee toda la fila de un registro
            archivoRegistro= registroLeido.archivo;
            if(registroLeido.finArchivo==pdTRUE){
                finArchivo=pdTRUE;
            }
        }   
        FS_FClose(archivoRegistro);
        FS_GetFileTime(rutaRegistro,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
    }
    
    //actualice el archivo
    archivoRegistro = FS_FOpen(rutaRegistro,"a");
    if(archivoRegistro != NULL){
        if(fechaCreacion == timeStamp){
            sprintf(string,"Fila;Tipo;Evento;Hora\r\n");
            FS_Write(archivoRegistro,string,strlen(string));
            registroLeido.filaLeida = 0;
        }
        registroLeido.filaLeida++;
        
        sprintf(string,"%d;%s;%s;%d:%d:%d\r\n",
            registroLeido.filaLeida,
            informacion.tipo,
            informacion.evento,
            fechaHora.Hour,
            fechaHora.Min,
            fechaHora.Sec
        );
        FS_Write(archivoRegistro,string,strlen(string));
        FS_FClose(archivoRegistro);
        if(FS_SetFileTimeEx(rutaRegistro,fechaCreacion,FS_FILETIME_CREATE)||
            FS_SetFileTimeEx(rutaRegistro,timeStamp,FS_FILETIME_MODIFY)||
            FS_SetFileTimeEx(rutaRegistro,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

xCeldasArchivo leerRegistroSD(int registro, RTC_RTC_TIME_DATE fechaHora, long puntero, CYBIT primeraLectura){
    xCeldasArchivo celdas;
    xLecturaSD fechaLeida;
    FS_FILE *archivoRegistro;
    char rutaRegistro[50];
    uint32 timeStamp = obtenerMarcaTiempoSD();
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    sprintf(rutaRegistro,"%s\\%d_%d_%d.csv",
        pName_Dir_Registro,
        fechaHora.Year,
        fechaHora.Month,
        fechaHora.DayOfMonth
    );
    archivoRegistro = FS_FOpen(rutaRegistro,"r");
    if(archivoRegistro != NULL){
        if(primeraLectura==pdTRUE){
            do{ //indique cuantas filas hay disponibles para mostrar
                fechaLeida = leerArchivoSD(archivoRegistro);
                archivoRegistro = fechaLeida.archivo;
            }while(fechaLeida.finArchivo!=pdTRUE);
            celdas.filasDisponibles = fechaLeida.filaLeida;
        }
        else{
            //ubique el cursor en el principio del documento
            FS_FSeek(archivoRegistro,puntero,FS_SEEK_SET); 
            do{ //busque el primer registro que concuerde con el solicitado
                fechaLeida = leerArchivoSD(archivoRegistro);
                archivoRegistro = fechaLeida.archivo;
                celdas.celdaPorFila = separarCeldasSD(fechaLeida);
            }while(fechaLeida.filaLeida!=registro);
        }
        celdas.puntero = FS_FTell(archivoRegistro)-1;
        FS_FClose(archivoRegistro);
        FS_SetFileTimeEx(rutaRegistro,timeStamp,FS_FILETIME_ACCESS);
    }
    else{
        celdas.filasDisponibles=0;
    }
    return celdas;
}

/*******************************************************************************
* Crédito ingresado
********************************************************************************
* Función para escribir y leer el crédito ingresado
*******************************************************************************/
CYBIT escribirCreditoSD(int credito){
    FS_FILE *archivoCredito;
    char string[30];
    CYBIT comando_ok = pdTRUE;
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    //verifica si hay un archivo existente
    archivoCredito = FS_FOpen(ruta_credito,"r");
    if(archivoCredito != NULL){
        //si existe, guarde la fecha de creación
        FS_FClose(archivoCredito);
        FS_GetFileTime(ruta_credito,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
    }
    archivoCredito = FS_FOpen(ruta_credito,"w");
    if(archivoCredito != NULL){
        if((credito>=0)&&(credito<100000)){
            sprintf(string,"1;Credito;%d\r\n",credito);
        }
        else{
            sprintf(string,"1;Credito;%d\r\n",0);
        }
        FS_Write(archivoCredito,string,strlen(string));
        FS_FClose(archivoCredito);
        if(FS_SetFileTimeEx(ruta_credito,fechaCreacion,FS_FILETIME_CREATE)||
            FS_SetFileTimeEx(ruta_credito,timeStamp,FS_FILETIME_MODIFY)||
            FS_SetFileTimeEx(ruta_credito,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

int leerCreditoSD(){
    int credito;
    xCeldas celdas;
    FS_FILE *archivoCredito;
    xLecturaSD creditoLeido;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    archivoCredito = FS_FOpen(ruta_credito,"r");
    if(archivoCredito != NULL){
        creditoLeido = leerArchivoSD(archivoCredito);
        if(creditoLeido.filaLeida==1){
            celdas = separarCeldasSD(creditoLeido);
            sscanf(celdas.celda2,"%d",&credito);
        }
        FS_FClose(archivoCredito);
        if(FS_SetFileTimeEx(ruta_credito,timeStamp,FS_FILETIME_ACCESS)){
            credito = 0;
        }
    }
    else{
        credito = 0;
    }
    return credito;
}

/*******************************************************************************
* copiarArchivoDescargado
********************************************************************************
* Función especial, gestionada directamente por la telemetria para guardar un
* archivo que esté siendo descargsdo
*******************************************************************************/
CYBIT guardarArchivoDescargaSD(){
    uint32 fechaCreacion = 0;
    uint32 timeStamp = obtenerMarcaTiempoSD();
    FS_FILE *archivoDescargado;
    CYBIT comando_ok=pdTRUE, fin=pdFALSE;
    xinfoDescargada datos;
    Clock_lento_SetDividerRegister(freq_10_Hz,0);
    
    xQueuePeek(datosDescargados,(void*)&datos,10);
    //verifica si hay un archivo existente
    archivoDescargado = FS_FOpen(datos.ruta,"r");
    if(archivoDescargado != NULL){
        //si existe, guarde la fecha de creación
        FS_FClose(archivoDescargado);
        FS_GetFileTime(datos.ruta,&fechaCreacion);
    }
    else{
        //no existe, use la misma fecha de modificación actual
        fechaCreacion = timeStamp;
    }
    archivoDescargado = FS_FOpen(datos.ruta,"w");
    if(archivoDescargado != NULL){
        while(!fin){
            if(xQueueReceive(datosDescargados,(void*)&datos,10)){
                if(datos.estado==pdTRUE){
                    FS_Write(archivoDescargado,datos.datos,datos.numBytes);
                }
                else{
                    comando_ok = pdTRUE;
                    fin = pdTRUE;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        FS_FClose(archivoDescargado);
        if(FS_SetFileTimeEx(ruta_productos,fechaCreacion,FS_FILETIME_CREATE)||
            FS_SetFileTimeEx(ruta_productos,timeStamp,FS_FILETIME_MODIFY)||
            FS_SetFileTimeEx(ruta_productos,timeStamp,FS_FILETIME_ACCESS)){
            comando_ok=pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}
/* [] END OF FILE */