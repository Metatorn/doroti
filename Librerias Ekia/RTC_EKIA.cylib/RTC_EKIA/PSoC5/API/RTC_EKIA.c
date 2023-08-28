/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * RTC_EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PEQUEÑA LIBRERIA QUE COMPLEMENTA LAS FUNCIONES DEL
;RTC PROPIO DE CYPRESS
====================================================*/

#include "`$INSTANCE_NAME`_RTC.h"
#include "`$INSTANCE_NAME`_EKIA.h"

const uint8 `$INSTANCE_NAME`_diasEnMes_tbl[`$INSTANCE_NAME`_MESES_POR_ANO] = {`$INSTANCE_NAME`_DIAS_EN_ENERO,
                                                        `$INSTANCE_NAME`_DIAS_EN_FEBRERO,
                                                        `$INSTANCE_NAME`_DIAS_EN_MARZO,
                                                        `$INSTANCE_NAME`_DIAS_EN_ABRIL,
                                                        `$INSTANCE_NAME`_DIAS_EN_MAYO,
                                                        `$INSTANCE_NAME`_DIAS_EN_JUNIO,
                                                        `$INSTANCE_NAME`_DIAS_EN_JULIO,
                                                        `$INSTANCE_NAME`_DIAS_EN_AGOSTO,
                                                        `$INSTANCE_NAME`_DIAS_EN_SEPTIEMBRE,
                                                        `$INSTANCE_NAME`_DIAS_EN_OCTUBRE,
                                                        `$INSTANCE_NAME`_DIAS_EN_NOVIEMBRE,
                                                        `$INSTANCE_NAME`_DIAS_EN_DICIEMBRE};

/*******************************************************************************
* Nombre Función: `$INSTANCE_NAME`_bisiesto
********************************************************************************
*
* Resumen:
* Verifica si el año ingresado como parámetro es bisiesto o no
*
* Parametros:
*  year: El año a ser verificado
*
* Retorna:
*  0u - el año no es bisiesto.
*  1u - el año si es bisiesto.
*
*******************************************************************************/
uint32 `$INSTANCE_NAME`_bisiesto(uint32 year){
    uint32 retVal;
    if(((0u == (year % 4Lu)) && (0u != (year % 100Lu))) || (0u == (year % 400Lu))){
        retVal = 1uL;
    }
    else{
        retVal = 0uL;
    }
    return(retVal);
}

/*******************************************************************************
* Nombre Función: `$INSTANCE_NAME`_diasEnMes
********************************************************************************
*
* Resumen:
*  Retorna el numero de dias en el mes que haya sido ingresado como parametro
*
* Parameters:
*  Mes: Un mes de un año:
*                  `$INSTANCE_NAME`_ENERO
*                  `$INSTANCE_NAME`_FEBRERO
*                  `$INSTANCE_NAME`_MARZO
*                  `$INSTANCE_NAME`_ABRIL
*                  `$INSTANCE_NAME`_MAYO
*                  `$INSTANCE_NAME`_JUNIO
*                  `$INSTANCE_NAME`_JULIO
*                  `$INSTANCE_NAME`_AGOSTO
*                  `$INSTANCE_NAME`_SEPTIEMBRE
*                  `$INSTANCE_NAME`_OCTUBRE
*                  `$INSTANCE_NAME`_NOVIEMBRE
*                  `$INSTANCE_NAME`_DICIEMBRE
*
*  year:  un valor de año
*
* Retorna:
*  El numero de dias que hay en el mes que haya sido ingresado como parametro
*
*******************************************************************************/
uint8 `$INSTANCE_NAME`_DiasEnMes(uint32 month, uint32 year){
    uint8 retVal;
    retVal = `$INSTANCE_NAME`_diasEnMes_tbl[month - 1u];
    if((uint8)`$INSTANCE_NAME`_FEBRERO == month)
    {
        if(0u != `$INSTANCE_NAME`_bisiesto(year))
        {
            retVal++;
        }
    }
    
    return(retVal);
}

/*******************************************************************************
* Nombre Función: `$INSTANCE_NAME`_actualizarMesRelativo
********************************************************************************
*
* Resumen:
*  Retorna una fecha con un mes relativo a la fecha actual configurada
*
* Parameters:
*  `$INSTANCE_NAME``[RTC]`_TIME_DATE fechaActual: fecha a partir de la cual desee encontrar una fecha relativa
*
*  Mes: cantidad de meses, negativos o positivos que desee mover la fecha
*
* Retorna:
*  la fecha relativa.
*
*******************************************************************************/
`$INSTANCE_NAME``[RTC]`TIME_DATE `$INSTANCE_NAME`_actualizarMesRelativo(`$INSTANCE_NAME``[RTC]`TIME_DATE fecha, int mes){
    while(mes!=0){
        if(mes>0){
            fecha.Month++;
            mes--;
            if(fecha.Month>12){
                fecha.Month=`$INSTANCE_NAME`_ENERO;
                fecha.Year++;
            }
        }
        if(mes<0){
            fecha.Month--;
            mes++;
            if(fecha.Month<1){
                fecha.Month=`$INSTANCE_NAME`_DICIEMBRE;
                fecha.Year--;
            }
        }
    }
    return (fecha);
}
/*******************************************************************************
* Nombre Función: `$INSTANCE_NAME`_fechaRelativa
********************************************************************************
*
* Resumen:
*  Retorna una fecha relativa a la fecha actual configurada
*
* Parameters:
*  `$INSTANCE_NAME`_TIME_DATE fechaActual: fecha a partir de la cual desee encontrar una fecha relativa
*
*  Dias: cantidad de dias, negativos o positivos que desee mover la fecha
*
*  Mes: cantidad de meses, negativos o positivos que desee mover la fecha
*
*  year: cantidad de año, negativo o positivo que desee mover la fecha
*
* Retorna:
*  la fecha relativa.
*
*******************************************************************************/
`$INSTANCE_NAME``[RTC]`TIME_DATE `$INSTANCE_NAME`_fechaRelativa(`$INSTANCE_NAME``[RTC]`TIME_DATE fecha, int dias, int mes, int year){
    `$INSTANCE_NAME``[RTC]`TIME_DATE fechaRelativa = fecha;
    
    //actualice los dias respectivos
    while(dias!=0){
        if(dias>0){
            fechaRelativa.DayOfMonth++;
            dias--;
            if(fechaRelativa.DayOfMonth>`$INSTANCE_NAME`_DiasEnMes(fechaRelativa.Month,fechaRelativa.Year)){
                fechaRelativa = `$INSTANCE_NAME`_actualizarMesRelativo(fechaRelativa,1);
                fechaRelativa.DayOfMonth = 1;
            }
        }
        if(dias<0){
            fechaRelativa.DayOfMonth--;
            dias++;
            if(fechaRelativa.DayOfMonth<1){
                fechaRelativa = `$INSTANCE_NAME`_actualizarMesRelativo(fechaRelativa,-1);
                fechaRelativa.DayOfMonth = `$INSTANCE_NAME`_DiasEnMes(fechaRelativa.Month,fechaRelativa.Year);
            }
        }
    }
    //actualice el mes respectivo
    while(mes!=0){
        if(mes>0){
            fechaRelativa.Month++;
            mes--;
            if(fechaRelativa.Month>12){
                fechaRelativa.Month=`$INSTANCE_NAME`_ENERO;
                fechaRelativa.Year++;
            }
        }
        if(mes<0){
            fechaRelativa.Month--;
            mes++;
            if(fechaRelativa.Month<1){
                fechaRelativa.Month=`$INSTANCE_NAME`_DICIEMBRE;
                fechaRelativa.Year--;
            }
        }
    }
    //actualice el año respecto al corrimiento solicitado
    fechaRelativa.Year+=year;
    
    return(fechaRelativa);
}

/* [] END OF FILE */
