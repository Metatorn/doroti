/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2020
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
;RUTINA PRINCIPAL PARA SENSADO Y TOMA DE MEDIDAS
====================================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "tipos.h"
#include "colas.h"
#include "ADC.h"
#include "semaforos.h"

#include "EKIA_CONFIG.h"
#include "math.h"
#include "LED.h"
#include "medidas.h"

#include "DHT21_1.h"
#include "DHT21_2.h"
#include "hx711.h"
#include "entradaEcho.h"
#include "salidaTrigger.h"
#include "Ultrasonido.h"

xAmbiente leerSensorDHT1(){
    xAmbiente medida;
    int16 temperatura = 9999;
    int16 humedad = 9999;
    uint8 IState;
    taskENTER_CRITICAL();
    IState=CyEnterCriticalSection();
    uint8 buffer[5]; 
	uint8 cnt = 7; 
	uint8 idx = 0, i=0; 
    uint8 checksum = 0; 
    int calc=0; 
    int timeout=0; 
    //se limpia el buffer
    for (i=0; i< 5; i++){ 
       buffer[i] = 0; 
    }
    //se inicia la lectura
    DHT21_1_Write(0u);
    CyDelay(19); 
    DHT21_1_Write(1u);
        
    while(DHT21_1_Read()==1){ 
        timeout++; 
        if(timeout>500){ 
            goto error;  //DHT error
        }
        CyDelayUs(1);
    } 
    while(DHT21_1_Read()==0){         
        timeout++; 
        if(timeout>500){ 
            goto error; //DHT error
        }
        CyDelayUs(1);
    } 
    calc=timeout; 
    timeout=0; 
    while(DHT21_1_Read()==1); 
    for (i=0; i<40; i++) 
	{ 
        timeout=0; 
        while(DHT21_1_Read()==0){
        }
        while(DHT21_1_Read()==1){ 
            timeout++;
            CyDelayUs(1);
        }
        //Punto en que se inicia adquisición de datos
        if ((timeout) > (calc/2)){ 
            buffer[idx] |= (1 << cnt);
        }
        if (cnt == 0)   //siguiente byte?
   	    { 
   		    cnt = 7;    // reinicie mas significativo
   		    idx++;      // Ubique siguiente byte
   	    } 
   	    else cnt--; 
    }
    //se calcula checksum para verificar datos
    for (i=0; i< 4; i++){ 
        checksum += buffer[i]; 
    }
    //si el checksum es correcto se valida el dato
    //if(checksum == buffer[4]){
        humedad = (buffer[0]<<8)|buffer[1];  
        temperatura = (buffer[2]<<8)|buffer[3]; 
    //}
    error:  medida.humedad = humedad / 10.0f;
            medida.temperatura = temperatura / 10.0f;
            CyExitCriticalSection(IState);
            taskEXIT_CRITICAL();
            return medida; 
}

xAmbiente leerSensorDHT2(){
    xAmbiente medida;
    int16 temperatura = 9999;
    int16 humedad = 9999;
    uint8 IState;
    IState=CyEnterCriticalSection();
    //taskENTER_CRITICAL();
    uint8 buffer[5]; 
	uint8 cnt = 7; 
	uint8 idx = 0, i=0; 
    uint8 checksum = 0; 
    int calc=0; 
    int timeout=0; 
    //se limpia el buffer
    for (i=0; i< 5; i++){ 
       buffer[i] = 0; 
    }
    //se inicia la lectura
    DHT21_2_Write(0u);
    CyDelay(19); 
    DHT21_2_Write(1u);
        
    while(DHT21_2_Read()==1){ 
        timeout++; 
        if(timeout>500){ 
            goto error1;  //DHT error
        }
        CyDelayUs(1);
    } 
    while(DHT21_2_Read()==0){         
        timeout++; 
        if(timeout>500){ 
            goto error1; //DHT error
        }
        CyDelayUs(1);
    } 
    calc=timeout; 
    timeout=0; 
    while(DHT21_2_Read()==1); 
    for (i=0; i<40; i++) 
	{ 
        timeout=0; 
        while(DHT21_2_Read()==0); 
        while(DHT21_2_Read()==1){ 
            timeout++;
            CyDelayUs(1);
        }
        //Punto en que se inicia adquisición de datos
        if ((timeout) > (calc/2)){ 
            buffer[idx] |= (1 << cnt);
        }
        if (cnt == 0)   //siguiente byte?
   	    { 
   		    cnt = 7;    // reinicie mas significativo
   		    idx++;      // Ubique siguiente byte
   	    } 
   	    else cnt--; 
    }
    //se calcula checksum para verificar datos
    for (i=0; i< 4; i++){ 
        checksum += buffer[i]; 
    }
    //si el checksum es correcto se valida el dato
    if(checksum == buffer[4]){
        humedad = (buffer[0]<<8)|buffer[1];  
        temperatura = (buffer[2]<<8)|buffer[3]; 
    }
    error1:  medida.humedad = humedad / 10.0f;
            medida.temperatura = temperatura / 10.0f;
            //taskEXIT_CRITICAL();
            CyExitCriticalSection(IState);
            return medida; 
}

float medirDistancia(int sensor){
    float distancia = 0.0f;
    uint8 echo = ((sensor-1)/NUMERO_PRODUCTOS_BANDEJA)+1;
    uint8 trigger = ((sensor-1)%NUMERO_PRODUCTOS_BANDEJA)+1;
    entradaEcho_Write(echo);
    salidaTrigger_Write(trigger);
    distancia = Ultrasonido_medirDistancia(4);
    entradaEcho_Write(0);
    salidaTrigger_Write(0);
    return distancia;
}

float medirVoltaje(){
    float voltaje = 0.0f;
    voltaje = ADC_CountsTo_Volts(ADC_GetResult16(0))*5.1302f;
    return voltaje;
}

static void medir(void* pvParameters){ 
    (void)pvParameters;
    int contadorDHT=0;
    float medida=0.0f;
    int i = 0;
    uint8 solicitud;
    xMedidas lecturas;
    ADC_Start();
    ADC_StartConvert();
    Ultrasonido_Start();
    peso_Start();
    definirEscalaPeso(ESCALA);
    xQueueOverwrite(peso,(void*)&medida);
    xQueueReset(medidas);
    for(;;)
    {
        xQueuePeek(medidas,(void*)&lecturas,10);
        lecturas.voltaje = medirVoltaje();
        contadorDHT++;
        if(contadorDHT>=15){
            contadorDHT=0;
            lecturas.ambiente1=leerSensorDHT1();
            lecturas.ambiente2=leerSensorDHT2();
            lecturas.ambienteProm.temperatura =(lecturas.ambiente1.temperatura+lecturas.ambiente2.temperatura)/2.0f;
            lecturas.ambienteProm.humedad=(lecturas.ambiente1.humedad+lecturas.ambiente2.humedad)/2.0f;
        }
        
        if(i>=SENSORES_ULTRASONIDO){
            i=0;
        }
        lecturas.distancia[i] = medirDistancia(i+1);
        i++;
        vTaskDelay(pdMS_TO_TICKS(1));
        
        if(xSemaphoreTake(pesoOcupado,10)){
            if(xQueueReceive(solicitudPeso,(void*)&solicitud,100)){
                if(solicitud==SET_CERO_PESO){
                    setCeroPeso();
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
            lecturas.peso = leerSensorPeso();
            xSemaphoreGive(pesoOcupado);
        }
        xQueueOverwrite(medidas,(void*)&lecturas);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/*******************************************************************************
* DECLARACIÓN DEL TASK DE FUNCIONES GENERALES
********************************************************************************
*
* Tarea que realiza:
*  Crea el task para control de perifericos generales
*
*******************************************************************************/
void mediciones_Init(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(medir, //puntero de la función que crea la tarea (nombre de la tarea)
                   "tarea mediciones", //nombre textual de la tarea, solo útil para depuración
                   memMediciones, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorMediciones, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] END OF FILE */
