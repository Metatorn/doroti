/* ==============================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * Bootloader 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;FUNCIONES PROPIAS DE LA MEMORIA SD
===============================================================*/

#ifndef SD_BOOTLOAD_H
#define SD_BOOTLOAD_H

#include "cypins.h"   
    
#define LARGO_CABECERA      10
#define LARGO_LINEA         525
#define LARGO_MIN           13 
#define DATA_OFFSET         11    
    
#define CYRET_ERR_FILE          0x01 //archivo no accesible o no encontrada
#define CYRET_ERR_EOF           0x02 //se ha alcanzado final del archivo
#define CYRET_ERR_LENGTH        0x03 //la cantidad de datos disponibles está fuera de rango
#define CYRET_ERR_DATA          0x04 //Los datos no están con el formato apropiado
#define CYRET_ERR_CMD           0x05 //comando no reconocido 
    
#define pName_Dir_Conf          "\\CONFIGURACION"     
    
int SD_Bootload(char *file); 
CYBIT habilitarSD();
    
#endif    

/* [] END OF FILE */
