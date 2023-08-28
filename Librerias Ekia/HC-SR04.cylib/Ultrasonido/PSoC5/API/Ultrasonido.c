/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * ULTRASONIDO_EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Libreria para el mandejo del sensor de distancia
;ultrasonico hc-sr04
====================================================*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "`$INSTANCE_NAME`.h"
#include "`$INSTANCE_NAME`_Timer_Ultrasonido.h"
#include "`$INSTANCE_NAME`_Control_Trigger.h"

void `$INSTANCE_NAME`_Start(){
    `$INSTANCE_NAME``[Timer_Ultrasonido]`Start();
}

void `$INSTANCE_NAME`_Stop(){
    `$INSTANCE_NAME``[Timer_Ultrasonido]`Stop();
}

float `$INSTANCE_NAME`_medirDistancia(uint8 preescaler){
    float distancia=-1.0f;
    int counter = 0, timeOut=0;
    //uint8 estado = CyEnterCriticalSection();
    `$INSTANCE_NAME``[Control_Trigger]`Write(1);//activar en 1 lógico la salida al pin trigg y el reinicio del timer
    CyDelay(1);
    `$INSTANCE_NAME``[Control_Trigger]`Write(0);//Desactivar el pin trigg generando así el pulso de entrada
    CyDelay(1);
    //CyExitCriticalSection(estado);
    while(timeOut<200){
        if(`$INSTANCE_NAME``[Timer_Ultrasonido]`ReadStatusRegister()&`$INSTANCE_NAME``[Timer_Ultrasonido]`STATUS_CAPTURE){
            counter = 65535-`$INSTANCE_NAME``[Timer_Ultrasonido]`ReadCapture();//Lectura del timer y conversion de descendente a ascendente
            distancia = counter / (58.0f*preescaler) ;
            break;
        }
        timeOut++;
        CyDelay(1);
    }
    return distancia;
}

/* [] END OF FILE */
