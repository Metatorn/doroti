 /* ==============================================================
 *
 * Copyright EKIA Technology SAS, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;PROGRAMA ENCARGADO DE COMANDAR LA PANTALLA HMI Y DE 
;ESTABLECER LA GESTION DE INFORMACIÓN CON LA INTERFACE DE USUARIO
===============================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h" 
#include "semphr.h"
#include "cypins.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"
#include "math.h"

#include "HMI.h"
//#include "isr_rxHMI.h"
#include "LED_DEBUG.h"
//#include "Clock_HMI.h"
#include "ADC_Vending.h"
#include "dispensador.h"
#include "RTC_EKIA.h"
#include "RTC_RTC.h"
#include "Clock_lento.h"
#include "Telemetria.h"

#include "colas.h"
#include "tipos.h"
#include "semaforos.h"
#include "interface.h"
//#include "pantallaTactil.h"
//#include "billetero.h"
#include "general.h"
#include "memoriaExt.h"
#include "memEEPROM.h"
#include "Ventilador.h"
#include "MDB_Tipos.h"
#include "MDB_billetero.c"
#include "MDB.h"
#include "lectorHuella.h"
#include "dispensador.h"
#include "I2C.h"

#include "BuzzerPWM.h"
#include "AUDIO.h"


//DEFINICION DE CONSTANTES

xnumSerie serie;
xHorasMaquina horasDeMaquina;
xbill_type_recycler recicladorTipo;
xbill_recycler_status recicladorEstado;
CYBIT flag=pdFALSE, flag2=pdFALSE;
uint8 bandera = 0;
int producto = 0;
int contadorRetorno = 0;
int numProducto=0;
uint8 bandeja = 1;
CYBIT fabricante = pdTRUE;
CYBIT volcado= pdFALSE;
CYBIT prueba_motores = pdFALSE;
CYBIT simulacion_venta = pdFALSE;
CYBIT cambio = pdFALSE;
CYBIT TelemetriaEncendido = pdFALSE;
CYBIT TelemetriaApagado = pdFALSE;
CYBIT encendido = pdFALSE;
CYBIT banderaBluetooth = pdFALSE;
CYBIT banderaGSM = pdFALSE;
uint8 offset = 0;
uint16 brilloAnterior = 0;
CYBIT unicaVez = pdFALSE;
CYBIT borradoHuellaEspecifico = pdTRUE;
uint8 contabilidadSolicitada = pdFALSE;
static int matrizProductos[NUMERO_BANDEJAS_MAX][NUMERO_PRODUCTOS_BANDEJA_MAX][6];
xclave clave_temporal;
uint8 usuario = TEMPORAL;
xDatosUsuario usuarioActivo;


/*******************************************************************************
* FUNCION DE ACTUALIZACION DE LA FECHA A PARTIR DE LA PANTALLA HMI
********************************************************************************
* Funcion dedicada a actualizar la fecha y hora del controlador a partir del
* RTC que incorpora la pantalla HMI.
*******************************************************************************/
void actualizarFechaPsoc(){
    RTC_RTC_WriteTime(HMI_read_time());
    //HMI_write_time(); //actualice la fecha de la pantalla (con fines de depuración)
}

/*******************************************************************************
* FUNCION DE ACTUALIZACION DE LA PANTALLA PRINCIPAL EN SEGUNDO PLANO
********************************************************************************
* Funcion dedicada a modificar el valor de credito de la pantalla principal
* esta función es importante ya que el importe de crédito ingresado puede variar
* aunque se encuentre en otra pantalla
*******************************************************************************/
void escribirPrincipal(){
    //int credito=0;
    //char label[29];
    //CYBIT cancelado = pdFALSE;
    xConfiguracion parametros;
    //xQueuePeek(creditoValido,&credito,10);
    xQueuePeek(configuracionesMaquina,&parametros,10);
    //verifique si existe credito disponible o no
    if(parametros.estadoMaquina == ERROR_FUSIBLE){
        /*HMI_set_value("Principal.Estado",1);
        HMI_tx_comando("Principal.t","0","pco",color_error);
        HMI_tx_comando("Principal.t","2","pco",color_error);
        HMI_tx_comando("Principal.t","3","pco",color_error);
        HMI_tx_comando("Principal.t","0","txt",label_error_Maquina1);
        HMI_tx_comando("Principal.t","2","txt",label_error_Maquina2);
        HMI_tx_comando("Principal.t","3","txt",label_error_Maquina3);*/
        //HMI_tx_comando("Pagos.t","2","pco",color_error);
        //HMI_tx_comando("Pagos.t","2","txt",label_error_Maquina4);
    }
}

/*******************************************************************************
* FUNCIONES DE LECTURA Y CONFIGURACIÓN DE PRODUCTOS - MODO CONFIGURACIÓN
********************************************************************************
* Funciones para graficar la pantalla de configuración de productos y para la 
* actualización de esta misma configuración
*******************************************************************************/
void graficarConfProductos(uint8 bandeja,CYBIT primeraVez){
    CYBIT comando_ok = pdFALSE;
    char label[10], string[10];
    int i=0, cantidadProducto = 0;
    int checkSum = 0;
    int /*HMIcheckCant=0,*/ HMIcheckPrecio=0;
    uint8 mascaraGabinete = 0;
    uint16 productosPantalla = 0;
    bufferEntradaHMI respuesta;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesBandejas, &bandejas, 10); 
    cantidadProducto = bandejas.configBandejas.numMotores[bandeja-1];
    for(i=0;i<cantidadProducto;i++){
        mascaraGabinete |= (1<<i);
    }
    if(usuario>=ADMINISTRADOR){
        productosPantalla = (bandeja<<6) | mascaraGabinete;
        HMI_set_value("Producto.Gabinete",productosPantalla);
    }
    while(!comando_ok){
        comando_ok=HMI_ventana(VENTANA_PRODUCTOS);
    }
    do{
        checkSum =0;
        sprintf(string,"GABINETE:%d",bandeja);
        HMI_tx_comando("t","0","txt",string);
        for(i=0; i<cantidadProducto;i++){
            sprintf(string,"%d",i);
            //sprintf(label,"%d",matrizProductos[bandeja-1][i][0]); //numero de producto
            //HMI_tx_comando("Producto.btA",string,"txt",label);
            sprintf(label,"%d",matrizProductos[bandeja-1][i][1]); //habilitacion de producto
            HMI_tx_comando("bt",string,"val",label);
            
            if(matrizProductos[bandeja-1][i][2]==0){
                HMI_tx_comando("bP",string,"txt","PRECIO");
            }
            else{
                sprintf(label,"$%d",matrizProductos[bandeja-1][i][2]); //precio de producto
                HMI_tx_comando("bP",string,"txt",label);
            }
            checkSum += matrizProductos[bandeja-1][i][2];
            if(matrizProductos[bandeja-1][i][3]==0){
                HMI_tx_comando("bC",string,"txt","C");
            }
            else{
                sprintf(label,"%d",matrizProductos[bandeja-1][i][3]);
                HMI_tx_comando("bC",string,"txt",label);
            }
            checkSum += matrizProductos[bandeja-1][i][3];
            
            if(matrizProductos[bandeja-1][i][4]==0){
                HMI_tx_comando("bA",string,"txt","ALTURA");
            }
            else{
                sprintf(label,"%d",matrizProductos[bandeja-1][i][4]);
                HMI_tx_comando("bA",string,"txt",label);
            }
            checkSum += matrizProductos[bandeja-1][i][4];
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        if(!primeraVez){
            do{
                HMI_get_value("checksumtext.txt");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
            sscanf(respuesta.buffer,"%d",&HMIcheckPrecio);
        }
    }while((HMIcheckPrecio!=checkSum)&&(!primeraVez));
    HMI_visualizar("t1",0);
}

void actualizarConfProductos(uint8 bandeja, CYBIT primeraVez){
    int checkSum;
    int /*HMIcheckCant=0,*/ HMIcheckPrecio=0;
    char string[10];
    bufferEntradaHMI respuesta;
    int i=0, precio=0, cantidad=0, altura=0;
    int cantidadProducto;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    cantidadProducto = bandejas.configBandejas.numMotores[bandeja-1];
    HMI_visualizar("t1",0);
    HMI_tx_comando("t","1","txt","ESPERE POR FAVOR...");
    do{
        checkSum = 0;
        for(i=0; i<cantidadProducto;i++){
            //obtiene el valor de habilitacion de productos
            sprintf(string,"bt%d.val",i);
            do{
                //solicite el valor en numero de cada cuadro hasta que sea correctamente recibido
                HMI_get_value(string);
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));    

            if(respuesta.buffer[0]==pdTRUE){
                matrizProductos[bandeja-1][i][1]=pdTRUE;
            }else{
                matrizProductos[bandeja-1][i][1]=pdFALSE;
            }
            
            //obtiene el valor del precio
            sprintf(string,"bP%d.txt",i);
            do{
                //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
                HMI_get_value(string);
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
            
            if(!strcmp(respuesta.buffer,"PRECIO")){
                precio=0;
            }else{
                sscanf(respuesta.buffer,"$%d",&precio);
            }
            matrizProductos[bandeja-1][i][2]=precio;
            checkSum+=matrizProductos[bandeja-1][i][2];
            
            //obtiene el valor de cantidad
            sprintf(string,"bC%d.txt",i);
            do{
                HMI_get_value(string);
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));   
            
            if(!strcmp(respuesta.buffer,"C")){
                matrizProductos[bandeja-1][i][3]=0;
            }else{
                sscanf(respuesta.buffer,"%d",&cantidad);
                matrizProductos[bandeja-1][i][3]=cantidad;
            }
            checkSum+=matrizProductos[bandeja-1][i][3];
            
            //obtiene el valor de altura
            sprintf(string,"bA%d.txt",i);
            do{
                HMI_get_value(string);
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));   
            
            if(!strcmp(respuesta.buffer,"ALTURA")){
                matrizProductos[bandeja-1][i][4]=0;
            }else{
                sscanf(respuesta.buffer,"%d",&altura);
                matrizProductos[bandeja-1][i][4]=altura;
            }
            checkSum+=matrizProductos[bandeja-1][i][4];
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        if(!primeraVez){
            do{
                HMI_get_value("checksumtext.txt");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
            sscanf(respuesta.buffer,"%d",&HMIcheckPrecio);
        }
    }while((HMIcheckPrecio!=checkSum)&&(!primeraVez));
}

/*******************************************************************************
* FUNCIONES DE LECTURA Y CONFIGURACIÓN DE BANDEJAS - MODO CONFIGURACIÓN
********************************************************************************
* Funciones para graficar la pantalla de configuración de bandejas y para la 
* actualización de esta misma configuración
*******************************************************************************/
void graficarConfBandejas(){
    char label[10], string[10];
    int i=0, j=0;
    uint8 cantidadBandejas = 0;
    xConfiguracionBandejas parametros;
    xQueuePeek(configuracionesBandejas, &parametros, 10);
    cantidadBandejas = parametros.configBandejas.numBandejas;
    sprintf(string,"%d",cantidadBandejas);
    HMI_tx_comando("Calibrar.","NumBandeja","val",string);
    j=0;
    for(i=0; i<NUMERO_BANDEJAS_MAX;i++){
        sprintf(label,"%d",i+1);
        sprintf(string,"%d",j+1);
        HMI_tx_comando("Calibrar.btA",string,"txt",label);//numero de bandeja
        if(parametros.tiemposBandeja[i]==0){
            HMI_tx_comando("Calibrar.btB",string,"txt","Tiempo");
        }
        else{
            sprintf(label,"%dms",parametros.tiemposBandeja[i]); //tiempo de arranque
            HMI_tx_comando("Calibrar.btB",string,"txt",label);
        }
        if(parametros.configBandejas.numMotores[i]==0){
            HMI_tx_comando("Calibrar.btC",string,"txt","Cant");
        }
        else{
            sprintf(label,"%d",parametros.configBandejas.numMotores[i]);
            HMI_tx_comando("Calibrar.btC",string,"txt",label);
        }
        j++;
    }
    while(j<=NUMERO_BANDEJAS_MAX){
        sprintf(string,"%d",j+1);
        HMI_tx_comando("Calibrar.btA",string,"txt"," ");
        HMI_tx_comando("Calibrar.btA",string,"val"," ");
        HMI_tx_comando("Calibrar.btB",string,"txt"," ");
        HMI_tx_comando("Calibrar.btC",string,"txt"," ");
        j++;
    }
    if(parametros.parametrosMotores.tiempoMovimiento==0){
        HMI_tx_comando("Calibrar.btB","8","txt","Tiempo");
    }
    else{
        sprintf(label,"%dms",parametros.parametrosMotores.tiempoMovimiento); //tiempo de arranque
        HMI_tx_comando("Calibrar.btB","8","txt",label);
    }
    if(parametros.parametrosMotores.cicloUtil==0){
        HMI_tx_comando("Calibrar.btB","9","txt","Ciclo");
    }
    else{
        sprintf(label,"%d%%",parametros.parametrosMotores.cicloUtil); //tiempo de arranque
        HMI_tx_comando("Calibrar.btB","9","txt",label);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    HMI_visualizar("p0",pdFALSE);
    xQueueOverwrite(configuracionesBandejas,(void*)&parametros);
}

void actualizarConfBandejas(){
    char string[10];
    bufferEntradaHMI respuesta;
    int i=0, j=0;
    uint8 cantidadBandejas = NUMERO_BANDEJAS_MAX;
    int cantidad = 0;
    xConfiguracionBandejas parametros;
    xQueuePeek(configuracionesBandejas, &parametros, 10);
    j=0;
    parametros.configBandejas.numBandejas=0;
    for(i=0; i<cantidadBandejas;i++){
        //obtiene el valor del tiempo de arranque
        sprintf(string,"btB%d.txt",j+1);
        do{
            //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
            HMI_get_value(string);
            respuesta = HMI_rx_comando();
        }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
        
        if(!strcmp(respuesta.buffer,"Tiempo")){
            parametros.tiemposBandeja[i]=0;
        }else{
            sscanf(respuesta.buffer,"%hdms",&parametros.tiemposBandeja[i]);
        }
        
    
        //obtiene el valor de habilitacion de bandejas
        sprintf(string,"btA%d.val",j+1);
        do{
            //solicite el valor en numero de cada cuadro hasta que sea correctamente recibido
            HMI_get_value(string);
            respuesta = HMI_rx_comando();
        }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));    

        if(respuesta.buffer[0]==pdTRUE){
            parametros.configBandejas.estado[i]=pdTRUE;
            parametros.configBandejas.numBandejas++;
        }else{
            parametros.configBandejas.estado[i]=pdFALSE;
        }
        
        //obtiene el valor de cantidad de motores
        sprintf(string,"btC%d.txt",j+1);
        do{
            HMI_get_value(string);
            respuesta = HMI_rx_comando();
        }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));   
        
        if(!strcmp(respuesta.buffer,"Cant")){
            parametros.configBandejas.numMotores[i]=0;
        }else{
            sscanf(respuesta.buffer,"%d",&cantidad);
            parametros.configBandejas.numMotores[i]=cantidad;
        }
        j++;
    }
    do{
        HMI_get_value("btB8.txt");
        respuesta = HMI_rx_comando();
    }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));   
    if(!strcmp(respuesta.buffer,"Tiempo")){
        parametros.parametrosMotores.tiempoMovimiento=0;
    }else{
        sscanf(respuesta.buffer,"%hdms",&parametros.parametrosMotores.tiempoMovimiento);
    }
    do{
        HMI_get_value("btB9.txt");
        respuesta = HMI_rx_comando();
    }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));   
    if(!strcmp(respuesta.buffer,"Ciclo")){
        parametros.parametrosMotores.cicloUtil=0;
    }else{
        sscanf(respuesta.buffer,"%hd%%",&parametros.parametrosMotores.cicloUtil);
    }
    //parametros.tiempoMovimiento = 3500;
    xQueueOverwrite(configuracionesBandejas,(void*)&parametros);
    vTaskDelay(pdMS_TO_TICKS(100));
}

/*******************************************************************************
* FUNCIONES DE VALIDACIÓN DE PRODUCTOS SOLICITADOS POR EL USUARIO
********************************************************************************
* Función que determina si el producto es valido y se encuentra habilitado
*******************************************************************************/
xProducto validarProducto(){
    xProducto productoSolicitado;
    xConfiguracionBandejas bandejas;
    productoSolicitado.habilitado = pdFALSE;
    int temp, tempBandeja;
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    //si el producto es menor a 100 ya indica un error
    if(producto>=100){ 
        tempBandeja = producto/100;
        temp = tempBandeja;
        //si el numero de bandeja solicitada no esta disponible, hay un error
        if(bandejas.configBandejas.estado[tempBandeja-1]){
            temp=producto-(temp*100);
            //si el numero de columna solicitada es mayor a la disponible, hay un error
            if(temp<bandejas.configBandejas.numMotores[tempBandeja-1]){
                //no hay error en el ingreso del producto, verifique si está habilitado
                while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
                productoSolicitado = leerProducto(producto);
                xSemaphoreGive(eepromOcupada);
            }
        }
    }
    return productoSolicitado;
}


/*******************************************************************************
* FUNCIONES DE GRAFICACION DE CONTABILIDAD
********************************************************************************
* Funciones que permiten graficar la contabilidad para diferentes intervalos de tiempo
*******************************************************************************/
int graficarContabilidadDiaria(){
    xCeldasArchivo contabilidadLeida;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHora = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    uint8 fila=0;
    char objeto[30];
    char texto[30];
    int puntero=0;
    int registros=0;
    
    while(!xSemaphoreTake(memoriaOcupada,100));
    contabilidadLeida = leerContabilidadDiariaSD(1,fechaHora,puntero,pdTRUE);
    registros = contabilidadLeida.filasDisponibles;
    //hay menos de 13 registros? entonces no mueva la tabla
    if(registros<=NUM_CELDAS_CONTABILIDAD){
        offset = 0;
        HMI_visualizar("b1",pdFALSE);
        HMI_visualizar("b2",pdFALSE);
    }
    else{
        HMI_visualizar("b1",pdTRUE);
        HMI_visualizar("b2",pdTRUE);
        if(NUM_CELDAS_CONTABILIDAD+offset>=registros){
            offset=registros-NUM_CELDAS_CONTABILIDAD;
        }
    }
    //grafique las celdas solicitadas
    for(fila=1;fila<=NUM_CELDAS_CONTABILIDAD;fila++){
        if(fila<=registros){
            contabilidadLeida = leerContabilidadDiariaSD(fila+offset,fechaHora,puntero,pdFALSE);
            puntero = contabilidadLeida.puntero;
            sprintf(objeto,"%d",fila+1);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda1); //numero de producto
            HMI_tx_comando("tA",objeto,"txt",texto);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda2); //precio producto
            HMI_tx_comando("tB",objeto,"txt",texto);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda3); //Hora
            HMI_tx_comando("tC",objeto,"txt",texto);
        }
        else{
            break;
        }
    }
    xSemaphoreGive(memoriaOcupada);
    return registros;
}

void graficarContabilidadSemanal(){
    xCeldasArchivo contabilidadLeida;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHoraActual = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    RTC_RTC_TIME_DATE fechaHoraAnterior = fechaHoraActual;
    uint8 fila=1;
    int indice = 0;
    char objeto[30];
    char texto[30];
    long puntero=0;
    int registros = 0;
    int ventas = 0, precio = 0;
    
    HMI_visualizar("b1",pdFALSE);
    HMI_visualizar("b2",pdFALSE);
    
    //obtenga la fecha de 7 dias anteriores
    fechaHoraAnterior = RTC_fechaRelativa(fechaHoraActual,-NUM_DIAS_CONT_SEMANAL,0,0);
    //realiza la lectura de los archivos de contabilidad de cada dia
    while(!xSemaphoreTake(memoriaOcupada,100));
    while(fila<=NUM_DIAS_CONT_SEMANAL+1){
        puntero=0;
        contabilidadLeida = leerContabilidadDiariaSD(1,fechaHoraAnterior,puntero,pdTRUE);
        registros = contabilidadLeida.filasDisponibles;
        ventas = 0;
        for(indice=1;indice<=registros;indice++){
            contabilidadLeida = leerContabilidadDiariaSD(indice,fechaHoraAnterior,puntero,pdFALSE);
            puntero = contabilidadLeida.puntero;
            //precio = atoi(contabilidadLeida.celdaPorFila.celda2);
            sscanf(contabilidadLeida.celdaPorFila.celda2,"$%d",&precio);
            ventas += precio;
        }
        sprintf(objeto,"%d",fila+1);
        sprintf(texto,"%d/%d",fechaHoraAnterior.DayOfMonth,fechaHoraAnterior.Month);//Dia
        HMI_tx_comando("tA",objeto,"txt",texto);
        sprintf(texto,"$%d",ventas);//Ventas
        //strcpy(texto,contabilidadLeida.celdaPorFila.celda2);
        HMI_tx_comando("tB",objeto,"txt",texto);
        sprintf(texto,"%d",registros);//Cantidad de Ventas
        HMI_tx_comando("tC",objeto,"txt",texto);
        fila++;
        fechaHoraAnterior = RTC_fechaRelativa(fechaHoraAnterior,1,0,0);
    }
    xSemaphoreGive(memoriaOcupada);
}

int graficarContabilidadMensual(){
    xCeldasArchivo contabilidadLeida;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHora = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    RTC_RTC_TIME_DATE fechaInicial = fechaHora;
    uint8 fila=0;
    char objeto[30];
    char texto[30];
    int puntero=0;
    int registros=0;
    
    while(!xSemaphoreTake(memoriaOcupada,100));
    fechaInicial.DayOfMonth = 1;
    contabilidadLeida = leerContabilidadMensualSD(fechaInicial,puntero,pdTRUE);
    registros = contabilidadLeida.filasDisponibles;
    //hay menos de 13 registros? entonces no mueva la tabla
    if(registros<=NUM_CELDAS_CONTABILIDAD){
        offset = 0;
        HMI_visualizar("b1",pdFALSE);
        HMI_visualizar("b2",pdFALSE);
    }
    else{
        HMI_visualizar("b1",pdTRUE);
        HMI_visualizar("b2",pdTRUE);
        if(NUM_CELDAS_CONTABILIDAD+offset>=registros){
            offset=registros-NUM_CELDAS_CONTABILIDAD;
        }
    }
    //grafique las celdas solicitadas
    for(fila=1;fila<=NUM_CELDAS_CONTABILIDAD;fila++){
        if(fila<=registros){
            fechaHora.DayOfMonth = fila+offset;
            contabilidadLeida = leerContabilidadMensualSD(fechaHora,puntero,pdFALSE);
            puntero = contabilidadLeida.puntero;
            sprintf(objeto,"%d",fila+1);
            sprintf(texto,"%d",fechaHora.DayOfMonth);//numero del dia
            HMI_tx_comando("tA",objeto,"txt",texto);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda1); //ventas del dia
            HMI_tx_comando("tB",objeto,"txt",texto);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda2); //cantidad de ventas en el dia
            HMI_tx_comando("tC",objeto,"txt",texto);
        }
        else{
            break;
        }
    }
    xSemaphoreGive(memoriaOcupada);
    return registros;
}

void graficarContabilidadAnual(){
    xCeldasArchivo contabilidadLeida;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHora = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    RTC_RTC_TIME_DATE fechaInicial = fechaHora;
    uint8 fila=0;
    char objeto[30];
    char texto[30];
    int puntero=0;
    int registros=0;
    
    while(!xSemaphoreTake(memoriaOcupada,100));
    fechaInicial.DayOfMonth = 1;
    fechaInicial.Month = 1;
    contabilidadLeida = leerContabilidadAnualSD(fechaInicial,puntero,pdTRUE);
    registros = contabilidadLeida.filasDisponibles;
    //hay menos de 13 registros? entonces no mueva la tabla
    if(registros<=NUM_CELDAS_CONTABILIDAD){
        offset = 0;
        HMI_visualizar("b1",pdFALSE);
        HMI_visualizar("b2",pdFALSE);
    }
    else{
        HMI_visualizar("b1",pdTRUE);
        HMI_visualizar("b2",pdTRUE);
        if(NUM_CELDAS_CONTABILIDAD+offset>=registros){
            offset=registros-NUM_CELDAS_CONTABILIDAD;
        }
    }
    //grafique las celdas solicitadas
    for(fila=1;fila<=NUM_CELDAS_CONTABILIDAD;fila++){
        if(fila<=registros){
            fechaHora.Month = fila+offset;
            contabilidadLeida = leerContabilidadAnualSD(fechaHora,puntero,pdFALSE);
            puntero = contabilidadLeida.puntero;
            sprintf(objeto,"%d",fila+1);
            sprintf(texto,"%d",fechaHora.Month);//numero del mes
            HMI_tx_comando("tA",objeto,"txt",texto);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda1); //ventas del mes
            HMI_tx_comando("tB",objeto,"txt",texto);
            strcpy(texto,contabilidadLeida.celdaPorFila.celda2); //cantidad de ventas en el mes
            HMI_tx_comando("tC",objeto,"txt",texto);
        }
        else{
            break;
        }
    }
    xSemaphoreGive(memoriaOcupada);
}

/*******************************************************************************
* FUNCION DE BRILLO DE LA PANTALLA
********************************************************************************
* Funcion dedicada a modificar el brillo de la pantalla de acuerdo al nivel de
* iluminación de la habitación donde se encuentre la máquina
*******************************************************************************/
void brillo(){
    uint16 brillo;
    xConfiguracion parametros;
    xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
    if(parametros.BrilloPantalla.activo){
        brillo = (ADC_Vending_GetResult16()*100)/4095;
        if(brillo<10){
            brillo=10;
        }
    }
    else{
        brillo = parametros.BrilloPantalla.nivel;
        if(brillo<10){
            brillo=10;
        };
        if(brillo>=100){
            brillo = 100;
        }
    }
    if(brillo!=brilloAnterior){
        HMI_brillo(brillo);
        brilloAnterior=brillo;
    }
}

/*******************************************************************************
* FUNCION DE POTENCIA DE SEÑAL
********************************************************************************
* Funcion dedicada a mostrar la potencia de la señal de comunicación GSM en la pantalla
*******************************************************************************/
void potencia(){
    int poten;
    xConfiguracionTelemetria parametros;
    xInformacionGSM infoGSM;
    xQueuePeek(datosGSM, (void*)&infoGSM,10);
    xQueuePeek(configuracionesRemotas, (void*)&parametros, 10);
    if(parametros.estadoTelemetria==pdTRUE){
        if((infoGSM.potenciaSenal.resultado1>0)&&(infoGSM.potenciaSenal.resultado1<35)){
            poten=infoGSM.potenciaSenal.resultado1/7;
            HMI_set_value("senal",poten);
            if(!(strcmp(infoGSM.estado,"CONECTADO"))){
                HMI_set_value("ping",1);
            }
            else{
                HMI_set_value("ping",0);
            }
        }
    }
    else{
        HMI_set_value("senal",0);
        HMI_set_value("ping",0);
    }
}

/*******************************************************************************
* FUNCION DE TEMPERATURA
********************************************************************************
* Funcion dedicada a mostrar la ptemperatura de la maquina en pantalla de espera
*******************************************************************************/
/*void temperaturaEspera(){
    int entero;
    xConfiguracionTemperatura parametros;
    xMedidaDispensador temperatura;
    xQueuePeek(configuracionesTemperatura, (void*)&parametros, 10);
    HMI_set_value("Servicios.descanso",parametros.refrigeracion.descanso);
    HMI_set_value("Principal.descanso",parametros.refrigeracion.descanso);
    while(!xSemaphoreTake(busOcupado,10));
    temperatura = Dispensador_LeerTemperatura();
    xSemaphoreGive(busOcupado);
    entero = temperatura.medida*10;
    HMI_set_value("Servicios.temperatura",entero);
    HMI_set_value("Principal.temperatura",entero);
}*/

/*******************************************************************************
* FUNCION DE ACTUALIZACION DE REGISTRO DE EVENTOS
********************************************************************************
* Funcion que condensa todos los posibles mensajes que serán guardados en
* la memoria SD en el archivo de registro diario
*******************************************************************************/
void actualizar_registro(int contexto, int producto, int precio, char* textoAux){
    xEventos evento;
    switch (contexto){
        case contexto_arranque:
            strcpy(evento.tipo,tipo_anuncio);
            strcpy(evento.evento,text_registro_arranque);
            evento.operacion = ACTUALIZAR_REGISTRO;  
        break;
        case contexto_error_caida:
            strcpy(evento.tipo,tipo_error);
            sprintf(evento.evento,"%s%d",text_registro_error_caida,producto);
            evento.operacion = ACTUALIZAR_REGISTRO;  
        break;
        case contexto_credito:
            strcpy(evento.tipo,tipo_operacion);
            sprintf(evento.evento,"%s%d",text_registro_credito,precio);
            evento.operacion = ACTUALIZAR_REGISTRO;  
        break;   
        case contexto_venta_ok:
            strcpy(evento.tipo,tipo_operacion);
            sprintf(evento.evento,"%s%d",text_registro_venta_ok,producto);
            evento.operacion = ACTUALIZAR_REGISTRO;  
        break;
        case contexto_venta_aviso:
            strcpy(evento.tipo,tipo_operacion);
            sprintf(evento.evento,"%s%d",text_registro_venta_aviso,precio);
            evento.operacion = ACTUALIZAR_REGISTRO;  
        break; 
	    case contexto_error_comunicacion:
            strcpy(evento.tipo,tipo_error);
	        sprintf(evento.evento,"%s%d",text_registro_comunicacion,producto);
            evento.operacion = ACTUALIZAR_REGISTRO;  
        break;     
        default:
        break;
    }
    xQueueSendToBack(actualizarRegistro,&evento,10);  
}

void graficarRegistroDiaria(){
    xCeldasArchivo registroLeido;
    taskENTER_CRITICAL();
    RTC_RTC_TIME_DATE fechaHora = *RTC_RTC_ReadTime();
    taskEXIT_CRITICAL();
    uint8 fila=0;
    char objeto[30];
    char texto[30];
    int puntero=0;
    int registros=0;
    
    while(!xSemaphoreTake(memoriaOcupada,100));
    registroLeido = leerRegistroSD(1,fechaHora,puntero,pdTRUE);
    registros = registroLeido.filasDisponibles;
    //hay menos de 13 registros? entonces no mueva la tabla
    if(registros<=NUM_CELDAS_CONTABILIDAD){
        offset = 0;
        HMI_visualizar("b1",pdFALSE);
        HMI_visualizar("b2",pdFALSE);
    }
    else{
        HMI_visualizar("b1",pdTRUE);
        HMI_visualizar("b2",pdTRUE);
        if(NUM_CELDAS_CONTABILIDAD+offset>=registros){
            offset=registros-NUM_CELDAS_CONTABILIDAD;
        }
    }
    //grafique las celdas solicitadas
    for(fila=1;fila<=NUM_CELDAS_CONTABILIDAD;fila++){
        if(fila<=registros){
            registroLeido = leerRegistroSD(fila+offset,fechaHora, puntero,pdFALSE);
            puntero = registroLeido.puntero;
            sprintf(objeto,"%d",fila+1);
            strcpy(texto,registroLeido.celdaPorFila.celda1); //tipo de evento
            HMI_tx_comando("tA",objeto,"txt",texto);
            strcpy(texto,registroLeido.celdaPorFila.celda2); //evento
            HMI_tx_comando("tB",objeto,"txt",texto);
            strcpy(texto,registroLeido.celdaPorFila.celda3); //Hora
            HMI_tx_comando("tC",objeto,"txt",texto);
        }
        else{
            break;
        }
    }
    xSemaphoreGive(memoriaOcupada);
}

/*******************************************************************************
* FUNCION DE GRAFICACION DE NOTIFICACIONES
********************************************************************************
* Funcion que genera las pantallas para notificaciones, el parametro de entrada
* es la cantidad de argumentos que va a graficar en la pantalla
*******************************************************************************/
void notificacion(CYBIT primeraVez, int tiempo, char* imagen, char* imagenEspera, int cantArgumentos, ...){
    va_list parametros;
    char texto[10], numero[10];
    uint8 i=0;
    CYBIT comando_ok = pdFALSE, fin = pdFALSE;
    int contador = 0;
    bufferEntradaHMI respuesta;
    va_start(parametros, cantArgumentos);

    HMI_tx_comando("Notificacion.va","0","val",imagenEspera);
    vTaskDelay(pdMS_TO_TICKS(1));
    HMI_tx_comando("Notificacion.p","0","pic",imagen);
    vTaskDelay(pdMS_TO_TICKS(1));
    while(!fin){
        sprintf(numero,"%d",i);
        HMI_tx_comando("Notificacion.t",numero,"txt",va_arg(parametros, char*));
        i++;
        if((i>=cantArgumentos)||(i>=3)){
            fin = pdTRUE;
        }
    }
    while(i<3){
        sprintf(numero,"%d",i);
        HMI_tx_comando("Notificacion.t",numero,"txt",label_vacio);
        i++;
    }
    if(primeraVez){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_NOTIFICACIONES);
        }
    }
    fin = pdFALSE;
    if(cantArgumentos>3){
        while(!fin){
            sprintf(numero,"%d",i);
            sprintf(texto,"t%d",i);
            HMI_tx_comando("t",numero,"txt",va_arg(parametros, char*));
            HMI_visualizar(texto,pdTRUE);
            i++;
            if(i>=cantArgumentos){
                fin = pdTRUE;
            }
        }
    }
    va_end(parametros);
    comando_ok = pdFALSE;
    while((!comando_ok)&&(contador<tiempo)){
        HMI_get_value("estado.val");
        respuesta = HMI_rx_comando();
        if(respuesta.objeto==HMI_DATO_NUMERICO){
            if(respuesta.buffer[0]==pdTRUE){
                comando_ok = pdTRUE;
            }
        }
        contador++;
        //vTaskDelay(pdMS_TO_TICKS(tiempo));
    }
    bandera = 0;
}

/*******************************************************************************
* FUNCION DE GRAFICACION DE CONFIGURACION GENERAL
********************************************************************************
* Funcion que genera las pantallas para configuración general, el parametro de entrada
* es similar a la funcion sprintfque envia un formato de cadena primero
*******************************************************************************/
void confGeneral(CYBIT primeraVez, char* formato, ...){
    va_list parametros;
    char *c,numero;
    char string[10];
    int visualizacion, toque;
    CYBIT comando_ok = pdFALSE;
    
    va_start(parametros, formato);

    for(c=formato; *c != '\0'; c++){
        if(*c!='%'){
            continue;
        }
        switch(*++c){
            case 't':
                numero = *++c;
                sprintf(string,"%c",numero);
                HMI_tx_comando("ConfGeneral.t",string,"txt",va_arg(parametros, char*));
            break;
            case 'b':
                if((*++c)!='t'){
                    numero = *c;
                    sprintf(string,"%c",numero);
                    HMI_tx_comando("ConfGeneral.b",string,"txt",va_arg(parametros, char*));
                }
            break;
            case 'j':
                numero = *++c;
                sprintf(string,"%c",numero);
                HMI_tx_comando("ConfGeneral.j",string,"val",va_arg(parametros, char*));
            break;
        }
    }
    va_end(parametros);
    if(primeraVez){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CONF_GENERAL);
        }
    }
    
    for(c=formato; *c != '\0'; c++){
        if(*c!='%'){
            continue;
        }
        switch(*++c){
            case 'b':
                if(*++c != 't'){
                    numero = *c;
                    if((*++c)=='-'){
                        visualizacion = atoi(++c);
                        sprintf(string,"b%c",numero);
                        HMI_visualizar(string,visualizacion);
                        if((*++c)=='-'){
                            toque=atoi(++c);
                            HMI_touch(string,"",toque);
                        }
                        sprintf(string,"b1%c",numero);
                        HMI_visualizar(string,visualizacion);
                    }
                }
                else{
                    numero = *++c;
                    if((*++c)=='-'){
                        visualizacion = atoi(++c);
                        sprintf(string,"bt%c",numero);
                        HMI_visualizar(string,visualizacion);
                    }
                }
            break;
            case 'j':
                numero = *++c;
                if((*++c)=='-'){
                    visualizacion = atoi(++c);
                    sprintf(string,"j%c",numero);
                    HMI_visualizar(string,visualizacion);
                }
            break;
        }
    }
    bandera = pdTRUE;
}

/*******************************************************************************
* FUNCION DE GRAFICACION DE CONFIRMACION
********************************************************************************
* Funcion que genera las pantallas para confirmacion general
*******************************************************************************/
void confCancel(CYBIT primeraVez, char* formato, ...){
    va_list parametros;
    char *c,numero;
    char string[10];
    int visualizacion;
    CYBIT comando_ok = pdFALSE;
    
    va_start(parametros, formato);

    for(c=formato; *c != '\0'; c++){
        if(*c!='%'){
            continue;
        }
        if(*++c == 't'){
            numero = *++c;
            sprintf(string,"%c",numero);
            HMI_tx_comando("ConfCancel.t",string,"txt",va_arg(parametros, char*));
        }
    }
    va_end(parametros);
    if(primeraVez){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CONFIRMACION);
        }
    }
    
    for(c=formato; *c != '\0'; c++){
        if(*c!='%'){
            continue;
        }
        switch(*++c){
            case 'b':
                numero = *++c;
                if((*++c)=='-'){
                    visualizacion = atoi(++c);
                    sprintf(string,"b%c",numero);
                    HMI_visualizar(string,visualizacion);
                    HMI_touch(string,"",visualizacion);
                }
            break;
            case 't':
                numero = *++c;
                if((*++c)=='-'){
                    visualizacion = atoi(++c);
                    sprintf(string,"t%c",numero);
                    HMI_visualizar(string,visualizacion);
                }
            break;
        }
    }
    bandera = 1;
}

/*******************************************************************************
* FUNCION DE GRAFICACION DE MENU DE SELECCION GENERAL
********************************************************************************
* Funcion que genera las pantallas para seleccion general, el parametro de entrada
* es similar a la funcion sprintf que envia un formato de cadena primero
*******************************************************************************/
void seleccion(CYBIT primeraVez, CYBIT selMultiple, char* formato, ...){
    va_list parametros;
    char *c,numero;
    char string[10];
    int visualizacion, valor;
    CYBIT comando_ok = pdFALSE;
    
    va_start(parametros, formato);

    for(c=formato; *c != '\0'; c++){
        if(*c!='%'){
            continue;
        }
        if(*++c == 't'){
            numero = *++c;
            sprintf(string,"%c",numero);
            HMI_tx_comando("Seleccion.t",string,"txt",va_arg(parametros, char*));
        }
    }
    va_end(parametros);
    if(primeraVez){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_SELECCION);
        }
    }
    if(selMultiple){
        HMI_set_value("tipo",1);
    }
    else{
        HMI_set_value("tipo",2);
         
    }
    
    for(c=formato; *c != '\0'; c++){
        if(*c!='%'){
            continue;
        }
        switch(*++c){   
            case 't':
                numero = *++c;
                if((*++c)=='-'){
                    visualizacion = atoi(++c);
                    sprintf(string,"t%c",numero);
                    HMI_visualizar(string,visualizacion);
                    sprintf(string,"c%c",numero-1);
                    HMI_visualizar(string,visualizacion);
                    if((*++c)=='-'){
                        valor=atoi(++c);
                        HMI_set_value(string,valor);
                    }
                }
            break;    
        }
    }
    bandera = pdTRUE;
}

/*******************************************************************************
* FUNCIONES DE ESCRITURA DE VENTANA INFORMACION
********************************************************************************
* Funciones que escriben en cada celda de la pantalla de información
*******************************************************************************/
void escribirInformacionNumero(uint8 fila, char* descripcion, int numero){
    char objeto[30], celdas[4];
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",descripcion);
    sprintf(objeto," %d", numero);
    HMI_tx_comando("tB",celdas,"txt",objeto);
}

void escribirInformacionTexto(uint8 fila, char* descripcion, char* contenido){
    char objeto[30], celdas[4];
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",descripcion);
    strcpy(objeto,contenido);
    HMI_tx_comando("tB",celdas,"txt",objeto);
}

/*******************************************************************************
* FUNCION DE DISPENSACION DE PRODUCTO
********************************************************************************
* Funcion que dispensa el producto solicitado ya se como venta o como prueba
*******************************************************************************/
void dispensar(){
    CYBIT primeravez=pdFALSE;
    xProducto productoSolicitado;
    //int credito = 0, creditoAnt = 0;
    //xresPantalla peticion;
    xCanasta canasta;
    xContabilidad contabilidad;
    xConfiguracion parametros;
    uint8 memoriaSD;
    int precioTemp;
    CYBIT transaccion = pdFALSE;
    uint8 comando = 0;
    int timeOut=0;
    CYBIT finMovimiento = pdFALSE;
    
    xQueuePeek(estadoMemoriaSD, (void*)&memoriaSD, 10);
    xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
    bandera=pdFALSE; 
    //indica que hay una operación en proceso, para evitar que se pueda devolver dinero de más
    while(!xSemaphoreTake(operacionOcupada,( TickType_t )10));
    //lee los parametros del producto solicitado
    while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
    productoSolicitado = leerProducto(producto);
    xSemaphoreGive(eepromOcupada);
    //solicita el credito que haya sido ingresado hasta el momento
    /*xQueuePeek(creditoValido,(void*)&credito,100);
    xQueuePeek(respuestaPantalla,(void*)&peticion,100);
    actualizar_registro(contexto_credito,0,credito,NULL);  
    creditoAnt = credito;
    //guarda el valor original y actualiza el saldo restando el precio del producto
    peticion.operacion = MODIFICAR_SALDO;
    precioTemp = productoSolicitado.precio;
    peticion.valor = -productoSolicitado.precio;
    xQueueOverwrite(respuestaPantalla, (void*)&peticion);
    while((credito==creditoAnt)&&(contador<INTENTOS_CREDITO)){//espera a que se actualice el saldo disponible
        xQueuePeek(creditoValido,(void*)&credito,10);
        contador++;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    contador = 0;*/
    xSemaphoreGive(operacionOcupada);
    //verifica que la canasta se encuentre cerrada
    primeravez=pdFALSE;
    transaccion = pdTRUE;
    while(transaccion){
        while(!xSemaphoreTake(busOcupado,( TickType_t )10));
        canasta = Dispensador_leer_canasta();
        xSemaphoreGive(busOcupado);
        if(canasta.comando==COMANDO_CORRECTO){
            if(canasta.canastaCerrada==pdTRUE){
                vTaskDelay(pdMS_TO_TICKS(500));
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                if(Dispensador_leer_canasta().canastaCerrada==pdTRUE){
                    transaccion=pdFALSE;
                }
                xSemaphoreGive(busOcupado);
            }
            else if(primeravez==pdFALSE){
                primeravez=pdTRUE;
                notificacion(pdTRUE,0,imagenNull,imagenPuerta,5,label_adver_canasta1,label_adver_canasta2,label_adver_canasta3,label_adver_canasta4,label_adver_canasta5);
            }
        }
    }
    //si la canasta está cerrada procede a entregar el producto
    transaccion = pdTRUE;
    notificacion(pdTRUE,5,imagenNull,imagenEsperaON,5,label_entrega1,label_entrega2,label_entrega3,label_entrega4,label_entrega5);
    while(transaccion){
        //le solicita al dispensador el estado de entrega del producto
        while(!xSemaphoreTake(busOcupado,( TickType_t )10));
        vTaskDelay(pdMS_TO_TICKS(10));
        Dispensador_Modo_Prueba(pdFALSE);
        vTaskDelay(pdMS_TO_TICKS(10));
        comando = Dispensador_dispensar(producto);
        xSemaphoreGive(busOcupado);
        vTaskDelay(pdMS_TO_TICKS(200));
        if(comando==COMANDO_CORRECTO){//se envio la solicitud de movimiento correctamente
            while(!xSemaphoreTake(busOcupado,( TickType_t )1));
            comando = Dispensador_leer_motor();
            xSemaphoreGive(busOcupado);
            timeOut=0;
            do{
                if(xSemaphoreTake(busOcupado,( TickType_t )1)){
                    comando = Dispensador_leer_motor();
                    xSemaphoreGive(busOcupado);
                }
                switch(comando){
                    case COMANDO_CORRECTO:
                        finMovimiento=pdTRUE;
                    break;
                    case COMANDO_NO_CAYO:
                        finMovimiento=pdTRUE;
                    break;
                    case COMANDO_ALERTA:
                        finMovimiento=pdTRUE;
                    break;
                    case COMANDO_ESPERA:
                        timeOut=0;
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
                timeOut++;
            }while((!finMovimiento)&&(timeOut<500));
            if(timeOut>=500){
                comando = COMUNICACION_FALLIDA;
            }
        }
        //si la entrega fue satisfactoria, valide el pago
        if((comando==COMANDO_CORRECTO)||(comando==COMANDO_ALERTA)){
            if(comando==COMANDO_ALERTA){
                actualizar_registro(contexto_venta_aviso,productoSolicitado.numero,productoSolicitado.precio,NULL);
            }
            actualizar_registro(contexto_venta_ok,productoSolicitado.numero,productoSolicitado.precio,NULL);
            transaccion = pdFALSE;
            notificacion(pdFALSE,30,imagenOk,imagenEsperaOFF,5,label_entregado1,label_entregado2,label_vacio,label_entregado3,label_entregado4);
            /*escribirPrincipal();
            xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
            xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = CANCELAR_VENTA;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);
            vTaskDelay(pdMS_TO_TICKS(100));*/
            contabilidad.operacion = ACTUALIZAR_CONTABILIDAD;
            contabilidad.producto = productoSolicitado;
            if(memoriaSD == LISTO){
                xQueueSend(actualizarInformacion,(void*)&contabilidad,10);
            }
            productoSolicitado.cantidad--;
        }
        //si hubo un error en la entrega, devuelva el dinero que habia sido ingresado originalmente
        else if(comando==COMANDO_NO_CAYO){
            actualizar_registro(contexto_error_caida,productoSolicitado.numero,productoSolicitado.precio,NULL);
            transaccion = pdFALSE;
            /*xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = MODIFICAR_SALDO;
            peticion.valor = productoSolicitado.precio;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);
            vTaskDelay(pdMS_TO_TICKS(10));
            while((credito!=creditoAnt)&&(contador<INTENTOS_CREDITO)){
                xQueuePeek(creditoValido,(void*)&credito,10);
                contador++;
                vTaskDelay(pdMS_TO_TICKS(1));
            }*/
            productoSolicitado.habilitado = pdFALSE;
            notificacion(pdFALSE,30,imagenError,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega4,label_error_entrega5);
            /*escribirPrincipal();
            xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = CANCELAR_VENTA;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);*/
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else if(comando==ERROR_PRODUCTO){
            actualizar_registro(contexto_error_caida,productoSolicitado.numero,productoSolicitado.precio,NULL);
            transaccion = pdFALSE;
            /*xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = MODIFICAR_SALDO;
            peticion.valor = productoSolicitado.precio;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);
            vTaskDelay(pdMS_TO_TICKS(10));
            while((credito!=creditoAnt)&&(contador<INTENTOS_CREDITO)){
                xQueuePeek(creditoValido,(void*)&credito,10);
                contador++;
                vTaskDelay(pdMS_TO_TICKS(1));
            }*/
            notificacion(pdFALSE,30,imagenError,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega4,label_error_entrega5);
            /*escribirPrincipal();
            xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = CANCELAR_VENTA;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);*/
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else if(comando==COMUNICACION_FALLIDA){
            actualizar_registro(contexto_error_comunicacion,0,0,NULL);
            transaccion = pdFALSE;
            notificacion(pdFALSE,30,imagenAlerta,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega6,label_vacio);
            /*escribirPrincipal();
            xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
            xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = CANCELAR_VENTA;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);*/
            vTaskDelay(pdMS_TO_TICKS(100));
            contabilidad.operacion = ACTUALIZAR_CONTABILIDAD;
            contabilidad.producto = productoSolicitado;
            if(memoriaSD == LISTO){
                xQueueSend(actualizarInformacion,(void*)&contabilidad,10);
            }
            productoSolicitado.cantidad--;
        }
    }
    while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
    if(productoSolicitado.cantidad==0){
        productoSolicitado.habilitado = pdFALSE;
    }
    productoSolicitado.precio = precioTemp;
    escribirProducto(productoSolicitado);
    xSemaphoreGive(eepromOcupada);
}

/*******************************************************************************
* FUNCION DE DISPENSACION DE DINERO
********************************************************************************
* Funcion que muestra el proceso de devolucion de dinero
*******************************************************************************/
void devolucionDinero(){
    int credito=0;
    char cadena[10];
    vTaskDelay(pdMS_TO_TICKS(500));
    xQueuePeek(creditoValido,&credito,10);
    sprintf(cadena,"$%d",credito);
    notificacion(pdTRUE,20,imagenNull,imagenEsperaON,5,label_devolucionDinero1,label_devolucionDinero2,cadena,label_devolucionDinero4,label_devolucionDinero5);     
    while(credito>0){
        xQueuePeek(creditoValido,&credito,10);
        sprintf(cadena,"$%d",credito);
        HMI_tx_comando("t","2","txt",cadena);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    notificacion(pdTRUE,0,imagenOk,imagenEsperaOFF,5,label_devolucionDinero7,label_devolucionDinero8,label_devolucionDinero3,label_devolucionDinero4,label_devolucionDinero6); 
}

/*******************************************************************************
* FUNCIONES PARA CADA VENTANA
********************************************************************************
* Cada función se encarga de mostrar una ventana diferente y modificar cada elemento
* en estas, devuelven un valor entero que indica la ventana que deberá ser mostrada
* a continuación.
*******************************************************************************/

uint8 borrarHuella_id(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_BORRAR_HUELLA_ID;
    
    if(bandera==pdFALSE){
        while(!bandera){
            bandera=HMI_ventana(VENTANA_NUMERICO);
        }
        HMI_tx_comando("t","0","txt",label_borrarHuella1);
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"PINCANCEL")){
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
        }
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            usuarioActivo.idHuella = atoi(respuesta.numero);
            bandera=pdFALSE;
            ventana = PANTALLA_CONFIR_BORRADO_HUELLA;
        }
    }
    return ventana;
}

uint8 bienvenida(){
    char string[50];
    int progreso = 0; //credito = 0, creditoValidado = 0;
    CYBIT comando_ok=pdFALSE;
    uint8 RTOS,MDB,memoriaSD,configuraciones;
    xestadosBilletero billetero;
    xestadosMonedero monedero;
    xInformacionGSM infoGSM;
    xConfiguracion parametros;
    xConfiguracionTelemetria confTelemetria;
    xConfiguracionAccesorios confAccesorios;
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_BIENVENIDA);
        }
        //HMI_calibrar();
        bandera = pdTRUE;
    }
    xQueuePeek(estadoRTOS, &RTOS,100);
    xQueuePeek(estadoMDB, &MDB,100);
    xQueuePeek(estadoMonedero, &monedero,100);
    xQueuePeek(estadoBilletero, &billetero,100);
    xQueuePeek(estadoMemoriaSD, &memoriaSD,100);
    xQueuePeek(estadoConfiguraciones, &configuraciones,100);
    xQueuePeek(configuracionesMaquina, &parametros,100);
    xQueuePeek(configuracionesRemotas, &confTelemetria,100);
    xQueuePeek(configuracionesAccesorios, (void*)&confAccesorios,100);
    BuzzerPWM_WritePeriod(20000);
    BuzzerPWM_WriteCompare(20000/150);
    
    progreso = RTOS; //1
    if(memoriaSD==NO_LISTO){
        HMI_tx_comando("t","4","txt",label_nota_memoria);
        vTaskDelay(pdMS_TO_TICKS(1000));
        progreso++; //2
    }
    else{
        progreso++; //2
    }
    while(!xSemaphoreTake(busOcupado,10));
    switch(Dispensador_leer_estado()){
        case DISPENSADOR_ESPERA:
            progreso++; //3
        break;
        case DISPENSADOR_NO_LISTO:
            HMI_tx_comando("t","4","txt",label_estado_dispensador1);
            vTaskDelay(pdMS_TO_TICKS(1000));
        break;
        default:
            HMI_tx_comando("t","4","txt",label_estado_dispensador2);
            vTaskDelay(pdMS_TO_TICKS(1000));
            progreso++; //3
        break;
    }
    xSemaphoreGive(busOcupado);
    if(MDB==LISTO){
        progreso++; //4
    }
    if((monedero.monedero==MDB_NO_DETECT)&&(billetero.billetero==MDB_NO_DETECT)){
        HMI_tx_comando("t","4","txt",label_nota_mdb);
        vTaskDelay(pdMS_TO_TICKS(1000));
        progreso += 2; //6
    }
    else{
        if(billetero.billetero==MDB_NO_DETECT){
            HMI_tx_comando("t","4","txt",label_nota_billetero);
            vTaskDelay(pdMS_TO_TICKS(1000));
            progreso++; //5
        }
        if(monedero.monedero==MDB_NO_DETECT){
            HMI_tx_comando("t","4","txt",label_nota_monedero);
            vTaskDelay(pdMS_TO_TICKS(1000));
            progreso++; //6
        }
        if(billetero.billetero==MDB_DETECT){
            progreso++; //5
        }
        
        if(monedero.monedero==MDB_DETECT){
            progreso++; //6
        }        
    }    
    //if(configuraciones==LISTO){
    //    progreso++;
    //}
    progreso = (progreso*100)/6;
    
    sprintf(string, "%d",progreso);
    HMI_tx_comando("j","0","val",string);
    HMI_tx_comando("n","0","val",string);
    vTaskDelay(pdMS_TO_TICKS(500));
    if(progreso >= 100){
        bandera = 0;
        actualizarFechaPsoc();
        actualizar_registro(contexto_arranque,0, 0, NULL);
        infoGSM.encendido = pdFALSE;
        xQueueOverwrite(datosGSM,&infoGSM);
        vQueueDelete(estadoRTOS);
        vQueueDelete(estadoMDB);
        vQueueDelete(estadoConfiguraciones);
        if(confTelemetria.estadoTelemetria==pdTRUE){
            Telemetria_Start();
        }
        return PANTALLA_SALVAPANTALLAS; 
        
    }
    return PANTALLA_BIENVENIDA;
}

uint8 billetesAceptados(){
    uint8 ventana = PANTALLA_BILLETES_ACEPTADOS;
    bufferEntradaHMI respuesta;
    xbill_setup billeteroConfig;
    uint8 indice=0, habilitado = pdFALSE;
    uint16 billetesAceptados = 0;
    char formato[10],string[10];
    xSistemasPago parametros;
    xcomandoHMI funcionHMI;
    
    xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    if(bandera==pdFALSE){
        seleccion(pdTRUE, pdTRUE, "%t0",label_billeteAceptado1);
        for(indice=0;indice<LONGITUD_TIPO_BILLETES;indice++){
            if(billeteroConfig.tipo[indice]!=0){
                habilitado = (parametros.billetesAceptados>>indice)&0x01;
                sprintf(formato,"%%t%d-1-%d",indice+1,habilitado);
                sprintf(string,"%d",billeteroConfig.tipo[indice]*billeteroConfig.factorEscala/billeteroConfig.factorDecimal);
                seleccion(pdFALSE, pdTRUE, formato,string);
            }
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=pdFALSE;
            ventana = PANTALLA_BILLETERO;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            bandera = pdFALSE;
            for(indice=0; indice<7;indice++){
                sprintf(string,"c%d.val",indice);
                do{
                    HMI_get_value(string);
                    respuesta = HMI_rx_comando();
                }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
                if(respuesta.buffer[0]==pdTRUE){
                    billetesAceptados |= (1<<indice);
                }
            }
            while(!xSemaphoreTake(eepromOcupada,10));
            EEPROM_escribirBilletesAceptados(billetesAceptados);
            xSemaphoreGive(eepromOcupada);
            parametros.billetesAceptados = billetesAceptados;
            xQueueOverwrite(configuracionSistemasPago,(void*)&parametros);
            funcionHMI.comando = MDB_EKIA_TIPO_BILLETE;
            xQueueSend(comandosHMI, &funcionHMI, 10);
        }
    }
    return ventana;
}

uint8 billetesRetenidos(){
    uint8 ventana = PANTALLA_BILLETES_RETENIDOS;
    bufferEntradaHMI respuesta;
    xbill_setup billeteroConfig;
    uint8 indice=0, habilitado = pdFALSE;;
    char formato[10],string[10];
    uint billeteRetenido = 0;
    xSistemasPago parametros;
    
    xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    if(bandera==pdFALSE){
        seleccion(pdTRUE, pdFALSE, "%t0",label_billeteRetenido1);
        for(indice=0;indice<LONGITUD_TIPO_BILLETES;indice++){
            if(billeteroConfig.tipo[indice]!=0){
                habilitado = pdFALSE;
                billeteRetenido = billeteroConfig.tipo[indice]*billeteroConfig.factorEscala/billeteroConfig.factorDecimal;
                sprintf(formato,"%%t%d-1-%d",indice+1,habilitado);
                sprintf(string,"%d",billeteRetenido);
                seleccion(pdFALSE, pdFALSE, formato,string);
            }
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_BILLETERO;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            bandera = pdFALSE;
            for(indice=0; indice<7;indice++){
                sprintf(string,"c%d.val",indice);
                do{
                    HMI_get_value(string);
                    respuesta = HMI_rx_comando();
                }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
                if(respuesta.buffer[0]==pdTRUE){
                    billeteRetenido = billeteroConfig.tipo[indice]*billeteroConfig.factorEscala/billeteroConfig.factorDecimal;
                }
            }
            while(!xSemaphoreTake(eepromOcupada,10));
            EEPROM_escribirBilletesRetenidos(billeteRetenido);
            xSemaphoreGive(eepromOcupada);
            xQueueOverwrite(configuracionSistemasPago,(void*)&parametros);
        }
    }
    return ventana;
}

uint8 billetesReciclados(){
    uint8 ventana = PANTALLA_BILLETES_RECICLADOS;
    bufferEntradaHMI respuesta;
    xbill_type_recycler recicladorConfig;
    xbill_setup billeteroConfig;
    uint8 indice=0, habilitado = pdFALSE;;
    char formato[10],string[10];
    uint billeteReciclado = 0;
    xSistemasPago parametros;
    
    xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    xQueuePeek(typeRecycler,(void*)&recicladorConfig,10);
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    if(bandera==pdFALSE){
        seleccion(pdTRUE, pdFALSE, "%t0",label_billeteReciclado1);
        for(indice=0;indice<LONGITUD_TIPO_BILLETES;indice++){
            if(billeteroConfig.tipo[indice]!=0){
                habilitado = pdFALSE;
                billeteReciclado = billeteroConfig.tipo[indice]*billeteroConfig.factorEscala/billeteroConfig.factorDecimal;
                if(billeteReciclado == parametros.billetesReciclados){
                    habilitado = pdTRUE;
                }
                sprintf(formato,"%%t%d-1-%d",indice+1,habilitado);
                sprintf(string,"%d",billeteReciclado);
                seleccion(pdFALSE, pdFALSE, formato,string);
            }
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_BILLETERO;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            bandera = pdFALSE;
            for(indice=0; indice<7;indice++){
                sprintf(string,"c%d.val",indice);
                do{
                    HMI_get_value(string);
                    respuesta = HMI_rx_comando();
                }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
                if(respuesta.buffer[0]==pdTRUE){
                    billeteReciclado = billeteroConfig.tipo[indice]*billeteroConfig.factorEscala/billeteroConfig.factorDecimal;
                }
            }
            while(!xSemaphoreTake(eepromOcupada,10));
            EEPROM_escribirBilletesReciclados(billeteReciclado);
            xSemaphoreGive(eepromOcupada);
            parametros.billetesReciclados = billeteReciclado;
            xQueueOverwrite(configuracionSistemasPago,(void*)&parametros);
        }
    }
    return ventana;
}

uint8 billeteroEstado(){
    uint8 ventana = PANTALLA_BILLETERO_ESTADO;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xbill_setup billeteroConfig;
    xbill_state billeteroStatus;
    xbill_recycler_status recicladorEstado;
    xSistemasPago parametros;
    char objeto[20], celdas[4];
    uint8 actividad = 0;
    int fila = 2;
    
    xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    xQueuePeek(stateBill,(void*)&billeteroStatus,10);
    xQueuePeek(stateRecycler, (void*)&recicladorEstado,10);
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    
    /*En esta pantalla se muesta información importante del billetero*/
    if(bandera==pdFALSE){
        HMI_tx_comando("Depuracion.t","0","txt",label_estadoBilletero1);
        HMI_tx_comando("Depuracion.tA","1","txt",label_estadoBilletero2);
        HMI_tx_comando("Depuracion.tB","1","txt",label_estadoBilletero3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_INFORMACION);
        }
        bandera = pdTRUE;
    }
    
    //Se muestra el estado del stacker de billetero
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_estadoBilletero4);
    if(billeteroStatus.lleno == 1){
        strcpy(objeto,"SI");
    }
    else{
        strcpy(objeto,"NO");
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Se muestra la cantidad de billetes almacenados
    escribirInformacionNumero(fila,label_estadoBilletero5,billeteroStatus.estado);
    fila++;
    
    escribirInformacionTexto(fila,label_vacio,label_vacio);
    fila++;
    
    //Se muestran los avisos propios del billetero
    xQueuePeek(actividadBilletero,(void*)&actividad,100);
    switch(actividad){
        case 0x03:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete1);
        break;
        case 0x05:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete2);
        break;
        case 0x07:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete3);
        break;    
        case 0x09:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete4);
        break;
        case 0x0B:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete5);
        break;     
        case 0x22:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete6);
        break;
        case 0x23:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete1);
        break;    
        case 0x29:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete4);
        break;    
        case 0x2A:
            escribirInformacionTexto(fila,label_estadoBilletero6,label_activi_billete7);
        break;    
    }
    fila++;
    
    if(billeteroConfig.nivel==2){
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt",label_estadoBilletero9);
        fila++;
        sprintf(celdas," %d",parametros.billetesReciclados);
        escribirInformacionNumero(fila,celdas,recicladorEstado.cantidadBilletes[2]);
        fila++;
    }
    //se limpian todas las celdas que queden vacias
    for(;fila<=22;fila++){
        escribirInformacionTexto(fila,label_vacio,label_vacio);
    }
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_BILLETERO;
        }
    }
    return ventana;
}

uint8 pantallaBrillo(){
    uint8 ventana = PANTALLA_BRILLO_PANTALLA;
    bufferEntradaHMI respuesta;
    xConfiguracion parametros;
    xQueuePeek(configuracionesMaquina,&parametros,10);
    if(bandera==pdFALSE){
        cambio = pdFALSE;
        confGeneral(pdTRUE,"%t0 %b0-1-0 %b1-0-0 %bt0-1",label_brillo_pantalla1, label_brillo_pantalla2, label_brillo_pantalla3);
        HMI_set_value("bt0",parametros.BrilloPantalla.activo);
        HMI_set_value("h0",parametros.BrilloPantalla.nivel);
        bandera = pdTRUE;
    }
    HMI_get_value("bt0.val");//obtiene la configuración automatica o manual
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        parametros.BrilloPantalla.activo = respuesta.buffer[0];
        if(!parametros.BrilloPantalla.activo){
            if(!cambio){
                HMI_visualizar("b1",pdTRUE);
                HMI_visualizar("b11",pdTRUE);
                HMI_visualizar("h0",pdTRUE);
                cambio = pdTRUE;
            }
        }
        else{
            cambio = pdFALSE;
            HMI_visualizar("b1",pdFALSE);
            HMI_visualizar("b11",pdFALSE);
            HMI_visualizar("h0",pdFALSE);
        }
    }
    HMI_get_value("h0.val");//obtiene el valor de brillo guardado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        parametros.BrilloPantalla.nivel = respuesta.buffer[0];
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        if(respuesta.buffer[0]==botonVolver){
            bandera=pdFALSE;
            while(!xSemaphoreTake(eepromOcupada,10));
            escribirBrilloPantalla(parametros.BrilloPantalla);
            xSemaphoreGive(eepromOcupada);
            ventana = PANTALLA_ILUMINACION;
        }
    }
    xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
    return ventana;
}

uint8 claveAutorizacion(){
    bufferEntradaHMI respuesta;
    xAutorizacion autorizacion;
    ventana = PANTALLA_CLAVE_AUTORIZACION;
    CYBIT comando_ok = pdFALSE;
    
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_ALFANUMERICO);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"PINCANCEL")){
            bandera=0;
            ventana = PANTALLA_PRINCIPAL;
        }
        if(!strcmp(respuesta.buffer,"CLAVEFABRICANTE")){
            usuario = FABRICANTE;
            bandera = pdFALSE; 
            ventana = PANTALLA_MENU;
        }
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            xQueueReceive(autorizaciones,(void*)&autorizacion,10);
            if(!strcmp(respuesta.numero,autorizacion.clave)){
                        //while(!xSemaphoreTake(busOcupado,10));
                        //Dispensador_dispensar(autorizacion.producto);
                        //vTaskDelay(pdMS_TO_TICKS(10));
                        //xSemaphoreGive(busOcupado);
                notificacion(pdTRUE,20,imagenNull,imagenEsperaON,4,"DISPENSANDO","PRODUCTO","ESPERE...");
            }
            else{
                notificacion(pdTRUE,20,imagenError,imagenEsperaOFF,4,"ERROR","CLAVE","ERRONEA");
            }
            //bandera=0;
            //ventana = PANTALLA_PRINCIPAL;
        }
    }
    return ventana;
}

uint8 claveIngreso(){
    bufferEntradaHMI respuesta;
    xclave contraAdmin;
    uint8 ventana = PANTALLA_CLAVE;
    CYBIT comando_ok = pdFALSE;
    xConfiguracion parametros;
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CLAVE);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        xQueuePeek(configuracionesMaquina, &parametros,10);
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_PRINCIPAL;
        }
        if(!strcmp(respuesta.buffer,"CONFIRMAR")){
            do{
                HMI_get_value("t0.txt");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
            while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
            contraAdmin = EEPROM_leerClaveAdmin();
            //contraOper = EEPROM_leerClaveOperador();
            //contraTesorero = EEPROM_leerClaveTesorero();
            xSemaphoreGive(eepromOcupada);
            
            if(!strcmp(respuesta.buffer,contraAdmin.clave)){
                usuario = ADMINISTRADOR;
                bandera = pdFALSE; 
                ventana = PANTALLA_MENU;
            }
            else{
                HMI_tx_comando("t","0","txt",label_error1);
                HMI_tx_comando("t","1","txt",label_error2);
                HMI_tx_comando("t","1","pco",color_error);
            }
        }
        if(!strcmp(respuesta.buffer,"CLAVEEMERGENCIA")){
            usuario = FABRICANTE;
            bandera = pdFALSE; 
            ventana = PANTALLA_MENU;
        }
        if(!strcmp(respuesta.buffer,"CLAVEINICIAL")){
            usuario = TEMPORAL;
            bandera = pdFALSE; 
            ventana = PANTALLA_MENU;
        }
        if(!strcmp(respuesta.buffer,"DESBLOQUEAR")){
            while(!xSemaphoreTake(eepromOcupada,1));
            EEPROM_escribirBloqueo(pdFALSE);
            xSemaphoreGive(eepromOcupada);
            parametros.bloqueoMaquina = pdFALSE;
            xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
            notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenOk,imagenEsperaOFF,5,"Maquina desbloqueada","Correctamente");
            ventana = PANTALLA_PRINCIPAL;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ventana;
}

uint8 claveCambiar(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CAMBIAR_CLAVE;
    CYBIT comando_ok = pdFALSE;
    
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CLAVE);
        }
        HMI_tx_comando("t","1","txt",label_claveAdmin);
        //HMI_tx_comando("t","1","txt",label_clave2);
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
        }
        if(!strcmp(respuesta.buffer,"CONFIRMAR")){
            do{
                HMI_get_value("t0.txt");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
        
            stpcpy(clave_temporal.clave,respuesta.buffer);
            bandera=0;
            ventana = PANTALLA_CONFIRMAR_CLAVE;
        }
    }
    return ventana;
}

uint8 dispensarManual(){
    uint8 ventana = PANTALLA_DISPENSAR_MANUAL;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xProducto productoSolicitado; 
    xConfiguracion parametros;
    
    if(bandera==pdFALSE){  
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_NUMERICO);
        }    
        while(!xSemaphoreTake(busOcupado,1));
        Dispensador_Modo_Prueba(pdFALSE);
        xSemaphoreGive(busOcupado);
        bandera = pdTRUE;
    }
    //escribirPrincipal();
    respuesta=HMI_rx_comando();
    if(respuesta.objeto=='B'){
        xQueuePeek(configuracionesMaquina, &parametros,10);
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            producto = atoi(respuesta.numero);
            bandera = pdFALSE;
            productoSolicitado = validarProducto();
            if(productoSolicitado.habilitado){
                dispensar();
                ventana = PANTALLA_PRINCIPAL;
            }
            //hay un problema, indique el error
            else{
                notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenError,imagenEsperaOFF,4,label_compra_error1,label_compra_error2,label_compra_error3,label_compra_error4,label_compra_error5);
                ventana = PANTALLA_DISPENSAR_MANUAL;
            }
        }
        if(!strcmp(respuesta.buffer,"CANCELAR")){
            bandera = pdFALSE; 
            ventana = PANTALLA_PRINCIPAL;
        }
        if(!strcmp(respuesta.buffer,"DESBLOQUEAR")){
            while(!xSemaphoreTake(eepromOcupada,1));
            EEPROM_escribirBloqueo(pdFALSE);
            xSemaphoreGive(eepromOcupada);
            parametros.bloqueoMaquina = pdFALSE;
            xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
            notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenOk,imagenEsperaOFF,5,"Maquina desbloqueada","Correctamente");
            ventana = PANTALLA_PRINCIPAL;
        }
        if(!strcmp(respuesta.buffer,"BLOQUEAR")){
            while(!xSemaphoreTake(eepromOcupada,1));
            EEPROM_escribirBloqueo(pdTRUE);
            xSemaphoreGive(eepromOcupada);
            parametros.bloqueoMaquina = pdTRUE;
            xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
            notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenAlerta,imagenEsperaOFF,5,"Maquina bloqueada","Contacte a","soporte");
            ventana = PANTALLA_PRINCIPAL;
        }
    }
    return ventana;
}

uint8 serieCambiar(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CAMBIAR_SERIE;
    CYBIT comando_ok = pdFALSE;
    char version[3], fecha[5], numero[5];
    uint8 indice = 0;
    
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CLAVE);
        }
        HMI_tx_comando("t","1","txt",label_Serie1);
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
        }
        if(!strcmp(respuesta.buffer,"CONFIRMAR")){
            do{
                HMI_get_value("t0.txt");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
            bandera=0;
            version[0] = respuesta.buffer[0];
            version[1] = respuesta.buffer[1];
            version[2] = 0;
            for(indice=0;indice<4;indice++){
                fecha[indice]=respuesta.buffer[indice+2];
            }
            fecha[4]=0;
            for(indice=0;indice<4;indice++){
                numero[indice]=respuesta.buffer[indice+6];
            }
            numero[4]=0;
            while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
            actualizarSerie(
                version,
                fecha,
                numero
            );
            xSemaphoreGive(eepromOcupada);
            ventana = PANTALLA_CONFIGURACION;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
    return ventana;
}

uint8 cancelar_productos(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CANCEL_PRODUCTO;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_cancelarConfProd1,label_cancelarConfProd2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            bandera = pdFALSE;
            ventana = PANTALLA_MENU_OPERATIVO;
        }
    }
    return ventana;
}

uint8 config_Billetero(){
    uint8 ventana = PANTALLA_BILLETERO;
    bufferEntradaHMI respuesta;
    xbill_setup billeteroConfig;
    
    xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    if(bandera==0){
        if(billeteroConfig.nivel==2){
            confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1 %b4-1 %bt0-0",label_billetero1, label_billetero2, label_billetero3, label_billetero4, label_billetero5, label_billetero6);
        }
        else{
            confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1 %bt0-0",label_billetero1, label_billetero2, label_billetero3, label_billetero4, label_billetero5);
        
        }
    }
    HMI_get_value("estado.val");//verifica si un boton fue presionado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton1:{ 
                bandera=0;
                ventana = PANTALLA_BILLETES_ACEPTADOS;
                break;
            }
            case boton2:{ 
                bandera=0;
                ventana = PANTALLA_BILLETES_RETENIDOS;
                break;
            }
            case boton3:{ 
                bandera=0;
                ventana = PANTALLA_BILLETERO_INFO;
                break;
            }
            case boton4:{ 
                bandera=0;
                ventana = PANTALLA_BILLETERO_ESTADO;
                break;
            }
            case boton5:{ 
                bandera=0;
                ventana = PANTALLA_RECICLADOR;
                break;
            }
            case botonVolver:{
                bandera=0;
                ventana = PANTALLA_SISTEMAS_PAGO;
                break;
            }
        }
    }
    return ventana;
}

uint8 config_Monedero(){
    uint8 ventana = PANTALLA_MONEDERO;
    bufferEntradaHMI respuesta;
    
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1",label_monedero1,label_monedero3,label_monedero4);
    }
    HMI_get_value("estado.val");//verifica si un boton fue presionado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton1:{ 
                bandera=0;
                ventana = PANTALLA_MONEDERO_INFO;
                break;
            }
            case boton2:{ 
                bandera=0;
                ventana = PANTALLA_MONEDERO_ESTADO;
                break;
            }
            case botonVolver:{
                bandera=0;
                ventana = PANTALLA_SISTEMAS_PAGO;
                break;
            }
        }
    }
    return ventana;
}

uint8 config_Conectividad(){
    uint8 ventana = PANTALLA_CONF_CONECTIVIDAD;
    bufferEntradaHMI respuesta;
    TaskHandle_t xHandle = NULL;
    xConfiguracionTelemetria parametros;
    xQueuePeek(configuracionesRemotas,(void*)&parametros,10);
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-0",label_conf_redes1, label_conf_redes2, label_vacio);
        if (parametros.existenciaTelemetria == pdTRUE){
            HMI_set_value("bt0",pdTRUE);
        }
        else{
            HMI_set_value("bt0",pdFALSE);
        }
        HMI_visualizar("bt0",pdTRUE);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        if(respuesta.buffer[0]==botonVolver){
            while(!xSemaphoreTake(eepromOcupada,1));
            escribirExistenciaTelemetria(parametros.existenciaTelemetria);
            if(!parametros.existenciaTelemetria){
                escribirEstadoTelemetria(pdFALSE);    
            }
            xSemaphoreGive(eepromOcupada);
            bandera=pdFALSE;
            ventana = PANTALLA_MENU_CONECTIVIDAD;
        }
    }
    HMI_get_value("bt0.val");//obtiene la configuración instalado o no instalado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        if(respuesta.buffer[0]==pdTRUE){
            if(!parametros.existenciaTelemetria){
                parametros.existenciaTelemetria = pdTRUE;
                xQueueOverwrite(configuracionesRemotas,(void*)&parametros);
            }
        }
        else{
            if(parametros.existenciaTelemetria){
                parametros.existenciaTelemetria = pdFALSE;
                parametros.estadoTelemetria = pdFALSE;
                xQueueOverwrite(configuracionesRemotas,(void*)&parametros);
                xHandle = xTaskGetHandle("Telemetria");
                if(xHandle!=NULL){
                    Telemetria_Stop();
                }
            }
        }
    }
    return ventana;
}

uint8 config_bandejas(){
    uint8 ventana = PANTALLA_CONFIG_BANDEJAS;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    
    xConfiguracion parametros;
    xQueuePeek(configuracionesMaquina, &parametros, 10);
    if(bandera==pdFALSE){
        graficarConfBandejas();
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_BANDEJAS);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CANCELAR")){
            HMI_visualizar("p0",pdTRUE);
            bandera=0;
            ventana = PANTALLA_MENU_OPERATIVO;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            HMI_visualizar("p0",pdTRUE);
            actualizarConfBandejas();
            bandera = pdFALSE;
            ventana = PANTALLA_CONFIRMAR_BANDEJA;
        }
    }
    return ventana;
}

uint8 config_motores(){
    uint8 ventana = PANTALLA_MENU_MOTORES;
    CYBIT test_terminado = pdFALSE;
    char cadena[20];
    uint8 tempBandeja=1;
    uint8 tempColumna=0;
    bufferEntradaHMI respuesta;
    xProducto productoSolicitado;
    xConfiguracion parametros;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesMaquina, &parametros, 10);
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1-0 %b2-0 %b3-0",label_movimiento_motor1, label_movimiento_motor2, label_movimiento_motor3, label_vacio,label_vacio);
        HMI_set_value("limFila",bandejas.configBandejas.numBandejas);
        HMI_set_value("limCol",MOTORES_DOROTI-1);
        HMI_tx_comando("t","1","txt","1");
        HMI_tx_comando("t","2","txt","00");
        HMI_visualizar("b20",pdTRUE);
        HMI_visualizar("b21",pdTRUE);
        HMI_visualizar("b22",pdTRUE);
        HMI_visualizar("b23",pdTRUE);
        HMI_visualizar("b24",pdTRUE);
        HMI_visualizar("t1",pdTRUE);
        HMI_visualizar("t2",pdTRUE);
    }
    
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:{
                producto = 100;
                test_terminado = pdFALSE;
                sprintf(cadena,"Motor: %d",producto);
                notificacion(pdTRUE,0,imagenNull,imagenEsperaON,5,label_test_completo0,label_test_completo1,cadena,label_test_completo5,label_test_completo6);
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                Dispensador_Modo_Prueba(pdTRUE);
                xSemaphoreGive(busOcupado);
                while(!test_terminado){
                    for(tempBandeja=1;tempBandeja<=bandejas.configBandejas.numBandejas;tempBandeja++){
                        if(test_terminado){
                            break;
                        }
                        for(tempColumna=0;tempColumna<bandejas.configBandejas.numMotores[tempBandeja-1];tempColumna++){
                            producto = (tempBandeja*100)+tempColumna;
                            sprintf(cadena,"MOTOR: %d",producto);
                            HMI_tx_comando("Notificacion.t","2","txt",cadena);
                            while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                            Dispensador_dispensar(producto);
                            xSemaphoreGive(busOcupado);
                            while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
                            productoSolicitado = leerProducto(producto);
                            escribirProducto(productoSolicitado);
                            xSemaphoreGive(eepromOcupada);
                            HMI_get_value("estado.val");
                            respuesta = HMI_rx_comando();
                            if(respuesta.objeto==HMI_DATO_NUMERICO){
                                if(respuesta.buffer[0]==pdTRUE){
                                    test_terminado = pdTRUE;
                                    break;
                                }
                            }
                        }
                    }
                    test_terminado = pdTRUE;
                }
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                Dispensador_Modo_Prueba(pdFALSE);
                xSemaphoreGive(busOcupado);
                notificacion(pdFALSE,T_NOTIFICACIONES,imagenOk,imagenEsperaOFF,2,label_test_completo3,label_test_completo4);
                bandera = pdFALSE;
            break;}
            case botonMover:{
                HMI_set_value("estado.val",pdFALSE);
                do{
                    HMI_get_value("va1.txt");
                    respuesta = HMI_rx_comando();
                }while(!((respuesta.objeto==HMI_DATO_TEXTO)&&(respuesta.estado==HMI_CORRECTO)));
                producto = atoi(respuesta.numero);
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                Dispensador_Modo_Prueba(pdTRUE);
                xSemaphoreGive(busOcupado);
                vTaskDelay(pdMS_TO_TICKS(10));
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                Dispensador_dispensar(producto);
                xSemaphoreGive(busOcupado);
                vTaskDelay(pdMS_TO_TICKS(10));
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                Dispensador_Modo_Prueba(pdFALSE);
                xSemaphoreGive(busOcupado);
                while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
                productoSolicitado = leerProducto(producto);
                escribirProducto(productoSolicitado);
                xSemaphoreGive(eepromOcupada);
            break;}
            case botonVolver:{
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                while(Dispensador_Modo_Prueba(pdFALSE)!=COMANDO_CORRECTO);
                xSemaphoreGive(busOcupado);
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_OPERATIVO;
            break;}
        }
    }
    return ventana;
}

uint8 config_producto(){
    uint8 ventana = PANTALLA_CONFIG_PRODUCTO;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xProducto producto;
    uint16 indiceProducto = 100;
    uint16 i=0;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    if(bandera==pdFALSE){
        i=0;
        HMI_set_value("Producto.HabPrecio",0);
        while(!volcado){
            if(xSemaphoreTake(eepromOcupada,( TickType_t )10)){
                producto = leerProducto(indiceProducto);
                matrizProductos[bandeja-1][i][0] = producto.numero; //numero producto
                matrizProductos[bandeja-1][i][1] = producto.habilitado; //habilitacion producto
                matrizProductos[bandeja-1][i][2] = producto.precio; //precio producto
                matrizProductos[bandeja-1][i][3] = producto.cantidad; //cantidad del producto
                matrizProductos[bandeja-1][i][4] = producto.alturaXItem; //altura del producto
                matrizProductos[bandeja-1][i][5] = producto.proveedor; //cantidad del producto
                i++;
                indiceProducto++;
                xSemaphoreGive(eepromOcupada);
            }
            if(i>NUMERO_PRODUCTOS_BANDEJA_MAX){
                i = 0;
                bandeja++;
                indiceProducto = bandeja*100;
            }
            if(bandeja>(NUMERO_BANDEJAS_MAX)){
                volcado = pdTRUE;
                bandeja = 1;
                graficarConfProductos(bandeja, pdTRUE);
            }
        }
        if(volcado){
            while(!comando_ok){
                comando_ok=HMI_ventana(VENTANA_PRODUCTOS);
            }
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CANCELAR")){
            HMI_visualizar("p0",pdTRUE);
            actualizarConfProductos(bandeja,pdFALSE);
            bandera=pdFALSE;
            ventana = PANTALLA_CANCEL_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"SIGUIENTE")){
            HMI_visualizar("p0",pdTRUE);
            actualizarConfProductos(bandeja,pdFALSE);
            do{
                bandeja++;
                if(bandeja>NUMERO_BANDEJAS_MAX){
                    bandeja = 1;
                    break;
                }
            }while(!bandejas.configBandejas.estado[bandeja-1]);    
            graficarConfProductos(bandeja, pdFALSE);
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"ANTERIOR")){
            HMI_visualizar("p0",pdTRUE);
            actualizarConfProductos(bandeja, pdFALSE);
            do{
                bandeja--;
                if(bandeja<1){
                    bandeja = NUMERO_BANDEJAS_MAX;
                }
            }while(!bandejas.configBandejas.estado[bandeja-1]);
            graficarConfProductos(bandeja, pdFALSE);
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            HMI_visualizar("p0",pdTRUE);
            actualizarConfProductos(bandeja, pdFALSE);
            bandera = pdFALSE;
            ventana = PANTALLA_CONFIRMAR_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"PRODUCTO#")){
            numProducto = atoi(respuesta.numero);
            while(!xSemaphoreTake(busOcupado, pdMS_TO_TICKS(100)));
            Dispensador_abrirProducto(numProducto);
            xSemaphoreGive(busOcupado);
        }
        if(!strcmp(respuesta.buffer,"PRECIO#")){
            numProducto = atoi(respuesta.numero);
            bandera = pdFALSE;
            ventana = PANTALLA_INGRESO_PRECIO;
        }
        if(!strcmp(respuesta.buffer,"CANTIDAD#")){
            numProducto = atoi(respuesta.numero);
            bandera = pdFALSE;
            ventana = PANTALLA_INGRESO_CANTIDAD;
        }
        if(!strcmp(respuesta.buffer,"ALTURA#")){
            numProducto = atoi(respuesta.numero);
            bandera = pdFALSE;
            ventana = PANTALLA_INGRESO_ALTURA;
        }
    }
    return ventana;
}

uint8 configuracion(){
    uint8 ventana = PANTALLA_CONFIGURACION;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    uint8 memoriaSD;
    
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 10);
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CONFIGURACION);
        }
        if(fabricante){
            HMI_visualizar("b10",pdTRUE);
            HMI_visualizar("b11",pdTRUE);
            HMI_visualizar("b19",pdTRUE);
            HMI_visualizar("b22",pdTRUE);
            HMI_visualizar("b23",pdTRUE);
            HMI_visualizar("b24",pdTRUE);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_MENU;
        }
        if(!strcmp(respuesta.buffer,"MEMORIA")){
            if(memoriaSD==LISTO){
                bandera=0;
                ventana = PANTALLA_CONF_MEMORIA;
            }
        }
        if(!strcmp(respuesta.buffer,"CONTRASENA")){
            bandera=0;
            ventana = PANTALLA_CAMBIAR_CLAVE;
        }
        if(!strcmp(respuesta.buffer,"ILUMINACION")){
            bandera=0;
            ventana = PANTALLA_ILUMINACION;
        }
        if(!strcmp(respuesta.buffer,"FECHAYHORA")){
            bandera=0;
            ventana = PANTALLA_FECHAYHORA;
        }
        if(!strcmp(respuesta.buffer,"TEMPERATURA")){
            bandera=0;
            ventana = PANTALLA_TEMPERATURA;
        }
        if(!strcmp(respuesta.buffer,"DIAGNOSTICO")){
            bandera=0;
            ventana = PANTALLA_DIAGNOSTICO;
        }
        if(!strcmp(respuesta.buffer,"INFOSISTEMA")){
            bandera=0;
            ventana = PANTALLA_INFORMACION;
        }
        if(!strcmp(respuesta.buffer,"SERIE")){
            bandera=0;
            ventana = PANTALLA_CAMBIAR_SERIE;
        }
        if(!strcmp(respuesta.buffer,"SPAGO")){
            notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenAlerta,imagenEsperaOFF,4,label_noDisponible1,label_noDisponible2,label_noDisponible3,label_noDisponible4,label_noDisponible5); 
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
            //ventana = PANTALLA_SISTEMAS_PAGO;
        }
    }
    return ventana;
}

uint8 config_temp(){
    bufferEntradaHMI respuesta;
    xConfiguracionTemperatura parametros;
    uint8 ventana = PANTALLA_CONFIGURACION_TEMP;
    CYBIT comando_ok = pdFALSE;
    xQueuePeek(configuracionesTemperatura, (void*)&parametros, 10);
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CONFIGURTEMP);
        }
        HMI_set_value("n0", parametros.HisteresisSuper);
        HMI_set_value("n1", parametros.HisteresisInfer);
        HMI_set_value("n2", parametros.HorasDescanso);
        HMI_set_value("n3", parametros.MinutosDescanso);
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_TEMPERATURA;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            do{
                //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
                HMI_get_value("n0.val");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            parametros.HisteresisSuper = (respuesta.buffer[1]<<8) | respuesta.buffer[0];
            
            do{
                //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
                HMI_get_value("n1.val");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            parametros.HisteresisInfer = (respuesta.buffer[1]<<8) | respuesta.buffer[0];
            
            do{
                //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
                HMI_get_value("n2.val");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            parametros.HorasDescanso = respuesta.buffer[0];
            do{
                //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
                HMI_get_value("n3.val");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            parametros.MinutosDescanso = respuesta.buffer[0];
            
             xQueueOverwrite(configuracionesTemperatura, (void*)&parametros);
            
            while(!xSemaphoreTake(eepromOcupada, pdMS_TO_TICKS(10)));
            EEPROM_escribirHisterSuper(parametros.HisteresisSuper);
            EEPROM_escribirHisterInfer(parametros.HisteresisInfer);
            EEPROM_escribirHorasDescanso(parametros.HorasDescanso);
            EEPROM_escribirMinutosDescanso(parametros.MinutosDescanso);
            xSemaphoreGive(eepromOcupada);
            while(!xSemaphoreTake(busOcupado, pdMS_TO_TICKS(10)));
            Dispensador_definir_rango_superior(parametros.HisteresisSuper);
            Dispensador_definir_rango_inferior(parametros.HisteresisInfer);
            xSemaphoreGive(busOcupado);
            bandera = pdFALSE;
        }
    }
    return ventana;
}

uint8 confirmar_bandeja(){
    bufferEntradaHMI respuesta;
    xConfiguracionBandejas bandejas;
    uint8 ventana = PANTALLA_CONFIRMAR_BANDEJA;
    CYBIT comando_ok = pdFALSE;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_guardar1,label_conf_guardar2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CONFIG_BANDEJAS;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            xQueuePeek(configuracionesBandejas, &bandejas, 10);
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,3,label_conf_guardar8,label_conf_guardar9,label_conf_guardar10);
            comando_ok=pdFALSE;
            while(!xSemaphoreTake(busOcupado,pdMS_TO_TICKS(1)));
            Dispensador_definir_Tiempos(bandejas.tiemposBandeja,bandejas.parametrosMotores.tiempoMovimiento, bandejas.parametrosMotores.cicloUtil);
            xSemaphoreGive(busOcupado);
            if(xSemaphoreTake(eepromOcupada,( TickType_t )100)){
                comando_ok=EEPROM_escribirBandejas(bandejas.configBandejas);
                if(comando_ok){
                    comando_ok = pdFALSE;
                    comando_ok=EEPROM_escribirTiempos(bandejas.tiemposBandeja, bandejas.parametrosMotores.tiempoMovimiento, bandejas.parametrosMotores.cicloUtil);
                }
                xSemaphoreGive(eepromOcupada);
                if(comando_ok == pdTRUE){
                    notificacion(pdFALSE,T_NOTIFICACIONES,imagenOk,imagenEsperaOFF,3,label_conf_guardar14,label_conf_guardar15,label_conf_guardar16);
                }
                else{
                    notificacion(pdFALSE,T_NOTIFICACIONES,imagenError,imagenEsperaOFF,3,label_conf_guardar17,label_conf_guardar18,label_conf_guardar19);
                
                }
            }
            else{
                notificacion(pdFALSE,T_NOTIFICACIONES,imagenAlerta,imagenEsperaOFF,2,label_conf_guardar6,label_conf_guardar7);
            }
            bandera = pdFALSE;
            ventana = PANTALLA_MENU_OPERATIVO;
        }
    }
    return ventana;
}

uint8 confirmar_borradoHuella(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CONFIR_BORRADO_HUELLA;
    CYBIT comando_ok = pdFALSE;
    char string[15];
    if(bandera==pdFALSE){
        sprintf(string,"%s%d?",label_conf_borraHuella2,usuarioActivo.idHuella);
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_borraHuella1,string);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            if(borradoHuellaEspecifico==pdTRUE){
                ventana = PANTALLA_BORRAR_HUELLA_ID;
            }
            else{
                ventana = PANTALLA_BORRAR_HUELLA;
            }
        }
        if(!strcmp(respuesta.buffer,"SI")){
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,2,label_actualizar_clave1,label_actualizar_clave2);
            while(!xSemaphoreTake(lectorOcupado,( TickType_t )10));
            comando_ok = finger_borrarInscripcion(usuarioActivo.idHuella);
            xSemaphoreGive(lectorOcupado);
            if(comando_ok){
                comando_ok=pdFALSE;
                while(!xSemaphoreTake(memoriaOcupada,( TickType_t )10));
                if(EEPROM_escribirNombreUsuario(" ",usuarioActivo.idHuella)){
                    comando_ok = EEPROM_escribirCedulaUsuario(" ",usuarioActivo.idHuella);
                }
                xSemaphoreGive(memoriaOcupada);
            }
            if(comando_ok){
                notificacion(pdFALSE,0,imagenOk,imagenEsperaOFF,2,label_conf_borraHuella3,label_conf_borraHuella4);
            }
            else{
                notificacion(pdFALSE,0,imagenError,imagenEsperaOFF,2,label_conf_borraHuella5,label_conf_borraHuella6);
            }  
            ventana = PANTALLA_MENU_HUELLAS;
        }
    }
    return ventana;
}

uint8 confirmar_clave(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CONFIRMAR_CLAVE;
    CYBIT comando_ok = pdFALSE;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_clave1,label_conf_clave2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CAMBIAR_CLAVE;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,2,label_actualizar_clave1,label_actualizar_clave2);
            if(xSemaphoreTake(eepromOcupada,( TickType_t )10)){
                comando_ok = EEPROM_escribirClaveAdmin(clave_temporal);  
                if(comando_ok){
                    notificacion(pdFALSE,0,imagenOk,imagenEsperaOFF,2,label_actualizar_clave5,label_actualizar_clave6);
                }
                else{
                    notificacion(pdFALSE,0,imagenError,imagenEsperaOFF,2,label_actualizar_clave3,label_actualizar_clave4);
                }
                xSemaphoreGive(eepromOcupada);
            }
            else{
                notificacion(pdFALSE,0,imagenAlerta,imagenEsperaOFF,2,label_actualizar_clave7,label_actualizar_clave8);
            }
            vTaskDelay(pdMS_TO_TICKS(T_NOTIFICACIONES));
            ventana = PANTALLA_CONFIGURACION;
        }
    }
    return ventana;
}

uint8 confirmar_copia(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CONFIRMAR_COPIA;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_copiar1,label_conf_copiar2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CONF_MEMORIA;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,3,label_copia_parametros1,label_copia_parametros2,label_copia_parametros3);
            if(xSemaphoreTake(memoriaOcupada,( TickType_t )10)){
                if(copiarConfiguracionSD()&&copiarProductosSD(ruta_productos)){
                    notificacion(pdFALSE,0,imagenOk,imagenEsperaOFF,5,label_copia_parametros4,label_copia_parametros5,label_copia_parametros6,label_copia_parametros7,label_copia_parametros8);
                }
                else{
                    notificacion(pdFALSE,0,imagenError,imagenEsperaOFF,3,label_copia_parametros9,label_copia_parametros10,label_copia_parametros11);
                }
                xSemaphoreGive(memoriaOcupada);
            }
            else{
                notificacion(pdFALSE,0,imagenAlerta,imagenEsperaOFF,2,label_copia_parametros12,label_copia_parametros13);
            }
            vTaskDelay(pdMS_TO_TICKS(T_NOTIFICACIONES));
            ventana = PANTALLA_CONF_MEMORIA;
        }
    }
    return ventana;
}

uint8 confirmar_formateo(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CONFIRMAR_FORMATEO;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_format1,label_conf_format2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CONF_MEMORIA;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,3,label_formato7,label_formato8,label_formato9);
            if(xSemaphoreTake(memoriaOcupada,( TickType_t )1000)){
                if(formatearSD()){
                    notificacion(pdFALSE,0,imagenOk,imagenEsperaOFF,3,label_formato1,label_formato2,label_formato3);
                }
                else{
                    notificacion(pdFALSE,0,imagenError,imagenEsperaOFF,3,label_formato4,label_formato5,label_formato6);
                }
                xSemaphoreGive(memoriaOcupada);
            }
            else{
                notificacion(pdFALSE,0,imagenAlerta,imagenEsperaOFF,2,label_formato10,label_formato11);
            }
            vTaskDelay(pdMS_TO_TICKS(T_NOTIFICACIONES));
            ventana = PANTALLA_CONF_MEMORIA;
        }
    }
    return ventana;
}

uint8 confirmar_productos(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CONFIRMAR_PRODUCTO;
    CYBIT comando_ok = pdFALSE;
    CYBIT fin = pdFALSE;
    uint8 memoriaSD;
    uint16 i=0;
    //char ruta[50];
    //RTC_RTC_TIME_DATE fechaHora;
    xProducto productoSolicitado;
    
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_guardar1,label_conf_guardar2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 10);
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,3,label_conf_guardar8,label_conf_guardar9,label_conf_guardar10);
            comando_ok=pdFALSE;
            bandeja=1;
            while(!fin){
                comando_ok = pdFALSE;
                if(xSemaphoreTake(eepromOcupada,( TickType_t )200)){
                    volcado = pdFALSE;
                    productoSolicitado.numero = matrizProductos[bandeja-1][i][0];
                    productoSolicitado.habilitado = matrizProductos[bandeja-1][i][1];
                    productoSolicitado.precio = matrizProductos[bandeja-1][i][2];
                    productoSolicitado.cantidad = matrizProductos[bandeja-1][i][3];
                    productoSolicitado.alturaXItem = matrizProductos[bandeja-1][i][4];
                    productoSolicitado.proveedor = matrizProductos[bandeja-1][i][5];
                    comando_ok=escribirProducto(productoSolicitado);
                    xSemaphoreGive(eepromOcupada);
                }
                else{
                    notificacion(pdFALSE,0,imagenAlerta,imagenEsperaOFF,2,label_conf_guardar6,label_conf_guardar7);
                    fin=pdTRUE;
                }
                i++;
                if(i>=(NUMERO_PRODUCTOS_BANDEJA_MAX/*MAQUINAS_INTERCONECTADAS*/)){
                    bandeja++;
                    i=0;
                }
                if((comando_ok==pdFALSE)||(bandeja>NUMERO_BANDEJAS_MAX)){
                    fin=pdTRUE;
                }
            }
            if(comando_ok==pdTRUE){
                notificacion(pdFALSE,0,imagenOk,imagenEsperaOFF,3,label_conf_guardar3,label_conf_guardar4,label_conf_guardar5);
            }
            else{
                notificacion(pdFALSE,0,imagenError,imagenEsperaOFF,3,label_conf_guardar11,label_conf_guardar12,label_conf_guardar13);
            }
            vTaskDelay(pdMS_TO_TICKS(T_NOTIFICACIONES));
            bandera = pdFALSE;
            ventana = PANTALLA_MENU_OPERATIVO;
        }
    }
    return ventana;
}

uint8 confirmar_restauracion(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_CONFIRMAR_REST;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_conf_parametros1,label_conf_parametros2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_CONF_MEMORIA;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            notificacion(pdTRUE,0,imagenNull,imagenEsperaON,3,label_rest_parametros1,label_rest_parametros2,label_rest_parametros3);
            if(xSemaphoreTake(memoriaOcupada,( TickType_t )100)){
                if(restaurarConfiguracionSD()&&restaurarProductosSD()){
                    notificacion(pdFALSE,0,imagenOk,imagenEsperaOFF,5,label_rest_parametros4,label_rest_parametros5,label_rest_parametros6,label_rest_parametros7,label_rest_parametros8);
                }
                else{
                    notificacion(pdFALSE,0,imagenError,imagenEsperaOFF,5,label_rest_parametros9,label_rest_parametros10,label_rest_parametros11,label_rest_parametros14,label_rest_parametros15);
                }
                xSemaphoreGive(memoriaOcupada);
            }
            else{
                notificacion(pdFALSE,0,imagenAlerta,imagenEsperaOFF,2,label_rest_parametros12,label_rest_parametros13);
            }
            vTaskDelay(pdMS_TO_TICKS(T_NOTIFICACIONES));
            ventana = PANTALLA_CONF_MEMORIA;
        }
    }
    return ventana;
}

uint8 confirmar_salida(){
    bufferEntradaHMI respuesta;
    xresPantalla peticion;
    uint8 ventana = PANTALLA_CONFIRMAR_SALIDA;
    if(bandera==pdFALSE){
        confCancel(pdTRUE,"%t0-1 %t1-1", label_cancelarMenu1,label_cancelarMenu2);  
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"NO")){
            bandera = pdFALSE;
            ventana = PANTALLA_MENU;
        }
        if(!strcmp(respuesta.buffer,"SI")){
            bandera = pdFALSE;
            xQueuePeek(respuestaPantalla,(void*)&peticion,100);
            peticion.operacion = ACTIVAR_SISTEMAS;
            xQueueOverwrite(respuestaPantalla, (void*)&peticion);
            ventana = PANTALLA_PRINCIPAL;
        }
    }
    return ventana;
}

uint8 contabilidad(){
    uint8 ventana = PANTALLA_CONTABILIDAD;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    uint8 memoriaSD;
    
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 1);
    if(memoriaSD==LISTO){
        //int registros=0;
        if(bandera==pdFALSE){
            offset = 0;
            while(!comando_ok){
                comando_ok=HMI_ventana(VENTANA_CONTABILIDAD);
            }
            switch(contabilidadSolicitada){
                case CONTABILIDAD_DIARIA:
                    graficarContabilidadDiaria();
                break;
                case CONTABILIDAD_SEMANAL:
                    graficarContabilidadSemanal();
                break;  
                case CONTABILIDAD_MENSUAL:
                    graficarContabilidadMensual();
                break;    
                case CONTABILIDAD_ANUAL:
                    graficarContabilidadAnual();
                break;
            }
            bandera = pdTRUE;
        }
        respuesta = HMI_rx_comando();
        if(respuesta.objeto=='B'){
            if(!strcmp(respuesta.buffer,"VOLVER")){
                bandera=0;
                ventana = PANTALLA_MENU_CONTABILIDAD;
            }
            if(!strcmp(respuesta.buffer,"ARRIBA")){
                offset -= SALTOS;
                if(offset<0){
                    offset = 0;
                }
                switch(contabilidadSolicitada){
                    case CONTABILIDAD_DIARIA:
                        graficarContabilidadDiaria();
                    break;
                    case CONTABILIDAD_SEMANAL:
                        graficarContabilidadSemanal();
                    break;  
                    case CONTABILIDAD_MENSUAL:
                        graficarContabilidadMensual();
                    break;    
                    case CONTABILIDAD_ANUAL:
                        graficarContabilidadAnual();
                    break;
                }
                ventana = PANTALLA_CONTABILIDAD;
            }
            if(!strcmp(respuesta.buffer,"ABAJO")){
                offset += SALTOS;
                switch(contabilidadSolicitada){
                    case CONTABILIDAD_DIARIA:
                        graficarContabilidadDiaria();
                    break;
                    case CONTABILIDAD_SEMANAL:
                        graficarContabilidadSemanal();
                    break;  
                    case CONTABILIDAD_MENSUAL:
                        graficarContabilidadMensual();
                    break;    
                    case CONTABILIDAD_ANUAL:
                        graficarContabilidadAnual();
                    break;
                }
                ventana = PANTALLA_CONTABILIDAD;
            }
        }
    }
    else{
        notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenAlerta,imagenEsperaOFF,3,label_nota_memoriaSD1,label_nota_memoriaSD2,label_nota_memoriaSD3);
        bandera = pdFALSE;
        ventana = PANTALLA_MENU_CONTABILIDAD;
    }
    return ventana;
}

uint8 espacio(){
    uint8 ventana = PANTALLA_ESPACIO;
    //CYBIT comando_ok = FALSE;
    bufferEntradaHMI respuesta;
    xEspacioMemoria espacio;
    char string[20];
    int porcentaje=0;
    if(!bandera){
        confGeneral(pdFALSE,"%t0 %b0-0-0 %b1-1-0 %b2-0-0 %b3-0-0 %j0-1",label_espacio1, label_vacio, label_vacio, label_vacio, label_vacio, "0");
        espacio = espacioSD();
        sprintf(string,"%2.1f Mb/ %2.1f Mb", espacio.utilizado, espacio.capacidad);
        HMI_tx_comando("ConfGeneral.b","1","txt",string);
        porcentaje=(espacio.utilizado/espacio.capacidad)*100;
        sprintf(string, "%d", porcentaje);
        HMI_tx_comando("ConfGeneral.j","0","val",string);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        if(respuesta.buffer[0]==botonVolver){
            bandera=pdFALSE;
            confGeneral(pdFALSE,"%t0 %b0-1-1 %b1-1-1 %b2-1-1 %b3-1-1 %j0-0",label_memoria1,label_memoria2,label_memoria3,label_memoria4,label_memoria5,"0");
            HMI_set_value("volver",0);
            ventana = PANTALLA_CONF_MEMORIA;
        }
    }
    return ventana;
}

uint8 fecha_hora(){
    uint8 ventana = PANTALLA_FECHAYHORA;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    if(bandera==pdFAIL){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_FECHA_HORA);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CANCELAR")){
            bandera=pdFALSE;
            actualizarFechaPsoc();
            ventana = PANTALLA_CONFIGURACION;
        }
    }
    return ventana;
}

uint8 informacion(){
    uint8 ventana = PANTALLA_INFORMACION;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    int fila = 2;
    char version[10];
    //char celdas[4];
    /*En esta pantalla se muesta información importante de la máquina*/
    if(bandera==0){
        while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
        serie = leerSerie();
        horasDeMaquina = leerHoras();
        xSemaphoreGive(eepromOcupada);
        HMI_tx_comando("Informacion.t","0","txt",label_informacion1);
        HMI_tx_comando("Informacion.tA","1","txt",label_informacion2);
        HMI_tx_comando("Informacion.tB","1","txt",label_informacion3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_INFORMACION);
        }
        bandera = 1;
    }
    //Se muestra el nombre
    escribirInformacionTexto(fila,label_informacion4,serie.nombre);
    fila++;
    
    //Se muestra la serie de la máquina
    escribirInformacionTexto(fila,label_informacion5,serie.serie);
    fila++;
    
    //Se muestra la versión de software
    escribirInformacionTexto(fila,label_informacion6,VERSION_SOFTWARE);
    fila++;
    
    //Se muestra la versión de pantalla
    do{
        HMI_get_value("version.txt");
        respuesta = HMI_rx_comando();
    }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO ));
    escribirInformacionTexto(fila,label_informacion11,respuesta.buffer);
    fila++;
    
    //Se muestra la versión de Máquina
    escribirInformacionTexto(fila,label_informacion7,serie.version);
    fila++;
    
    //Se muestra la fecha de fabricación
    escribirInformacionTexto(fila,label_informacion9,serie.fecha);
    fila++;
    
    //Se muestra las horas de trabajo
    escribirInformacionNumero(fila,label_informacion10,horasDeMaquina.horas);
    fila++;
    while(!xSemaphoreTake(busOcupado,10));
    xMedidaDispensador voltaje = Dispensador_LeerVoltaje();
    xSemaphoreGive(busOcupado);
    sprintf(version,"%f",voltaje.medida);
    escribirInformacionTexto(fila,"voltaje",version);
    fila++;
    
    //Se muestra el serial completo
    fila++;
    escribirInformacionTexto(fila,serie.numSerie,label_vacio);
    fila++;
    
    /*//Se muestra valor de depuracion
    sprintf(celdas,"%d",fila); 
    sprintf(objeto,"%.6f",leerEscalaPeso()); 
    HMI_tx_comando("tA",celdas,"txt",objeto);
    HMI_tx_comando("tB",celdas,"txt",label_vacio);
    fila++;*/
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
        }
    }
    return ventana;
}

uint8 informacionBilletero(){
    uint8 ventana = PANTALLA_BILLETERO_INFO;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xbill_setup billeteroConfig;
    xbill_id1 billeteroIdent1;
    xbill_id2 billeteroIdent2;
    xbill_type_recycler recicladorTipo;
    char objeto[20], celdas[4];
    int fila = 2;
    
    xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    xQueuePeek(identBill1,(void*)&billeteroIdent1,10);
    xQueuePeek(identBill2,(void*)&billeteroIdent2,10);
    xQueuePeek(typeRecycler,(void*)&recicladorTipo,10);
    /*En esta pantalla se muesta información importante del billetero*/
    if(bandera==pdFALSE){
        HMI_tx_comando("Depuracion.t","0","txt",label_infoBilletero1);
        HMI_tx_comando("Depuracion.tA","1","txt",label_infoBilletero2);
        HMI_tx_comando("Depuracion.tB","1","txt",label_infoBilletero3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_INFORMACION);
        }
        bandera = pdTRUE;
    }
    
    //Se muestra el nivel de billetero
    escribirInformacionNumero(fila,label_infoBilletero4,billeteroConfig.nivel);
    fila++;
    
    //Se muestra información de pais
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoBilletero5);
    if(billeteroConfig.pais & 0xF000){
        sprintf(objeto,"%X (iso)",billeteroConfig.pais & 0x0FFF);      
    }
    else{
        sprintf(objeto,"%X",billeteroConfig.pais & 0x0FFF);
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Se muestra el factor de escala
    escribirInformacionNumero(fila,label_infoBilletero6,billeteroConfig.factorEscala);
    fila++;
    
    //Se muestra punto decimal
    escribirInformacionNumero(fila,label_infoBilletero7,billeteroConfig.factorDecimal);
    fila++;

    //Se muestra capacidad de stacker
    escribirInformacionNumero(fila,label_infoBilletero8,billeteroConfig.capacidad);
    fila++;
    
    //Se muestra escrow
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoBilletero9);
    if(billeteroConfig.escrow == 0xFF){
        strcpy(objeto,"SI");
    }
    else{
        strcpy(objeto,"NO");
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Se muestra serial
    escribirInformacionTexto(fila,label_infoBilletero10,billeteroIdent1.serie);
    fila++;
    
    //Se muestra el modelo
    escribirInformacionTexto(fila,label_infoBilletero11,billeteroIdent2.modelo);
    fila++;
    
    //Se muestra la version
    escribirInformacionNumero(fila,label_infoBilletero12,billeteroIdent1.version);
    fila++;
    
    //Soporte FTL
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoBilletero13);
    if(billeteroIdent2.opcional == 0x0001){
        strcpy(objeto,"SI");
    }
    else{
        strcpy(objeto,"NO");
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Soporte Reciclador de billetes
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoBilletero14);
    if(billeteroIdent2.reciclador == pdTRUE){
        strcpy(objeto,"SI");
    }
    else{
        strcpy(objeto,"NO");
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    if(billeteroIdent2.reciclador == pdTRUE){
        //billetes que pueden ser reciclados
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt",label_infoBilletero15);
        sprintf(objeto,"%X",recicladorTipo.tipo);
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //se limpian todas las celdas que queden vacias
    for(;fila<=22;fila++){
        escribirInformacionTexto(fila,label_vacio,label_vacio);
    }
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_BILLETERO;
        }
    }
    return ventana;
}

uint8 informacionGSM(){
    uint8 ventana = PANTALLA_INFO_GSM;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xInformacionGSM infoGSM;
    xConfiguracionTelemetria confGSM;
    char objeto[30], celdas[4];
    int fila = 2;
    xQueuePeek(datosGSM, (void*)&infoGSM,10);
    xQueuePeek(configuracionesRemotas, (void*)&confGSM, 10);
    /*En esta pantalla se muesta información importante del modulo GSM*/
    if(bandera==0){
        HMI_tx_comando("Depuracion.t","0","txt",label_infoGSM1);
        HMI_tx_comando("Depuracion.tA","1","txt",label_infoGSM2);
        HMI_tx_comando("Depuracion.tB","1","txt",label_infoGSM3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_INFORMACION);
        }
        bandera = 1;
    }
    
    //Se muestra el registro en el operador
    switch(infoGSM.registro.resultado2){
        case 0:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg0);
        break;
        case 1:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg1);
        break;
        case 2:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg2);
        break; 
        case 3:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg3);
        break; 
        case 4:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg4);
        break;     
        case 5:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg5);
        break;     
        default:
            escribirInformacionTexto(fila,label_infoGSM5,label_infoReg4);
        break;      
    }
    fila++;
    
    //Se muestra la potencia de la señal
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoGSM6);
    sprintf(objeto,"%d,%d",infoGSM.potenciaSenal.resultado1,infoGSM.potenciaSenal.resultado2);
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Se muestra el Operador
    if((infoGSM.registro.resultado2==1)||(infoGSM.registro.resultado2==5)){
        escribirInformacionTexto(fila,label_infoGSM11,infoGSM.operador);
        fila++;
    }
    
    //Se muestra el estado del GPRS
    switch(infoGSM.GPRS.resultado1){
        case 0:
            escribirInformacionTexto(fila,label_infoGSM10,label_infoGPRS0);
        break;
        case 1:
            escribirInformacionTexto(fila,label_infoGSM10,label_infoGPRS1);
        break;    
        default:
            escribirInformacionTexto(fila,label_infoGSM10,label_infoGPRS2);
        break;      
    }
    fila++;
    
    //Se muestra el fabricante
    escribirInformacionTexto(fila,label_infoGSM4,infoGSM.fabricante);
    fila++;

    
    //Se muestra el modelo
    escribirInformacionTexto(fila,label_infoGSM7,infoGSM.modelo);
    fila++;
    
    //Se muestra el IMEI
    escribirInformacionTexto(fila,label_infoGSM8,infoGSM.IMEI);
    fila++;
    
    //Se muestra el IMSI
    escribirInformacionTexto(fila,label_infoGSM9,infoGSM.IMSI);
    fila++;
    
    if((infoGSM.registro.resultado2==1)||(infoGSM.registro.resultado2==5)){
        //Se muestra el estado de conexion
        escribirInformacionTexto(fila,label_infoGSM12,infoGSM.estado);
        fila++;
        
        if(strcmp(infoGSM.IpLocal,"")){
            //Se muestra la IP local de la conexion GPRS
            escribirInformacionTexto(fila,label_infoGSM13,infoGSM.IpLocal);
            fila++;
        }
    }
    
    //Se muestra depuracion
    escribirInformacionNumero(fila,"depurando",infoGSM.debug.resultado1);
    fila++;
    escribirInformacionNumero(fila,"depurando",infoGSM.debug.resultado2);
    fila++;
    
    //se limpian todas las celdas que queden vacias
    for(;fila<=22;fila++){
        escribirInformacionTexto(fila,label_vacio,label_vacio);
    }
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_TELEMETRIA;
        }
    }
    return ventana;
}

uint8 informacionMonedero(){
    uint8 ventana = PANTALLA_MONEDERO_INFO;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xcoin_setup monederoConfig;
    xcoin_id monederoIdent;
    char objeto[20], celdas[4];
    int fila = 2;
    
    xQueuePeek(setupCoin,(void*)&monederoConfig,10);
    xQueuePeek(identCoin,(void*)&monederoIdent,10);
    /*En esta pantalla se muesta información importante del billetero*/
    if(bandera==pdFALSE){
        HMI_tx_comando("Depuracion.t","0","txt",label_infoMonedero1);
        HMI_tx_comando("Depuracion.tA","1","txt",label_infoMonedero2);
        HMI_tx_comando("Depuracion.tB","1","txt",label_infoMonedero3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_INFORMACION);
        }
        bandera = pdTRUE;
    }
    
    //Se muestra el nivel de billetero
    escribirInformacionNumero(fila,label_infoMonedero4,monederoConfig.nivel);
    fila++;
    
    //Se muestra información de pais
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoMonedero5);
    if(monederoConfig.pais & 0xF000){
        sprintf(objeto,"%X (iso)",monederoConfig.pais & 0x0FFF);      
    }
    else{
        sprintf(objeto,"%X",monederoConfig.pais & 0x0FFF);
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Se muestra el factor de escala
    escribirInformacionNumero(fila,label_infoMonedero6,monederoConfig.factorEscala);
    fila++;
    
    //Se muestra punto decimal
    escribirInformacionNumero(fila,label_infoMonedero7,monederoConfig.decimales);
    fila++;
    
    //Se muestra serial
    escribirInformacionTexto(fila,label_infoMonedero8,monederoIdent.serie);
    fila++;
    
    //Se muestra el modelo
    escribirInformacionTexto(fila,label_infoMonedero9,monederoIdent.modelo);
    fila++;
    
    //Se muestra la version
    escribirInformacionNumero(fila,label_infoMonedero10,monederoIdent.version);
    fila++;
    
    //Soporte FTL
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoMonedero11);
    if(monederoIdent.opcional & 0x0008){
        strcpy(objeto,"SI");
    }
    else{
        strcpy(objeto,"NO");
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //Soporte algoritmo de devolucion avanzada
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_infoMonedero12);
    if(monederoIdent.opcional & 0x0001){
        strcpy(objeto,"SI");
    }
    else{
        strcpy(objeto,"NO");
    }
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //se limpian todas las celdas que queden vacias
    for(;fila<=22;fila++){
        escribirInformacionTexto(fila,label_vacio,label_vacio);
    }
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_MONEDERO;
        }
    }
    return ventana;
}

uint8 ingresoAltura(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_INGRESO_ALTURA;
    CYBIT comando_ok = pdFALSE;
    char label[10], string[10];
    uint8 bandeja = numProducto/100;
    uint8 columna = numProducto-(bandeja*100); //encuentre la columna correspondiente
    if(bandera==pdFALSE){
        HMI_tx_comando("Numerico.t","0","txt",label_ingresoAltura);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_NUMERICO);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            matrizProductos[bandeja-1][columna][4]=atoi(respuesta.numero);
            sprintf(string,"%d",columna);
            if(matrizProductos[bandeja-1][columna][4]==0){
                HMI_tx_comando("Producto.bA",string,"txt","ALTURA");
            }
            else{
                sprintf(label,"%d",matrizProductos[bandeja-1][columna][4]);
                HMI_tx_comando("Producto.bA",string,"txt",label);
            }
            bandera=0;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ventana;
}

uint8 ingresoCantidad(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_INGRESO_CANTIDAD;
    CYBIT comando_ok = pdFALSE;
    uint8 bandeja = numProducto/100;
    uint8 columna = numProducto-(bandeja*100); //encuentre la columna correspondiente
    char label[10], string[10];
    if(bandera==pdFALSE){
        HMI_tx_comando("Numerico.t","0","txt",label_ingresoCantidad);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_NUMERICO);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            matrizProductos[bandeja-1][columna][3]=atoi(respuesta.numero);
            sprintf(string,"%d",columna);
            if(matrizProductos[bandeja-1][columna][3]==0){
                HMI_tx_comando("Producto.bC",string,"txt","ALTURA");
            }
            else{
                sprintf(label,"%d",matrizProductos[bandeja-1][columna][3]);
                HMI_tx_comando("Producto.bC",string,"txt",label);
            }
            bandera=0;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ventana;
}

uint8 ingresoCedula(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_INGRESO_CEDULA;
    CYBIT comando_ok = pdFALSE;
    if(bandera==pdFALSE){
        HMI_tx_comando("Numerico.t","0","txt",label_ingresoCedula);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_NUMERICO);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_REGISTRO;
        }
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            strcpy(usuarioActivo.identificador,respuesta.numero);
            HMI_tx_comando("Registro.t","8","txt",usuarioActivo.identificador);
            bandera=0;
            ventana = PANTALLA_REGISTRO;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ventana;
}

uint8 ingresoNombre(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_INGRESO_NOMBRE;
    CYBIT comando_ok = pdFALSE;
    if(bandera==pdFALSE){
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_CLAVE);
        }
        HMI_tx_comando("t","1","txt",label_ingresoNombre);
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_REGISTRO;
        }
        if(!strcmp(respuesta.buffer,"CONFIRMAR")){
            do{
                HMI_get_value("t0.txt");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_TEXTO)||(respuesta.estado!=HMI_CORRECTO));
            strcpy(usuarioActivo.nombre,respuesta.buffer);
            HMI_tx_comando("Registro.t","7","txt",usuarioActivo.nombre);
            bandera=0;
            ventana = PANTALLA_REGISTRO;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ventana;
}

uint8 ingresoPrecio(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_INGRESO_PRECIO;
    CYBIT comando_ok = pdFALSE;
    uint8 bandeja = numProducto/100;
    uint8 columna = numProducto-(bandeja*100); //encuentre la columna correspondiente
    char label[10], string[10];
    if(bandera==pdFALSE){
        HMI_tx_comando("Numerico.t","0","txt",label_ingresoPrecio);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_NUMERICO);
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"CLAVECANCEL")){
            bandera=0;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
        if(!strcmp(respuesta.buffer,"DISPENSAR#")){
            matrizProductos[bandeja-1][columna][2]=atoi(respuesta.numero);
            sprintf(string,"%d",columna);
            if(matrizProductos[bandeja-1][columna][2]==0){
                HMI_tx_comando("Producto.bP",string,"txt","ALTURA");
            }
            else{
                sprintf(label,"%d",matrizProductos[bandeja-1][columna][2]);
                HMI_tx_comando("Producto.bP",string,"txt",label);
            }
            bandera=0;
            ventana = PANTALLA_CONFIG_PRODUCTO;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    return ventana;
}

uint8 luz_maquina(){
    uint8 ventana = PANTALLA_LUZ_MAQUINA;
    bufferEntradaHMI respuesta;
    xConfiguracion parametros;
    
    xQueuePeek(configuracionesMaquina,&parametros,10);
    if(bandera==pdFALSE){
        cambio = pdFALSE;
        confGeneral(pdTRUE,"%t0 %b0-1-0 %b1-0-0 %bt0-1",label_luz_maquina1, label_luz_maquina2, label_luz_maquina3);
        HMI_set_value("bt0",parametros.BrilloMaquina.activo);
        HMI_set_value("h0",parametros.BrilloMaquina.nivel);
    }
    HMI_get_value("h0.val");//obtiene el valor de brillo guardado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        parametros.BrilloMaquina.nivel = respuesta.buffer[0];
    }
    HMI_get_value("bt0.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        parametros.BrilloMaquina.activo = respuesta.buffer[0];
        if(parametros.BrilloMaquina.activo==1){
            if(!cambio){
                HMI_visualizar("b1",pdTRUE);
                HMI_visualizar("b11",pdTRUE);
                HMI_visualizar("h0",pdTRUE);
                cambio = pdTRUE;
            }
        }
        else{
            cambio = pdFALSE;
            HMI_visualizar("b1",pdFALSE);
            HMI_visualizar("b11",pdFALSE);
            HMI_visualizar("h0",pdFALSE);
        }
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        if(respuesta.buffer[0]==botonVolver){
            bandera=pdFALSE;
            while(!xSemaphoreTake(eepromOcupada,10));
            escribirBrilloMaquina(parametros.BrilloMaquina);
            xSemaphoreGive(eepromOcupada);
            ventana = PANTALLA_ILUMINACION;
        }
    }
    xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
    return ventana;
}

uint8 menu(){
    uint8 ventana = PANTALLA_MENU;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xEventos evento;
    
    if(bandera==pdFALSE){
        while(!xSemaphoreTake(busOcupado,1));
        if(Dispensador_leer_puerta()==EMERGENCIA)
        {
            HMI_set_value("Menu.puerta",2);
        }
        else
        {
            HMI_set_value("Menu.puerta",pdFALSE);
            if(Dispensador_leer_puerta()==pdTRUE){
                HMI_set_value("Menu.puerta",pdTRUE);
            }
            else{
                HMI_set_value("Menu.puerta",pdFALSE);
            }
        }
        xSemaphoreGive(busOcupado);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_MENU);
        }
        switch(usuario){
            case FABRICANTE:
                HMI_tx_comando("t","1","txt",label_menu1);
            break;
            case ADMINISTRADOR:
                HMI_tx_comando("t","1","txt",label_menu2);
            break;
            case TEMPORAL:
                HMI_tx_comando("t","1","txt",label_menu3);
            break;    
            case PROVEEDOR:
                HMI_tx_comando("t","1","txt",label_menu4);
            break;    
        }
        bandera = pdTRUE;
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_CONFIRMAR_SALIDA;
        }
        if(!strcmp(respuesta.buffer,"CONFIG")){
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
        }
        if(!strcmp(respuesta.buffer,"ABRIR")){
            while(!xSemaphoreTake(busOcupado,( TickType_t )10));
            Dispensador_abrir_puerta(pdTRUE);
            vTaskDelay(pdMS_TO_TICKS(1));
            xSemaphoreGive(busOcupado);
            strcpy(evento.tipo,tipo_anuncio);
            strcpy(evento.evento,text_registro_puerta_ab);
            evento.operacion = ACTUALIZAR_REGISTRO; 
            xQueueSendToBack(actualizarRegistro,&evento,200);
        }
        if(!strcmp(respuesta.buffer,"CERRAR")){
            while(!xSemaphoreTake(busOcupado,( TickType_t )10));
            if(Dispensador_leer_puerta()!=EMERGENCIA){
                Dispensador_abrir_puerta(pdFALSE);
                HMI_set_value("bt1",pdFALSE);
                HMI_set_value("puerta",pdFALSE);
                HMI_visualizar("p0",pdFALSE);
                strcpy(evento.tipo,tipo_anuncio);
                strcpy(evento.evento,text_registro_puerta_ce);
                evento.operacion = ACTUALIZAR_REGISTRO; 
                xQueueSendToBack(actualizarRegistro,&evento,200);
            }
            else{
                HMI_set_value("puerta",pdTRUE);
                HMI_set_value("bt1",pdTRUE);
                HMI_visualizar("p0",pdTRUE);
            }
            xSemaphoreGive(busOcupado);
        }
        if(!strcmp(respuesta.buffer,"MOTORES")){
            bandera=0;
            ventana = PANTALLA_MENU_OPERATIVO;
        }
        if(!strcmp(respuesta.buffer,"CONTABILIDAD")){
            bandera=0;
            ventana = PANTALLA_MENU_CONT_EST;
        }
        if(!strcmp(respuesta.buffer,"CONECTIVIDAD")){
            bandera=0;
            ventana = PANTALLA_MENU_CONECTIVIDAD;
        }
    }
    return ventana;
}

uint8 menu_conectividad(){
    uint8 ventana = PANTALLA_MENU_CONECTIVIDAD;
    bufferEntradaHMI respuesta;
    xConfiguracionTelemetria parametros;
    xQueuePeek(configuracionesRemotas,(void*)&parametros,10);
    if(bandera==pdFALSE){
        if(fabricante){
            confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-0",label_menu_conect1,label_menu_conect2,label_menu_conect4);
        }
        else{
            confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-0",label_menu_conect1,label_menu_conect2,label_menu_conect4);
        }
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton1:
                if(parametros.existenciaTelemetria){
                    bandera=pdFALSE;
                    ventana = PANTALLA_TELEMETRIA;
                }
                else{
                    notificacion(pdTRUE,20,imagenAlerta,imagenEsperaOFF,3,label_nota_telemetria1,label_nota_telemetria2,label_nota_telemetria3);
                    ventana = PANTALLA_MENU_CONECTIVIDAD;
                }
            break;     
            case boton2:
                bandera=pdFALSE;
                ventana = PANTALLA_CONF_CONECTIVIDAD;
            break;    
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU;
            break;
        }
    }
    return ventana;
}

uint8 menu_contabilidad(){
    uint8 ventana = PANTALLA_MENU_CONTABILIDAD;
    bufferEntradaHMI respuesta;
    uint8 memoriaSD;
    
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 10);
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1",label_menu_contabilidad1, label_menu_contabilidad2, label_menu_contabilidad3, label_menu_contabilidad4, label_menu_contabilidad5);
    } 
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:
                HMI_tx_comando("Contabilidad.t","0","txt",label_cont_diaria1);
                HMI_tx_comando("Contabilidad.tA","1","txt",label_cont_diaria2);
                HMI_tx_comando("Contabilidad.tB","1","txt",label_cont_diaria3);
                HMI_tx_comando("Contabilidad.tC","1","txt",label_cont_diaria4);
                contabilidadSolicitada = CONTABILIDAD_DIARIA;
                ventana = PANTALLA_CONTABILIDAD;
                bandera=pdFALSE;
            break;
            case boton2:
                HMI_tx_comando("Contabilidad.t","0","txt",label_cont_semanal1);
                HMI_tx_comando("Contabilidad.tA","1","txt",label_cont_semanal2);
                HMI_tx_comando("Contabilidad.tB","1","txt",label_cont_semanal3);
                HMI_tx_comando("Contabilidad.tC","1","txt",label_cont_semanal4);
                contabilidadSolicitada = CONTABILIDAD_SEMANAL;
                ventana = PANTALLA_CONTABILIDAD;
                bandera=pdFALSE;
            break;
            case boton3:
                HMI_tx_comando("Contabilidad.t","0","txt",label_cont_mensual1);
                HMI_tx_comando("Contabilidad.tA","1","txt",label_cont_mensual2);
                HMI_tx_comando("Contabilidad.tB","1","txt",label_cont_mensual3);
                HMI_tx_comando("Contabilidad.tC","1","txt",label_cont_mensual4);
                contabilidadSolicitada = CONTABILIDAD_MENSUAL;
                ventana = PANTALLA_CONTABILIDAD;
                bandera=pdFALSE;
            break; 
            case boton4:
                HMI_tx_comando("Contabilidad.t","0","txt",label_cont_anual1);
                HMI_tx_comando("Contabilidad.tA","1","txt",label_cont_anual2);
                HMI_tx_comando("Contabilidad.tB","1","txt",label_cont_anual3);
                HMI_tx_comando("Contabilidad.tC","1","txt",label_cont_anual4);
                contabilidadSolicitada = CONTABILIDAD_ANUAL;
                ventana = PANTALLA_CONTABILIDAD;
                bandera=pdFALSE;
            break; 
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_CONT_EST;
            break;
        }
    }
    return ventana;
}

uint8 menu_cont_estad(){
    uint8 ventana = PANTALLA_MENU_CONT_EST;
    bufferEntradaHMI respuesta;
    uint8 memoriaSD;   
    xConfiguracionAccesorios confAccesorios;
    
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 10);
    xQueuePeek(configuracionesAccesorios, (void*)&confAccesorios,100);
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1",label_menu_cont_est1,label_menu_cont_est2,label_menu_cont_est3);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_CONTABILIDAD;
            break;
            case boton2:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_REGISTRO;
            break;
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU;
            break;
        }
    }
    return ventana;
}

uint8 menu_confGSM(){
    uint8 ventana = PANTALLA_MENU_CONFGSM;
    bufferEntradaHMI respuesta;
    
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-0",label_conf_GSM1, label_vacio, label_vacio);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_TELEMETRIA;
            break;
        }
    }
    return ventana;
}

uint8 menuDiagnostico(){
    uint8 ventana = PANTALLA_DIAGNOSTICO;
    bufferEntradaHMI respuesta;
    
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1",label_diagnostico1, label_diagnostico2, label_diagnostico3, label_diagnostico4, label_diagnostico5);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:
                bandera=pdFALSE;
                ventana = PANTALLA_PESO;
            break;
            case boton2:
                bandera=pdFALSE;
                ventana = PANTALLA_RTOS;
            break;
            case boton3:
            break;
            case boton4:
                HMI_calibrar();
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIGURACION;
            break;
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIGURACION;
            break;
        }
    }
    return ventana;
}

uint8 menu_estadistica(){
    uint8 ventana = PANTALLA_MENU_ESTADISTICA;
    bufferEntradaHMI respuesta;
    uint8 memoriaSD;
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 10);
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1,%b3-1",label_menu_estadistica1, label_menu_estadistica2, label_menu_estadistica3, label_menu_estadistica4, label_menu_estadistica5);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_CONT_EST;
            break;
        }
    }
    return ventana;
}

uint8 menu_iluminacion(){
    uint8 ventana = PANTALLA_ILUMINACION;
    bufferEntradaHMI respuesta;
    
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1",label_menu_iluminacion1,label_menu_iluminacion2,label_menu_iluminacion3);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton1:{
                bandera=0;
                ventana = PANTALLA_BRILLO_PANTALLA;
                break;
            }
            case boton2:{
                bandera=0;
                ventana = PANTALLA_LUZ_MAQUINA;
                break;
            }
            case botonVolver:{
                bandera=0;
                ventana = PANTALLA_CONFIGURACION;
                break;
            }
        }
    }
    return ventana;
}

uint8 menu_memoria(){
    uint8 ventana = PANTALLA_CONF_MEMORIA;
    bufferEntradaHMI respuesta;
    
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1",label_memoria1,label_memoria2,label_memoria3,label_memoria4,label_memoria5);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIRMAR_FORMATEO;
            break;
            case boton2:
                bandera=pdFALSE;
                ventana = PANTALLA_ESPACIO;
            break;
            case boton3:
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIRMAR_COPIA;
            break;
            case boton4:
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIRMAR_REST;
            break;
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIGURACION;
            break;
        }
    }
    return ventana;
}

uint8 menu_operativo(){
    uint8 ventana = PANTALLA_MENU_OPERATIVO;
    bufferEntradaHMI respuesta;
    
    if(bandera==pdFALSE){
        if(fabricante){
            confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1 %b4-1",label_motores1, label_motores2, label_motores3, label_motores4, label_motores5, label_motores6);
        }
        else{
            confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1",label_motores1, label_motores2, label_motores3, label_motores4, label_motores5);
        }
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:
                bandera=pdFALSE;
                volcado = pdFALSE; 
                bandeja = 1;
                ventana = PANTALLA_CONFIG_PRODUCTO;
            break;
            case boton2:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_MOTORES;
            break;
            case boton3:
                bandera=pdFALSE;
                ventana = PANTALLA_SIMULACION_VENTA;
            break;
            case boton4:
                bandera=pdFALSE;
                //ventana = PANTALLA_CONFIG_USUARIOS;
                ventana = PANTALLA_MENU_HUELLAS;
            break;
            case boton5:
                bandera=pdFALSE;
                ventana = PANTALLA_CONFIG_BANDEJAS;
            break;
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU;
            break;
        }
    }
    return ventana;
}

uint8 menu_reciclador(){
    uint8 ventana = PANTALLA_RECICLADOR;
    bufferEntradaHMI respuesta;
    xcomandoHMI funcionHMI;
    xSistemasPago parametros;
    
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %bt0-1",label_reciclador1,label_reciclador2,label_reciclador3,label_reciclador4);
        HMI_set_value("bt0",parametros.recicladorHabilitado);
    }
    HMI_get_value("bt0.val");//obtiene la configuración automatica o manual
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        parametros.recicladorHabilitado = respuesta.buffer[0];
    }
    HMI_get_value("estado.val");//verifica si un boton fue presionado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton2:{
                funcionHMI.comando = MDB_EKIA_VACIAR_RECICLADOR;
                funcionHMI.opcional1 = EKIA_ACK;
                xQueueSend(comandosHMI, &funcionHMI, 10);
                devolucionDinero();
                bandera = 0;
                ventana = PANTALLA_RECICLADOR;
                break;
            }
            case boton3:{
                bandera=0;
                ventana = PANTALLA_BILLETES_RECICLADOS;
                break;
            }
            case botonVolver:{
                bandera=0;
                while(!xSemaphoreTake(eepromOcupada,10));
                EEPROM_escribirHabilitacionReciclador(parametros.recicladorHabilitado);
                xSemaphoreGive(eepromOcupada);
                ventana = PANTALLA_BILLETERO;
                break;
            }
        }
    }
    xQueueOverwrite(configuracionSistemasPago,(void*)&parametros);
    return ventana;
}

uint8 menu_telemetria(){
    uint8 ventana = PANTALLA_TELEMETRIA;
    bufferEntradaHMI respuesta;
    xInformacionGSM infoGSM;
    xQueuePeek(datosGSM,&infoGSM,10);
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %bt0-1",label_telemetria1,label_telemetria2,label_telemetria3,label_telemetria4);
        if(xTaskGetHandle("Telemetria") != NULL){
            TelemetriaEncendido = pdTRUE;
            TelemetriaApagado = pdFALSE;
            HMI_set_value("bt0",pdTRUE);
        }
        else{
            TelemetriaEncendido = pdFALSE;
            TelemetriaApagado = pdTRUE;
            HMI_set_value("bt0",pdFALSE);
        }
        banderaGSM=!infoGSM.encendido;
        bandera = 1;
    }
    if(infoGSM.encendido!=banderaGSM){
        HMI_visualizar("b1",infoGSM.encendido);
        HMI_visualizar("b2",infoGSM.encendido);
        HMI_visualizar("b3",pdFALSE);
        HMI_visualizar("b11",infoGSM.encendido);
        HMI_visualizar("b12",infoGSM.encendido);
        HMI_visualizar("b13",pdFALSE);
        HMI_touch("b","1",infoGSM.encendido);
        HMI_touch("b","2",infoGSM.encendido);
        HMI_touch("b","3",pdFALSE);
        banderaGSM = infoGSM.encendido;
    }
    HMI_get_value("bt0.val");//obtiene la configuración activado o desactivado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        if(respuesta.buffer[0]==pdTRUE){
            if(!TelemetriaEncendido){
                TelemetriaEncendido = pdTRUE;
                TelemetriaApagado = pdFALSE;
                Telemetria_Start();
            }
        }
        else{
            if(!TelemetriaApagado){
                TelemetriaEncendido = pdFALSE;
                TelemetriaApagado = pdTRUE;
                Telemetria_Stop();
            }
        }
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton2:{
                bandera=0;
                ventana = PANTALLA_MENU_CONFGSM;
                break;
            }
            case boton3:{
                bandera=0;
                ventana = PANTALLA_INFO_GSM;
                break;
            }
            case botonVolver:{
                bandera=0;
                while(!xSemaphoreTake(eepromOcupada,1));
                escribirEstadoTelemetria(TelemetriaEncendido);
                xSemaphoreGive(eepromOcupada);
                ventana = PANTALLA_MENU_CONECTIVIDAD;
                break;
            }
        }
    }
    return ventana;
}

uint8 menu_huellas(){
    uint8 ventana = PANTALLA_MENU_HUELLAS;
    bufferEntradaHMI respuesta; 
    int resultado=0, timeOut=0;
    char string[6];
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1 %b3-1",label_menu_huella1, label_menu_huella2, label_menu_huella3, label_menu_huella5, label_menu_huella6);
    }
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case boton1:{
                bandera=pdFALSE;
                HMI_tx_comando("Registro.t","7","txt",label_registroUsuario1);
                HMI_tx_comando("Registro.t","8","txt",label_registroUsuario2);
                HMI_tx_comando("Registro.t","9","txt",label_registroUsuario3);
                if((usuario==TEMPORAL)||(usuario==FABRICANTE)){
                    usuarioActivo.idHuella = ID_FABRICANTE;
                }
                else{
                    usuarioActivo.idHuella = 0;
                }
                ventana = PANTALLA_REGISTRO;
            break;}
            case boton2:{
                while(!xSemaphoreTake(lectorOcupado,100));
                finger_SetLED(pdTRUE);
                notificacion(pdTRUE,1,imagenNull,imagenEsperaON,3,label_registroHuella2, label_registroHuella3, label_vacio);
                do{
                    resultado = finger_leerHuella();
                    timeOut++;
                    vTaskDelay(pdMS_TO_TICKS(300));
                }while((resultado < -1)&&(timeOut<10));
                finger_SetLED(pdFALSE);
                xSemaphoreGive(lectorOcupado);
                if(timeOut<10){
                    if(resultado == -1){
                        notificacion(pdTRUE,100,imagenError,imagenEsperaOFF,3,label_lecturaHuella3, label_lecturaHuella4, label_vacio);
                    }
                    else{
                        sprintf(string, "numero: %d\n\r",resultado);
                        notificacion(pdTRUE,100,imagenOk,imagenEsperaOFF,3,label_lecturaHuella1, label_lecturaHuella2, string);
                    }
                }
                else{
                    notificacion(pdTRUE,100,imagenAlerta,imagenEsperaOFF,3,label_lecturaHuella5, label_lecturaHuella6, label_vacio);
                }
            break;}
            case boton3:{
                bandera=pdFALSE;
                if(borradoHuellaEspecifico==pdTRUE){
                    ventana = PANTALLA_BORRAR_HUELLA_ID;
                }
                else{
                    ventana = PANTALLA_BORRAR_HUELLA;
                }
            break;}
            case boton4:{
                bandera=pdFALSE;
                HMI_tx_comando("Registro.t","7","txt",label_registroUsuario1);
                HMI_tx_comando("Registro.t","8","txt",label_registroUsuario2);
                HMI_tx_comando("Registro.t","9","txt",label_registroUsuario3);
                usuarioActivo.idHuella = ID_ADMINISTRADOR;
                ventana = PANTALLA_REGISTRO;
            break;}
            case botonVolver:{
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_OPERATIVO;
            break;}
        }
    }
    return ventana;
}

uint8 monederoEstado(){
    uint8 ventana = PANTALLA_MONEDERO_ESTADO;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xcoin_setup monederoConfig;
    xcoin_state monederoStatus;
    xcoin_tube monederoTubos;
    xSistemasPago parametros;
    xcomandoHMI funcionHMI;
    int cantidadTotal = 0;
    int tipoMoneda = 0;
    char celdas[4], moneda[2];
    uint8 actividad = 0, indice = 0;
    int fila = 2;
    
    funcionHMI.comando = MDB_EKIA_ESTADO_TUBOS;
    xQueueSend(comandosHMI, &funcionHMI, 10);
    xQueuePeek(setupCoin,(void*)&monederoConfig,10);
    xQueuePeek(stateCoin,(void*)&monederoStatus,10);
    xQueuePeek(tubeCoin, &monederoTubos,10);
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    
    /*En esta pantalla se muesta información importante del billetero*/
    if(bandera==pdFALSE){
        HMI_tx_comando("Depuracion.t","0","txt",label_estadoMonedero1);
        HMI_tx_comando("Depuracion.tA","1","txt",label_estadoMonedero2);
        HMI_tx_comando("Depuracion.tB","1","txt",label_estadoMonedero3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_INFORMACION);
        }
        bandera = pdTRUE;
    }
    
    //Se muestra el estado del cassette de monedas 
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt",label_estadoMonedero4);
    fila++;
    for(indice=0;indice<LONGITUD_TUBOS;indice++){
        tipoMoneda = monederoConfig.tipo[indice]*monederoConfig.factorEscala/monederoConfig.decimales;
        sprintf(moneda,"%d: %d",indice+1,tipoMoneda);
        escribirInformacionNumero(fila,moneda,monederoTubos.tuboEstado[indice]);
        fila++;
        cantidadTotal += tipoMoneda*monederoTubos.tuboEstado[indice];
    }
    escribirInformacionNumero(fila,label_estadoMonedero5,cantidadTotal);
    fila++;
    
    escribirInformacionTexto(fila,label_vacio,label_vacio);
    fila++;
    
    //Se muestran los avisos propios del billetero
    xQueuePeek(actividadMonedero,(void*)&actividad,100);
    switch(actividad){
        case 0x01:
            escribirInformacionTexto(fila,label_estadoMonedero6,label_activi_moneda1);
        break;
    }
    fila++;
    
    //se limpian todas las celdas que queden vacias
    for(;fila<=22;fila++){
        escribirInformacionTexto(fila,label_vacio,label_vacio);
    }
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_MONEDERO;
        }
    }
    return ventana;
}

uint8 peso(){
    uint8 ventana = PANTALLA_PESO;
    CYBIT comando_ok = pdFALSE;
    int entero, escala, pesoMinimo;
    float tempEscala = 0.0f;
    bufferEntradaHMI respuesta;
    xConfiguracion parametros;
    xMedidaDispensador medidaPeso;
    
    if(bandera==pdFALSE){
        xQueuePeek(configuracionesMaquina,&parametros,10);
        escala=(1/parametros.escalaPeso);
        HMI_set_value("Peso.escala",escala);
        pesoMinimo=(parametros.PesoMinimo);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_PESO);
        }
        HMI_set_float("x0",pesoMinimo,1);
        bandera = pdTRUE;
    }
    while(!xSemaphoreTake(busOcupado,1));
    medidaPeso = Dispensador_LeerPeso();
    vTaskDelay(pdMS_TO_TICKS(10));
    xSemaphoreGive(busOcupado);
    entero = medidaPeso.medida*100;
    HMI_set_value("rpeso",entero);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_DIAGNOSTICO;
        }
        
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            do{
                //solicite el valor en texto de cada cuadro hasta que sea correctamente recibido
                HMI_get_value("x0.val");
                respuesta = HMI_rx_comando();
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            xQueuePeek(configuracionesMaquina,&parametros,10);
            parametros.PesoMinimo = (respuesta.buffer[1]<<8) | respuesta.buffer[0];
            xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
            while(!xSemaphoreTake(eepromOcupada, pdMS_TO_TICKS(10)));
            EEPROM_escribirPesoMinimo(parametros.PesoMinimo);
            xSemaphoreGive(eepromOcupada);
            while(!xSemaphoreTake(busOcupado, pdMS_TO_TICKS(10)));
            Dispensador_definir_peso_minimo(parametros.PesoMinimo);
            xSemaphoreGive(busOcupado);
            bandera = pdFALSE;
        
        }
        if(!strcmp(respuesta.buffer,"CEROSET")){
            while(!xSemaphoreTake(busOcupado,10));
            Dispensador_setCero_peso();
            vTaskDelay(pdMS_TO_TICKS(10));
            xSemaphoreGive(busOcupado);
        }
        if(!strcmp(respuesta.buffer,"ESCALA")){
            //do{
            //    HMI_get_value("escala.val");
            //    respuesta = HMI_rx_comando();
            //}while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            xQueuePeek(configuracionesMaquina,&parametros,10);
            tempEscala = (float)((respuesta.buffer[1]<<8)|respuesta.buffer[0]);
            //tempEscala = 567.0f;
            tempEscala = (float)(1/tempEscala);
            parametros.escalaPeso = (float)tempEscala;
            xQueueOverwrite(configuracionesMaquina,&parametros);
            while(!xSemaphoreTake(memoriaOcupada,100));
            escribirEscalaPeso(tempEscala);
            xSemaphoreGive(memoriaOcupada);
            while(!xSemaphoreTake(busOcupado,10));
            Dispensador_definir_escala_peso(tempEscala);
            vTaskDelay(pdMS_TO_TICKS(10));
            xSemaphoreGive(busOcupado);
            HMI_set_value("escala",parametros.escalaPeso);
        } 
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    return ventana;
}

uint8 principal(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_PRINCIPAL;
    CYBIT comando_ok = pdFALSE;  
    xConfiguracion parametros;
    xConfiguracionTelemetria confTelemetria;
    xresPantalla peticion;
    xAutorizacion autorizacion;
    xRespuestaLector respuestaLector;
    int resultado=0;
    xQueuePeek(configuracionesMaquina, &parametros,10);
    xQueuePeek(configuracionesRemotas, &confTelemetria, 10);
    
    if(bandera==pdFALSE){  
        contadorRetorno = 0;
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_PRINCIPAL);
        }
        peticion.operacion = ACTIVAR_SISTEMAS;
        xQueueOverwrite(respuestaPantalla, (void*)&peticion);
        while(!xSemaphoreTake(lectorOcupado,100));
        finger_SetLED(pdTRUE);
        xSemaphoreGive(lectorOcupado);
        bandera = pdTRUE;
        if(parametros.bloqueoMaquina==pdTRUE){
            HMI_tx_comando("t","5","txt","BLOQUEADA");
            HMI_visualizar("t5",pdTRUE);
        }
        
    }
    if(parametros.bloqueoMaquina==pdTRUE){
        while(!xSemaphoreTake(lectorOcupado,100));
        finger_SetLED(pdFALSE);
        xSemaphoreGive(lectorOcupado);
    }
    else{
        while(!xSemaphoreTake(lectorOcupado,100));
        resultado = finger_leerHuella();
        xSemaphoreGive(lectorOcupado);
    }
    contadorRetorno++;
    respuesta=HMI_rx_comando();
    if((respuesta.objeto=='B')||(resultado>-2)){
        if(!strcmp(respuesta.buffer,"BLOQUEAR")){
            while(!xSemaphoreTake(eepromOcupada,1));
            EEPROM_escribirBloqueo(pdTRUE);
            xSemaphoreGive(eepromOcupada);
            parametros.bloqueoMaquina = pdTRUE;
            xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
            notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenAlerta,imagenEsperaOFF,5,"Maquina bloqueada","Contacte a","soporte");
            bandera = pdFALSE;
            ventana = PANTALLA_PRINCIPAL;
        }
        if(!strcmp(respuesta.buffer,"CLAVE")){
            bandera = pdFALSE;
            flag=pdFALSE;
            flag2=pdFALSE;
            ventana = PANTALLA_CLAVE;
            while(!xSemaphoreTake(lectorOcupado,100));
            finger_SetLED(pdFALSE);
            xSemaphoreGive(lectorOcupado);
        }
        if(!strcmp(respuesta.buffer,"PINDISPENSAR")){
            bandera = pdFALSE;
            ventana = PANTALLA_DISPENSAR_MANUAL;
        }
        if(resultado>-2){
            while(!xSemaphoreTake(lectorOcupado,100));
            finger_SetLED(pdFALSE);
            xSemaphoreGive(lectorOcupado);
            if(resultado==-1){
                notificacion(pdTRUE,20,imagenError,imagenEsperaOFF,3,"ERROR","HUELLA NO","REGISTRADA");
            }
            else if((resultado>100)&&(resultado<200)){
                //se debe solicitar clave dinamica para ingreso como fabricante
            }
            else if(resultado==255){
                usuario = ADMINISTRADOR;
                bandera = pdFALSE; 
                ventana = PANTALLA_MENU;
            }
            /*else if(xQueueReceive(autorizaciones,(void*)&autorizacion,10)){
                if(autorizacion.identificador==resultado){
                    ventana = PANTALLA_CLAVE_AUTORIZACION;
                    xQueueSendToFront(autorizaciones,(void*)&autorizacion,10);
                }
                else{
                    notificacion(pdTRUE,20,imagenError,imagenEsperaOFF,3,"ERROR","NO HA SIDO","AUTORIZADO");
                }
            }
            else{
                notificacion(pdTRUE,20,imagenError,imagenEsperaOFF,3,"ERROR","NO HAY","AUTORIZACIONES");
            }*/
            bandera = pdFALSE;
        }
    }
    if(contadorRetorno>TIEMPO_RETORNO){
        bandera = pdFALSE;
        ventana = PANTALLA_SALVAPANTALLAS;
    }
    return ventana;
}

uint8 registro_usuario(){
    CYBIT fin = pdFALSE;
    char string[20];
    xRespuestaLector respuestaLector;
    uint8 ventana = PANTALLA_REGISTRO;
    bufferEntradaHMI respuesta;
    int offset=usuarioActivo.idHuella;
    
    if(bandera==pdFALSE){
        while(!bandera){
            bandera=HMI_ventana(VENTANA_REGISTRO);
        }
    }
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            strcpy(usuarioActivo.nombre," ");
            strcpy(usuarioActivo.identificador," ");
            finger_borrarInscripcion(usuarioActivo.idHuella);
            ventana = PANTALLA_MENU_HUELLAS;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            bandera=0;
            EEPROM_escribirNombreUsuario(usuarioActivo.nombre,usuarioActivo.idHuella);
            EEPROM_escribirCedulaUsuario(usuarioActivo.identificador,usuarioActivo.idHuella);
            ventana = PANTALLA_MENU_HUELLAS;
        }
        if(!strcmp(respuesta.buffer,"NOMBRE")){
            bandera=0;
            ventana = PANTALLA_INGRESO_NOMBRE;
        }
        if(!strcmp(respuesta.buffer,"IDENTIFICACION")){
            bandera=0;
            ventana = PANTALLA_INGRESO_CEDULA;
        }
        if(!strcmp(respuesta.buffer,"HUELLA")){
            bandera=0;
            while(!xSemaphoreTake(lectorOcupado,100));
            respuestaLector.aux = 1;
            respuestaLector.id = 0;
            while(!fin){
                switch(usuario){
                    case FABRICANTE:{
                        if((usuarioActivo.idHuella<ID_FABRICANTE)||(usuarioActivo.idHuella>=ID_ADMINISTRADOR)){
                            respuestaLector.aux = 0;
                        }
                    break;}
                    case TEMPORAL:{
                        if((usuarioActivo.idHuella<ID_FABRICANTE)||(usuarioActivo.idHuella>=ID_ADMINISTRADOR)){
                            respuestaLector.aux = 0;
                        }
                    break;}
                    case ADMINISTRADOR:{
                        if((usuarioActivo.idHuella>=ID_FABRICANTE)&&(usuarioActivo.idHuella<ID_ADMINISTRADOR)){
                            respuestaLector.aux = 0;
                        }
                        if(usuarioActivo.idHuella>(ID_ADMINISTRADOR+1)){
                            respuestaLector.aux = 0;
                        }
                    break;}
                }
                switch(respuestaLector.aux){
                    case 1:{   
                        respuestaLector = finger_registroHuella(respuestaLector.aux,respuestaLector.id,offset);
                        usuarioActivo.idHuella = respuestaLector.id;
                        sprintf(string, "ID: %lu\n\r",respuestaLector.id);
                    break;}
                    case 2:{ 
                        notificacion(pdTRUE,1,imagenAlerta,imagenEsperaOFF,3,label_registroHuella1, string, label_vacio);
                    break;}
                    case 3:{
                        notificacion(pdFALSE,1,imagenNull,imagenEsperaON,3,label_registroHuella2, label_registroHuella3, label_vacio);
                    break;}
                    case 5:{
                        notificacion(pdFALSE,1,imagenAlerta,imagenEsperaOFF,3,label_registroHuella4, label_registroHuella3, label_vacio);
                    break;}
                    case 6:{
                        notificacion(pdFALSE,1,imagenNull,imagenEsperaON,3,label_registroHuella2, label_registroHuella5,label_registroHuella3);
                    break;}
                    case 8:{
                        notificacion(pdFALSE,1,imagenAlerta,imagenEsperaOFF,3,label_registroHuella4, label_registroHuella5, label_registroHuella3);
                    break;}
                    case 9:{
                        notificacion(pdFALSE,1,imagenNull,imagenEsperaON,3,label_registroHuella2, label_registroHuella6,label_registroHuella3);
                    break;}
                    case 12:{//correcto
                        notificacion(pdTRUE,100,imagenOk,imagenEsperaOFF,3,label_registroHuella9, label_registroHuella10,string);
                        sprintf(string,"%d",usuarioActivo.idHuella);
                        HMI_tx_comando("Registro.t","9","txt",string);
                        usuarioActivo.idHuella = 0;
                        fin = pdTRUE;
                    break;}
                    case 0:{//error
                        notificacion(pdTRUE,100,imagenError,imagenEsperaOFF,3,label_registroHuella7, label_registroHuella8,string);
                        usuarioActivo.idHuella = 0;
                        fin = pdTRUE;
                    break;}
                    }
                }
            xSemaphoreGive(lectorOcupado);
            ventana = PANTALLA_REGISTRO;
        }
    }
    return ventana;  
}

uint8 salvapantallas(){
    bufferEntradaHMI respuesta;
    uint8 ventana = PANTALLA_SALVAPANTALLAS;
    CYBIT comando_ok = pdFALSE;
    xConfiguracion parametros;
    xConfiguracionTelemetria confTelemetria;
    xresPantalla peticion;
    int resultado=0;
    xQueuePeek(configuracionesMaquina, &parametros,10);
    xQueuePeek(configuracionesRemotas, &confTelemetria, 10);
    
    if(bandera==pdFALSE){  
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_SALVAPANTALLAS);
        }
        peticion.operacion = DESACTIVAR_SISTEMAS;
        xQueueOverwrite(respuestaPantalla, (void*)&peticion);
        while(!xSemaphoreTake(lectorOcupado,100));
        finger_SetLED(pdFALSE);
        //finger_entrarDescanso();
        xSemaphoreGive(lectorOcupado);
        bandera = pdTRUE;
    };
    
    respuesta=HMI_rx_comando();
    if((respuesta.objeto=='B')||(resultado>0)){
        if(!strcmp(respuesta.buffer,"SALIR")){
            bandera = pdFALSE;
            ventana = PANTALLA_PRINCIPAL;
            while(!xSemaphoreTake(lectorOcupado,100));
            finger_SetLED(pdTRUE);
            //finger_salirDescanso();
            xSemaphoreGive(lectorOcupado);
        }
    }
    return ventana;
}

uint8 simular_venta(){
    uint8 ventana = PANTALLA_SIMULACION_VENTA;
    uint8 comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    xConfiguracion parametros;
    xConfiguracionBandejas bandejas;
    xCanasta canasta;
    CYBIT primeravez=pdFALSE;
    CYBIT transaccion = pdFALSE;
    int timeOut=0;
    xQueuePeek(configuracionesMaquina, &parametros, 10);
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    
    if(bandera==pdFALSE){
        confGeneral(pdTRUE,"%t0 %b0-0 %b1-1-0 %b2-0 %b3-0",label_simulacion1, label_vacio, label_simulacion2, label_vacio, label_vacio);
        HMI_set_value("limFila",bandejas.configBandejas.numBandejas);
        HMI_set_value("limCol",MOTORES_DOROTI-1);
        HMI_tx_comando("t","1","txt","1");
        HMI_tx_comando("t","2","txt","00");
        HMI_visualizar("b20",pdTRUE);
        HMI_visualizar("b21",pdTRUE);
        HMI_visualizar("b22",pdTRUE);
        HMI_visualizar("b23",pdTRUE);
        HMI_visualizar("b24",pdTRUE);
        HMI_visualizar("t1",pdTRUE);
        HMI_visualizar("t2",pdTRUE);
    }
    
    HMI_get_value("estado.val");
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch(respuesta.buffer[0]){
            case botonMover:
                do{
                    HMI_get_value("va1.txt");
                    respuesta = HMI_rx_comando();
                }while(!((respuesta.objeto==HMI_DATO_TEXTO)&&(respuesta.estado==HMI_CORRECTO)));
                producto = atoi(respuesta.numero);
                sprintf(cadena,"Producto: %d",producto);
                transaccion = pdTRUE;
                while(transaccion){
                    while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                    canasta = Dispensador_leer_canasta();
                    xSemaphoreGive(busOcupado);
                    if(canasta.comando==COMANDO_CORRECTO){
                        if(canasta.canastaCerrada==pdTRUE){
                            vTaskDelay(pdMS_TO_TICKS(500));
                            while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                            if(Dispensador_leer_canasta().canastaCerrada==pdTRUE){
                                transaccion=pdFALSE;
                            }
                            xSemaphoreGive(busOcupado);
                        }
                        else if(primeravez==pdFALSE){
                            primeravez=pdTRUE;
                            notificacion(pdTRUE,0,imagenNull,imagenPuerta,5,label_adver_canasta1,label_adver_canasta2,label_adver_canasta3,label_adver_canasta4,label_adver_canasta5);
                        }
                    }
                }
                notificacion(pdTRUE,0,imagenNull,imagenEsperaON,3,label_simulacion3,label_simulacion4,cadena);
                vTaskDelay(pdMS_TO_TICKS(10));
                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                Dispensador_Modo_Prueba(pdFALSE);
                vTaskDelay(pdMS_TO_TICKS(10));
                comando_ok = Dispensador_dispensar(producto);
                xSemaphoreGive(busOcupado);
                vTaskDelay(pdMS_TO_TICKS(200));
                CYBIT finMovimiento = pdFALSE;
                if(comando_ok==COMANDO_CORRECTO){//se envio la solicitud de movimiento correctamente
                    while(!xSemaphoreTake(busOcupado,( TickType_t )1));
                    comando_ok = Dispensador_leer_motor();
                    xSemaphoreGive(busOcupado);
                    timeOut=0;
                    do{
                        if(xSemaphoreTake(busOcupado,( TickType_t )1)){
                            comando_ok = Dispensador_leer_motor();
                            xSemaphoreGive(busOcupado);
                        }
                        switch(comando_ok){
                            case COMANDO_CORRECTO:
                                finMovimiento=pdTRUE;
                            break;
                            case COMANDO_NO_CAYO:
                                finMovimiento=pdTRUE;
                            break;
                            case COMANDO_ALERTA:
                                finMovimiento=pdTRUE;
                            break;
                            case COMANDO_ESPERA:
                                timeOut=0;
                            break;
                        }
                        vTaskDelay(pdMS_TO_TICKS(10));
                        timeOut++;
                    }while((!finMovimiento)&&(timeOut<500));
                    if(timeOut>=500){
                        comando_ok = COMUNICACION_FALLIDA;
                    }
                }
                switch(comando_ok){
                    case COMANDO_CORRECTO:
                        notificacion(pdFALSE,T_NOTIFICACIONES,imagenOk,imagenEsperaOFF,5,label_entregado1,label_entregado2,label_vacio,label_entregado3,label_entregado4);
                    break;
                    case COMANDO_NO_CAYO:
                        notificacion(pdFALSE,T_NOTIFICACIONES,imagenError,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega4,label_error_entrega5);
                    break;
                    case COMANDO_ALERTA:
                        notificacion(pdFALSE,T_NOTIFICACIONES,imagenAlerta,imagenEsperaOFF,5,label_entregado1,label_entregado2,label_vacio,label_entregado3,label_entregado4);
                    break;
                    case COMUNICACION_FALLIDA:
                        notificacion(pdFALSE,30,imagenAlerta,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega6,label_vacio);
                    break;
                    case ERROR_PRODUCTO:
                        notificacion(pdFALSE,30,imagenError,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega6,label_vacio);
                    break;
                    default:
                        notificacion(pdFALSE,30,imagenError,imagenEsperaOFF,5,label_error_entrega1,label_error_entrega2,label_error_entrega3,label_error_entrega6,label_vacio);
                    break;    
                }
                bandera = pdFALSE;
                ventana = PANTALLA_SIMULACION_VENTA;
            break;
            case botonVolver:
                bandera=pdFALSE;
                ventana = PANTALLA_MENU_OPERATIVO;
            break;
        }
    }
    return ventana;
}

uint8 sistemaOperativo(){
    uint8 ventana = PANTALLA_RTOS;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    TaskHandle_t xHandle = NULL;
    char objeto[30], celdas[4];
    int fila = 2, numTareas = 0;
    
    /*En esta pantalla se muesta información importante del sistema operativo
    como lo es la cantidad de tareas en ejecución, memoria libre de cada una de ellas*/
    if(bandera==pdFALSE){
        HMI_tx_comando("Depuracion.t","0","txt",label_sistemaOperativo1);
        HMI_tx_comando("Depuracion.tA","1","txt",label_sistemaOperativo2);
        HMI_tx_comando("Depuracion.tB","1","txt",label_sistemaOperativo3);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_SISTEMA_OPER);
        }
        bandera = pdTRUE;
    }
    //Se muestra la cantidad de tareas que existen, debe tenerse en cuenta que la
    //tarea Idle se cuenta, por eso se resta 1
    numTareas = uxTaskGetNumberOfTasks()-1;
    sprintf(objeto,"%d",numTareas);
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt","Tareas Ejecutando");
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    //memoria ocupada por tarea general
    xHandle = xTaskGetHandle("tarea general");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","General");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea interface
    xHandle = xTaskGetHandle("interface");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Interface");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea Modulos de pago
    xHandle = xTaskGetHandle("Sistemas de pago");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Sistemas Pago");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea Modulos de pago
    xHandle = xTaskGetHandle("tarea SD");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Sistema Archivos");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea LED
    xHandle = xTaskGetHandle("Led Blink");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Led RTOS");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea Puerto USB
    xHandle = xTaskGetHandle("Estado Puerto USB");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Puerto USB");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea comunicacion USB
    xHandle = xTaskGetHandle("Comunicacion USB");
    if(xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Comunicacion USB");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea comunicacion Bluetooth
    xHandle = xTaskGetHandle("Bluetooth");
    if (xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Bluetooth");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    //memoria ocupada por la tarea comunicacion Telemetria
    xHandle = xTaskGetHandle("Telemetria");
    if (xHandle != NULL){
        sprintf(objeto,"%lu",uxTaskGetStackHighWaterMark(xHandle));
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt","Telemetria");
        HMI_tx_comando("tB",celdas,"txt",objeto);
        fila++;
    }
    
    sprintf(objeto,"%d",xPortGetFreeHeapSize());
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt","Heap Libre");
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    sprintf(objeto,"%d",xPortGetMinimumEverFreeHeapSize());
    sprintf(celdas,"%d",fila);
    HMI_tx_comando("tA",celdas,"txt","Heap Libre (minima)");
    HMI_tx_comando("tB",celdas,"txt",objeto);
    fila++;
    
    while(fila!=14){
        sprintf(celdas,"%d",fila);
        HMI_tx_comando("tA",celdas,"txt",label_vacio);
        HMI_tx_comando("tB",celdas,"txt",label_vacio);
        fila++;
    }
    
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_DIAGNOSTICO;
        }
    }
    return ventana;
}

uint8 sistemasPago(){
    uint8 ventana = PANTALLA_SISTEMAS_PAGO;
    bufferEntradaHMI respuesta;
    xcomandoHMI funcionHMI;
    xConfiguracion parametros;
    
    xQueuePeek(configuracionesMaquina,&parametros,10);
    if(bandera==0){
        confGeneral(pdTRUE,"%t0 %b0-1 %b1-1 %b2-1",label_sistemas_pago1, label_sistemas_pago2, label_sistemas_pago3, label_sistemas_pago4);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    HMI_get_value("estado.val");//verifica si un boton fue presionado
    respuesta = HMI_rx_comando();
    if((respuesta.objeto==HMI_DATO_NUMERICO)&&(respuesta.estado==HMI_CORRECTO)){
        switch (respuesta.buffer[0]){
            case boton1:{
                bandera = 0;
                funcionHMI.comando = MDB_EKIA_RESETEO;
                xQueueSend(comandosHMI, &funcionHMI, 10);
                break;
            }
            case boton2:{
                bandera=0;
                ventana = PANTALLA_BILLETERO;
                break;
            }
            case boton3:{
                bandera=0;
                ventana = PANTALLA_MONEDERO;
                break;
            }
            case botonVolver:{
                bandera=0;
                ventana = PANTALLA_CONFIGURACION;
                break;
            }
        }
    }
    xQueueOverwrite(configuracionesMaquina,(void*)&parametros);
    return ventana;
}

uint8 temperatura(){
    uint8 ventana = PANTALLA_TEMPERATURA;
    CYBIT comando_ok = pdFALSE;
    int entero;
    int i = 0;
    bufferEntradaHMI respuesta;
    xConfiguracionTemperatura parametros;
    xMedidaDispensador temperatura[3];
    xQueuePeek(configuracionesTemperatura,&parametros,10);
    if(bandera==pdFALSE){
        HMI_visualizar("Temperatura.p1",parametros.refrigeracion.descanso);
        HMI_set_value("Temperatura.vtemp",parametros.refrigeracion.gradosC);
        while(!comando_ok){
            comando_ok=HMI_ventana(VENTANA_TEMPERATURA);
        }
        HMI_set_value("bt0",parametros.refrigeracion.activado);
        bandera = pdTRUE;
    }
    while(!xSemaphoreTake(busOcupado,10));
    for(i=0; i<3; i++){
        temperatura[i] = Dispensador_LeerTemperatura(i);
    }
    xSemaphoreGive(busOcupado);
    entero = temperatura[0].medida*10;
    HMI_set_value("rtemp",entero);
    HMI_visualizar("p1",parametros.refrigeracion.descanso);
    respuesta = HMI_rx_comando();
    if(respuesta.objeto=='B'){
        if(!strcmp(respuesta.buffer,"VOLVER")){
            bandera=0;
            ventana = PANTALLA_CONFIGURACION;
        }
        if(!strcmp(respuesta.buffer,"GUARDAR")){
            //solicita la informacion de temperatura configurada
            do{
                HMI_get_value("vtemp.val");
                respuesta = HMI_rx_comando();
                vTaskDelay(pdMS_TO_TICKS(1));
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            parametros.refrigeracion.gradosC = (respuesta.buffer[1]<<8)|respuesta.buffer[0];
            
            //solicita el estado de la refrigeracion
            do{
                HMI_get_value("bt0.val");
                respuesta = HMI_rx_comando();
                vTaskDelay(pdMS_TO_TICKS(1));
            }while((respuesta.objeto!=HMI_DATO_NUMERICO)||(respuesta.estado!=HMI_CORRECTO));
            parametros.refrigeracion.activado = respuesta.buffer[0];   
            
            while(!xSemaphoreTake(memoriaOcupada,100));
            escribirRefrigeracion(parametros.refrigeracion);
            parametros.refrigeracion=leerRefrigeracion();
            HMI_set_value("vtemp",parametros.refrigeracion.gradosC);
            HMI_set_value("bt0",parametros.refrigeracion.activado);
            xSemaphoreGive(memoriaOcupada);
            
            while(!xSemaphoreTake(busOcupado,10));
            Dispensador_definir_temperatura(parametros.refrigeracion.gradosC);
            vTaskDelay(pdMS_TO_TICKS(10));
            xSemaphoreGive(busOcupado);
            xQueueOverwrite(configuracionesTemperatura,&parametros);
            ventana = PANTALLA_TEMPERATURA;
        }
        if(!strcmp(respuesta.buffer,"CONFIG")){
            bandera=pdFALSE;
            ventana = PANTALLA_CONFIGURACION_TEMP;
        }
    }
    return ventana;
} 

uint8 verRegistro(){
    uint8 ventana = PANTALLA_MENU_REGISTRO;
    CYBIT comando_ok = pdFALSE;
    bufferEntradaHMI respuesta;
    uint8 memoriaSD;
    
    xQueuePeek(estadoMemoriaSD, &memoriaSD, 1);
    if(memoriaSD==LISTO){
        if(bandera==pdFALSE){
            offset = 0;
            while(!comando_ok){
                comando_ok=HMI_ventana(VENTANA_CONTABILIDAD);
            }
            graficarRegistroDiaria();
            bandera = pdTRUE;
        }
        respuesta = HMI_rx_comando();
        if(respuesta.objeto=='B'){
            if(!strcmp(respuesta.buffer,"VOLVER")){
                bandera=0;
                ventana = PANTALLA_MENU_CONT_EST;
            }
            if(!strcmp(respuesta.buffer,"ARRIBA")){
                offset -= SALTOS;
                if(offset<0){
                    offset=0;
                }
                graficarRegistroDiaria();
                ventana = PANTALLA_MENU_REGISTRO;
            }
            if(!strcmp(respuesta.buffer,"ABAJO")){
                offset+=SALTOS;
                graficarRegistroDiaria();
                ventana = PANTALLA_MENU_REGISTRO;
            }
        }
    }
    else{
        notificacion(pdTRUE,TIEMPO_NOTIFICACION,imagenAlerta,imagenEsperaOFF,3,label_nota_memoriaSD1,label_nota_memoriaSD2,label_nota_memoriaSD3);
        ventana = PANTALLA_MENU_CONT_EST;
    }
    return ventana;
}


/*******************************************************************************
* TASK PRINCIPAL DEL HMI
********************************************************************************
* esta tarea verifica en todo momento el estado del sistema para navegar por las
* ventanas respectivas para cada caso
*******************************************************************************/
void interface(void* pvParameters){ 
    (void)pvParameters;
    bandera = 0;
    uint8 notificacion = pdFALSE;
    ADC_Vending_Start();
    ADC_Vending_StartConvert();
    HMI_Start();
    HMI_setBaud(HMI_BAUDIOS_HMI);
    HMI_setBaud_default(HMI_BAUDIOS_HMI);
    vTaskDelay(pdMS_TO_TICKS(10));
    uint8 pantalla=PANTALLA_BIENVENIDA;
    for(;;){
        notificacion = pdFALSE;
        xQueueReceive(notificaciones,(void*)&notificacion,1);
        brillo();
        //potencia();
        /*if(tiempo>=TIEMPO_REFRESCO){
            temperaturaEspera();
            tiempo=0;
        }*/
        if(HMI_leerReinicio()==pdTRUE){
            bandera=pdFALSE;
        }
        switch(pantalla){
            case PANTALLA_BORRAR_HUELLA_ID:
                pantalla = borrarHuella_id();
            break;
            case PANTALLA_BIENVENIDA:
                pantalla = bienvenida();
            break;
            case PANTALLA_BILLETERO:
                pantalla = config_Billetero();
            break;    
            case PANTALLA_BILLETERO_ESTADO:
                pantalla = billeteroEstado();
            break;    
            case PANTALLA_BILLETERO_INFO:
                pantalla = informacionBilletero();
            break; 
            case PANTALLA_BILLETES_ACEPTADOS:
                pantalla = billetesAceptados();
            break;
            case PANTALLA_BILLETES_RECICLADOS:
                pantalla = billetesReciclados();
            break;     
            case PANTALLA_BILLETES_RETENIDOS:
                pantalla = billetesRetenidos();
            break;   
            case PANTALLA_BRILLO_PANTALLA:
                pantalla = pantallaBrillo();
            break;       
            case PANTALLA_CLAVE:
                pantalla = claveIngreso();
            break; 
            case PANTALLA_CLAVE_AUTORIZACION:
                pantalla = claveAutorizacion();
            break;     
            case PANTALLA_CAMBIAR_SERIE:
                pantalla = serieCambiar();
            break;    
            case PANTALLA_CONF_CONECTIVIDAD:
                pantalla = config_Conectividad();
            break;    
            case PANTALLA_CONF_MEMORIA:
                pantalla = menu_memoria();
            break;      
            case PANTALLA_CONFIGURACION:
                pantalla = configuracion();
            break; 
            //case PANTALLA_CONFIG_USUARIOS:
            //    pantalla = menu_usuarios();
            //break; 
            case PANTALLA_CONFIGURACION_TEMP:
                pantalla = config_temp();
            break;      
            case PANTALLA_CONFIR_BORRADO_HUELLA:
                pantalla = confirmar_borradoHuella();
            break;    
            case PANTALLA_CONFIRMAR_COPIA:
                pantalla = confirmar_copia();
            break;    
            case PANTALLA_CONFIRMAR_FORMATEO:
                pantalla = confirmar_formateo();
            break;    
            case PANTALLA_CONFIRMAR_REST:
                pantalla = confirmar_restauracion();
            break;                       
            case PANTALLA_CONTABILIDAD:
                pantalla = contabilidad();
            break;  
            case PANTALLA_DIAGNOSTICO:
                pantalla = menuDiagnostico();
            break;
            case PANTALLA_DISPENSAR_MANUAL:
                pantalla = dispensarManual();
            break;
            case PANTALLA_ESPACIO:
                pantalla = espacio();
            break; 
            case PANTALLA_ILUMINACION:
                pantalla = menu_iluminacion();
            break;    
            case PANTALLA_INFORMACION:
                pantalla = informacion();
            break; 
            case PANTALLA_INFO_GSM:
                pantalla = informacionGSM();
            break;
            case PANTALLA_INGRESO_ALTURA:
                pantalla = ingresoAltura();
            break;    
            case PANTALLA_INGRESO_CANTIDAD:
                pantalla = ingresoCantidad();
            break; 
            case PANTALLA_INGRESO_NOMBRE:
                pantalla = ingresoNombre();
            break; 
            case PANTALLA_INGRESO_CEDULA:
                pantalla = ingresoCedula();
            break;      
            case PANTALLA_INGRESO_PRECIO:
                pantalla = ingresoPrecio();
            break;    
            case PANTALLA_LUZ_MAQUINA:
                pantalla = luz_maquina();
            break;     
            case PANTALLA_MENU:
                pantalla = menu();
            break; 
            case PANTALLA_MENU_CONECTIVIDAD:
                pantalla = menu_conectividad();
            break;     
            case PANTALLA_MENU_CONTABILIDAD:
                pantalla = menu_contabilidad();
            break;      
            case PANTALLA_MENU_CONT_EST:
                pantalla = menu_cont_estad();
            break;     
            case PANTALLA_MENU_ESTADISTICA:
                pantalla = menu_estadistica();
            break; 
            case PANTALLA_MENU_HUELLAS:
                pantalla = menu_huellas();
            break;    
            case PANTALLA_MENU_REGISTRO:
                pantalla = verRegistro();
            break;    
            case PANTALLA_MONEDERO:
                pantalla = config_Monedero();
            break;
            case PANTALLA_MONEDERO_ESTADO:
                pantalla = monederoEstado();
            break;     
            case PANTALLA_MONEDERO_INFO:
                pantalla = informacionMonedero();
            break;  
            case PANTALLA_PESO:
                pantalla = peso();
            break;    
            case PANTALLA_PRINCIPAL:
                pantalla = principal();
            break;
            case PANTALLA_RECICLADOR:
                pantalla = menu_reciclador();
            break; 
            case PANTALLA_REGISTRO:
                pantalla = registro_usuario();
            break;     
            case PANTALLA_RTOS:
                pantalla = sistemaOperativo();
            break;
            case PANTALLA_TELEMETRIA:
                pantalla = menu_telemetria();
            break;  
            case PANTALLA_SALVAPANTALLAS:
                pantalla = salvapantallas();
            break;    
            case PANTALLA_SISTEMAS_PAGO:
                pantalla = sistemasPago();
            break;
            
            
            
            
            
            
             
            case PANTALLA_CONFIG_PRODUCTO:
                pantalla = config_producto();
            break;    
            case PANTALLA_MENU_MOTORES:
                pantalla = config_motores();
            break;
            case PANTALLA_MENU_OPERATIVO:
                pantalla = menu_operativo();
            break;    
            case PANTALLA_CAMBIAR_CLAVE:
                pantalla = claveCambiar();
            break;      
            case PANTALLA_FECHAYHORA:
                pantalla = fecha_hora();
            break; 
            case PANTALLA_TEMPERATURA:
                pantalla = temperatura();
            break;              
            case PANTALLA_CONFIG_BANDEJAS:
                pantalla = config_bandejas();
            break; 
            case PANTALLA_CONFIRMAR_CLAVE:
                pantalla = confirmar_clave();
            break;
            case PANTALLA_CONFIRMAR_PRODUCTO:
                pantalla = confirmar_productos();
            break;
            case PANTALLA_CONFIRMAR_BANDEJA:
                pantalla = confirmar_bandeja();
            break; 
            case PANTALLA_CONFIRMAR_SALIDA:
                pantalla = confirmar_salida();
            break;   
            case PANTALLA_CANCEL_PRODUCTO:
                pantalla = cancelar_productos();
            break;   
            case PANTALLA_SIMULACION_VENTA:
                pantalla = simular_venta();
            break;
            case PANTALLA_MENU_CONFGSM:
                pantalla = menu_confGSM();
            break;  
            default:
                pantalla = bienvenida();
            break;
        }
        //tiempo++;
    }
} 

/*******************************************************************************
* DECLARACIÓN DEL TASK DEL HMI
******************************************************w**************************
*
* Tarea que realiza:
*  Crea el task para manejo del HMI
* 
*******************************************************************************/
void interface_Init(void){
    //Creación de un nuevo task
    if(xTaskCreate(interface, //puntero de la función que crea la tarea (nombre de la tarea)
                   "interface", //nombre textual de la tarea, solo útil para depuración
                   memInterface, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorInterface, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] FIN DE ARCHIVO */
