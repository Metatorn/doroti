/* ===================================================
 *
 * Copyright EKIA Technology SAS, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
*/

#ifndef USB_CONFIG_H
#define USB_CONFIG_H

#define label_cantidad              "Ingrese cantidad: "
#define label_monto                 "Ingrese monto: "
#define label_productoUSB           "Ingrese producto: "
#define label_error_producto        "Error, ingrese un numero menor a 999\r\n"
#define label_error_valor           "Error, ingrese un numero no mayor a 4 cifras\r\n" 
#define label_modo_SIM800           "Ha entrado en el modo de comandos AT para el SIM800L\r\n" 
#define label_modo_normal           "Ha salido del modo de comandos AT\r\n"   
#define label_iniciandoSIM          "SIM800L encendido\r\n"
#define label_errorIniciandoSIM     "SIM800L ya esta encendido\r\n"    
#define label_apagandoSIM           "SIM800L apagado\r\n"  
#define label_errorApagandoSIM      "SIM800L ya esta apagado\r\n"      
    
void puertoUSB_Init();
    
#endif
/* [] END OF FILE */
