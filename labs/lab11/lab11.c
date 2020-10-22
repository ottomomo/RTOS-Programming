/*-------------------------------------------------------------------
**
**  Fichero:
**    lab11.c  26/2/2015
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Test del laboratorio 5
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <uart.h>
#include <rtc.h>

void isr_tick( void ) __attribute__ ((interrupt ("IRQ")));

void main( void )
{
    rtc_time_t rtc_time;
    
    sys_init();
    uart0_init();
    rtc_init();

    /************************************/

    rtc_gettime( &rtc_time );

    uart0_puts( "\n\nFecha y hora iniciales: " );
    uart0_putint( rtc_time.mday );
    uart0_putchar( '/' );
    uart0_putint( rtc_time.mon );
    uart0_putchar( '/' );
    uart0_putint( rtc_time.year );
    uart0_putchar( ' ' );
    uart0_putint( rtc_time.hour );
    uart0_putchar( ':' );
    uart0_putint( rtc_time.min );
    uart0_putchar( ':' );
    uart0_putint( rtc_time.sec );

    /************************************/

    uart0_puts( "\nIntroduzca nueva fecha\n" );
    uart0_puts( "  - Dia: " );
    rtc_time.mday = (uint8) uart0_getint();
    uart0_puts( "  - Mes: " );
    rtc_time.mon = (uint8) uart0_getint();
    uart0_puts( "  - Año (2 digitos): " );
    rtc_time.year = (uint8) uart0_getint();
    uart0_puts( "Introduzca nueva hora\n" );
    uart0_puts( "  - Hora: " );
    rtc_time.hour = (uint8) uart0_getint();
    uart0_puts( "  - Minuto: " );
    rtc_time.min = (uint8) uart0_getint();
    uart0_puts( "  - Segundo: " );
    rtc_time.sec = (uint8) uart0_getint();

    rtc_puttime( &rtc_time );

    /************************************/

    rtc_open( isr_tick, 127 );
    
    /************************************/

    while( 1 );
}

void isr_tick( void )
{
    rtc_time_t rtc_time;
    
    rtc_gettime( &rtc_time );

    uart0_puts( "\nFecha y hora: " );
    uart0_putint( rtc_time.mday );
    uart0_putchar( '/' );
    uart0_putint( rtc_time.mon );
    uart0_putchar( '/' );
    uart0_putint( rtc_time.year );
    uart0_putchar( ' ' );
    uart0_putint( rtc_time.hour );
    uart0_putchar( ':' );
    uart0_putint( rtc_time.min );
    uart0_putchar( ':' );
    uart0_putint( rtc_time.sec );

    I_ISPC = BIT_TICK;
}
