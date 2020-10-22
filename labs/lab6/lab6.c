/*-------------------------------------------------------------------
**
**  Fichero:
**    lab6.c  26/4/2013
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Test del laboratorio 6
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <uart.h>
#include <system.h>
#include <keypad.h>
#include <pbs.h>

void isr_keypad( void ) __attribute__ ((interrupt ("IRQ")));
void isr_pb( void ) __attribute__ ((interrupt ("IRQ")));

void main( void )
{

    sys_init();
    uart0_init();
    keypad_init();
    pbs_init();

    /************************************/

    uart0_puts( "\nDetección de presión:\n" );

    uart0_puts( "  - Pulse el boton izquierdo\n" );
    pb_wait_keydown( PB_LEFT );

    uart0_puts( "  - Pulse el boton derecho\n" );
    pb_wait_keydown( PB_RIGHT );

    uart0_puts( "  - Pulse alguna tecla\n" );
    keypad_wait_any_keydown( );

    uart0_puts( "  - Pulse la tecla 7 \n" );
    keypad_wait_keydown( KEYPAD_KEY7 );

    /************************************/

    uart0_puts( "\nDetección de depresión:\n" );

    uart0_puts( "  - Pulse el boton izquierdo\n" );
    pb_wait_keyup( PB_LEFT );

    uart0_puts( "  - Pulse el boton derecho\n" );
    pb_wait_keyup( PB_RIGHT );

    uart0_puts( "  - Pulse alguna tecla\n" );
    keypad_wait_any_keyup( );

    uart0_puts( "  - Pulse la tecla 7 \n" );
    keypad_wait_keyup( KEYPAD_KEY7 );

    /************************************/

    uart0_puts( "\nPulse botones y/o teclas del keypad:\n" );
    keypad_open( isr_keypad );
    pbs_open( isr_pb );

    /************************************/

    while( 1 );

}

void isr_keypad( void )
{
    uint8 scancode;
    uint16 time;

    scancode = keypad_getchartime( &time );
    switch( scancode )
    {
        case KEYPAD_TIMEOUT:
            uart0_puts( "  - KEYPAD TIMEOUT\n" );
            break;
        case KEYPAD_FAILURE:
            uart0_puts( "  - KEYPAD FAILURE\n" );
            break;            
        default:
            uart0_puts( "  - Tecla pulsada: 0x" );
            uart0_puthex( scancode );
            uart0_puts( " durante " );
            uart0_putint( time );
            uart0_puts( " ms \n" );
    }
    I_ISPC = BIT_KEYPAD;
}

void isr_pb( void )
{
    uint8 scancode;
    uint16 time;

    scancode = pb_getchartime( &time );
    switch( scancode )
    {
        case PB_TIMEOUT:
            uart0_puts( "  - PB TIMEOUT\n" );
            break;
        case PB_FAILURE:
            uart0_puts( "  - PB FAILURE\n" );
            break;
        case PB_LEFT:
            uart0_puts( "  - Boton izquierdo pulsado durante " );
            uart0_putint( time );
            uart0_puts( " ms \n" );
            EXTINTPND = BIT_LEFTPB;
            break;
        case PB_RIGHT:
            uart0_puts( "  - Boton derecho pulsado durante " );
            uart0_putint( time );
            uart0_puts( " ms \n" );
            EXTINTPND = BIT_RIGHTPB;
            break;
    }
    I_ISPC = BIT_PB;
}
