 /* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * DOROTI 1.0
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

#include "Clock_lento.h"
//#include "LED_DEBUG.h"
#include "RTC_RTC.h"
#include "RTC_EKIA.h"

#include "colas.h"
#include "tipos.h"
#include "semaforos.h"
#include "memEEPROM.h"
#include "general.h"
#include "I2C.h"
#include "dispensador.h"
#include "MDB_Tipos.h"
#include "Telemetria.h"
#include "lectorHuella.h"

uint8 contadorHoras = 0;
uint8 contadorDescanso = 0;
uint8 minutos = 0;
CYBIT descansoCompresor = pdFALSE;

void RTC_RTC_EveryMinuteHandler_Callback(void){
    xConfiguracion parametros;
    xConfiguracionTemperatura parametrosTemp;
    xEventos evento;
    xQueuePeekFromISR(configuracionesMaquina, (void*)&parametros);
    xQueuePeekFromISR(configuracionesTemperatura,(void*)&parametrosTemp);
    minutos++;
    contadorDescanso++;
    if(minutos==60){
        minutos = 0;
        contadorHoras++;
    }
    if((minutos==0)||(minutos==30)){
        while(!xSemaphoreTakeFromISR(eepromOcupada,NULL));
        escribirHoras(1);
        xSemaphoreGiveFromISR(eepromOcupada,NULL);
    }
    
    if((contadorDescanso>=parametrosTemp.MinutosDescanso)&&(descansoCompresor == pdTRUE)){
        parametrosTemp.refrigeracion.descanso = pdFALSE;
        escribirRefrigeracion(parametrosTemp.refrigeracion);
        contadorHoras = 0;
        contadorDescanso = 0;
        descansoCompresor = pdFALSE;
        strcpy(evento.tipo,tipo_anuncio);
        strcpy(evento.evento,text_registro_descanso_out);
        evento.operacion = ACTUALIZAR_REGISTRO; 
        xQueueSendToBackFromISR(actualizarRegistro,(void*)&evento,NULL);
        parametrosTemp.refrigeracion.descanso = descansoCompresor;
        xQueueOverwriteFromISR(configuracionesMaquina, (void*)&parametros, NULL);
    }
    if(contadorHoras >= parametrosTemp.HorasDescanso){
        parametrosTemp.refrigeracion.descanso = pdTRUE;
        escribirRefrigeracion(parametrosTemp.refrigeracion);
        contadorHoras = 0;
        contadorDescanso = 0;
        descansoCompresor = pdTRUE;
        strcpy(evento.tipo,tipo_anuncio);
        strcpy(evento.evento,text_registro_descanso_in);
        evento.operacion = ACTUALIZAR_REGISTRO; 
        xQueueSendToBackFromISR(actualizarRegistro,(void*)&evento,NULL);
        parametrosTemp.refrigeracion.descanso = descansoCompresor;
        xQueueOverwriteFromISR(configuracionesMaquina, (void*)&parametros, NULL);
    }
}

void configurarDispensador(){
    uint8 comando_ok=0;
    uint8 paso = 1, fin = pdFALSE;
    xConfiguracion configuraciones;
    xConfiguracionTemperatura configuracionesTemp;
    xConfiguracionBandejas confBandejas;
    xQueuePeek(configuracionesMaquina, &configuraciones, 10);
    xQueuePeek(configuracionesBandejas, &confBandejas, 10);
    xQueuePeek(configuracionesTemperatura,(void*)&configuracionesTemp,10);
    if(configuraciones.BrilloMaquina.activo){
        comando_ok=Dispensador_definir_brillo(configuraciones.BrilloMaquina.nivel+155);
    }
    else{
        comando_ok=Dispensador_definir_brillo(0);
    }
    while(!fin){
        switch(paso){
            case 1:{
                if(Dispensador_Refrigerador_write(configuracionesTemp.refrigeracion.activado)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 2:{
                if(Dispensador_definir_temperatura(configuracionesTemp.refrigeracion.gradosC)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 3:{ 
                if(Dispensador_definir_rango_inferior(configuracionesTemp.HisteresisInfer)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 4:{ 
                if(Dispensador_definir_rango_superior(configuracionesTemp.HisteresisSuper)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 5:{ 
                if(Dispensador_definir_peso_minimo(configuraciones.PesoMinimo)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 6:{ 
                if(Dispensador_definir_Tiempos(confBandejas.tiemposBandeja, confBandejas.parametrosMotores.tiempoMovimiento, confBandejas.parametrosMotores.cicloUtil)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 7:{ 
                if(Dispensador_definir_escala_peso(configuraciones.escalaPeso)==COMANDO_CORRECTO)
                    paso++;
                else
                    paso=0;
            break;}
            case 8:{ 
                if(Dispensador_definir_estado(DISPENSADOR_ESPERA)==COMANDO_CORRECTO)
                    fin = pdTRUE;
                else
                    paso=0;
            break;}
            default:{ 
                fin = pdTRUE;
            break;}
        }
    }
    return;
}

void cargaConfiguraciones(){
    xConfiguracion configuraciones;
    xConfiguracionTemperatura configuracionesTemp;
    xConfiguracionTelemetria confTelemetria;
    xConfiguracionAccesorios confAccesorios;
    xConfiguracionBandejas confBandejas;
    xSistemasPago confSistemasPago;
    while(!xSemaphoreTake(eepromOcupada,1));
    escribirSerie();
    //se cargan los parémetros de configuración de la máquina
    configuraciones.estadoMaquina = NORMAL;
    configuraciones.Serie = leerSerie();
    configuraciones.BrilloPantalla = leerBrilloPantalla();
    configuraciones.BrilloMaquina = leerBrilloMaquina();
    configuraciones.escalaPeso = leerEscalaPeso();
    configuraciones.bloqueoMaquina = EEPROM_leerBloqueo();
    //configuracionesTemp.escalaTemperatura = leerEscalaTemperatura();
    configuracionesTemp.refrigeracion = leerRefrigeracion();
    configuracionesTemp.HorasDescanso = EEPROM_leerHorasDescanso();
    configuracionesTemp.MinutosDescanso =EEPROM_leerMinutosDescanso();
    configuracionesTemp.HisteresisSuper =EEPROM_leerHisterSuper();
    configuracionesTemp.HisteresisInfer =EEPROM_leerHisterInfer();
    configuraciones.PesoMinimo =EEPROM_leerPesoMinimo();
    confSistemasPago.billetesAceptados = EEPROM_leerBilletesAceptados();
    confSistemasPago.billetesReciclados = EEPROM_leerBilletesReciclados();
    confSistemasPago.recicladorHabilitado = EEPROM_leerHabilitacionReciclador();
    //confSistemasPago.factorEscalaBillete = EEPROM_leerFactorBilletero();
    confBandejas.parametrosMotores = EEPROM_leerTiempos(confBandejas.tiemposBandeja);
    confBandejas.configBandejas = EEPROM_leerBandejas();
    confTelemetria.existenciaTelemetria = leerExistenciaTelemetria();
    confTelemetria.estadoTelemetria = leerEstadoTelemetria();
    xSemaphoreGive(eepromOcupada);
    xQueueOverwrite(configuracionesMaquina, (void*)&configuraciones);
    xQueueOverwrite(configuracionesRemotas, (void*)&confTelemetria);
    xQueueOverwrite(configuracionesBandejas,(void*)&confBandejas);
    xQueueOverwrite(configuracionSistemasPago, (void*)&confSistemasPago);
    xQueueOverwrite(configuracionesAccesorios,(void*)&confAccesorios);
    xQueueOverwrite(configuracionesTemperatura,(void*)&configuracionesTemp);
    configurarDispensador();
    return;
}

static void general(void* pvParameters){ 
    (void)pvParameters;
    CYBIT valor=pdFALSE;
    uint8 estadoCarga, contadorError=0, contador=0;
    uint8 timeOut = 0,estado = 0;
    //xcadenas depuracion; 
    xMedidaDispensador voltaje;
    xConfiguracion parametros;
    xConfiguracionTemperatura parametrosTemp;
    xcomandoUSB func;
    vTaskDelay(pdMS_TO_TICKS(200));
    xQueuePeek(estadoConfiguraciones, &estadoCarga, 10);
    habilitarEEPROM();
    finger_habilitacion();
    habilitar_busPrincipal();
    while(!xSemaphoreTake(busOcupado,1));
    Dispensador_LED_DEBUG_write(pdTRUE);
    cargaConfiguraciones();
    xSemaphoreGive(busOcupado); 
    
    estadoCarga = LISTO;
    xQueueOverwrite(estadoConfiguraciones, (void*)&estadoCarga); 
    //Telemetria_Start();
    while(!xSemaphoreTake(lectorOcupado,1));
    finger_SetLED(pdFALSE);
    finger_open();
    xSemaphoreGive(lectorOcupado);
    for(;;){
        xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
        xQueuePeek(configuracionesTemperatura,(void*)&parametrosTemp,10);
        if(xSemaphoreTake(busOcupado,10)){
            if(verificarConexionEsclavo(DIRECCION_DISPENSADOR)){
                //LED_DEBUG_Write(0);
                if(parametros.BrilloMaquina.activo){
                    Dispensador_definir_brillo(parametros.BrilloMaquina.nivel);
                }
                else{
                    Dispensador_definir_brillo(0);
                }
                if(func.comando == PESO_CERO){
                    Dispensador_setCero_peso();
                    func.comando = ESPERA;
                }
                vTaskDelay(pdMS_TO_TICKS(100));
                estado = Dispensador_leer_estado();
                if(estado==DISPENSADOR_ESPERA){
                    timeOut = 0;
                    valor = !valor;
                    Dispensador_LED_DEBUG_write(valor);
                    vTaskDelay(pdMS_TO_TICKS(100));
                    if(parametrosTemp.refrigeracion.activado==pdTRUE){
                        if(descansoCompresor==pdTRUE){
                            Dispensador_Refrigerador_write(pdFALSE);
                        }
                        else{
                            Dispensador_Refrigerador_write(pdTRUE);
                        }
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                    else{
                        Dispensador_Refrigerador_write(pdFALSE);
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                    voltaje = Dispensador_LeerVoltaje();
                    if(voltaje.medida>=18.0f){
                        xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
                        parametros.estadoMaquina = NORMAL;
                        contadorError = 0;
                        xQueueOverwrite(configuracionesMaquina, (void*)&parametros);
                    }
                    else{
                        contadorError++;
                    }
                    if(contadorError >= 5){
                        xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
                        parametros.estadoMaquina = ERROR_FUSIBLE;
                        xQueueOverwrite(configuracionesMaquina, (void*)&parametros);
                    }
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                else if(estado==DISPENSADOR_NO_LISTO){
                    timeOut = 0;
                    configurarDispensador();
                }
            }
            else{
                //LED_DEBUG_Write(1);
                //timeOut++;
                //reiniciar_busPrincipal();
                if(timeOut >= 10){
                    xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
                    parametros.estadoMaquina = ERROR_FUSIBLE;
                    xQueueOverwrite(configuracionesMaquina, (void*)&parametros);
                }
                if(timeOut >= 50){
                    //CySoftwareReset();
                }
            }
            xSemaphoreGive(busOcupado);
        }
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
void General_Init(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(general, //puntero de la función que crea la tarea (nombre de la tarea)
                   "tarea general", //nombre textual de la tarea, solo útil para depuración
                   memGeneral, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorGeneral, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] FIN DE ARCHIVO */