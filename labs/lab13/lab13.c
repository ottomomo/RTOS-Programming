/*-------------------------------------------------------------------
**
**  Fichero:
**    lab13.c  15/6/2015
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Ejemplo de una aplicación bajo un kernel de planificación
**    no expropiativa de tareas cooperativas multiestado
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include "kernelcoop.h"

#include <s3c44b0x.h>
#include <s3cev40.h>
#include <common_types.h>
#include <system.h>
#include <leds.h>
#include <segs.h>
#include <uart.h>
#include <pbs.h>
#include <keypad.h>
#include <timers.h>
#include <rtc.h>

/* Definicion de estados de tareas */

#define INIT    (0)
#define RUN        (1)

#define WAIT_KEYDOWN (1)
#define SCAN         (2)
#define WAIT_KEYUP   (3)
#define END_SCAN     (4)

/* Declaración de recursos */

uint8 scancode;
boolean flagTask5  = FALSE;
boolean flagTask6  = FALSE;
volatile boolean pb_pressed = FALSE;

/* Declaración de tareas */

void Task1( uint32 *state, uint32 *period );
void Task2( uint32 *state, uint32 *period );
void Task3( uint32 *state, uint32 *period );
void Task4( uint32 *state, uint32 *period );
void Task5( uint32 *state, uint32 *period );
void Task6( uint32 *state, uint32 *period );

/* Declaración de RTI */

void isr_pb( void ) __attribute__ ((interrupt ("IRQ")));

    
/*******************************************************************/

void main( void )
{
    sys_init();      /* Inicializa el sistema */
    timers_init();
    uart0_init();
    leds_init();
    segs_init();
    rtc_init();
    pbs_init();
    keypad_init();
    
    uart0_puts( "\n\n Ejecutando kernel de planificación no expropiativa\n" );
    uart0_puts( " --------------------------------------------------\n\n" ) ;

    scheduler_init();                  /* Inicializa el kernel              */

    create_task( Task1, INIT, 50 );   /* Crea las tareas de la aplicación */
    create_task( Task2, INIT, 10 );   /* La prioridad de las tareas es según el orden de creación: Task1 > Task2 > Task3 > ... */
    create_task( Task3, INIT, 100 );
    create_task( Task4, INIT, 1000 );
    create_task( Task5, INIT, 10 );
    create_task( Task6, INIT, 10 );

    timer0_open_tick( scheduler, TICKS_PER_SEC );  /* Instala scheduler como RTI del timer0  */
    pbs_open( isr_pb );                            /* Instala isr_pb como RTI por presión de pulsadores  */

    while( 1 )
    {
        sleep();        /* Entra en estado IDLE, sale por interrupción */
        if( pb_pressed )
        {
            uart0_puts( "  (INT) Se ha pulsado algún pushbutton...\n" );
            pb_pressed = FALSE;
        }
        dispacher();  /* Todas las tareas preparadas se ejecutan en esta hebra en orden de prioridad */
    }

}

/*******************************************************************/

void Task1( uint32 *state, uint32 *period )  /* Cada 0,5 segundos (50 ticks) alterna el led que se enciende */
{
    switch( *state )
    {
        case INIT:
            uart0_puts( " Task 1: iniciada.\n" );  /* Muestra un mensaje inicial en la UART0 (no es necesario semáforo) */
            led_on( LEFT_LED );
            led_off( RIGHT_LED );
            *state = RUN;
            break;
        default:
            led_toggle( LEFT_LED );
            led_toggle( RIGHT_LED );
            break;
    };
}

void Task2( uint32 *state, uint32 *period )  /* Cada 0,1 segundos (10 ticks) muestrea el keypad y envía el scancode a otras tareas */
{
    switch( *state )
    {
        case INIT:
            uart0_puts( " Task 2: iniciada.\n" );  /* Muestra un mensaje inicial en la UART0 (no es necesario semáforo) */
            *state = WAIT_KEYDOWN;
            break;
        case WAIT_KEYDOWN:
            if( !(PDATG & 0x2) )
            {
                *state = SCAN;
                *period = 3;  // rebote presión 30 ms (3 ticks)
            }
            break;
        case SCAN:
            scancode = keypad_scan();
            flagTask5 = TRUE;
            flagTask6 = TRUE;
            *state = WAIT_KEYUP;
            *period = 10;
            break;
        case WAIT_KEYUP:
            if( PDATG & 0x2 )
            {
                *state = END_SCAN;
                *period = 10; // rebote depresión 100 ms (10 ticks)
            }
            break;
        case END_SCAN:
            *state = WAIT_KEYDOWN;
            *period = 10;
            break;
    };
}

void Task3( uint32 *state, uint32 *period  )  /* Cada segundo (100 ticks) muestra por la UART0 la hora del RTC */
{
    rtc_time_t rtc_time;

    switch( *state )
    {
        case INIT:
            uart0_puts( " Task 3: iniciada.\n" );  /* Muestra un mensaje inicial en la UART0 (no es necesario semáforo) */
            *state = RUN;
            break;
        default:
            rtc_gettime( &rtc_time );
            uart0_puts( "  (Task 3) Hora: " );
            uart0_putint( rtc_time.hour );
            uart0_putchar( ':' );
            uart0_putint( rtc_time.min );
            uart0_putchar( ':' );
            uart0_putint( rtc_time.sec );
            uart0_puts( "\n" );
            break;
    };
}

void Task4( uint32 *state, uint32 *period )  /* Cada 10 segundos (1000 ticks) muestra por la UART0 los ticks transcurridos */
{
    static uint32 ticks;

    switch( *state )
    {
        case INIT:
            uart0_puts( " Task 4: iniciada.\n" );  /* Muestra un mensaje inicial en la UART0 (no es necesario semáforo) */
            ticks = 0;
            *state = RUN;
            break;
        default:
            ticks += TICKS_PER_SEC * 10;
            uart0_puts( "  (Task 4) Ticks: " );
            uart0_putint( ticks );
            uart0_puts( "\n" );
            break;
    };
}

void Task5( uint32 *state, uint32 *period )  /* Cada vez que reciba un scancode lo muestra por la UART0 */
{
    switch( *state )
    {
        case INIT:
            uart0_puts( " Task 5: iniciada.\n" );  /* Muestra un mensaje inicial en la UART0 (no es necesario semáforo) */
            *state = RUN;
            break;
        default:
            if( flagTask5 == TRUE )
            {
                uart0_puts( "  (Task 5) Tecla pulsada: " );
                uart0_puthex( scancode );
                uart0_puts( "\n" );
                flagTask5 = FALSE;
            }
            break;
    };
}

void Task6( uint32 *state, uint32 *period )  /* Cada vez que reciba un scancode lo muestra por los 7 segmentos */
{
    switch( *state )
    {
        case INIT:
            uart0_puts( " Task 6: iniciada.\n" );  /* Muestra un mensaje inicial en la UART0 (no es necesario semáforo) */
            *state = RUN;
            break;
        default:
            if( flagTask6 == TRUE )
            {
                segs_putchar( scancode );
                flagTask6 = FALSE;
            }
            break;
    };
}

/*******************************************************************/


void isr_pb( void )
{
    pb_pressed = TRUE;        /* Solo señaliza, el mensaje en la UART0 lo envía la hebra background para evitar proteger el recurso */
    EXTINTPND = BIT_RIGHTPB;
    EXTINTPND = BIT_LEFTPB;
    I_ISPC = BIT_PB;
}

/*******************************************************************/
