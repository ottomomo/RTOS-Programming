/*-------------------------------------------------------------------
**
**  Fichero:
**    lab4.c  13/3/2014
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Test del laboratorio 4
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include <common_types.h>
#include "systemLab4.h"
#include <uart.h>

void main( void )
{
    char   s[256];
    int32  i = 0;
    uint32 u = 0;
    
    sys_init();
    uart0_init();

    /************************************/

    uart0_puts( "\n\nEste programa fue compilado el dia " );
    uart0_puts( __DATE__ );
    uart0_puts( " a las " );
    uart0_puts( __TIME__ );
    uart0_puts( "\n" );

    /************************************/

    while( 1 ){
        uart0_puts( "  - Escriba algo: " );
        uart0_gets( s );
        uart0_puts( "  - Ha escrito: " );
        uart0_puts( s );
        uart0_putchar( '\n' );
        uart0_puts( "  - Escriba su nombre: " );
        uart0_gets( s );
        uart0_puts( "  - Escriba su edad: ");
        i = uart0_getint();
        uart0_puts( "  - D. " );
        uart0_puts( s );
        uart0_puts( ", tiene " );
        uart0_putint( i );
        uart0_puts( " annos.\n" );
        uart0_puts( "  - Escriba un numero hexadecimal: ");
        u = uart0_gethex();
        uart0_puts( "  - Ha escrito 0x" );
        uart0_puthex( u );
        uart0_puts( " (" );
        uart0_putint( u );
        uart0_puts( ").\n" );
    };

}

