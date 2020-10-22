/*-------------------------------------------------------------------
**
**  Fichero:
**    lab3.c  24/2/2015
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Test del laboratorio 3
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include <common_types.h>
#include "systemLab3.h"
#include <segs.h>
#include <leds.h>

void delay( void );

void main( void )
{
    uint8 i;
    
    sys_init();
    segs_init();
    leds_init();

    while( 1 ) 
    {

        /************************************/

        led_on( RIGHT_LED );
        segs_putchar( led_status( RIGHT_LED ) );
        delay();
        led_off( RIGHT_LED );
        segs_putchar( led_status( RIGHT_LED ) );
        delay();
        led_on( LEFT_LED );
        segs_putchar( led_status( LEFT_LED ) );
        delay();
        led_off( LEFT_LED );
        segs_putchar( led_status( LEFT_LED ) );
        delay();               

        /************************************/

        for( i=0; i<16; i++ )
        {
            led_toggle( RIGHT_LED );
            led_toggle( LEFT_LED );
            segs_putchar( i );
            delay();
        }
        led_off( RIGHT_LED );
        led_off( LEFT_LED );

        /************************************/

    }
}

void delay( void )
{
    register uint32 i;

    for( i=0; i<2000000; i++ );
} 
