/* ===================================================
 *
 * Copyright EKIA Technology S.A.S., 2020
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================*/

#include <project.h>

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "tipos.h"
#include "colas.h"
#include "puertoUSB.h"
#include "interpretador.h"
#include "medidas.h"
#include "monitor.h"
#include "controles.h"
#include "semaforos.h"

// Declaración de vector base NVIC para el manejo de excepciones del FreeRTOS
#define CORTEX_INTERRUPT_BASE (16)

// Declaraciones de los manipuladores de excepciones del FreeRTOS
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void vPortSVCHandler(void);

int main(void)
{   
    LED_Write(!0);
    // Controlador de llamadas de supervisión para Cortex (SVC, formerly SWI) - dirección 11 
    CyIntSetSysVector( CORTEX_INTERRUPT_BASE + SVCall_IRQn,
    (cyisraddress)vPortSVCHandler );
    // Controlador de llamada PendSV para procesador Cortex - dirección 14
    CyIntSetSysVector( CORTEX_INTERRUPT_BASE + PendSV_IRQn,
    (cyisraddress)xPortPendSVHandler );
    // Controlador del SYSTICK para procesadores Cortex - dirección 15
    CyIntSetSysVector( CORTEX_INTERRUPT_BASE + SysTick_IRQn,
    (cyisraddress)xPortSysTickHandler );
    CyDelay(TIEMPO_ARRANQUE);
    CyGlobalIntEnable;
    RTC_RTC_Start();
    RTC_RTC_EnableInt();  
    RTC_RTC_WriteIntervalMask(RTC_RTC_INTERVAL_MIN_MASK | RTC_RTC_INTERVAL_HOUR_MASK);
    colas_Init();
    semaforos_Init();
    interpretador_Init();
    mediciones_Init();
    monitoreo_Init();
    control_Init(); 
    puertoUSB_Init();
    int estado = DISPENSADOR_NO_LISTO;
    xQueueOverwrite(sistemaStatus,(void*)&estado);
    //arranque del scheduler (organizador)
    vTaskStartScheduler();
    for(;;)
    {
        /*proceso principal del main.c, todo el programa se ejecuta a través de los task
        por lo tanto no es necesario ni recomendable incluir codigo en esta sección*/    
    }
}

//---------------------------------------------------------

void vApplicationMallocFailedHook( void ){
//El tamaño de la pila ha sido excedido
    taskDISABLE_INTERRUPTS();//deshabilita interrupciones del FreeRTOS
    while( 1 )
    {
        LED_Write(1);
        //PWM_Led_WriteCompare(0);
        vTaskDelay(pdMS_TO_TICKS(200));
        LED_Write(0);
        //PWM_Led_WriteCompare(PWM_Led_ReadPeriod());
        vTaskDelay(pdMS_TO_TICKS(200));
        //no haga nada, este es un buen lugar para ubicar puntos de parada para depuración
    }
}

void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName ){
    // El espacio de tareas ha sido excedido por una de ellas
    taskDISABLE_INTERRUPTS();//deshabilita interrupciones del FreeRTOS
    while( 1 )
    {
        LED_Write(1);
        //PWM_Led_WriteCompare(0);
        vTaskDelay(pdMS_TO_TICKS(100));
        LED_Write(0);
        //PWM_Led_WriteCompare(PWM_Led_ReadPeriod());
        vTaskDelay(pdMS_TO_TICKS(100));
    
         //no haga nada, este es un buen lugar para ubicar puntos de parada para depuración
    }
}

/* [] END OF FILE */
