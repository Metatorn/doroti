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
 *
 * ===================================================
; LIBRERIA PARA MANEJO DEL SENSOR HX711
====================================================*/

#include "cytypes.h"
#include "stdio.h"
#include "tipos.h"
#include "sensorPeso.h"
#include "ContadorBitPeso.h" 
#include "Timer.h"
#include "isr_Peso.h"
#include "hx711.h"
#include "Control_Reg.h"
#include "LED.h"
#include "math.h"
#include "colas.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hx711.h"

float escala = ESCALA;
long pesosProm = 0;
float indice = 0;
static float offsetPeso = 0;
long OFFSET = 1500000;
long pesoMedida = 0;
volatile uint8 finCaptura = pdFALSE;
float pesoMinimo = PESO_MINIMO;
static float medida;

CY_ISR(leerPeso){
    uint32 hx711;
    hx711=sensorPeso_ReadData();
    //if(hx711<=0x6FFFFF){
        pesoMedida = hx711-OFFSET;
        pesosProm += pesoMedida;
        indice++;
        if(indice == MUESTRAS){ 
            medida = ((pesosProm/MUESTRAS)*escala)-offsetPeso; 
            pesosProm = 0;
            indice = 0;
        }
        if(medida>=pesoMinimo){
            LED_Write(!pdTRUE);
        }
        else{
            LED_Write(!pdFALSE);
        }
    //}
    isr_Peso_ClearPending();
}

void definirPesoMinimo(float peso){
    pesoMinimo = peso;
}

float leerPesoMinimo(){
    return pesoMinimo;
}

void definirEscalaPeso(float valorEscala){
    escala = 2.0f/valorEscala;
}

int leerEscalaPeso(){
    int valorEscala = 0; 
    valorEscala = 2.0f/escala;
    return valorEscala; 
}

void setCeroPeso(){
    offsetPeso = offsetPeso+medida;
}

void peso_Start(){
    isr_Peso_StartEx(leerPeso);
    sensorPeso_Start();
    ContadorBitPeso_Start();
    Timer_Start();
    wakeUp_Hx711();
    CyDelay(1000);
    hx711_set_channel('A');
    //pesosProm=0;
    //medida=0;
    //calibrarOffset();
    setCeroPeso();
}

void hx711_set_channel(char canal){
    if(canal=='A'){
        ContadorBitPeso_WritePeriod(25);
        ContadorBitPeso_WriteCompare(1);
    }
    if(canal=='B'){
        ContadorBitPeso_WritePeriod(25);
    }
    if(canal=='C'){
        ContadorBitPeso_WritePeriod(26);
    }
    CyDelay(10);
}

void hx711_set_offset(int offset){
    OFFSET = offset;  
}

uint32 hx711_get_offset(){
    return OFFSET;
}

void wakeUp_Hx711(){
    Control_Reg_Write(Control_Reg_Read()|0x03);
    Timer_Wakeup();
    ContadorBitPeso_Wakeup();
    sensorPeso_Wakeup();
}

void sleep_Hx711(){
    float medida = 0;
    Control_Reg_Write(Control_Reg_Read()&0xFE);
    Timer_Sleep();
    ContadorBitPeso_Sleep();
    sensorPeso_Sleep();
    pesosProm = 0;
    indice = 0;
}

float leerSensorPeso(){
    float temporal=0.0f;
    temporal = fabsf(medida);
    return temporal;
}