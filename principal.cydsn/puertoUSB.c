/* ===================================================
 *
 * Copyright EKIA Technologies, 2017
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Tareas de comunicación USB
====================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "semphr.h"
#include "cypins.h"
#include "USBUART_cdc.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"
//#include "LED_DEBUG.h"
#include "LED_USB.h"

#include "MDB.h"
#include "MDB_Tipos.h"
#include "RTC_RTC.h"
#include "RTC_EKIA.h"
#include "dispensador.h"
#include "memoriaExt.h"
#include "memEEPROM.h"
#include "puertoUSB.h"
#include "Telemetria.h"
#include "interface.h"
#include "lectorHuella.h"
//#include "Ultrasonido.h"

TaskHandle_t xComUSB = NULL;
static void transmision_USB(void* pvParameters);

/*******************************************************************************
* FUNCION DE INICIALIZACION DEL USB
********************************************************************************
* Funcion encargada de inicializar y habilitar el puerto USB
*******************************************************************************/
void inicio_usb(){
    int bandera = DESCONECTADO;
    xQueueSend(cUSB, (void*) &bandera, 2);
    USBUART_Start(0, USBUART_DWR_VDDD_OPERATION);
    while(!USBUART_GetConfiguration()){ //se espera a que sea recononcido el dispositivo USB
        LED_USB_Write(pdFALSE); //el LED apagado indica que no ha iniciado la comunicación USB
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    USBUART_CDC_Init();//si el puerto USB se inicaliza correctamente enciende el LED
    bandera = CONECTADO;
    xQueueSend(cUSB, (void*) &bandera, 2);
    LED_USB_Write(pdTRUE);
    //Creación del task de comunicación principal
    if(xTaskCreate(transmision_USB,  
                    "Comunicacion USB", 
                    memUSB, 
                    (void*)NULL, 
                    priorUSB,
                    &xComUSB)!= pdPASS)
            {
                for(;;){} //si llega aquí posiblemente hubo una falta de memoria
            }
}


/*******************************************************************************
* FUNCIONES VARIAS DEL PUERTO USB
********************************************************************************
* funciones varias para el correcto funcionamiento de las tareas USB
*******************************************************************************/
void depuracionUSB(){
    char PC[100];
    xcadenas bufEntrada,bufEntrada2;
    xQueueReceive(cadena, &bufEntrada, 100);
    xQueueReceive(cadena2, &bufEntrada2, 100);
    /*if((bufEntrada.dato1!=0)&&(bufEntrada.dato1!=9)){
        sprintf(PC, "%d,%d,%f\n\r", 
            bufEntrada.dato1, 
            bufEntrada.dato2, 
            bufEntrada.dato7
            //bufEntrada.dato4
        );
        while(!USBUART_CDCIsReady()){
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        USBUART_PutString(PC);
        bufEntrada.dato1 = 0;
        bufEntrada.dato2 = 0; 
        bufEntrada.dato3 = 0;
    }*/
    if(strcmp(bufEntrada2.dato8,"")){
    //sprintf(PC,"Distancia1: %.2f\r\n",
        //Ultrasonido_medirDistancia(4)
    sprintf(PC,"Humedad: %d, Temperatura: %d\r\n",
        bufEntrada2.dato1,
        bufEntrada2.dato2
        //bufEntrada2.dato3,
        //bufEntrada2.dato4
        //bufEntrada2.dato5,
        //bufEntrada2.dato6
        //bufEntrada2.dato8
        ); 
    while(!USBUART_CDCIsReady()){
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    USBUART_PutString(PC);
    strcpy(bufEntrada2.dato8,"");
    }
}

xbufferEntradaUSB USB_verificarComando(){
    xbufferEntradaUSB comandoRecibido;
    int indice = 0;
    int numBytes = 0;
    char* token = 0;
    char* puntero = 0;
    const char delimitadores[6] = {'\r','\n',0xFF,0xFF,0xFF};
    uint8 bufferEntrada[RX_BUF_USB_LENGHT];
    comandoRecibido.finCadena = pdFALSE;
    
    for(indice=0;indice<RX_BUF_USB_LENGHT;indice++){
        comandoRecibido.entrada[indice] = 0;
        bufferEntrada[indice] = 0;
    }
    indice = 0;
    
    do{
        if(USBUART_DataIsReady()){
            numBytes = USBUART_GetAll(bufferEntrada+indice);
            indice += numBytes;
            if(indice>60){
                for(indice=0;indice<RX_BUF_USB_LENGHT;indice++){
                    comandoRecibido.entrada[indice] = 0;
                    bufferEntrada[indice] = 0;
                }
                indice = 0;
            }
            puntero = strchr((char*)bufferEntrada,'\n');
            if(*puntero == '\n'){
                comandoRecibido.finCadena = pdTRUE;
            }
            while(!USBUART_CDCIsReady());
            USBUART_PutString((char*)bufferEntrada+indice-numBytes);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        depuracionUSB();
    }while(comandoRecibido.finCadena != pdTRUE);
    
    token = strtok((char*)bufferEntrada,delimitadores);
    sprintf(comandoRecibido.entrada,"%s",token);
    
    return comandoRecibido;
}

void USB_enviarCadena(char* cadena){
    char string[TX_BUF_USB_LENGHT];
    sprintf(string,"%s",cadena);
    while(!USBUART_CDCIsReady());
    USBUART_PutString(string);
    vTaskDelay(pdMS_TO_TICKS(5));
}

int convertir_entero(char *cadena, uint8 indice) { 
   int potencias[7]={1,10,100,1000,10000,100000,1000000}; 
   int i; 
   int valor = 0; 
   int lon = indice; 

   for(i=lon-1; i>=0; i--) 
      valor += (cadena[ i ]-'0') * potencias[ lon-i-1 ]; 

   return valor; 
}


/*******************************************************************************
* FUNCION DE ENVIO DE DATOS AL COMPUTADOR
********************************************************************************
* Funcion encargada de enviar información de la máquina al PC
*******************************************************************************/
static void transmision_USB (void* pvParameters){ 
    (void)pvParameters;
    int bandera;
    char PC[100], comando=0;
    xcomandoUSB func, funcI2C;
    func.comando = ESPERA;
    xbufferEntradaUSB comandoRecibido;
    int valor = 0, resultado = 0, timeOut = 0;
    xContabilidad contabilidad;
    xProducto productoSolicitado;
    xRespuestaLector respuestaLector;
    TaskHandle_t xHandle = NULL;
    char* token = 0;
    uint8 memoriaSD = pdFALSE;
    const char delimitadores[6] = {'=',';',0xFF,0xFF,0xFF};
    
    xQueueReset(cadena2);
    sprintf(PC, "OK\n\r");
    while(!USBUART_CDCIsReady()){
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    USBUART_PutString(PC);
    for(;;){
        xQueueReceive(cUSB, &bandera, 10);
        while(bandera==CONECTADO){
            xQueueReceive(cUSB, &bandera, 10);
            comandoRecibido=USB_verificarComando();
            if(strcmp(comandoRecibido.entrada,"")){
                token = strtok(comandoRecibido.entrada, delimitadores);
                func.opcional1 = 0;
                func.opcional2 = 0;
                xQueuePeek(estadoMemoriaSD, (void*)&memoriaSD, 10);
                if(!strcmp(comandoRecibido.entrada,"RESET")){
                    func.comando = MDB_EKIA_RESETEO;
                }
                else if(!strcmp(comandoRecibido.entrada,"GUARDAR")){
                    func.comando = MDB_EKIA_GUARDAR_BILL;
                }
                else if(!strcmp(comandoRecibido.entrada,"REGRESAR")){
                    func.comando = MDB_EKIA_DEVOLVER_BILL;
                }
                else if(!strcmp(comandoRecibido.entrada,"ENABLE")){
                    func.comando = MDB_EKIA_ENABLE_BILL;
                }
                else if(!strcmp(comandoRecibido.entrada,"DISABLE")){
                    func.comando = MDB_EKIA_DISABLE_BILL;
                }
                else if(!strcmp(comandoRecibido.entrada,"SETCERO")){
                    funcI2C.comando = PESO_CERO;
                }
                else if(!strcmp(comandoRecibido.entrada,"PAYOUT")||(comando=='d')){
                    if(comando=='d'){
                        valor = atoi(comandoRecibido.entrada);
                        if(valor<9999){
                            func.comando = MDB_EKIA_DISPENSE;
                            func.opcional1 = valor;
                            comando='.';
                        }
                        else{
                            USB_enviarCadena(label_error_valor);
                            USB_enviarCadena(label_monto);
                        }
                    }
                    else{
                        comando='d';
                        USB_enviarCadena(label_monto);
                    }
                }
                else if(!strcmp(comandoRecibido.entrada,"VENTA")||(comando=='e')){
                    if(comando=='e'){
                        valor = atoi(comandoRecibido.entrada);
                        if(valor<999){
                            while(!xSemaphoreTake(eepromOcupada,( TickType_t )1));
                            productoSolicitado = leerProducto(valor);
                            xSemaphoreGive(eepromOcupada);
                            contabilidad.operacion = ACTUALIZAR_CONTABILIDAD;
                            contabilidad.producto = productoSolicitado;
                            xQueueSend(actualizarInformacion,(void*)&contabilidad,10);
                            comando='.';
                        }
                        else{
                            USB_enviarCadena(label_error_producto);
                            USB_enviarCadena(label_productoUSB);
                        }
                    }
                    else{
                        comando='e';
                        USB_enviarCadena(label_productoUSB);
                    }
                }
                else if(!strcmp(comandoRecibido.entrada,"HORAS MAQUINA")||(comando=='f')){
                    if(comando=='f'){
                        valor = atoi(comandoRecibido.entrada);
                        while(!xSemaphoreTake(eepromOcupada,( TickType_t )1));
                        escribirNumeroHoras(valor);
                        xSemaphoreGive(eepromOcupada);
                        comando='.';
                    }
                    else{
                        comando='f';
                        USB_enviarCadena(label_cantidad);
                    }
                }
                else if(!strcmp(comandoRecibido.entrada,"SIM800 ON")){
                    xHandle = xTaskGetHandle("Telemetria");
                    if (xHandle == NULL){
                        Telemetria_Start();
                        USB_enviarCadena(label_iniciandoSIM);
                        while(!xSemaphoreTake(eepromOcupada,1));
                        escribirEstadoTelemetria(pdTRUE);
                        xSemaphoreGive(eepromOcupada);
                    }
                    else{
                        USB_enviarCadena(label_errorIniciandoSIM);
                    }
                }
                else if(!strcmp(comandoRecibido.entrada,"SIM800 OFF")){
                    xHandle = xTaskGetHandle("Telemetria");
                    if (xHandle != NULL){
                        Telemetria_Stop();
                        USB_enviarCadena(label_apagandoSIM);
                        while(!xSemaphoreTake(eepromOcupada,1));
                        escribirEstadoTelemetria(pdFALSE);
                        xSemaphoreGive(eepromOcupada);
                    }
                    else{
                        USB_enviarCadena(label_errorApagandoSIM);
                    }
                }
                else if(!strcmp(comandoRecibido.entrada,"HUELLA NUEVA")){
                    CYBIT fin = pdFALSE;
                    respuestaLector.aux = 1;
                    while(!xSemaphoreTake(lectorOcupado,10));
                    while(!fin){
                        respuestaLector = finger_registroHuella(respuestaLector.aux,respuestaLector.id,0);
                        switch(respuestaLector.aux){
                            case 2:{
                                sprintf(PC, "Registrando ID %lu\n\r",respuestaLector.id);
                            break;}
                            case 3:{
                                sprintf(PC, "Coloque un dedo para registrar la huella\n\r");
                            break;}
                            case 4:{
                                sprintf(PC, "Registrando Plantilla 1\n\r");
                            break;}
                            case 5:{
                                sprintf(PC, "Levante el dedo\n\r");
                            break;}
                            case 6:{
                                sprintf(PC, "Coloque nuevamente el dedo\n\r");
                            break;}
                            case 7:{
                                sprintf(PC, "Registrando Plantilla 2\n\r");
                            break;}
                            case 8:{
                                sprintf(PC, "Levante el dedo\n\r");
                            break;}
                            case 9:{
                                sprintf(PC, "Coloque nuevamente el dedo\n\r");
                            break;}
                            case 12:{
                                sprintf(PC, "Huella %lu Guardada Correctamente\n\r", respuestaLector.id);
                                fin = pdTRUE;
                            break;}
                            case 0:{
                                sprintf(PC, "Error registrando huella: %lu\n\r",respuestaLector.id);
                                fin = pdTRUE;
                            break;}
                        }
                        while(!USBUART_CDCIsReady()){
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                        USBUART_PutString(PC);
                    }
                    xSemaphoreGive(lectorOcupado);
                }
                else if(!strcmp(comandoRecibido.entrada,"HUELLA IN")){
                        resultado = -2;
                        sprintf(PC, "Coloque un dedo para identificar la huella\n\r");
                        while(!xSemaphoreTake(lectorOcupado,10));
                        finger_SetLED(pdTRUE);
                        while(!USBUART_CDCIsReady()){
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                        USBUART_PutString(PC);
                        timeOut = 0;
                        do{
                            resultado = finger_leerHuella();
                            timeOut++;
                            vTaskDelay(pdMS_TO_TICKS(300));
                        }while((resultado < -1)&&(timeOut<10));
                        finger_SetLED(pdFALSE);
                        xSemaphoreGive(lectorOcupado);
                        if(timeOut<10){
                            if(resultado == -1){
                                sprintf(PC, "Huella no se haya registrada\n\r");
                            }
                            else{
                                sprintf(PC, "Huella identificada con el numero: %d\n\r",resultado);
                            }
                        }
                        else{
                            sprintf(PC, "Ningun dedo ha sido detectado\n\r");
                        }
                        while(!USBUART_CDCIsReady()){
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                        USBUART_PutString(PC);
                    }
                else if(!strcmp(comandoRecibido.entrada,"VERIFICAR ID")){
                    token = strtok(NULL, delimitadores);
                    while(!xSemaphoreTake(lectorOcupado,10));
                    if(finger_verificarInscripcion(atoi(token))){
                        sprintf(PC, "Huella inscrita\n\r");
                    }
                    else{
                        sprintf(PC, "Huella no se haya inscrita\n\r");
                    }
                    xSemaphoreGive(lectorOcupado);
                    while(!USBUART_CDCIsReady()){
                        vTaskDelay(pdMS_TO_TICKS(1));
                    }
                    USBUART_PutString(PC);
                }
                else if(!strcmp(comandoRecibido.entrada,"BORRAR ID")){
                    token = strtok(NULL, delimitadores);
                    while(!xSemaphoreTake(lectorOcupado,10));
                    if(finger_borrarInscripcion(atoi(token))){
                        sprintf(PC, "Huella borrada\n\r");
                    }
                    else{
                        sprintf(PC, "Huella no se haya inscrita\n\r");
                    }
                    xSemaphoreGive(lectorOcupado);
                    while(!USBUART_CDCIsReady()){
                        vTaskDelay(pdMS_TO_TICKS(1));
                    }
                    USBUART_PutString(PC);
                }
                else if(!strcmp(comandoRecibido.entrada,"")||(comando=='f')){
                    if(comando=='f'){
                        valor = atoi(comandoRecibido.entrada);
                        while(!xSemaphoreTake(eepromOcupada,( TickType_t )1));
                        escribirNumeroHoras(valor);
                        xSemaphoreGive(eepromOcupada);
                        comando='.';
                    }
                    else{
                        comando='f';
                        USB_enviarCadena(label_cantidad);
                    }
                }
                else {
                    //token = strtok(comandoRecibido.entrada, delimitadores);
                    #if MYZENT==1
                    xEventos evento;    
                    xMedidaDispensador temperatura;  
                    CYBIT transaccion = pdFALSE;  
                    if(!strcmp(comandoRecibido.entrada,"DISPENSAR")){
                        token = strtok(NULL, delimitadores);
                        productoSolicitado.numero = atoi(token);
                        if(productoSolicitado.numero!=0){
                            //lee los parametros del producto solicitado
                            while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
                            productoSolicitado = leerProducto(productoSolicitado.numero);
                            xSemaphoreGive(eepromOcupada);
                            /*transaccion = pdTRUE;
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
                            }*/
                            //si la canasta está cerrada procede a entregar el producto
                            sprintf(PC, "Dispensando, espere por favor...\n\r");
                            while(!USBUART_CDCIsReady()){
                                vTaskDelay(pdMS_TO_TICKS(1));
                            }
                            USBUART_PutString(PC);
                            transaccion = pdTRUE;
                            while(transaccion){
                                //le solicita al dispensador el estado de entrega del producto
                                while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                                Dispensador_Modo_Prueba(pdFALSE);
                                vTaskDelay(pdMS_TO_TICKS(10));
                                comando=Dispensador_dispensar(productoSolicitado.numero);
                                xSemaphoreGive(busOcupado);
                                //indique si la entrega fue satisfactoria
                                if((comando==COMANDO_CORRECTO)||(comando==COMANDO_ALERTA)){
                                    sprintf(PC, "OK: 0\n\r");
                                    if(comando==COMANDO_ALERTA){
                                        sprintf(PC, "OK: 1\n\r");
                                        actualizar_registro(contexto_venta_aviso,productoSolicitado.numero,productoSolicitado.precio,NULL);
                                    }
                                    actualizar_registro(contexto_venta_ok,productoSolicitado.numero,productoSolicitado.precio,NULL);
                                    transaccion = pdFALSE;
                                    contabilidad.operacion = ACTUALIZAR_CONTABILIDAD;
                                    contabilidad.producto = productoSolicitado;
                                    if(memoriaSD == LISTO){
                                        xQueueSend(actualizarInformacion,(void*)&contabilidad,10);
                                    }
                                    productoSolicitado.cantidad--;
                                }
                                //si hubo un error en la entrega
                                if(comando==COMANDO_ERROR){
                                    sprintf(PC, "ERROR: 0\n\r");
                                    actualizar_registro(contexto_error_caida,productoSolicitado.numero,productoSolicitado.precio,NULL);
                                    transaccion = pdFALSE;
                                    productoSolicitado.habilitado = pdFALSE;
                                }
                                if(comando==COMUNICACION_FALLIDA){
                                    sprintf(PC, "ERROR: 1\n\r");
                                    actualizar_registro(contexto_error_comunicacion,0,0,NULL);
                                    transaccion = pdFALSE;
                                    contabilidad.operacion = ACTUALIZAR_CONTABILIDAD;
                                    contabilidad.producto = productoSolicitado;
                                    if(memoriaSD == LISTO){
                                        xQueueSend(actualizarInformacion,(void*)&contabilidad,10);
                                    }
                                    productoSolicitado.cantidad--;
                                }
                            }
                            while(!USBUART_CDCIsReady()){
                                vTaskDelay(pdMS_TO_TICKS(1));
                            }
                            USBUART_PutString(PC);
                            while(!xSemaphoreTake(eepromOcupada,( TickType_t )10));
                            if(productoSolicitado.cantidad==0){
                                productoSolicitado.habilitado = pdFALSE;
                            }
                            escribirProducto(productoSolicitado);
                            xSemaphoreGive(eepromOcupada);
                        }
                    }
                    else if(!strcmp(comandoRecibido.entrada,"ABRIR")){
                        while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                        Dispensador_abrir_puerta(pdTRUE);
                        vTaskDelay(pdMS_TO_TICKS(10));
                        xSemaphoreGive(busOcupado);
                        strcpy(evento.tipo,tipo_anuncio);
                        strcpy(evento.evento,text_registro_puerta_ab);
                        evento.operacion = ACTUALIZAR_REGISTRO; 
                        xQueueSendToBack(actualizarRegistro,&evento,200);
                        sprintf(PC, "OK\n\r");
                        while(!USBUART_CDCIsReady()){
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                        USBUART_PutString(PC);
                    }
                    else if(!strcmp(comandoRecibido.entrada,"CERRAR")){
                        while(!xSemaphoreTake(busOcupado,( TickType_t )10));
                        if(Dispensador_leer_puerta()!=EMERGENCIA){
                            Dispensador_abrir_puerta(pdFALSE);
                            strcpy(evento.tipo,tipo_anuncio);
                            strcpy(evento.evento,text_registro_puerta_ce);
                            evento.operacion = ACTUALIZAR_REGISTRO; 
                            xQueueSendToBack(actualizarRegistro,&evento,200);
                            sprintf(PC, "OK\n\r");
                        }
                        else{
                            sprintf(PC, "ERROR\n\r");
                        }
                        xSemaphoreGive(busOcupado);
                        while(!USBUART_CDCIsReady()){
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                        USBUART_PutString(PC);
                    }
                    else if(!strcmp(comandoRecibido.entrada,"TEMPERATURA")){
                        while(!xSemaphoreTake(busOcupado,10));
                        temperatura = Dispensador_LeerTemperatura();
                        xSemaphoreGive(busOcupado);
                        sprintf(PC, "Temperatura: %.2f\n\r",temperatura.medida);
                        while(!USBUART_CDCIsReady()){
                            vTaskDelay(pdMS_TO_TICKS(1));
                        }
                        USBUART_PutString(PC);
                    }
                    #endif
                }
            }  
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


/*******************************************************************************
* FUNCION DE VERIFICACIÓN DE ESTADO Y RECEPCIÓN DE COMANDOS DEL PC
********************************************************************************
* Funcion encargada de recibir e interpretar comandos enviados desde el PC
*******************************************************************************/
static void estado_USB (void* pvParameters){ 
    (void)pvParameters;
    int bandera = DESCONECTADO;
    //Ultrasonido_Start();
    
    xQueueSend(cUSB, (void*) &bandera, 2);
    inicio_usb();
    for(;;){
        if(!USBUART_CheckActivity()){ //si llega aquí es porque hubo un cambio en el bus USB
            USBUART_Stop(); //detiene el puerto USB por completo y espera antes de reiniciar
            xComUSB = xTaskGetHandle("Comunicacion USB");
            if(xComUSB != NULL){
                vTaskDelete(xComUSB);
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            inicio_usb(); //reinicia el puerto USB
        }
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

void puertoUSB_Init(void){
    //creación del task de verificación de estado de conexion
    if(xTaskCreate(estado_USB, 
            "Estado Puerto USB", 
            memEstadoUSB, 
            (void*)NULL, 
            priorEstadoUSB,
            (xTaskHandle*)NULL )!= pdPASS)
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}
/* [] END OF FILE */
