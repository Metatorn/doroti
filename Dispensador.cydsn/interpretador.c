 /* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * CAFETERA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA ENCARGADO DE INICIALIZAR Y COMANDAR PERIFERICOS
;VARIOS DE LA MAQUINA VENDING
====================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cyapicallbacks.h"
#include "BusPrincipal.h"

#include "tipos.h"
#include "colas.h"
#include "semaforos.h"

#include "EKIA_CONFIG.h"
#include "math.h"
#include "LED_DEBUG.h"
#include "LED.h"
#include "I2C.h"
#include "PWM_LED.h"
#include "interpretador.h"
#include "hx711.h"
#include "llave.h"
#include "Puerta.h"
#include "Sensor_Canasta1.h"
#include "Sensor_Canasta2.h"


uint8 buffer_Lectura [LARGO_BUFFER] = {PAQUETE_INICIO, CMD_NULO, PAQUETE_FIN,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint8 buffer_Escritura [LARGO_BUFFER];
int i=0, j=0;


/*******************************************************************************
* FUNCION DE INTERPRETACION DE COMANDOS
********************************************************************************
* Función que interpreta los comandos enviados por el maestro para ejecutar
* acciones y cambiar configuraciones
*
* Retorna: no tiene valores de retorno, aunque actualiza el buffer de lectura
*          automáticamente, el resultado queda guardado en buffer_Lectura
*******************************************************************************/

void interpretar_comando(){
    int estadoDispensador = 0;
    int temp=0;
    xMedidas lecturas;
    CYBIT sensorPuerta = pdFALSE;
    xConfiguracionTemperatura configuracionTemp;
    xConfiguraciones parametrosMaquina;
    int sensor=0;
    //buffer_Lectura[PAQUETE_ESTADO_POS] = 0;
    //buffer_Lectura[PAQUETE_WR_FIN_POS] = 0;
    xQueuePeek(parametros,(void*)&parametrosMaquina,10);
    xQueuePeek(sistemaStatus,(void*)&estadoDispensador,10);
    switch (buffer_Escritura[PAQUETE_CMD_POS]){
        case ENCENDER_LED_DEBUG:{
            LED_DEBUG_Write(!pdTRUE);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
            
        case APAGAR_LED_DEBUG:{
            LED_DEBUG_Write(!pdFALSE);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case DEFINIR_BRILLO:{
            //PrISM_WritePulse1(buffer_Escritura[PAQUETE_CMD_POS+1]);
            PWM_LED_WriteCompare(buffer_Escritura[PAQUETE_CMD_POS+1]*2);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            CyDelay(3);
        break;}    
        
        case LEER_PESO:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.peso);
        break;}
        
        case DEFINIR_ESCALA_PESO:{
            parametrosMaquina.escala = buffer_Escritura[PAQUETE_CMD_POS+1];
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            xQueueOverwrite(parametros,(void*)&parametrosMaquina);
        break;}  
            
        case LEER_ESCALA_PESO:{
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = parametrosMaquina.escala;
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = PAQUETE_FIN;
        break;} 
            
        case PESO_CERO:{
            while(!xSemaphoreTake(pesoOcupado,100));
            setCeroPeso();
            xSemaphoreGive(pesoOcupado);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case LEER_VOLTAJE:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.voltaje);
        break;}    
        
        /*case LEER_CORRIENTE:
            medida = 1;//leerCorriente
            sprintf((char*)buffer_Lectura,"=C%.6f;",medida);
        break; */
            
        case LEER_TEMPERATURA1:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.ambiente1.temperatura);
        break;}
        
        case LEER_TEMPERATURA2:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.ambiente2.temperatura);
        break;}
        
        case LEER_TEMPERATURA_PROM:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.ambienteProm.temperatura);
        break;}
        
        case LEER_TEMPERATURA_DEFINIDA:{
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            temp = configuracionTemp.gradosC*100;
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = temp>>8;
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = temp & 0xFF;
            buffer_Lectura[PAQUETE_ESTADO_POS+3] = PAQUETE_FIN;
        break;}
        
        case LEER_HUMEDAD1:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.ambiente1.humedad);
        break;}
        
        case LEER_HUMEDAD2:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.ambiente2.humedad);
        break;}
        
        case LEER_HUMEDAD_PROM:{
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.ambienteProm.humedad);
        break;}
        
        case DEFINIR_TIEMPO_BANDEJAS:{
            int i = 0;
            for(i=0;i<NUMERO_BANDEJAS;i++){
                parametrosMaquina.tiempoBandeja[i] = buffer_Escritura[PAQUETE_CMD_POS+(2*i)+1]<<8;
                parametrosMaquina.tiempoBandeja[i] |= buffer_Escritura[PAQUETE_CMD_POS+(2*i)+2];
            }
            parametrosMaquina.tiempoMovimientoMax = buffer_Escritura[PAQUETE_CMD_POS+(2*NUMERO_BANDEJAS)+1]<<8;
            parametrosMaquina.tiempoMovimientoMax |= buffer_Escritura[PAQUETE_CMD_POS+(2*NUMERO_BANDEJAS)+2];
            parametrosMaquina.cicloUtil = buffer_Escritura[PAQUETE_CMD_POS+(2*NUMERO_BANDEJAS)+3]<<8;
            parametrosMaquina.cicloUtil |= buffer_Escritura[PAQUETE_CMD_POS+(2*NUMERO_BANDEJAS)+4];
            xQueueOverwrite(parametros,(void*)&parametrosMaquina);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case MOVER_MOTOR:{
            uint8 motor = buffer_Escritura[PAQUETE_CMD_POS+1];
            uint8 respuesta = 3;
            xQueueOverwrite(respuestaDispensar,(void*)&respuesta);
            if(xQueueSendToBack(solicitudDispensar,(void*)&motor,100)){
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
                buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            }
            else{
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_FALLA;
                buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            }
            CyDelay(200);
        break;}
        
        case LEER_MOTOR:{
            uint8 respuesta=pdFALSE;
            //while(!(BusPrincipal_SlaveStatus() & BusPrincipal_SSTAT_RD_CMPLT)){};
            xQueuePeek(respuestaDispensar,(void*)&respuesta,100);
            switch(respuesta){
                case 0:
                    buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_NO_CAYO;
                break;
                case 1:
                    buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
                break;   
                case 2:
                    buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_ALERTA;
                break;
                case 3:
                    buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_ESPERA;
                break;    
            }
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case ABRIR_SLOT:{
            uint8 slot = buffer_Escritura[PAQUETE_CMD_POS+1];
            if(xQueueSendToBack(solicitudApertura,(void*)&slot,100)){
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
                buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            }
            else{
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_FALLA;
                buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            }
            CyDelay(200);
        break;}
        
        case MODO_NORMAL:{
            xSemaphoreGive(pruebaDesactivada);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
            
        case MODO_PRUEBA:{
            if(xSemaphoreTake(pruebaDesactivada,100)){
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            }
            else{
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_FALLA;
            }
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;} 
        
        case DEFINIR_PESO_MINIMO:{
            parametrosMaquina.pesoMinimo = (buffer_Escritura[PAQUETE_CMD_POS+1]<<8)| buffer_Escritura[PAQUETE_CMD_POS+2] ;
            parametrosMaquina.pesoMinimo = parametrosMaquina.pesoMinimo/100.0f;
            xQueueOverwrite(parametros,(void*)&parametrosMaquina);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}    
            
        case LEER_PESO_MINIMO:{
            int pesoMinimo = parametrosMaquina.pesoMinimo*100.0f;
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = pesoMinimo>>8;
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = pesoMinimo & 0xFF;
            buffer_Lectura[PAQUETE_ESTADO_POS+3] = PAQUETE_FIN;
        break;}
        
        case DEFINIR_TEMPERATURA:{
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            temp = buffer_Escritura[PAQUETE_CMD_POS+1]<<8;
            temp = temp | buffer_Escritura[PAQUETE_CMD_POS+2];
            configuracionTemp.gradosC = temp/100.0f;
            xQueueOverwrite(configuracionesTemperatura,(void*)&configuracionTemp);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case DEFINIR_RANGO_SUPERIOR:{
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            temp = buffer_Escritura[PAQUETE_CMD_POS+1]<<8;
            temp = temp | buffer_Escritura[PAQUETE_CMD_POS+2];
            configuracionTemp.HisteresisSuper = temp/100.0f;
            xQueueOverwrite(configuracionesTemperatura,(void*)&configuracionTemp);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case DEFINIR_RANGO_INFERIOR:{
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            temp = buffer_Escritura[PAQUETE_CMD_POS+1]<<8;
            temp = temp | buffer_Escritura[PAQUETE_CMD_POS+2];
            configuracionTemp.HisteresisInfer = temp/100.0f;
            xQueueOverwrite(configuracionesTemperatura,(void*)&configuracionTemp);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
        break;}
        
        case APAGAR_REFRIGERACION:{
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            configuracionTemp.activado = pdFALSE,
            xQueueOverwrite(configuracionesTemperatura,(void*)&configuracionTemp);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = PAQUETE_FIN;
        break;}
            
        case ENCENDER_REFRIGERACION:{
            xQueuePeek(configuracionesTemperatura,(void*)&configuracionTemp,100);
            configuracionTemp.activado = pdTRUE,
            xQueueOverwrite(configuracionesTemperatura,(void*)&configuracionTemp);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = PAQUETE_FIN;
        break;}
        
        case LEER_ESTADO_PUERTA:{
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            if(llave_Read()==pdTRUE){
                buffer_Lectura[PAQUETE_ESTADO_POS+1] = Puerta_Read();
            }
            else{
                buffer_Lectura[PAQUETE_ESTADO_POS+1] = EMERGENCIA;
            }
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = PAQUETE_FIN;
        break;}
            
        case ABRIR_PUERTA:{
            Puerta_Write(pdTRUE);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = llave_Read();
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = PAQUETE_FIN;
        break;}
            
        case CERRAR_PUERTA:{
            Puerta_Write(pdFALSE); 
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = llave_Read();
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = PAQUETE_FIN;
        break;}
            
        case LEER_ESTADO_CANASTA:{
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            #if(NUMERO_SENSORES==1)
                if(!Sensor_Canasta1_Read()){
                    sensorPuerta = pdTRUE;
                }
                else{
                    sensorPuerta = pdFALSE;
                }
            #endif
            #if(NUMERO_SENSORES==2)
                if((!Sensor_Canasta1_Read())&&(!Sensor_Canasta2_Read())){
                    sensorPuerta = pdTRUE;
                }
                else{
                    sensorPuerta = pdFALSE;
                }
            #endif
            buffer_Lectura[PAQUETE_ESTADO_POS+1] = sensorPuerta;
            buffer_Lectura[PAQUETE_ESTADO_POS+2] = PAQUETE_FIN;
            sensorPuerta = pdFALSE;
            xQueueOverwrite(sensorPuerta1,(void*)&sensorPuerta);
            xQueueOverwrite(sensorPuerta2,(void*)&sensorPuerta);
        break;}
        
        case LEER_DISTANCIA:{
            sensor = buffer_Escritura[PAQUETE_CMD_POS+1];
            xQueuePeek(medidas,(void*)&lecturas,10);
            sprintf((char*)buffer_Lectura,"=C%.2f;",lecturas.distancia[sensor]);
        break;}
            
        case DEFINIR_ESTADO_TARJETA:{
            //LED_OFF;
            uint8 estadoDispensador = buffer_Escritura[PAQUETE_CMD_POS+1];
            xQueueOverwrite(sistemaStatus,(void*)&estadoDispensador);
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            CyDelay(3);
            break;
        }
        
        case LEER_ESTADO_TARJETA:{
            int estadoDispensador = 0;
            if(xQueuePeek(sistemaStatus,(void*)&estadoDispensador,100)){
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_CORRECTO;
                buffer_Lectura[PAQUETE_ESTADO_POS+1] = estadoDispensador;
                buffer_Lectura[PAQUETE_ESTADO_POS+2] = PAQUETE_FIN;
            }
            else{
                buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_FALLA;
                buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            }
            break;
        }         
        
        default:{
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_FALLA;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            break;
        }
    }
}

static void interpretador(void* pvParameters){ 
    (void)pvParameters;
    Puerta_Write(pdFALSE);
    I2C_habilitar_busPrincipal(buffer_Lectura, buffer_Escritura);
    if(llave_Read()==pdTRUE){
        Puerta_Write(pdTRUE);
    }
    for(;;)
    {
        if(I2C_ComandoRecibido(buffer_Escritura)){
            interpretar_comando();
            //I2C_limpiarBuffer();
        }
        //vTaskDelay(pdMS_TO_TICKS(10));
        if (BusPrincipal_SlaveStatus() & BusPrincipal_SSTAT_RD_CMPLT){
            buffer_Lectura[PAQUETE_ESTADO_POS] = CMD_NULO;
            buffer_Lectura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
            BusPrincipal_SlaveClearReadBuf();
            (void) BusPrincipal_SlaveClearReadStatus();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
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
void interpretador_Init(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(interpretador, //puntero de la función que crea la tarea (nombre de la tarea)
                   "tarea general", //nombre textual de la tarea, solo útil para depuración
                   memInterpretador, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorInterpretador, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] FIN DE ARCHIVO */
