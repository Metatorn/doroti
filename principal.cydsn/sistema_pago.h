/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
*/

#ifndef SISTEMAS_PAGO_CONFIG_H
#define SISTEMAS_PAGO_CONFIG_H

#include "cypins.h"
#include "tipos.h"
#include "MDB_Tipos.h"    
#include "MDB_monedero.h"
#include "MDB_billetero.h"    

    
/*=====================================================
;ETIQUETAS PROPIAS PARA MANEJO DEL MDB
====================================================*/     
#define label_retenido                  "RETENIDO"
#define label_guardado                  "GUARDADO"
#define label_devuelto                  "RETORNADO"    
   
#define SISPAGO_TMAX                    (120)  
#define SISPAGO_TPERIODO                (2)    
#define SISPAGO_TIMEOUT                 (1000)      
     
/*=====================================================
;FUNCIONES DEL SISTEMA DE PAGO
====================================================*/        
void pago_Init();
int estado_tubos();
long estado_tambor();
void funciones();
uint dispensarBilletes(uint cantidadDevolucion);
CYBIT vaciar_reciclador();
int sesion_venta(int credito_anterior, xbillete_ingresado billete, xmoneda_ingresada moneda);
xpollCoin monedero_Poll();
xpollBill billetero_Poll();
xcredito valCredito(xcredito credito_anterior, xbillete_ingresado billete, xmoneda_ingresada moneda);

#endif
/* [] END OF FILE */
