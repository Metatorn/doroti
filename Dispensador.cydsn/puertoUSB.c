/* ===================================================
 *
 * Copyright EKIA Technologies, 2020
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * DOROTI 1.0
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

#include "puertoUSB.h"

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
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    USBUART_CDC_Init();//si el puerto USB se inicaliza correctamente enciende el LED
    bandera = CONECTADO;
    xQueueSend(cUSB, (void*) &bandera, 2);
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
    int i =0;
    xMedidas lecturas;
    xcadenas bufEntrada,bufEntrada2;
    //xQueueReceive(cadena, &bufEntrada, 100);
    //xQueueReceive(cadena2, &bufEntrada2, 100);
    xQueuePeek(medidas,(void*)&lecturas,100);

    sprintf(PC,"Humedad 1: %f, Temperatura 1: %f\r\n",
        lecturas.ambiente1.humedad,
        lecturas.ambiente1.temperatura
        ); 
    while(!USBUART_CDCIsReady()){
        CyDelay(1);
    }
    USBUART_PutString(PC);
    
    sprintf(PC,"Humedad 2: %f, Temperatura 2: %f\r\n",
        lecturas.ambiente2.humedad,
        lecturas.ambiente2.temperatura
        ); 
    while(!USBUART_CDCIsReady()){
        CyDelay(1);
    }
    USBUART_PutString(PC);
        
    for(i=0; i<36; i++){
        sprintf(PC,"Distancia %d: %f\r\n",
            i+1,
            lecturas.distancia[i]
            ); 
        while(!USBUART_CDCIsReady()){
            CyDelay(1);
        }
        USBUART_PutString(PC);
    }
    
    sprintf(PC,"Peso: %f\r\n",
        lecturas.peso
        ); 
    while(!USBUART_CDCIsReady()){
        CyDelay(1);
    }
    USBUART_PutString(PC);
    
    sprintf(PC,"Voltaje: %f\r\n",
        lecturas.voltaje
        ); 
    while(!USBUART_CDCIsReady()){
        CyDelay(1);
    }
    USBUART_PutString(PC);
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
    depuracionUSB();
    token = strtok((char*)bufferEntrada,delimitadores);
    sprintf(comandoRecibido.entrada,"%s",token);
    
    return comandoRecibido;
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
    char PC[100], comando;
    xcomandoUSB func, funcI2C;
    func.comando = ESPERA;
    xbufferEntradaUSB comandoRecibido;
    int valor = 0, resultado = 0, timeOut = 0;
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
            }  
        }
        vTaskDelay(pdMS_TO_TICKS(100));
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
